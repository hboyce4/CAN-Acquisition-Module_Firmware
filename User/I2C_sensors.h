/*
 * I2C_sensors.h
 *
 *  Created on: Jul 8, 2022
 *      Author: Hugo Boyce
 */

#ifndef I2C_SENSORS_H_
#define I2C_SENSORS_H_

#include <stdint.h>

#define I2C_SPEED 50000 /* SCD30 Datasheet recommends a maximum speed of 50 kbit/s */
#define I2C_DATA_BUFF_LEN 32 /* Length of I2C data buffers (Tx and Rx) in bytes */

void I2C_sensorReceiveWithCommand(uint8_t adr, uint16_t cmd, uint8_t exp_data_len);
void I2C_sensorReceive(uint8_t adr, uint8_t exp_data_len);
void I2C_sensorSend(uint8_t adr, uint16_t cmd, uint8_t* data, uint8_t data_len);
void I2C_sensorFetchReceviedData(uint8_t* data, uint8_t exp_data_len);

#endif /* I2C_SENSORS_H_ */
