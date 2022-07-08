/*
 * I2C_sensors.h
 *
 *  Created on: Jul 8, 2022
 *      Author: Hugo Boyce
 */

#ifndef I2C_SENSORS_H_
#define I2C_SENSORS_H_

#define I2C_SPEED 50000 /* SCD30 Datasheet recommends a maximum speed of 50 kbit/s */

typedef void (*I2C_FUNC)(uint32_t u32Status);

#endif /* I2C_SENSORS_H_ */
