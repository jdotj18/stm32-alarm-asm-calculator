// Main program

#include <stdio.h>

// Header files for drivers
#include "systick.h"
#include "i2c.h"
#include "display.h"
#include "touchpad.h"
//Header files for apps
#include "alarm.h"
#include "calc.h"


int main(void)
{


	ConfigureSysClk();

    printf("Welcome to %c%c%c%d!\n", 67, 69, 71, 0xC40);

    //Initialize apps
    Init_Alarm();
    Init_Calc();

    StartSysTick();

    while(1) {
    	//Run apps
    	Task_Alarm();
    	Task_Calc();

    	//Housekeeping
    	UpdateDisplay();
    	ScanTouchpad();
    	ServiceI2CRequests();

    	WaitForSysTick();
    }
}

