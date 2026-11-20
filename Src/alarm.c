/*
 * alarm.c
 *
 *  Created on: Oct 7, 2026
 *      Author: sdavi056
 */

/*
 * alarm.c
 *
 *  Created on: Oct 7, 2026
 *      Author: sdavi056
 */




// Alarm system app
#include <stdio.h>
#include <stdbool.h>
#include "alarm.h"
#include "systick.h"
#include "gpio.h"
#include "display.h"

// GPIO pins
static const Pin_t BlueLED = {GPIOB, 7}; //PB7 -> User LD2
static const Pin_t RedLED = {GPIOA, 9}; //PA9 -> User LD2
static const Pin_t GreenLED = {GPIOC,7}; //PC7 -> User LD2

static const Pin_t Motion = {GPIOB,9}; //PB9 <- User B9
static const Pin_t Button = {GPIOB,2}; //PB2 <- User B2


static enum {DISARMED, ARMED, TRIGGERED} state = DISARMED;
// Constants
#define LED_alternation 1000 //Blue and Green LED alternation
#define Short_press 2000 //Duration for short press
#define Long_press 3000 //Duration for long press
#define Button_debounce 50 //Minimum time needed to avoid debounce

// Variables
static volatile bool motionDetected = false; //When motion interrupt happens
static volatile bool shortPress = false; //When valid short button press

static volatile Time_t pressTimestamp = 0; //When button was pressed
static Time_t lastBlinkTime = 0; //When Green/Blue LED switched

// Declare interrupt callback functions
static void CallbackMotionDetect();
static void CallbackButtonPress();
static void CallbackButtonRelease();

// Initialization code
void Init_Alarm (void) {
	//ENABLE GPIO PORTS
	GPIO_Enable (RedLED);
	GPIO_Enable (BlueLED);
	GPIO_Enable (GreenLED);


	// SET I/O MODE of LEDs, User Button, and Motion Sensor
	GPIO_Mode(RedLED, OUTPUT);
	GPIO_Mode(BlueLED, OUTPUT);
	GPIO_Mode(GreenLED, OUTPUT);
	GPIO_Mode(Motion, INPUT);
	GPIO_Mode(Button, INPUT);

	// Register Callback
	GPIO_Callback(Motion, CallbackMotionDetect, RISE);
	GPIO_Callback(Button, CallbackButtonPress, RISE);
	GPIO_Callback(Button, CallbackButtonRelease, FALL);

	//Display
	DisplayEnable();
	DisplayColor(ALARM, WHITE);
	DisplayPrint(ALARM, 0, "DISARMED");
}

// Task code (state machine)
void Task_Alarm (void) {
	switch (state) {

	case DISARMED:
		//sets all LED to Off
		GPIO_Output(RedLED,LOW);
		GPIO_Output(BlueLED,LOW);
		GPIO_Output(GreenLED,LOW);

		//sets motion detected to Off
		motionDetected = false;

		if (shortPress) {
			//clears the flag
			shortPress = false;
			state = ARMED;
			//Starts Green and Blue LED
			GPIO_Output(RedLED,LOW);
			GPIO_Output(BlueLED,HIGH);
			GPIO_Output(GreenLED,LOW);
			//Display
			DisplayColor(ALARM, YELLOW);
			DisplayPrint(ALARM, 0, "ARMED");
			//Saves Time to Blink
			lastBlinkTime = TimeNow();
			printf("Button Pressed (Short) at time %u\n", TimeNow());

			printf("Alarm is Armed\n");

			printf("Reason is because short button pressed\n");

			break;
		}

		break;

	case ARMED:
		if (TimePassedSince(lastBlinkTime) >= LED_alternation){
			GPIO_Toggle(BlueLED);
			GPIO_Toggle(GreenLED);
			lastBlinkTime = TimeNow();
		}

		if(motionDetected){
			//clears flag
			motionDetected = false;
			state = TRIGGERED;
			//red LED on
			GPIO_Output(RedLED,HIGH);
			GPIO_Output(BlueLED,LOW);
			GPIO_Output(GreenLED,LOW);

			//Display
			DisplayColor(ALARM, RED);
			DisplayPrint(ALARM, 0, "TRIGGERED");

			printf("Motion Detected at time %u\n", TimeNow());

			printf("Alarm is Triggered \n");

			printf("Reason is because motion detected\n");

			break;
		}


		if(GPIO_Input(Button) == HIGH && TimePassedSince(pressTimestamp) >= Long_press){
			state = DISARMED;
			//ALL LEDs OFF
			GPIO_Output(RedLED, LOW);
			GPIO_Output(BlueLED,LOW);
			GPIO_Output(GreenLED,LOW);

			//Display
			DisplayColor(ALARM, WHITE);
			DisplayPrint(ALARM, 0, "DISARMED");

			printf("Button Pressed (Long) at time %u\n", TimeNow());

			printf("Alarm is disarmed\n");

			printf("Reason is because long button pressed\n");
			break;
		}
		if (shortPress){
			shortPress = false;
		}
		break;
	case TRIGGERED:
		//red LED ON
		GPIO_Output(RedLED,HIGH);
		GPIO_Output(BlueLED,LOW);
		GPIO_Output(GreenLED,LOW);


		if (shortPress) {
			//clears the flag
			shortPress = false;
			motionDetected = false;
			state = ARMED;
			//Starts Green and Blue LED
			GPIO_Output(RedLED,LOW);
			GPIO_Output(BlueLED,HIGH);
			GPIO_Output(GreenLED,LOW);
			//Display
			DisplayColor(ALARM, YELLOW);
			DisplayPrint(ALARM, 0, "ARMED");
			//Saves Time to Blink
			lastBlinkTime = TimeNow();
			printf("Button Pressed (Short) at time %u\n", TimeNow());

			printf("Alarm is Armed\n");

			printf("Reason is because short button pressed\n");

			break;
		}

		if(GPIO_Input(Button) == HIGH && TimePassedSince(pressTimestamp) >= Long_press){
			state = DISARMED;
			//ALL LEDs OFF
			GPIO_Output(RedLED, LOW);
			GPIO_Output(BlueLED,LOW);
			GPIO_Output(GreenLED,LOW);

			//Display
			DisplayColor(ALARM, WHITE);
			DisplayPrint(ALARM, 0, "DISARMED");

			printf("Button Pressed (Long) at time %u\n", TimeNow());

			printf("Alarm is disarmed\n");

			printf("Reason is because long button pressed\n");

			break;
		}
		break;

	}
}
static void CallbackMotionDetect (void) {
	motionDetected = true;
}

static void CallbackButtonPress (void) {
	pressTimestamp = TimeNow();
}

static void CallbackButtonRelease (void) {
	if(/*TimePassedSince(pressTimestamp) > Button_debounce &&  */TimePassedSince(pressTimestamp) < Short_press){
		shortPress = true;
	}
}


