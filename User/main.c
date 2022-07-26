/**************************************************************************//**
 * @file     main.c
 * @brief    Demonstrate how to implement a USB virtual com port device.
 * @version  2.0.0
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "NuMicro.h"
#include "initialization.h"
#include "interrupt.h"
#include "vcom_serial.h"
#include "UI.h"
#include "analog.h"
#include "I2C_sensors.h"
#include "user_sys.h"
#include "CAN_message.h"
#include "SPI_LCD.h"
#include "persistent_data.h"

int main (void)
{

    SYS_Init();


    UART_Open(UART0, DEBUG_UART_SPEED); // Debug port. printf() is piped to this. See retarget.c file
    printf("CAN Acquisition Module - Debug port\n");

    PD_Init();
    PD_SaveConfig();


    ADC_Init();

    /* Initialize virtual COM port (over USB) */
    VCOM_Init();

    /* Init I2C0 */
    I2C0_Init();

    CAN_Init();

    SPI_Init();


    interrupt_setPriorities();


    I2C_sensorAutodetect();
    I2C_sensorConfigure();

    LCD_Init();
    g_LCDSize=16;
    LCD_ShowString(10,2,"Buydisplay.com");


    while(1)
    {
    	// All the time
        VCOM_TransferData();

        int8_t i8RowSel, i8ColumnSel;
        UI_read_user_input(&i8RowSel,&i8ColumnSel);

        I2C_sensorCheckIfNewDataAndConvert();

        CAN_Service();




        if(gbDrawNewUIFrame){// Every UI_FRAME_INTERVAL_MS milliseconds
        	gbDrawNewUIFrame = false;

        	if(gbTerminalActive){
        		UI_draw(i8RowSel, i8ColumnSel);

        	}

        }

        if(gbSecondsFlag){// Every second
        	gbSecondsFlag = false;// Clear the flag
        	// Do things that need to be done every second

        	//ADC_StartAcquisition(); /* Run an acquisition every second */

        	cycleLED();

        	I2C_sensorOncePerSecondRoutine();/* This easily takes 10 ms*/

        }

        if(gbSampleAnalog){
        	gbSampleAnalog = false;

        	Analog_Acquire(); /* Sample analog inputs */

        }


    }


}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
