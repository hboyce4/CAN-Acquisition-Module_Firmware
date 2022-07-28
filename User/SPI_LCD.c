/*
 * LCD.c
 *
 *  Created on: Jul 20, 2022
 *      Author: Hugo Boyce
 */

/* Adapted from BuyDisplay.com code example */
/* https://www.buydisplay.com/cog-serial-spi-132x32-graphic-lcd-display-no-backlight-st7567a */
/* Driver IC is a ST7567A
 * https://www.crystalfontz.com/controllers/Sitronix/ST7567A/483/ */


#include "NuMicro.h"
#include "SPI_LCD.h"
#include "user_sys.h"
#include "CAN_message.h"
#include "analog.h"
#include "I2C_sensors.h"
#include "interrupt.h"



uint8_t	 g_LCDFontSize;
bool g_LastWriteType;
uint8_t g_DisplayChannelIndex;


extern const uint8_t F6x8[][6];
extern const uint8_t F8X16[];

void SPI_Init(void){

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init SPI                                                                                                */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Configure as a master, clock idle high, 8-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	/* Set IP clock divider. SPI clock rate = 10 MHz */
	SPI_Open(SPI0, SPI_MASTER, SPI_MODE_3, 8, SPI_SPEED);

	/* Enable the automatic hardware slave select function. Select the SS pin and configure as low-active. */
	SPI_EnableAutoSS(SPI0, SPI_SS, SPI_SS_ACTIVE_LOW);


}



void LCD_Write_Data(uint8_t dat){/* Blocking mode! Be careful */

	if(g_LastWriteType == LCD_COMMAND){/* If the last transaction was of a different type */

		while(SPI_IS_BUSY(SPI0)){/* Wait as long as the HW has not finished transmitting */

		}


	}else{/* Else the last command was of the same type,*/
		while(SPI_GET_TX_FIFO_FULL_FLAG(SPI0)){/* Just wait until there's room in the hardware buffer */
			// Wait
		}
	}
	LCD_CD_PIN = LCD_DATA;/* Switch to data mode (This signal is latched on the 8th bit) */
	SPI_WRITE_TX(SPI0, dat); /* Push the data to the hardware buffer */
	g_LastWriteType = LCD_DATA; /* Note what was the last transaction */

}

void LCD_Write_Command(uint8_t cmd){/* Blocking mode! Be careful */

	if(g_LastWriteType == LCD_DATA){/* If the last transaction was of a different type */

		while(SPI_IS_BUSY(SPI0)){/* Wait as long as the HW has not finished transmitting */

		}
		LCD_CD_PIN = LCD_COMMAND;/* Switch to command mode */

	}else{/* Else the last command was of the same type,*/
		while(SPI_GET_TX_FIFO_FULL_FLAG(SPI0)){/* Just wait until there's room in the hardware buffer */
			// Wait
		}
	}
	LCD_CD_PIN = LCD_COMMAND;/* Switch to command mode (This signal is latched on the 8th bit) */
	SPI_WRITE_TX(SPI0, cmd); /* Push the data to the hardware buffer */
	g_LastWriteType = LCD_COMMAND; /* Note what was the last transaction */

}


void LCD_Init(void)
{

	//This may end up being necassary
	//RST=1;
	//delay_ms(10);
	//RST=0;
	//delay_ms(10);
	//RST=1;
	//delay_ms(10);


	LCD_Write_Command(0xe2);	//soft reset
	delay_ms(10);
	LCD_Write_Command(0x2c);  	//Control built-in power circuit ON	    VB
	LCD_Write_Command(0x2e);  	//Control built-in power circuit ON	    VR
	LCD_Write_Command(0x2f);  	//Control built-in power circuit ON	    VFgu64SysTickIntCnt
	delay_ms(10);

	LCD_Write_Command(0xf8);  //Set booster level
 	LCD_Write_Command(0x00);  //Set  4X booster level

	LCD_Write_Command(0x25);  //Select regulation resistor ratio
	LCD_Write_Command(0x81);  	 //Setelectronic volume (EV) level
	LCD_Write_Command(LCD_CONTRAST);  //Setelectronic volume (EV) level
	LCD_Write_Command(0xa2);  //1/9bias
	LCD_Write_Command(0xc8);  //Set output direction of COM
	LCD_Write_Command(0xa0);  //Set scan direction of SEG
 	LCD_Write_Command(0xa4);  //Set normal display

  	LCD_Write_Command(0xa6);  //Set normal display

	LCD_Write_Command(0x40);  //start line
	LCD_Write_Command(0xaf);  //  display ON


}


void LCD_Set_Contrast_Control_Register(uint8_t mod)
{
    LCD_Write_Command(0x81);
	LCD_Write_Command(mod);
	return;
}


// Set page address 0~8
void LCD_Set_Page_Address(uint8_t add)
{
    add=0xb0|add;
    LCD_Write_Command(add);
	return;
}

void LCD_Set_Column_Address(uint8_t add)
{
    LCD_Write_Command((0x10|(add>>4)));
	LCD_Write_Command((0x0f&add));
	return;
}



void LCD_Set_Pos(uint8_t x, uint8_t y)
{
	LCD_Write_Command(0xb0+y);
	LCD_Write_Command(((x&0xf0)>>4)|0x10);
	LCD_Write_Command((x&0x0f));
}



// ShowChar   g_LCDSize: 16/8
void LCD_ShowChar(uint8_t x,uint8_t y,char chr)
{
	uint8_t c=0,i=0;
		c=chr-' ';
		if(x>LCD_MAX_COLUMN-1){x=0;if(g_LCDFontSize==16)y+=2;else y+=1;}
		if(g_LCDFontSize ==16)
			{
			LCD_Set_Pos(x,y);
			for(i=0;i<8;i++)
			LCD_Write_Data(F8X16[c*16+i]);
			LCD_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			LCD_Write_Data(F8X16[c*16+i+8]);
			}
			else {
				LCD_Set_Pos(x,y);
				for(i=0;i<6;i++)
				LCD_Write_Data(F6x8[c][i]);

			}
}



//LCD_ShowString
void LCD_ShowString(uint8_t x,uint8_t y,char *chr)
{
	uint8_t j=0;
	while (chr[j]!='\0')
	{		LCD_ShowChar(x,y,chr[j]);
			x+=8;
		if(x>124){x=0; if(g_LCDFontSize==16)y+=2;else y+=1;}
			j++;
	}
}

void LCD_ClearScreen(void)
{
    uint8_t i,j;
	for(i=0;i<8;i++)
	{
	LCD_Set_Page_Address(i);
    LCD_Set_Column_Address(0x00);
        for(j=0;j<LCD_MAX_COLUMN;j++)
		{
		    LCD_Write_Data(0x00);
		}
	}
    return;
}

void LCD_Draw(void){


	char buff_str[LCD_STR_BUFF_LEN];
	char channel_str[LCD_STR_BUFF_LEN];
	char unit_str[LCD_STR_BUFF_LEN];

	LCD_ClearScreen();
	sprintf(buff_str,"Node ID: %u", g_CANNodeID);
	g_LCDFontSize=LCD_SIZE_LARGE; /* Display in large font cuz that's the most useful info */
	LCD_ShowString(10,0,buff_str);

	sprintf(buff_str,"Tx ID: 0x%X", (g_CANNodeID | 0x80));/* Node ID in hex, plus the 0x80 that is present on all transmitted IDs */
	g_LCDFontSize=LCD_SIZE_SMALL; /* The rest will be displayed in small font */
	LCD_ShowString(0,2,buff_str);

	sprintf(buff_str,"Err. Code:%X", Error_GetCode());/* Display current error code */
	LCD_ShowString((LCD_MAX_COLUMN/2),2,buff_str);/* Draw half way down the screen */

	LCD_SetChannelString(g_DisplayChannelIndex, channel_str);

	float meas;
	if(g_DisplayChannelIndex <= EADC_LAST_GP_CHANNEL){
		meas = analog_channels[g_DisplayChannelIndex].processValue;
		sys_get_unit_string(analog_channels[g_DisplayChannelIndex].processUnit,unit_str);
	}else if(g_DisplayChannelIndex == (EADC_LAST_GP_CHANNEL + 1)){
		meas = env_sensor.temperature_processValue;
		sys_get_unit_string(env_sensor.temperature_processUnit,unit_str);
	}else if(g_DisplayChannelIndex == (EADC_LAST_GP_CHANNEL + 2)){
		meas = env_sensor.humidity_processValue;
		sys_get_unit_string(env_sensor.humidity_processUnit,unit_str);
	}else if(g_DisplayChannelIndex == (EADC_LAST_GP_CHANNEL + 3)){
		meas = env_sensor.CO2_processValue;
		sys_get_unit_string(env_sensor.CO2_processUnit,unit_str);
	}


	sprintf(buff_str,"%s%f %s", channel_str, meas, unit_str);/* Display current error code */

}


bool LCD_CheckIfChannelEnabled(uint8_t index){

	if(index <= EADC_LAST_GP_CHANNEL){
		return analog_channels[index].isEnabled;
	}else if(index == (EADC_LAST_GP_CHANNEL + 1)){
		return env_sensor.temperature_isEnabled;
	}else if(index == (EADC_LAST_GP_CHANNEL + 2)){
		return env_sensor.humidity_isEnabled;
	}else if(index == (EADC_LAST_GP_CHANNEL + 3)){
		return env_sensor.CO2_isEnabled;
	}else{/* Should never happen theoretically */
		return false;
	}


}

void LCD_FindNextChannel(void){

	g_DisplayChannelIndex++; /* Try next channel */
	if(g_DisplayChannelIndex > EADC_LAST_GP_CHANNEL + 3){/* If that gets us after the last index*/
		g_DisplayChannelIndex = 0;/* Rollover */
	}

	while(!(LCD_CheckIfChannelEnabled(g_DisplayChannelIndex))){/* While the selected channel is disabled*/
		g_DisplayChannelIndex++; /* Try next channel */
		if(g_DisplayChannelIndex > EADC_LAST_GP_CHANNEL + 3){/* If that gets us after the last index*/
			g_DisplayChannelIndex = 0;/* Rollover */
		}
	}

}

void LCD_SetChannelString(uint8_t index, char* string){

	if(index <= EADC_LAST_GP_CHANNEL){
			sprintf(string, "CH%u: ");
	}else{
			sprintf(string, "Sens.: ");
	}

}



