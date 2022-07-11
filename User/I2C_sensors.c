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
#include "NuMicro.h"
#include "I2C_sensors.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_au8Data[I2C_DATA_BUFF_LEN];
volatile uint8_t g_u8DataLen;
volatile uint8_t g_u8Index;
volatile bool g_bEndFlag = false;
volatile bool g_bReceiving = false;

//typedef void (*I2C_FUNC)(uint32_t u32Status);

//static volatile I2C_FUNC s_I2C0HandlerFn = NULL;


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

	g_au8Data[0] = (uint8_t)((cmd & 0xFF00) >> 8); /* Command MSB placed as the first data byte to transmit */
	g_au8Data[1] = (uint8_t)(cmd & 0x00FF); /* Command LSB placed as the second data byte to transmit */

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

void I2C_sensorSend(uint8_t adr, uint16_t cmd, uint8_t* data, uint8_t data_len){

	g_bReceiving = false;

	g_u8DeviceAddr = adr;

	g_au8Data[0] = (uint8_t)((cmd & 0xFF00) >> 8); /* Command MSB placed as the first data byte to transmit */
	g_au8Data[1] = (uint8_t)(cmd & 0x00FF); /* Command LSB placed as the second data byte to transmit */

	g_u8Index = 0;
	g_u8DataLen = 2 + data_len; /* The total data to be exchanged is the 2 command bytes (sent) plus the data we want to send. */

	uint8_t i;
	for(i = 0; i < data_len; i++){ /* Copy data to driver buffer */
		g_au8Data[i] = data[i];
	}

	g_bEndFlag = false;

	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);

}

void I2C_sensorFetchReceviedData(uint8_t* data, uint8_t exp_data_len){

	if(g_bEndFlag){/* If transfer finished */
		g_bEndFlag = false;

		uint8_t i;
		for(i = 0; i < exp_data_len; i++){ /* Copy data to driver buffer */
			data[i] = g_au8Data[i];
		}
	}

}
