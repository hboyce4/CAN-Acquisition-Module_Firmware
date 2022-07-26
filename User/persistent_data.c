/*
 * persistent_data.c
 *
 *  Created on: Jul 25, 2022
 *      Author: Hugo Boyce
 */

#include <stdbool.h>
#include "NuMicro.h"
#include "persistent_data.h"
#include "analog.h"
#include "CAN_message.h"
#include "errors.h"


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

#define CHECKSUM_OFFSET 0x400 /* Comes after everything else*/


uint32_t g_u32UID[UID_SIZE];

bool g_Error_InvalidConfig = true;


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
    }else{

    	uint32_t checksum = PD_ComputeChecksum(DataFlashBaseAdress, CHECKSUM_OFFSET);/* Compute the config's checksum */

    	if(checksum == inpw(DataFlashBaseAdress + CHECKSUM_OFFSET)){/* If the computed checksum is equal to the checksum stored in memory */
    		printf("Config CRC32 OK\n");
    		printf("Loading config from memory...\n");
    		// PD_LoadConfig();
    		Error_Clear(ERROR_INVALID_CONFIG);

    	}else{

    		printf("Computed CRC32:%X\n", checksum);
    		printf("CRC32 found in memory: %X\n", inpw(DataFlashBaseAdress + CHECKSUM_OFFSET));
    		printf("Config CRC32 invalid!!!\n");
    		printf("Using default config!\n");
    	}


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

		extern uint32_t g_CANSpeed;
		FMC_Write(DataFlashBaseAdress + PD_CAN_SPEED_OFFSET, g_CANSpeed);
		extern uint8_t g_CANNodeID;
		FMC_Write(DataFlashBaseAdress + PD_CAN_NODE_ID_OFFSET, (uint32_t)g_CANNodeID);


		//while ((FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk)) {}

		uint32_t CRC32 = PD_ComputeChecksum(DataFlashBaseAdress, CHECKSUM_OFFSET);

		printf("Config. CRC32: %X\n", CRC32);

		FMC_Write(DataFlashBaseAdress + CHECKSUM_OFFSET, CRC32);



		//uint32_t *ptr = (uint32_t *)(DataFlashBaseAdress);

		//printf("Start of config data\n");
		//printf("%X, %X, %X, %X\n", ptr[0],  ptr[1],  ptr[2],  ptr[3]);

		//printf("%X, %X, %X, %X\n", ptr[4],  ptr[5],  ptr[6],  ptr[7]);

		FMC_Close();
		SYS_LockReg();

		Error_Clear(ERROR_INVALID_CONFIG);

	}


}

uint32_t PD_ComputeChecksum(uint32_t start, uint32_t len){

	//uint32_t CRC32;
	//CRC32 = FMC_GetChkSum(DataFlashBaseAdress, CHECKSUM_OFFSET);/* Getting the checksum with the FMC is annoying because it only does 4K chunks */

	/* Configure CRC controller for CRC-CRC32 mode. Taken from Nuvoton BSP V3.05.003. Don't forget to enable clock! */
	CRC_Open(CRC_32, (CRC_WDATA_RVS | CRC_CHECKSUM_RVS | CRC_CHECKSUM_COM), 0xFFFFFFFF, CRC_CPU_WDATA_32);
	/* Start to execute "manual" CRC-CRC32 operation. Could be done with PDMA but we're not in a hurry. */
	uint32_t addr;
	for(addr = start; addr < start + len; addr+=4)
	{
		CRC_WRITE_DATA(inpw(addr));
	}
	return CRC_GetChecksum();

}


void PD_LoadConfig(void){


	uint32_t DataFlashBaseAdress = FMC->DFBA;

	uint8_t i;
	float temp;
	for(i = 0; i < EADC_TOTAL_CHANNELS; i++){

		analog_channels[i].isEnabled = (bool)inpw(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_ENABLE_OFFSET);
		analog_channels[i].sensorType = (analog_sensor_t)inpw(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_SENSOR_TYPE_OFFSET);
		analog_channels[i].fieldUnit = (physical_unit_t)inpw(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_FIELD_UNIT_OFFSET);
		analog_channels[i].processUnit = (physical_unit_t)inpw(DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_PROCESS_UNIT_OFFSET);

		memcpy(&temp, DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_BIAS_RESISTOR_OFFSET, 4);/* Since we're storing floats we must use memcpy instead of casting to int */
		analog_channels[i].biasResistor = temp;

		memcpy(&temp, DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_NTC_R_ZERO_OFFSET, 4);
		analog_channels[i].NTCRZero = temp;

		memcpy(&temp, DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_NTC_BETA_OFFSET, 4);
		analog_channels[i].NTCBeta = temp;

		memcpy(&temp, DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_VOLTAGE_GAIN_OFFSET, 4);
		analog_channels[i].voltageGain = temp;

		memcpy(&temp, DataFlashBaseAdress + (i * PD_ANALOG_CHANNEL_OFFSET) +  PD_SENSOR_GAIN_OFFSET, 4);
		analog_channels[i].sensorGain = temp;

	}

	extern uint32_t g_CANSpeed;
	g_CANSpeed = inpw(DataFlashBaseAdress + PD_CAN_SPEED_OFFSET);
	extern uint8_t g_CANNodeID;
	g_CANNodeID = (uint8_t)inpw(DataFlashBaseAdress + PD_CAN_NODE_ID_OFFSET);


}


