/*
 * sys.c
 *
 *  Created on: May 24, 2022
 *      Author: hboyce4
 */
#include "sys.h"

extern volatile uint32_t g_SysTickIntCnt;

void delay_ms(uint32_t delay){ /*Generates a milliseconds delay. NOT ACCURATE. Use a hardware timer for accuracy*/

	uint32_t end_time = g_SysTickIntCnt + delay;

	while(g_SysTickIntCnt <= end_time){ /* As long as the end time is not reached*/
		/* Do nothing*/
	}
}
