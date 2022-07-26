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
		[0] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[1] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[2] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[3] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[4] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[5] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[6] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3900},

		[7] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_VALUE,
				.NTCRZero = 10000,
				.NTCBeta = 3977},

		[8] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

		[9] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

		[10] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

		[11] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.voltageGain = ANALOG_VOLTAGE_GAIN},

};

