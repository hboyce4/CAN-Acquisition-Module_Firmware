/*
 * sys.c
 *
 *  Created on: May 24, 2022
 *      Author: hboyce4
 */
#include "interrupt.h"
#include "NuMicro.h"
#include "user_sys.h"

void delay_ms(uint32_t delay){ /*Generates a milliseconds delay. NOT ACCURATE. Use a hardware timer for accuracy*/

	uint32_t end_time = gu32SysTickIntCnt + delay;

	while(gu32SysTickIntCnt <= end_time){ /* As long as the end time is not reached*/
		/* Do nothing*/
	}
}

void cycleLED(void){

	static uint8_t u8LEDState = 0;

	PB->DOUT &= ~(BIT13|BIT14|BIT15);
	PB->DOUT |= (u8LEDState << 13);
	//printf("LED State is %i\n", u8LEDState);
	u8LEDState++;
	if(u8LEDState > 0b111){
		u8LEDState =0;
	}


}
