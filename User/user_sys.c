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

	uint32_t end_time = gu64SysTickIntCnt + delay;

	while(gu64SysTickIntCnt <= end_time){ /* As long as the end time is not reached*/
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
void sys_get_unit_string(physical_unit_t unit_type, char* string){/* Max. 8 characters long */

	switch(unit_type){

		case UNIT_OHM:
			strcpy(string, "Ohm");
			break;

		case UNIT_CELSIUS:
			strcpy(string, "degC");
			break;

		case UNIT_KELVIN:
			strcpy(string, "K");
			break;

		case UNIT_VOLT:
			strcpy(string, "V");
			break;

		case UNIT_MILLIVOLT:
			strcpy(string, "mV");
			break;

		case UNIT_CNT:
			strcpy(string, "cnt.");
			break;

		case UNIT_PERCENT_RH:
			strcpy(string, "% RH");
			break;

		case UNIT_PPM:
			strcpy(string, "ppm");
			break;

		default: /*If the unit is not known */
			strcpy(string, "");
			break;

	}
}
