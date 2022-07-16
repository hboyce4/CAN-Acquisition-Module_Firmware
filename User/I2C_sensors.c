/* I2C_sensors.c
 *
 *  Created on: Jul 8, 2022
 *      Author: Hugo Boyce
 */

/**************************************************************************//**
 * @file     main.c
 * @version  V2.00
 * @brief    M480 I2C Driver Sample Code
 *           This is a I2C master mode demo and need to be tested with a slave device.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "NuMicro.h"
#include "I2C_sensors.h"



/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_au8Buff[I2C_DATA_BUFF_LEN];
//volatile uint8_t g_au8Data[I2C_DATA_BUFF_LEN];
volatile uint8_t g_u8DataLen;
volatile uint8_t g_u8Index;
volatile bool g_bEndFlag = false;
volatile bool g_bReceiving = false;

volatile bool g_bACKFlag = false; /* Flag to track when an ACK was received following an adress write */

// TODO: Overhaul this completely, make a job buffer, so we can get rid of the delay_ms() .

/* Code supports one sensor at a time for now */
volatile digital_sensor_channel_t env_sensor = { /* Sensor initialisation */




};

void I2C0_Init(void)
{
    /* Open I2C0 and set clock to 100k */
    I2C_Open(I2C0, I2C_SPEED);

    /* Get I2C0 Bus Clock */
    //printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

    I2C_EnableInt(I2C0);
    NVIC_EnableIRQ(I2C0_IRQn);
}


/* adr is the 7-bit device address i.e. without the 8th (R/W) bit. */
/* cmd is a 16 bit command sent before receiving */
/* exp_data_len is the number of bytes the master expects to receive (i.e. the nb of bytes we will ACK). */
/* This uses repeated start */

void I2C_sensorReceiveWithCommand(uint8_t adr, uint16_t cmd, uint8_t exp_data_len){

	g_bReceiving = true;

	g_u8DeviceAddr = adr;

	g_au8Buff[0] = (uint8_t)((cmd & 0xFF00) >> 8); /* Command MSB placed as the first data byte to transmit */
	g_au8Buff[1] = (uint8_t)(cmd & 0x00FF); /* Command LSB placed as the second data byte to transmit */

	g_u8Index = 0;
	g_u8DataLen = 2 + exp_data_len; /* The total data to be exchanged is the 2 command bytes (sent) plus the data we expect to receive. */

	g_bEndFlag = false;

	/* I2C as master sends START signal */
	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);

}

/* adr is the 7-bit device address i.e. without the 8th (R/W) bit. */
/* exp_data_len is the number of bytes the master expects to receive (i.e. the nb of bytes we will ACK). */
/* This is for when repeated start can't be used. Ex.: with SCD30 */

void I2C_sensorReceive(uint8_t adr, uint8_t exp_data_len){

	g_bReceiving = true;

	g_u8DeviceAddr = adr;

	//g_au8TxData[0] = (uint8_t)((cmd & 0xFF00) >> 8); /* Command MSB placed as the first data byte to transmit */
	//g_au8TxData[1] = (uint8_t)(cmd & 0x00FF); /* Command LSB placed as the second data byte to transmit */

	g_u8Index = 2; /* since there's no command, we start at byte 2 in the buffer */
	g_u8DataLen = 2 + exp_data_len; /* we leave room for a command even if it's not used */

	g_bEndFlag = false;

	/* I2C as master sends START signal */
	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);
}

/* adr is the 7-bit device address i.e. without the 8th (R/W) bit. */
/* cmd is a 16 bit command sent before the data */
/* data is a pointer to the data */
/* exp_data_len is the number of bytes of DATA to send EXCLUDING the cmd (i.e. if just a cmd is  sent, this will be zero). */

void I2C_sensorSendWithCommand(uint8_t adr, uint16_t cmd, uint8_t* data, uint8_t data_len){

	g_bReceiving = false;

	g_u8DeviceAddr = adr;

	g_au8Buff[0] = (uint8_t)((cmd & 0xFF00) >> 8); /* Command MSB placed as the first data byte to transmit */
	g_au8Buff[1] = (uint8_t)(cmd & 0x00FF); /* Command LSB placed as the second data byte to transmit */

	g_u8Index = 0;
	g_u8DataLen = 2 + data_len; /* The total data to be exchanged is the 2 command bytes (sent) plus the data we want to send. */

	uint8_t i;
	for(i = 0; i < data_len; i++){ /* Copy data to driver buffer */
		g_au8Buff[i+2] = data[i];
	}

	g_bEndFlag = false;

	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);

}


bool I2C_sensorSpam(uint8_t adr){

	/* Do a few writes (spam SLA+W) and see the response from the bus. */

	/* Attempt at first device */
	g_u8DeviceAddr = adr;

	g_bReceiving = false; /* We'll be using SLA+W*/

	/* Initialize everything else zero */
	g_bACKFlag = false;
	g_u8Index = 0;
	g_u8DataLen = 0;

	uint8_t i;
	bool acked = false;
	for(i = 0; i < I2C_AUTODETECT_ATTEMPTS; i++){

		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA); /* Trigger start condition */
		delay_ms(2); /* Wait for the command to be sent*/

		if(g_bACKFlag){
			acked = true;
		}
	}

	return acked;
}


void I2C_sensorWaitForTransactionEnd(void){

	while(!g_bEndFlag){/* As long as the end flag is not raised */
		//Wait
	}
}

void I2C_sensorAutodetect(void){

	delay_ms(500); /* Wait to make sure the sensor has awaken */

	if(I2C_sensorSpam(SHT3x_ADR)){
		env_sensor.sensorType = I2C_SENSOR_SHT3x;

		env_sensor.temperature_fieldUnit = UNIT_CNT;
		env_sensor.temperature_processUnit = UNIT_CELSIUS;

		env_sensor.humidity_fieldUnit = UNIT_CNT;
		env_sensor.humidity_processUnit = UNIT_PERCENT_RH;

		env_sensor.CO2_fieldUnit = UNIT_NONE; /* No CO2 sensing on the SHT3x */
		env_sensor.CO2_processUnit = UNIT_NONE;


	}else if(I2C_sensorSpam(SCD30_ADR)){
		env_sensor.sensorType = I2C_SENSOR_SCD30;

		env_sensor.temperature_fieldUnit = UNIT_NONE;
		env_sensor.temperature_processUnit = UNIT_CELSIUS;

		env_sensor.humidity_fieldUnit = UNIT_NONE;
		env_sensor.humidity_processUnit = UNIT_PERCENT_RH;

		env_sensor.CO2_fieldUnit = UNIT_NONE; /* No CO2 sensing on the SHT3x */
		env_sensor.CO2_processUnit = UNIT_PPM;

	}else{
		env_sensor.sensorType = I2C_SENSOR_NONE;
	}







}

void I2C_sensorConfigure(void){

	if(env_sensor.sensorType == I2C_SENSOR_SCD30){/* If sensor is SCD30 */
			/* Perform initial configuration */

			uint8_t press_data[SCD30_MEAS_INTERVAL_LEN_BYTES];

			uint16_t measurement_interval = 2;/* Setting atmospheric pressure to zero will disable pressure compensation */
			press_data[0] = (uint8_t)((measurement_interval & 0xFF00)>>8); /* MSB first */
			press_data[1] = (uint8_t)(measurement_interval & 0x00FF);/* then LSB */

			I2C_sensorSendWithCommand(SCD30_ADR, SCD30_CMD_SET_MEAS_INTERVAL_ACQ, press_data, SCD30_MEAS_INTERVAL_LEN_BYTES);

			delay_ms(3); /* Allow at least 3ms for the sensor to process data */

			uint8_t atm_data[SCD30_ATM_PRESS_LEN_BYTES];

			uint16_t atmospheric_pressure = 0;/* Setting atmospheric pressure to zero will disable pressure compensation */
			atm_data[0] = (uint8_t)((atmospheric_pressure & 0xFF00)>>8); /* MSB first */
			atm_data[1] = (uint8_t)(atmospheric_pressure & 0x00FF);/* then LSB */

			I2C_sensorSendWithCommand(SCD30_ADR, SCD30_CMD_START_CONTINUOUS_ACQ, atm_data, SCD30_ATM_PRESS_LEN_BYTES);

			delay_ms(3); /* Allow at least 3ms for the sensor to process data */


		}


}


void I2C_sensorOncePerSecondRoutine(void){

	switch(env_sensor.sensorType){

		case I2C_SENSOR_SHT3x:
			I2C_sensorReceiveWithCommand(SHT3x_ADR, SHT3x_CMD_ONE_SHOT_ACQ, SHT3x_ACQ_LEN);
			break;

		case I2C_SENSOR_SCD30:

			I2C_sensorSendWithCommand(SCD30_ADR, SCD30_CMD_GET_MEAS_RDY, NULL, 0); /* Prepare a new readout for the next time this fct is run */
			I2C_sensorWaitForTransactionEnd();
			I2C_sensorReceive(SCD30_ADR, SCD30_DATA_RDY_LEN_BYTES);/* Get all the data */
			I2C_sensorWaitForTransactionEnd();

			if(g_au8Buff[3] == 1){

				I2C_sensorSendWithCommand(SCD30_ADR, SCD30_CMD_GET_MEAS, NULL, 0);
				I2C_sensorWaitForTransactionEnd();

				delay_ms(3);

				I2C_sensorReceive(SCD30_ADR, SCD30_FULL_READOUT_LEN_BYTES);/* Get all the data */

			}

			break;

		case I2C_SENSOR_NONE:
			break;

       }


}

void I2C_sensorCheckIfNewDataAndConvert(void){

	if(g_bEndFlag){/* If transfer finished */
		g_bEndFlag = false;

		uint16_t cmd = 0;

		switch(env_sensor.sensorType){/* This switch/case is probably not necessary since we check the last command */

			case I2C_SENSOR_SHT3x:
				/* Get the command that was sent */
				/* The first 2 bytes in the buffer are the 16 bit command */
				cmd |= (uint16_t)g_au8Buff[0];/* Command MSB */
				cmd <<= 8; /*Left shift by 8 */
				cmd |= (uint16_t)g_au8Buff[1];/* Command LSB */

				if(cmd == SHT3x_CMD_ONE_SHOT_ACQ){

					/* What comes after are the data bytes */
					uint32_t temperature = 0;
					temperature |= (uint32_t)g_au8Buff[2]; /* Temperature MSB */
					temperature <<= 8; /*Left shift by 8 */
					temperature |= (uint32_t)g_au8Buff[3]; /* Temperature LSB */

					/* Don't check CRC8 in g_au8Buff[4] for now. */

					uint32_t humidity = 0;
					humidity |= (uint32_t)g_au8Buff[5]; /* Humidity MSB */
					humidity <<= 8; /*Left shift by 8 */
					humidity |= (uint32_t)g_au8Buff[6]; /* Humidity LSB */

					/* Don't check CRC8 in g_au8Buff[7] for now. */

					env_sensor.temperature_fieldValue = temperature;
					env_sensor.humidity_fieldValue = humidity;


					if(env_sensor.temperature_processUnit == UNIT_CELSIUS){
						/* Conversion as per SHT3x-DIS datasheet version 6, page 14 */
						env_sensor.temperature_processValue = -45.0 + (175.0 *  ((float)temperature) / 65535.0);
					}


					if(env_sensor.humidity_processUnit == UNIT_PERCENT_RH){
						/* Conversion as per SHT3x-DIS datasheet version 6, page 14 */
						env_sensor.humidity_processValue = 100.0 * ((float)humidity) / 65535.0;
					}

				}
				break;

			case I2C_SENSOR_SCD30:

				/* Get the command that was sent */
				/* The first 2 bytes in the buffer are the 16 bit command */
				cmd |= (uint16_t)g_au8Buff[0];/* Command MSB */
				cmd <<= 8; /*Left shift by 8 */
				cmd |= (uint16_t)g_au8Buff[1];/* Command LSB */

				if(cmd == SCD30_CMD_GET_MEAS){ /* If the last command was "get the measurement" from SCD30 */

					/* What comes after are the data bytes */
					/* SDC30 doesn't really have a field unit, and returns a float directly */

					uint32_t CO2 = 0;/* Not really an unsigned int */
					CO2 |= (uint32_t)g_au8Buff[2]; /* CO2 MMSB */
					CO2 <<= 8; /*Left shift by 8 */
					CO2 |= (uint32_t)g_au8Buff[3]; /* CO2 MLSB */
					/* Don't check CRC8 in g_au8Buff[4] for now. */
					CO2 <<= 8; /*Left shift by 8 */
					CO2 |= (uint32_t)g_au8Buff[5]; /* CO2 LMSB */
					CO2 <<= 8; /*Left shift by 8 */
					CO2 |= (uint32_t)g_au8Buff[6]; /* CO2 LLSB */
					/* Don't check CRC8 in g_au8Buff[7] for now. */

					uint32_t temperature = 0;/* Not really an unsigned int */
					temperature |= (uint32_t)g_au8Buff[8]; /* Temperature MMSB */
					temperature <<= 8; /*Left shift by 8 */
					temperature |= (uint32_t)g_au8Buff[9]; /* Temperature MLSB */
					/* Don't check CRC8 in g_au8Buff[10] for now. */
					temperature <<= 8; /*Left shift by 8 */
					temperature |= (uint32_t)g_au8Buff[11]; /* Temperature LMSB */
					temperature <<= 8; /*Left shift by 8 */
					temperature |= (uint32_t)g_au8Buff[12]; /* Temperature LLSB */
					/* Don't check CRC8 in g_au8Buff[13] for now. */

					uint32_t humidity = 0;/* Not really an unsigned int */
					humidity |= (uint32_t)g_au8Buff[14]; /* Humidity  MMSB */
					humidity <<= 8; /*Left shift by 8 */
					humidity |= (uint32_t)g_au8Buff[15]; /* Humidity MLSB */
					/* Don't check CRC8 in g_au8Buff[16] for now. */
					humidity <<= 8; /*Left shift by 8 */
					humidity |= (uint32_t)g_au8Buff[17]; /* Humidity  MMSB */
					humidity <<= 8; /*Left shift by 8 */
					humidity |= (uint32_t)g_au8Buff[18]; /* Humidity MLSB */
					/* Don't check CRC8 in g_au8Buff[19] for now. */

					/* SDC30 doesn't really have a field unit, and returns a float directly */

					if(env_sensor.temperature_processUnit == UNIT_CELSIUS){
						/* SCD30 returns a float directly */
						memcpy(&env_sensor.temperature_processValue, &temperature, 4);
					}

					if(env_sensor.humidity_processUnit == UNIT_PERCENT_RH){
						/* SCD30 returns a float directly */
						memcpy(&env_sensor.humidity_processValue, &humidity, 4);
					}

					if(env_sensor.CO2_processUnit == UNIT_PPM){
						/* SCD30 returns a float directly */
						memcpy(&env_sensor.CO2_processValue, &CO2, 4);
					}

				}
				break;

			case I2C_SENSOR_NONE:
				break;

		}

	}

}

