/*
 * sort_original.c
 *
 *  Created on: Nov 4, 2026
 *      Author: sdavi056
 */
// USES Bubble Sort

#include <stdint.h>

void Sort(uint32_t *a, uint32_t n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - 1; j++) {
            if (a[j] > a[j + 1]) {
                int temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}
