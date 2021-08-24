#ifndef VECT_TABLE_H
#define VECT_TABLE_H

/*
 * INT_ITL (0x38)
 */

#define r_itl_interrupt vTickISR

void vTickISR(void) __attribute__ ((interrupt));

/*
 * INT_BRK_I (0x7E)
 */
#define INT_BRK_I vSoftwareInterruptISR

void vSoftwareInterruptISR(void) __attribute__ ((interrupt));

#endif
