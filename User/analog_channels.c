/*
 * analog_channels.c
 *
 *  Created on: Jul 7, 2022
 *      Author: Hugo Boyce
 */

#include "analog.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


volatile analog_channel_t analog_channels[EADC_TOTAL_CHANNELS] = { /* Default channel arrangement */
		[0] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[1] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[2] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[3] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[4] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[5] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[6] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[7] = {.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3977},

		[8] = {.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

		[9] = {.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

		[10] = {.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

		[11] = {.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

};

