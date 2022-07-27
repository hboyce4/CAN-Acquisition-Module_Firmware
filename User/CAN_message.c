/*
 * CAN_message.c
 *
 *  Created on: Jul 18, 2022
 *      Author: Hugo Boyce
 */

/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * @brief    Demonstrate CAN bus transmit a message with normal mode
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "NuMicro.h"
#include "CAN_message.h"
#include "I2C_sensors.h"
#include "analog.h"
#include "errors.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
STR_CANMSG_T rrMsg;

volatile bool g_bCANSyncFlag;
uint32_t g_CANSpeed = CAN_SPEED_DEFAULT;
uint8_t g_CANNodeID = CAN_NODE_ID_DEFAULT;


/*---------------------------------------------------------------------------------------------------------*/
/* ISR to handle CAN interrupt event                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void CAN_MsgInterrupt(uint32_t u32IIDR){

	if(u32IIDR== (0 + 1)){/* If INT is from message zero */

        //printf("Msg-0 INT and Callback\n");
        //CAN_Receive(tCAN, 0,&rrMsg); /* We could get the message data but we don't care */
        g_bCANSyncFlag = true; /* Set the CANbus SYNC flag. */

    }

}


/**
  * @brief      Init CAN driver
  */

void CAN_Init(void){

	CAN_Open(CAN0,  g_CANSpeed, CAN_NORMAL_MODE);

	/* Set RX Message (only SYNC message for now, in our case), as message zero */
	/* See CAN_message.h for SYNC message definition */
	if(CAN_SetRxMsg(CAN0, MSG(0),CAN_EXT_ID, 0x00000080) == FALSE)
	{
		printf("CAN0: Setting SYNC Msg Object failed\n");
	}

	CAN_EnableInt(CAN0, CAN_CON_IE_Msk);
	NVIC_SetPriority(CAN0_IRQn, (1<<__NVIC_PRIO_BITS) - 2);
	NVIC_EnableIRQ(CAN0_IRQn);


}

void CAN_Service(void){

	if(g_bCANSyncFlag){/* If SYNC message has been received */
		g_bCANSyncFlag = false; /* Clear the flag */
		/* and send messages */

		/* See CAN_message.h for definitions */
		CAN_BuildAndSendMessage_32(1, 0x00000180 + g_CANNodeID, &env_sensor.temperature_fieldValue, &env_sensor.temperature_processValue);
		CAN_BuildAndSendMessage_32(2, 0x00000280 + g_CANNodeID, &env_sensor.humidity_fieldValue, &env_sensor.humidity_processValue);
		CAN_BuildAndSendMessage_32(3, 0x00000380 + g_CANNodeID, &env_sensor.CO2_fieldValue, &env_sensor.CO2_processValue);
		uint8_t i;
		for(i = 0; i < EADC_TOTAL_CHANNELS; i++){/* Messages 4 to 15 */
			CAN_BuildAndSendMessage_32(i + 4, 0x00001080 + (0x100*i) + g_CANNodeID, &analog_channels[i].fieldValue, &analog_channels[i].processValue);
		}
		uint32_t err_l, err_h;
		err_l = Error_GetCode();
		err_h = 0;
		CAN_BuildAndSendMessage_32(16, 0x00002080 + g_CANNodeID, &err_l, &err_h);


		printf("CAN SYNC received.\n");
	}

}


/* number is the object (message) number */
/* id is the message's COB-ID (CAN-ID) */
/* lower is a pointer to the lower 32 bits of data (Bytes 0 to 3) */
/* upper is a pointer to the upper 32 bits of data (Bytes 4 to 7) */
void CAN_BuildAndSendMessage_32(uint8_t number, uint32_t id, void* lower, void* upper){

	STR_CANMSG_T tMsg;
	uint32_t temp = 0;
	int8_t i = 0;

	 /* Send a 29-bit message */
	tMsg.FrameType= CAN_DATA_FRAME;
	tMsg.IdType   = CAN_EXT_ID;
	tMsg.Id       = id;
	tMsg.DLC      = 8;
	memcpy(&temp, lower , sizeof(temp));
	for(i = 3; i >= 0; i--){/* For i = 3 to i = 0 (for the 4 lower bytes) */
		tMsg.Data[i] = (uint8_t)(temp & 0xFF); /* Put the 8 lowest bytes in the CAN message data*/
		temp >>= 8;/* Shift right by one byte */
	}
	memcpy(&temp, upper, sizeof(temp));
	for(i = 7; i >= 4; i--){ /* For i = 7 to i = 4 (for the 4 highest bytes) */
		tMsg.Data[i] = (uint8_t)(temp & 0xFF); /* Put the 8 lowest bytes */
		temp >>= 8;/* Shift right by one byte */
	}

	if(CAN_Transmit(CAN0, MSG(number),&tMsg) == FALSE)/* If sending the message to the message RAM fails */
	{
		printf("CAN0: Set Tx Msg %u Object failed\n", number);/* Print to debug console */

	}


}
