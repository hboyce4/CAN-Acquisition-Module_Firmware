/*
 * LCD.h
 *
 *  Created on: Jul 20, 2022
 *      Author: Hugo Boyce
 */

#ifndef SPI_LCD_H_
#define SPI_LCD_H_

#include <stdint.h>
#include <stdbool.h>

#define SPI_SPEED 10000000 // 10 MHz (20 MHz max according to ST7567A V. 1.2a )

//LCD command/data pin
#define LCD_CD_PIN PD1
#define LCD_COMMAND false
#define LCD_DATA true

//Resolution
#define LCD_MAX_COLUMN	132
#define LCD_MAX_ROW		32

#define LCD_SIZE_LARGE 16 /* 16 pixels tall */
#define LCD_SIZE_SMALL 8 /* 8 pixels tall */

#define LCD_CONTRAST 27

#define LCD_STR_BUFF_LEN 32 /* Longest string we're ever going to display on the screen */

#define LCD_SECONDS_PER_CHANNEL 3 /* How long a channel gets displayed on the LCD before going to the next */

extern uint8_t	 g_LCDFontSize;

void SPI_Init(void);

/* X (horizontal) position is in pixels (Valid values: 0 to 131).*/
/* Y position (vertical) is in bytes (8 pixels) (Valid values: 0 to 3) */

void LCD_WriteData(uint8_t dat);
void LCD_WriteCommand(uint8_t cmd);
void LCD_Init(void);
void LCD_SetContrastControlRegister(uint8_t mod);
void LCD_SetPageAddress(uint8_t add);
void LCD_SetColumnAddress(uint8_t add);
void LCD_SetPos(uint8_t x, uint8_t y);
void LCD_ShowChar(uint8_t x,uint8_t y,char chr);
void LCD_ShowString(uint8_t x,uint8_t y,char *chr);
void LCD_ClearScreen(void);

void LCD_Draw(void);

bool LCD_CheckIfChannelEnabled(uint8_t);
void LCD_FindNextChannel(void);
void LCD_SetChannelString(uint8_t, char*);

#endif /* SPI_LCD_H_ */
