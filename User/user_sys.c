/*
 * sys.c
 *
 *  Created on: May 24, 2022
 *      Author: hboyce4
 */
#include "sys.h"

#include "interrupt.h"


void delay_ms(uint32_t delay){ /*Generates a milliseconds delay. NOT ACCURATE. Use a hardware timer for accuracy*/

	uint32_t end_time = gu32SysTickIntCnt + delay;

	while(gu32SysTickIntCnt <= end_time){ /* As long as the end time is not reached*/
		/* Do nothing*/
	}
}
