// Calculator app
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "calc.h"
#include "display.h"
#include "touchpad.h"
#define NMAX 10 // Maximum number of operands
static Press_t operation; // Selected operation
static Entry_t operand[NMAX]; // Numeric operands
static bool done; // Operand entry complete
static int count; // Count of operands received
static Entry_t result; // Calculation result
static enum {MENU, MENU2, PROMPT, SUBMENU, ENTRY, RUN, SHOW, WAIT, AVG_SIZE,SORT_SHOW} state;
static int sortIndex = 0;
static Press_t arithOp; //Arithmetic operation selector
static int menuPage = 1; // menu page num
static int avgCount = 0;
static Entry_t avgSize = 0;
// Assembly subroutines to be called as functions
uint32_t Increment(uint32_t n);
uint32_t Decrement(uint32_t n);
uint32_t Fibonacci(uint32_t n);
uint32_t Arith(uint32_t op, uint32_t a, uint32_t b);
uint32_t Factorial(uint32_t n);
uint32_t GCD(uint32_t a, uint32_t b);
uint32_t Average(uint32_t *arr, uint32_t n);
void Sort(uint32_t *arr, uint32_t n);


// Initialization
void Init_Calc (void) {
	DisplayEnable();
	TouchEnable();
}
// Runtime
void Task_Calc (void) {
	switch (state) {
	case MENU:
		// Prepare for a new operation
		operation = NONE;
		arithOp = NONE;

		menuPage = 1;
		avgSize = 0;
		avgCount = 0;
		for (int i = 0; i < NMAX; i++)
			operand[i] = 0;
		count = 0;
		result = 0;
		sortIndex = 0;
		// Display menu
		DisplayPrint(CALC, 0, "1:INC 2:DEC 3:FIB");
		DisplayPrint(CALC, 1, "4:ARI 5:FAC 6:GCD");
		state = PROMPT;
		break;

	case MENU2:
		//Extended Display menu
		DisplayPrint(CALC, 0, "7: AVG 8: SORT");
		DisplayPrint(CALC, 1, "SHIFT:other page");
		state = PROMPT;
		break;

	case PROMPT:
		if (operation == NONE)
			operation = TouchInput(CALC);
		if(operation == SHIFT){
			menuPage = (menuPage == 1) ? 2 : 1;
			operation = NONE;
			state = (menuPage == 1) ? MENU : MENU2;
			break;
		}
		switch ((int)operation) {
		case 1: // Increment
		case 2: // Decrement
		case 3: // Fibonacci
		case 5: // Factorial
			if (count == 1)
				state = RUN;
			else {
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
			}
			break;

		case 4: // Arith
			if(arithOp == NONE){
				DisplayPrint(CALC, 0, "Arithmetic:");
				DisplayPrint(CALC, 1, "0:+ 1:- 2:x 3:/");
				state = SUBMENU;
			}
			else if (count == 2)
				state = RUN;
			else {
				DisplayPrint(CALC, 0, "Enter a number:", count + 1);
				state = ENTRY;
			}
			break;


		case 6: //GCD
			if(count == 2)
				state = RUN;
			else{
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
			}
			break;

		case 7: //Average
			if(avgSize == 0){
				DisplayPrint(CALC, 0,"How many? 1-10");
				state = AVG_SIZE;
			}
			else if(count == avgSize){
				state = RUN;
			}
			else{
				DisplayPrint(CALC, 0, "Enter a number:", count + 1);
				state = ENTRY;
			}
			break;

		case 8: //Sort
			if (avgSize == 0){
				DisplayPrint(CALC, 0, "How many? 1-10");
				state = AVG_SIZE;
			}
			else if (count == avgSize) {
				state = RUN;
			}
			else {
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
			}
			break;

		default:
			operation = NONE;
			break;
		}
		break;



		case SUBMENU:
			Press_t pad = TouchInput(CALC);
			if(pad >= N0 && pad <= N3){
				arithOp = pad;
				state = PROMPT;
			}
			else if(pad == SHIFT)
				state = MENU;
			break;

		case AVG_SIZE:
			done = TouchEntry(CALC, &avgSize);
			DisplayPrint(CALC, 1,"%u", avgSize);

			if(done){
				if (avgSize >= 1 && avgSize <= 10){
					state = PROMPT;
				}
				else{
					avgSize = 0;
					DisplayPrint(CALC, 0, "Enter 1 to 10");
				}
			}
			break;

		case ENTRY:
			done = TouchEntry(CALC, &operand[count]);
			DisplayPrint(CALC, 1, "%u", operand[count]);
			if (done) {
				count++;
				state = PROMPT;
			}
			break;
		case RUN:
			DisplayPrint(CALC, 0, "Calculating...");
			DisplayPrint(CALC, 1, "");
			state = (operation == 8) ? SORT_SHOW : SHOW;
			switch ((int)operation) {
			case 1: result = Increment(operand[0]); break;
			case 2: result = Decrement(operand[0]); break;
			case 3: result = Fibonacci(operand[0]); break;
			case 4: result = Arith(arithOp,operand[0],operand[1]); break;
			case 5: result = Factorial(operand[0]); break;
			case 6: result = GCD(operand[0],operand[1]); break;
			case 7: result = Average((uint32_t *) operand, count); break;
			case 8: Sort((uint32_t *) operand, count); break;
			default: break;
			}
			break;
			case SHOW:
				DisplayPrint(CALC, 0, "Result:");
				if (operation == 4 && arithOp == 3 && operand[1] == 0)
					DisplayPrint(CALC, 1, "DIV by 0 ERROR");
				else if(operation == 7)
					DisplayPrint(CALC, 1, "%u.%u", result / 10, result % 10);
				else
					DisplayPrint(CALC, 1, "%u", result);
				state = WAIT;
				break;
			case SORT_SHOW:
			    DisplayPrint(CALC, 0, "Sorted %d/%d", sortIndex + 1, count);
			    DisplayPrint(CALC, 1, "%u", operand[sortIndex]);

			    if (TouchInput(CALC) != NONE) {
			        sortIndex++;

			        if (sortIndex >= count) {
			            sortIndex = 0;
			            state = WAIT;
			        }
			    }
			    break;
			case WAIT:
				// Press any pad to return to the menu
				if (TouchInput(CALC) != NONE)
					state = MENU;
				break;
	}
}
