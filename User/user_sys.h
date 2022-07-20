/*
 * sys.h
 *
 *  Created on: May 24, 2022
 *      Author: hboyce4
 */

#ifndef USER_SYS_H_
#define USER_SYS_H_

#include <stdint.h>

/* Mishmash of everything I don't have anywhere to put */

#define DEBUG_UART_SPEED 115200

typedef enum {UNIT_NONE, UNIT_OHM, UNIT_CELSIUS, UNIT_KELVIN, UNIT_VOLT, UNIT_MILLIVOLT, UNIT_CNT, UNIT_PERCENT_RH, UNIT_PPM} physical_unit_t;


void delay_ms(uint32_t);
void cycleLED(void);


#endif /* USER_SYS_H_ */
