/*
 * interrupt.h
 *
 *  Created on: Jul 4, 2022
 *      Author: Hugo Boyce
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <stdint.h>
#include <stdbool.h>


/* The higher the number, the lower the priority */
#define ANALOG_PRIORITY 0 /* Since we're doing oversampling, the interrupt needs to be answered quickly.  */

#define USBD_PRIORITY 1
#define SYSTICK_PRIORITY 2

#define I2C_PRIORITY 12 /* I2C has low priority since it's slow and can easily wait. */


extern volatile uint64_t gu64SysTickIntCnt;

extern volatile bool gbTerminalActive;
extern volatile bool gbSecondsFlag;
extern volatile bool gbDrawNewUIFrame;
extern volatile bool gbSampleAnalog;

void interrupt_setPriorities(void);


#endif /* INTERRUPT_H_ */
