/*
 * LCD.h
 *
 *  Created on: Jul 20, 2022
 *      Author: Hugo Boyce
 */

#ifndef SPI_LCD_H_
#define SPI_LCD_H_

#include <stdint.h>

#define SPI_SPEED 100000

//LCD command/data pin
#define LCD_CD_PIN PD1
#define LCD_COMMAND false
#define LCD_DATA true

//Resolution
#define LCD_MAX_COLUMN	132
#define LCD_MAX_ROW		32

#define LCD_CONTRAST 27

extern uint8_t g_LCDSize;

void SPI_Init(void);

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

#endif /* SPI_LCD_H_ */
