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
#include "errors.h"

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


    while(1){

    	// All the time
    	WDT_RESET_COUNTER();

    	VCOM_TransferData();

        UI_read_user_input();

        I2C_sensorCheckIfNewDataAndConvert();

        CAN_Service();

        if(gbDrawNewUIFrame){// Every UI_FRAME_INTERVAL_MS milliseconds
        	gbDrawNewUIFrame = false;

        	/* Update all the user-facing stuff*/

        	if(Error_GetCode()){ /* If error code isn't zero */
        		LED_RED = LED_ON;
			}else{
				LED_RED = LED_OFF;
			}

        	LCD_Draw();/* Update LCD */

        	if(gbTerminalActive){/* If the VCOM port over USB is connected, and a terminal is open on the PC */
        		UI_draw();/* user interface on VCOM port */
        	}

        }

        if(gbSecondsFlag){// Every second
        	gbSecondsFlag = false;// Clear the flag
        	// Do things that need to be done every second

        	I2C_sensorOncePerSecondRoutine();/* This easily takes 10 ms*/

        	static uint8_t lcd_cnt = 0;
        	lcd_cnt++; /* Increment the counter everytime */
        	if(lcd_cnt >= LCD_SECONDS_PER_CHANNEL){
        		LCD_FindNextChannel();
        		lcd_cnt = 0;
        	}




        }

        if(gbSampleAnalog){
        	gbSampleAnalog = false;

        	Analog_Acquire(); /* Sample analog inputs */

        }


    }


}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
