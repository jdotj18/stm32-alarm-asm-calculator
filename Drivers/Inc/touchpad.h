#ifndef TOUCHPAD_H_
#define TOUCHPAD_H_

#include <stdint.h>
#include <stdbool.h>
#include "display.h"

// Single press on the Touchpad
typedef enum { NONE=-1, MIN = 0, N0=0, N1, N2, N3, N4, N5, N6,
               N7, N8, N9, SHIFT=10, NEXT=11, MAX=11 } Press_t;

// Numeric entry value
typedef uint32_t Entry_t;

void TouchEnable(void);
Press_t TouchInput(Page_t page);
bool TouchEntry(Page_t page, Entry_t *num);

void ScanTouchpad(void);
void ClearTouchpad(void);

#endif /* TOUCHPAD_H_ */
