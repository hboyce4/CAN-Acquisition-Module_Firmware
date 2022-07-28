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
    //g_LCDSize=16;
    //LCD_ShowString(10,2,"Buydisplay.com");


    while(1)
    {
    	// All the time
        VCOM_TransferData();


        UI_read_user_input();

        I2C_sensorCheckIfNewDataAndConvert();

        CAN_Service();

        if(){ /* If CAN peripheral is not busy anymore */
        	LED_GREEN = LED_OFF; /* turn on green LED */
        }



        if(gbDrawNewUIFrame){// Every UI_FRAME_INTERVAL_MS milliseconds
        	gbDrawNewUIFrame = false;

        	/* Update all the user-facing stuff*/

        	if(Error_GetCode()){ /* If error code isn't zero */
        		LED_RED = LED_ON;
			}else{
				LED_RED = LED_OFF;
			}

        	LCD_Draw();

        	if(gbTerminalActive){
        		UI_draw();/* user interface on VCOM port */
        	}

        }

        if(gbSecondsFlag){// Every second
        	gbSecondsFlag = false;// Clear the flag
        	// Do things that need to be done every second

        	//ADC_StartAcquisition(); /* Run an acquisition every second */

        	cycleLED();

        	I2C_sensorOncePerSecondRoutine();/* This easily takes 10 ms*/

        	LCD_FindNextChannel();

        }

        if(gbSampleAnalog){
        	gbSampleAnalog = false;

        	Analog_Acquire(); /* Sample analog inputs */

        }


    }


}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
