// General-purpose input/output (GPIO) driver

#include "gpio.h"

#include "mcu.h"

// --------------------------------------------------------
// Initialization
// --------------------------------------------------------

// Enable the specified GPIO pin
void GPIO_Enable (Pin_t pin) {
	GPIO_PortEnable(pin.port);
}

// Enable the peripheral clock for a GPIO port
void GPIO_PortEnable (Port_t port)
{
	/*
	uint32_t portNumber;
	uint32_t regVal;

	portNumber = ((uint32_t)port - (uint32_t)GPIOA) / 0x400; (Offsets the registers by 1024 (Their spacing  from eachother))
	regVal = RCC -> AHB2ENR; (reads register)
	regVal |= (1U << portNumber); (Sets clk enable bit)
	RCC ->AHB2ENR = regVal; (write register)
	*/

	// Find the port number from the spacing between port addresses
	uint32_t portNumber =
		((uint32_t)port - (uint32_t)GPIOA) / 0x400;

	// Set this port's clock-enable bit and preserve the other bits
	RCC->AHB2ENR |= (1U << portNumber);
}

// Set the operating mode of a GPIO pin
void GPIO_Mode (Pin_t pin, PinMode_t mode)
{
    /*
    uint32_t regVal;

    regVal = pin.port -> MODER; (reads mode register)
    regVal &= ~(0x03U << (2 * pin.index)); (clears mode)
    regVal |= ((uint32_t)mode << (2 * pin.index)); (Insert new mode)
    pin.port -> MODER = regVal; (writes register)
    */

    // clear mode bits
    pin.port->MODER &= ~(0x03U << (2 * pin.index));

    // set desired mode to 2 bits
    pin.port->MODER |= ((uint32_t)mode << (2 * pin.index));
}

// Configure additional settings for a GPIO pin
void GPIO_Config (Pin_t pin, PinType_t ot, PinSpeed_t osp, PinPUPD_t pupd) {
    // Output type: 1 bit per pin (PP=0, OD=1)
    pin.port->OTYPER  = (pin.port->OTYPER  & ~(0x1UL << pin.index))   | ((uint32_t)ot   << pin.index);
    // Output speed: 2 bits per pin
    pin.port->OSPEEDR = (pin.port->OSPEEDR & ~(0x3UL << 2*pin.index)) | ((uint32_t)osp  << 2*pin.index);
    // Pull-up / pull-down: 2 bits per pin
    pin.port->PUPDR   = (pin.port->PUPDR   & ~(0x3UL << 2*pin.index)) | ((uint32_t)pupd << 2*pin.index);
}

// Select which alternate function is to be used in AF mode
void GPIO_AltFunc (Pin_t pin, int af) {
    int reg   = pin.index / 8;   // AFR[0] = AFRL (pins 0-7), AFR[1] = AFRH (pins 8-15)
    int field = pin.index % 8;   // AFSEL field number within that register
    uint32_t regVal;
    regVal  = pin.port->AFR[reg];               // (1) read
    regVal &= ~(0xFUL << 4*field);              // (2) clear 4-bit field
    regVal |= ((uint32_t)af & 0xF) << 4*field;  // (3) assign new AF
    pin.port->AFR[reg] = regVal;                // (4) write back
}

// --------------------------------------------------------
// Pin observation and control
// --------------------------------------------------------

// Observe the state of an input pin
PinState_t GPIO_Input (const Pin_t pin)
{
    /*
    uint32_t regVal;

    regVal = pin.port -> IDR; (reads input data register)
    if(regVal & (1U << pin.index)) (checks if pin is HIGH if it is return HIGH, else return LOW)
    {
        return HIGH;
    }
    return LOW;
    */

    // Check whether this pin's input bit is 1, set to high
    if (pin.port->IDR & (1U << pin.index))
    {
        return HIGH;
    }

    // The input bit is 0
    return LOW;
}


// Control the state of an output pin
void GPIO_Output (Pin_t pin, const PinState_t state)
{
    /*
    uint32_t regVal;

    if(state == HIGH){
        regVal = 1U << pin.index; (set register to pin index)
    }
    else
    {
        regVal = 1U << (pin.index + 16); (Select reset bit)
    }

    pin.port->BSRR = regVal; (write to hardware)
    */

    if (state == HIGH)
    {
        // set to pin index
        pin.port->BSRR = 1U << pin.index;
    }
    else
    {
        // select reset bit
        pin.port->BSRR = 1U << (pin.index + 16);
    }
}

// Toggle the state of an output pin
void GPIO_Toggle (Pin_t pin)
{
    /*
    uint32_t regVal;

    regVal = pin.port -> ODR; (reads register to output data register)
    regVal ^= (1U << pin.index); (Inverts/toggles selected bit)
    pin.port->ODR = regVal; (writes inverted register to output data register)
    */

    // writes inverted register to output data register
    pin.port->ODR ^= (1U << pin.index);
}

// --------------------------------------------------------
// Interrupt handling
// --------------------------------------------------------

// Array of callback function pointers
// Bits 0 to 15 (each can select one port GPIOA to GPIOH)
// Rising and falling edge triggers for each
static void (*callbacks[16][2]) (void);

// Register a function to be called when an interrupt occurs
void GPIO_Callback(Pin_t pin, void (*func)(void), PinEdge_t edge)
{
	callbacks[pin.index][edge] = func;

	// Enable interrupt generation
	if (edge == RISE)
	  EXTI->RTSR1 |= 1 << pin.index;
	else     // FALL
	  EXTI->FTSR1 |= 1 << pin.index;

	int portNum = ((uint32_t)pin.port - (uint32_t)GPIOA) / 0x400;
	EXTI->EXTICR[pin.index / 4] |= portNum << 8*(pin.index % 4);
	EXTI->IMR1 |= 1 << pin.index;

	// Enable interrupt vector
	NVIC->IPR[EXTI0_IRQn + pin.index] = 0;
	__COMPILER_BARRIER();
	NVIC->ISER[(EXTI0_IRQn + pin.index) / 32] = 1 << ((EXTI0_IRQn + pin.index) % 32);
	__COMPILER_BARRIER();
}

// Interrupt handler for all GPIO pins
void GPIO_IRQHandler (int i) {
	// Clear pending IRQ
    NVIC->ICPR[(EXTI0_IRQn + i) / 32] = 1 << ((EXTI0_IRQn + i) % 32);

    // Detect rising edge
	if (EXTI->RPR1 & (1 << i)) {
		EXTI->RPR1 = (1 << i);  // Service interrupt
		callbacks[i][RISE]();   // Invoke callback function
	}

    // Detect falling edge
	if (EXTI->FPR1 & (1 << i)) {
		EXTI->FPR1 = (1 << i);  // Service interrupt
		callbacks[i][FALL]();   // Invoke callback function
	}
}

// Dispatch all GPIO IRQs to common handler function
void  EXTI0_IRQHandler() { GPIO_IRQHandler( 0); }
void  EXTI1_IRQHandler() { GPIO_IRQHandler( 1); }
void  EXTI2_IRQHandler() { GPIO_IRQHandler( 2); }
void  EXTI3_IRQHandler() { GPIO_IRQHandler( 3); }
void  EXTI4_IRQHandler() { GPIO_IRQHandler( 4); }
void  EXTI5_IRQHandler() { GPIO_IRQHandler( 5); }
void  EXTI6_IRQHandler() { GPIO_IRQHandler( 6); }
void  EXTI7_IRQHandler() { GPIO_IRQHandler( 7); }
void  EXTI8_IRQHandler() { GPIO_IRQHandler( 8); }
void  EXTI9_IRQHandler() { GPIO_IRQHandler( 9); }
void EXTI10_IRQHandler() { GPIO_IRQHandler(10); }
void EXTI11_IRQHandler() { GPIO_IRQHandler(11); }
void EXTI12_IRQHandler() { GPIO_IRQHandler(12); }
void EXTI13_IRQHandler() { GPIO_IRQHandler(13); }
void EXTI14_IRQHandler() { GPIO_IRQHandler(14); }
void EXTI15_IRQHandler() { GPIO_IRQHandler(15); }
