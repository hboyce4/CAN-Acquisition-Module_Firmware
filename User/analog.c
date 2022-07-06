/*
 /* analog.c
 *
 *  Created on: Jul 5, 2022
 *      Author: Hugo Boyce
 */

#include "analog.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

volatile analog_temperatures_t analog_temperatures; /* [°C] */
volatile analog_voltages_t analog_voltages; /* [mV] */
volatile uint32_t ADC_raw_val[EADC_TOTAL_CHANNELS];
volatile uint16_t ADC_acq_buff[EADC_TOTAL_CHANNELS];
volatile uint8_t ADC_acq_count;


/*---------------------------------------------------------------------------------------------------------*/
/* Function definitions                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
// TODO: Add conversion to celsius  for the temperature channels

void ADC_RunCal(void){

	//EADC->CALCTL |= EADC_CALCTL_CALSTART_Msk|EADC_CALCTL_CALSEL_Msk;/* Set CALSTART to start calibration. Set CALSEL so it doesn't try to load user calibration word*/
	/* EADCDIV must be zero to run calibration */
}

void ADC_ServiceOversampling(void){

	/* Divide,copy and reset */
	uint8_t channel;
	for(channel = 0; channel < EADC_TOTAL_CHANNELS;channel++){/* For every channel */

		/* this step could be skipped by integrating this calculation in the float conversion step*/
		ADC_raw_val[channel]= (ADC_acq_buff[channel] >> EADC_SHIFT_FOR_OVERSAMPLING_DIVISION); /* Copy the acquisition buffer, dividing by the number of oversamples. */

		ADC_acq_buff[channel] = 0;
	}

	/* Start ADC acquisition batch*/
	ADC_acq_count = EADC_OVERSAMPLING_NUMBER;

	EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);      /* Clear the A/D ADINT0 interrupt flag */
	NVIC_EnableIRQ(EADC00_IRQn);

}



void ADC_ConvertToFloat(void){

	/* Vbus plus */
	//analog_in.V_DC_plus = ((float)ADC_raw_val[VBUS_PLUS_CHANNEL])*(VREF_VOLTAGE/(ADC_RES_COUNT*VBUS_PLUS_GAIN))-VBUS_PLUS_OFFSET;

}







