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


volatile analog_channel_t analog_channels[EADC_TOTAL_CHANNELS] = { /* Default channel configuration */
		[0] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[1] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[2] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[3] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[4] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[5] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[6] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[7] = {.isEnabled = true,
				.fieldUnit = UNIT_OHM,
				.processUnit = UNIT_CELSIUS,
				.sensorType = ANALOG_SENSOR_NTC,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[8] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[9] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[10] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

		[11] = {.isEnabled = true,
				.fieldUnit = UNIT_MILLIVOLT,
				.processUnit = UNIT_MILLIVOLT,
				.sensorType = ANALOG_SENSOR_NONE,
				.biasResistor = ANALOG_BIAS_RESISTOR_DEFAULT,
				.NTCRZero = 10000,
				.NTCBeta = 3900,
				.voltageGain = ANALOG_VOLTAGE_GAIN_DEFAULT,
				.sensorGain = 1},

};

