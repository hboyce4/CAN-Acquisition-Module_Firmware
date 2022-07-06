/*
 * analog.h
 *
 *  Created on: Jul 5, 2022
 *      Author: Hugo Boyce
 */

#ifndef ANALOG_H_
#define ANALOG_H_

/*---------------------------------------------------------------------------------------------------------*/
/* Includes           				                                                                      */
/*---------------------------------------------------------------------------------------------------------*/

#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macros           				                                                                      */
/*---------------------------------------------------------------------------------------------------------*/

//#define EADC_DMA_CHANNEL 2
#define EADC_OVERSAMPLING_NUMBER 8 /* MAX = 16 !!! 2^EADC_SHIFT_FOR_OVERSAMPLING_DIVISION = EADC_OVERSAMPLING_NUMBER*/
#define EADC_SHIFT_FOR_OVERSAMPLING_DIVISION 3  /* MAX = 4 !!! Log base 2 of EADC_OVERSAMPLING_NUMBER equals EADC_SHIFT_FOR_OVERSAMPLING_DIVISION */

#define EADC_TOTAL_CHANNELS 18 /* Total from the first to the last used channel*/
#define EADC_TEMPERATURE_CHANNELS 8
#define EADC_VOLTAGE_CHANNELS 4


#define EADC_VREF_VOLTAGE 1.800 /*[V] Voltage of the chip's analog voltage reference (Vref)*/

/*---------------------------------------------------------------------------------------------------------*/
/* Type definitions           				                                                               */
/*---------------------------------------------------------------------------------------------------------*/

typedef struct analog_temperatures{ /* Process values or state variables */

	volatile float CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7; /* [°C] */

} analog_temperatures_t;

typedef struct analog_voltages{ /* Process values or state variables */

	volatile float CH8, CH9, CH10, CH11; /* [mV] */

} analog_voltages_t;

/*---------------------------------------------------------------------------------------------------------*/
/* External global variables                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
extern volatile analog_temperatures_t analog_temperatures; /* [°C] */
extern volatile analog_voltages_t analog_voltages;/* [mV] */

// extern volatile uint16_t ADC_acq_buff[EADC_TOTAL_CHANNELS];
// extern volatile uint8_t ADC_acq_count;

/*---------------------------------------------------------------------------------------------------------*/
/* Functions declaration                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

void ADC_RunCal(void);

void ADC_ServiceOversampling(void);

void ADC_ConvertToFloat(void);

#endif /* ANALOG_H_ */
