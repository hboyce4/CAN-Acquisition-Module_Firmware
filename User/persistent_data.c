/*
 * persistent_data.c
 *
 *  Created on: Jul 25, 2022
 *      Author: Hugo Boyce
 */

#include "NuMicro.h"
#include "persistent_data.h"
#include "analog.h"
#include "CAN_message.h"


#define PD_ANALOG_CHANNEL_OFFSET 0x40 /* 64 bytes or 16 floats per channel */
#define PD_ENABLE_OFFSET 0
#define PD_SENSOR_TYPE_OFFSET 4
#define PD_FIELD_UNIT_OFFSET 8
#define PD_PROCESS_UNIT_OFFSET 12
#define PD_BIAS_RESISTOR_OFFSET 16
#define PD_NTC_R_ZERO_OFFSET 20
#define PD_NTC_BETA_OFFSET 24
#define PD_VOLTAGE_GAIN_OFFSET 28
#define PD_SENSOR_GAIN_OFFSET 32

#define PD_CAN_SPEED_OFFSET 0x300
#define PD_CAN_NODE_ID_OFFSET 0x304

#define CHECKSUM_OFFSET 0x1000 /* Must be a multiple of 512 to be able to calc. the CRC32 using HW. */


uint32_t g_u32UID[UID_SIZE];


void PD_Init(void){

	SYS_UnlockReg();
	FMC_Open();       /* Enable FMC ISP function   */


	/* Get UID*/
    uint8_t i;
    for(i = 0; i < UID_SIZE; i++){
    	g_u32UID[i] = FMC_ReadUID(i);
    }

    /* Get data flash base address */
    uint32_t DataFlashBaseAdress = FMC_ReadDataFlashBaseAddr();

    if((FMC_Read(FMC_CONFIG_BASE) & 0x01) ||  DataFlashBaseAdress == 0xFFFFFFFF){/* If the DFEN bit if the CONFIG says "data flash disabled" or the data flash base address is all "F"s */

    	printf("No Data Flash Allocated!!\n");
    	printf("Allocate some using Nuvoton ICP Prog. Tool\n");
    }


    FMC_Close();
    SYS_LockReg();
}

void PD_SaveConfig(void){

	SYS_UnlockReg();
	FMC_Open();       /* Enable FMC ISP function   */
	FMC_ENABLE_AP_UPDATE();

	uint32_t DataFlashBaseAdress = FMC_ReadDataFlashBaseAddr();

	if(!(FMC_Read(FMC_CONFIG_BASE) & 0x01) && (DataFlashBaseAdress != 0xFFFFFFFF)){ /* If the data flash base address is not set to the obviously invalid value... */
		/* .. and the DFEN bit is set to "data flash enabled" */


		uint8_t i;
		uint32_t temp;
		for(i = 0; i < EADC_TOTAL_CHANNELS; i++){

			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_ENABLE_OFFSET, (uint32_t)analog_channels[i].isEnabled);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_SENSOR_TYPE_OFFSET, (uint32_t)analog_channels[i].sensorType);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_FIELD_UNIT_OFFSET, (uint32_t)analog_channels[i].fieldUnit);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_PROCESS_UNIT_OFFSET, (uint32_t)analog_channels[i].processUnit);

			memcpy(&temp, &analog_channels[i].biasResistor, 4);/* Since we're storing floats we must use memcpy instead of casting to int */
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_BIAS_RESISTOR_OFFSET, temp);

			memcpy(&temp, &analog_channels[i].NTCRZero, 4);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_NTC_R_ZERO_OFFSET, temp);

			memcpy(&temp, &analog_channels[i].NTCBeta, 4);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_NTC_BETA_OFFSET, temp);

			memcpy(&temp, &analog_channels[i].voltageGain, 4);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_VOLTAGE_GAIN_OFFSET, temp);

			memcpy(&temp, &analog_channels[i].sensorGain, 4);
			FMC_Write(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_SENSOR_GAIN_OFFSET, temp);

		}

		FMC_Write(DataFlashBaseAdress + PD_CAN_SPEED_OFFSET, CAN_SPEED);
		FMC_Write(DataFlashBaseAdress + PD_CAN_NODE_ID_OFFSET, CAN_NODE_ID);


		//while ((FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk)) {}


		uint32_t CRC32 = FMC_GetChkSum(DataFlashBaseAdress, CHECKSUM_OFFSET);

		printf("Config. CRC32: %X\n", CRC32);

		//FMC_Write(DataFlashBaseAdress + CHECKSUM_OFFSET, CRC32);

		uint32_t *ptr = (uint32_t *)(DataFlashBaseAdress);

		printf("Start of config data\n");
		printf("%X, %X, %X, %X\n", ptr[0],  ptr[1],  ptr[2],  ptr[3]);

		printf("%X, %X, %X, %X\n", ptr[4],  ptr[5],  ptr[6],  ptr[7]);

		FMC_Close();
		SYS_LockReg();

		inpw(1);

	}


}

