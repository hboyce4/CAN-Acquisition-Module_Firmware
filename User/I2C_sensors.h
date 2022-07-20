/*
 * I2C_sensors.h
 *
 *  Created on: Jul 8, 2022
 *      Author: Hugo Boyce
 */

#ifndef I2C_SENSORS_H_
#define I2C_SENSORS_H_

#include <stdint.h>
#include "user_sys.h"

#define I2C_SPEED 50000 /* SCD30 Datasheet recommends a maximum speed of 50 kbit/s */
#define I2C_DATA_BUFF_LEN 32 /* Length of I2C data buffers (Tx and Rx) in bytes */
#define I2C_AUTODETECT_ATTEMPTS 3 /* Number of times Autodetect will check if a specific device responds */


#define SHT3x_ADR 0x44 /* SHT3x-DIS I2C 7-bit address */
#define SHT3x_CMD_ONE_SHOT_ACQ 0x2C06 /* Single shot mode, high repeatability, with clock stretching */
#define SHT3x_ACQ_LEN 6

#define SCD30_ADR 0x61 /* SHT3x-DIS I2C 7-bit address */
#define SCD30_CMD_START_CONTINUOUS_ACQ 0x0010 /* begin continuous measurement */
#define SCD30_CMD_SET_MEAS_INTERVAL_ACQ 0x4600 /* set automatic measurement interval */
#define SCD30_CMD_GET_MEAS 0x0300 /* After this command, the data following a read command will be all the measurements */
#define SCD30_CMD_GET_MEAS_RDY 0x0202 /* After this command, the data following a read command will be the data ready flag */

#define SCD30_DATA_RDY_LEN_BYTES 2
#define SCD30_ATM_PRESS_LEN_BYTES 2
#define SCD30_MEAS_INTERVAL_LEN_BYTES 2
#define SCD30_FULL_READOUT_LEN_BYTES 18

typedef enum {I2C_SENSOR_NONE, I2C_SENSOR_SHT3x, I2C_SENSOR_SCD30} I2C_sensor_t;

typedef struct digital_sensor_channel{ /* Process values or state variables */

	I2C_sensor_t sensorType;

	uint32_t temperature_fieldValue;/* [cnt] Raw value from the converter */
	float temperature_processValue;
	physical_unit_t temperature_fieldUnit;/* The field unit is what the sensor outputs and the acquisition module measures. Ex. Ohms or volts. */
	physical_unit_t temperature_processUnit; /* The process unit is what the sensor measures, the unit of the process. It is what we are interested in ultimately. Ex. celsius, watt/m^2. etc. */

	uint32_t humidity_fieldValue;/* [cnt] Raw value from the converter */
	float humidity_processValue;
	physical_unit_t humidity_fieldUnit;/* The field unit is what the sensor outputs and the acquisition module measures. Ex. Ohms or volts. */
	physical_unit_t humidity_processUnit; /* The process unit is what the sensor measures, the unit of the process. It is what we are interested in ultimately. Ex. celsius, watt/m^2. etc. */

	uint32_t CO2_fieldValue;/* [cnt] Raw value from the converter *Not used* */
	float CO2_processValue;
	physical_unit_t CO2_fieldUnit;/* The field unit is what the sensor outputs and the acquisition module measures. Ex. Ohms or volts. */
	physical_unit_t CO2_processUnit; /* The process unit is what the sensor measures, the unit of the process. It is what we are interested in ultimately. Ex. celsius, watt/m^2. etc. */

} digital_sensor_channel_t;

extern volatile digital_sensor_channel_t env_sensor;

void I2C0_Init(void);
void I2C_sensorReceiveWithCommand(uint8_t adr, uint16_t cmd, uint8_t exp_data_len);
void I2C_sensorReceive(uint8_t adr, uint8_t exp_data_len);
void I2C_sensorSendWithCommand(uint8_t adr, uint16_t cmd, uint8_t* data, uint8_t data_len);
bool I2C_sensorSpam(uint8_t);
void I2C_sensorWaitForTransactionEnd(void);

void I2C_sensorAutodetect(void);
void I2C_sensorConfigure(void);

void I2C_sensorOncePerSecondRoutine(void);
void I2C_sensorCheckIfNewDataAndConvert(void);
//void I2C_sensorUpdateValues(void);

#endif /* I2C_SENSORS_H_ */
