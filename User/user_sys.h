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

typedef enum {UNIT_NONE = 0,
				UNIT_OHM = 1,
				UNIT_CELSIUS = 2,
				UNIT_KELVIN = 3,
				UNIT_VOLT = 4,
				UNIT_MILLIVOLT = 5,
				UNIT_CNT = 6,
				UNIT_PERCENT_RH = 7,
				UNIT_PPM = 8
} physical_unit_t;


void delay_ms(uint32_t);
void cycleLED(void);


#endif /* USER_SYS_H_ */
