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
#include "user_sys.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macros           				                                                                      */
/*---------------------------------------------------------------------------------------------------------*/

#define EADC_12BIT_MAX_COUNT 4096.0
#define EADC_OVERSAMPLING_NUMBER 8 /* MAX = 16 !!! */
#define EADC_VREF_MILLIVOLTS 1800 /*[mV] Voltage of the chip's analog voltage reference (Vref)*/

/* The first channel has to be 0 cause it's cleaner that way. */
#define EADC_LAST_GP_CHANNEL 11 /* Last consecutive general purpose channel */
#define EADC_TOTAL_CHANNELS (EADC_LAST_GP_CHANNEL + 1) /* Total number of channels */


#define ANALOG_SAMPLE_INTERVAL_MS 100 /* [ms] Time interval for acquisition of new samples and filtering */
#define ANALOG_SAMPLE_T (ANALOG_SAMPLE_INTERVAL_MS/1000.0) /* [s] */

#define ANALOG_BIAS_RESISTOR_VALUE 6200.0 /* [Ohms] Value of the NTC biasing reistor */
#define ANALOG_NTC_T_ZERO 298.15 /* [K] Equals 25 degree C. Temperature at which the NTC's R_Zero is measured. */

#define ANALOG_VOLTAGE_GAIN 50 /* [unitless] voltage gain of the voltage inputs */

#define ANALOG_LPF_TAU 10 /* [s] Amount of time after which 63% of step response is reached. TAU = R*C = 1/(2*pi*f_c) */

/*---------------------------------------------------------------------------------------------------------*/
/* Type definitions           				                                                               */
/*---------------------------------------------------------------------------------------------------------*/

typedef enum {SENSOR_NONE, SENSOR_NTC, SENSOR_PYRANOMETER, SENSOR_THERMOCOUPLE} analog_sensor_t;

typedef struct analog_channel{ /* Process values or state variables */

	uint32_t rawValue; /* [cnt] Raw value from the converter */
	float fieldValue;
	float fieldValue_filtered;
	float processValue;

	physical_unit_t fieldUnit;/* The field unit is what the sensor outputs and the acquisition module measures. Ex. Ohms or volts. */
	physical_unit_t processUnit; /* The process unit is what the sensor measures, the unit of the process. It is what we are interested in ultimately. Ex. celsius, watt/m^2. etc. */
	analog_sensor_t sensorType;

	/* For NTC only */
	float biasResistor; /* [Ohms] Value of the biasing resistor of the sensor */
	float NTCRZero; /* [Ohms] Resistance of the NTC thermistor at 25 degrees C */
	float NTCBeta; /* [K] Beta constant of the NTC thermistor sensor */

	/* For voltage input (pyranometer or thermocouple) */
	float voltageGain; /* [Unitless] Gain from the screw terminals to the ADC input. Ex. for thermocouples: 50, ex. for 0-10V: 0.15  */
	float sensorGain; /* [mixed units] Gain of the sensor. From field unit to physical unit. */

	float RMSNoise; /* RMS measurement noise of the fieldValue. */


} analog_channel_t;

/*---------------------------------------------------------------------------------------------------------*/
/* External global variables                                                                               */
/*---------------------------------------------------------------------------------------------------------*/

extern volatile analog_channel_t analog_channels[EADC_TOTAL_CHANNELS];

/*---------------------------------------------------------------------------------------------------------*/
/* Functions declaration                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

void ADC_Init(void);

void ADC_StartAcquisition(void);
void ADC_StartAcquisition_ChipTemp(void);

void Analog_Acquire(void);
void Analog_ConvertToFieldValue(void);
void Analog_FilterFieldValues(void);
void Analog_FilterNoise(void);
void Analog_ConvertToProcessValues(void);

#endif /* ANALOG_H_ */
