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

#include <stdbool.h>
#include "NuMicro.h"
#include "SPI_LCD.h"
#include "user_sys.h"



uint8_t	 g_LCDSize;
bool g_LastWriteType;

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
	LCD_Write_Command(0x2f);  	//Control built-in power circuit ON	    VF
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
		if(x>LCD_MAX_COLUMN-1){x=0;if(g_LCDSize==16)y+=2;else y+=1;}
		if(g_LCDSize ==16)
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
		if(x>124){x=0; if(g_LCDSize==16)y+=2;else y+=1;}
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



