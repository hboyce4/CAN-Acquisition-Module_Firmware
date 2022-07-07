/*
 * UI.h
 *
 *  Created on: Jul 4, 2022
 *      Author: Hugo Boyce
 */

#ifndef UI_H_
#define UI_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "analog.h"

#define UI_FRAME_INTERVAL_MS	50	/* interval between UI refreshes. Max 255 */
#define LINE_WIDTH 256

#define UI_MENU_NB_ROWS 4
#define UI_MENU_NB_COLUMNS 2


#define COLOR_DEFAULT "\x1B[97m\x1B[40m" /* white text, black background \x1B[40m */
#define COLOR_SELECTED "\x1B[30m\x1B[105m" /* black text, bright magenta background*/
#define COLOR_NOT_SELECTED "\x1B[95m\x1B[40m" /* bright magenta text, black background*/

#define CURSOR_RIGHT_COLUMN "\x1B[32G"//Advance cursor to column 30 to draw right column


/*---------------------------------------------------------------------------------------------------------*/
/* Type definitions           				                                                               */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum {INFO_PAGE_SYS = 0, INFO_PAGE_CAN = 1, INFO_PAGE_PROCESS = 2, INFO_PAGE_FIELD = 3, INFO_PAGE_NOISE = 4, INFO_PAGE_HUM = 5, INFO_PAGE_CO2 = 6} info_page_state_t;
#define HIGHEST_INFO_PAGE 6 // Highest info page number

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables to be made available externally                                                        */
/*---------------------------------------------------------------------------------------------------------*/
// extern volatile bool g_New_startup_from_user;

// extern volatile uint32_t g_d_ff_zero_state;
/*---------------------------------------------------------------------------------------------------------*/
/* Functions declaration                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

void UI_draw(int8_t, int8_t);


void UI_draw_title(void);

void UI_draw_info_page_sys(void);
void UI_draw_info_page_PV(void);
void UI_draw_info_page_FV(void);
void UI_draw_info_page_noise(void);


void UI_draw_line_separator();

void UI_draw_parameter_line_0(int8_t, int8_t);
void UI_draw_parameter_line_1(int8_t, int8_t);
void UI_draw_parameter_line_2(int8_t, int8_t);
void UI_draw_parameter_line_3(int8_t, int8_t);

void UI_read_user_input(int8_t*, int8_t*);
void UI_increment_value(int8_t, int8_t);
void UI_decrement_value(int8_t, int8_t);

void UI_get_unit_string(physical_unit_t, char*);


//void UI_serialize_code(uint32_t*, uint8_t, bool);
//uint32_t UI_get_faults_code(void);



#endif /* UI_H_ */
