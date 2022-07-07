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


extern volatile uint64_t gu32SysTickIntCnt;

extern volatile bool gbTerminalActive;
extern volatile bool gbSecondsFlag;
extern volatile bool gbDrawNewUIFrame;
extern volatile bool gbSampleAnalog;


#endif /* INTERRUPT_H_ */
