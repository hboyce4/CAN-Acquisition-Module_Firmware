/*
 * UI.c
 *
 *  Created on: Jul 4, 2022
 *      Author: Hugo Boyce
 */

#include "UI.h"

#include <stdlib.h>
#include "NuMicro.h"
#include "vcom_serial.h"
#include "interrupt.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

info_page_state_t gInfoPage = 0; /* Track which page we're on*/


int8_t gi8Parameter0_0 = 0;
int8_t gi8Parameter0_1 = 0;
int8_t gi8Parameter1_0 = 0;
int8_t gi8Parameter1_1 = 0;
int8_t gi8Parameter2_0 = 0;
int8_t gi8Parameter2_1 = 0;
int8_t gi8Parameter3_1 = 0;


/* Dummy placeholder variables for future expansion */

int8_t gi8Variable4 = 0;
float gfVariable6 = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Function definitions                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/


/* XXXus in Debug, XXXus in Release ( -O2 ) */
void draw_UI(int8_t row_sel, int8_t col_sel){/* 60-ish characters width*/

	/* Cursor home : \x1B[H */

    /* Current state display */
	draw_UI_title();

	/* Menu input*/
	draw_UI_parameter_line_0(row_sel, col_sel);
	draw_UI_parameter_line_1(row_sel, col_sel);
	draw_UI_parameter_line_2(row_sel, col_sel);
	draw_UI_parameter_line_3(row_sel, col_sel);

	draw_UI_line_separator();

	/* Draw the info page*/
	switch(gInfoPage){ // Depending on which one is selected

		case INFO_PAGE_SYS:
			draw_UI_info_page_sys();
			break;

		case INFO_PAGE_CAN:
			break;

		case INFO_PAGE_TEMP:
			break;

		case INFO_PAGE_HUM:
			break;

		case INFO_PAGE_CO2:
			break;

		default:
			draw_UI_info_page_sys();
			break;
	}

	draw_UI_line_separator();

	/* Last line */
	/* Move cursor to position 1;1 command */
	VCOM_PushString("\x1B[1;1H");
	/* End of last line */

}

void read_user_input(int8_t* p_row_sel, int8_t* p_col_sel){

  	uint8_t user_char;
  	static uint8_t state = 0;

	if(VCOM_PopFromBuf(&user_char) == EXIT_SUCCESS){/* If there is data to be read*/

		if (state == 0){/* If no characters have been received*/
			if(user_char == 0x1B){/* If the first character is escape*/
				state++;/* Go to the next state*/
			}

		} else if (state == 1){/* If the first character of an escape sequence (ESC) has been received*/

			if(user_char == '['){/* If the next received character is the 2nd character of an escape sequence*/
				state++;/* Go to the next state*/
			}else{
				state = 0;/* Else reset the state machine*/
			}
		} else if (state == 2){

			if(user_char == 'A'){/* Up */
				(*p_row_sel)--;/*Move the selection up*/
				if(*p_row_sel<0){/* If the selection goes past the highest row*/
					*p_row_sel = 0;/* Stay at the highest row, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else if (user_char == 'B'){ /* Down*/
				(*p_row_sel)++;/*Move the selection down*/
				if(*p_row_sel >= UI_MENU_NB_ROWS){/* If the selection goes past the lowest row*/
					*p_row_sel = (UI_MENU_NB_ROWS-1);/* Stay at the lowest row, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else if (user_char == 'C'){ /* Right */
				(*p_col_sel)++;/*Move the selection right*/
				if(*p_col_sel >= UI_MENU_NB_COLUMNS){/* If the selection goes past the lowest row*/
					*p_col_sel = (UI_MENU_NB_COLUMNS-1);/* Stay at the lowest row, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else if (user_char == 'D'){ /* Left*/
				(*p_col_sel)--;/*Move the selection left*/
				if(*p_col_sel<0){/* If the selection goes past the highest row*/
					*p_col_sel = 0;/* Stay at the highest row, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else{
				state = 0;/* Else reset the state machine*/
			}

		}

		if(user_char == '+'){/* If a '+' is received, the selected value is incremented*/
			increment_UI_value(*p_row_sel,*p_col_sel);
		}else if (user_char == '-'){/* If a '-' is received, the selected value is decremented*/
			decrement_UI_value(*p_row_sel,*p_col_sel);
		}
		//printf("\nReceived character '0x%x' \n", user_char); /* for debug */

	}
}

void increment_UI_value(int8_t row_sel, int8_t col_sel){

	switch(row_sel){

		case 0:
			if(col_sel == 0){
				gi8Parameter0_0++;
			}else if (col_sel == 1){
				gi8Parameter0_1++;
			}
			break;


		case 1:
			if(col_sel == 0){

				gi8Parameter1_0++;
			}else if (col_sel == 1){
				/*inverter_setpoints.V_DC_diff_setpoint += FLOAT_INCREMENT;*/
				gi8Parameter1_1++;
			}
			break;


		case 2:
			if(col_sel == 0){
				gi8Parameter2_0++;
			}else if (col_sel == 1){
				gi8Parameter2_1++;
			}
			break;


		case 3:

			if(col_sel == 0){
				if(gInfoPage < HIGHEST_INFO_PAGE){
					gInfoPage++;
				}
			}else if (col_sel == 1){
				gi8Parameter3_1++;
			}
			break;

	}

}

void decrement_UI_value(int8_t row_sel, int8_t col_sel){

	switch(row_sel){

		case 0:
			if(col_sel == 0){
				gi8Parameter0_0--;
			}else if (col_sel == 1){
				gi8Parameter0_1--;
			}
			break;
		case 1:
			if(col_sel == 0){
				/*inverter_setpoints.precharge_threshold = FLOAT_INCREMENT;*/
				gi8Parameter1_0--;
			}else if (col_sel == 1){
				/*inverter_setpoints.V_DC_diff_setpoint -= FLOAT_INCREMENT;*/
				gi8Parameter1_1--;
			}
			break;

		case 2:
			if(col_sel == 0){
				gi8Parameter2_0--;
			}else if (col_sel == 1){
				gi8Parameter2_1--;
			}
			break;

		case 3:
			if(col_sel == 0){
				if(gInfoPage > 0){
					gInfoPage--;
				}
			}else if (col_sel == 1){
				gi8Parameter3_1--;
			}
			break;

	}
}


void draw_UI_title() {

	/* \x1B[3J is an escape sequence that clears scrollback */
	/* \x1B[3J is an escape sequence that clears the screen */
	VCOM_PushString("\x1B[3J\x1B[0J************* CAN Acquisition Module *************\n\r");

}


void draw_UI_info_page_sys(){

	/***************************************** First Line ********************************************/
	char line_str[LINE_WIDTH];

	uint32_t uptime = (uint32_t)(gu32SysTickIntCnt/1000); /* Uptime in seconds. Overflows after 127 years */

	sprintf(line_str, "Max. VCOM Tx Buf.: %i/%i %sUptime: %i s\n\r", comTbytesMax, TXBUFSIZE, CURSOR_RIGHT_COLUMN, uptime);

	VCOM_PushString((char*) line_str);

	/***************************************** Second Line ********************************************/

	sprintf(line_str, "Max. VCOM Rx Buf.: %i/%i %sVariable 4: %i\n\r", comRbytesMax, RXBUFSIZE, CURSOR_RIGHT_COLUMN, gi8Variable4);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	uint32_t u32LEDStatus = (PB->DOUT & (BIT13|BIT14|BIT15));
	u32LEDStatus >>= 13;
	sprintf(line_str, "LED status: %u %sVariable 6: %2.2f V\n\r", u32LEDStatus, CURSOR_RIGHT_COLUMN, gfVariable6);

	VCOM_PushString((char*) line_str);

}



void draw_UI_line_separator() {

	VCOM_PushString("**************************************************\n\r");
}

void draw_UI_parameter_line_0(int8_t row_sel, int8_t col_sel) {

	char line_A_str[LINE_WIDTH];
	char color_left_str[ESCAPE_SEQUENCE_LENGTH];
	char color_right_str[ESCAPE_SEQUENCE_LENGTH];

	/* Column 0 */
	if(row_sel == 0 && col_sel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(row_sel == 0 && col_sel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_A_str,"Parameter 1: %s%i%s %sParameter 2: %s%i V%s\n\r",color_left_str,gi8Parameter0_0,COLOR_DEFAULT,
			CURSOR_RIGHT_COLUMN, color_right_str,gi8Parameter0_1,COLOR_DEFAULT);

	VCOM_PushString((char*) line_A_str);

}

void draw_UI_parameter_line_1(int8_t row_sel, int8_t col_sel){

	char line_B_str[LINE_WIDTH];
	char color_left_str[ESCAPE_SEQUENCE_LENGTH];
	char color_right_str[ESCAPE_SEQUENCE_LENGTH];

	/* Column 0 */
	if(row_sel == 1 && col_sel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(row_sel == 1 && col_sel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_B_str,"Parameter 3: %s%i%s %sParameter 4: %s%i%s\n\r",color_left_str,gi8Parameter1_0,COLOR_DEFAULT,
	CURSOR_RIGHT_COLUMN, color_right_str,gi8Parameter1_1,COLOR_DEFAULT);

	VCOM_PushString((char*) line_B_str);

}

void draw_UI_parameter_line_2(int8_t row_sel, int8_t col_sel){

	char line_C_str[LINE_WIDTH];
	char color_left_str[ESCAPE_SEQUENCE_LENGTH];
	char color_right_str[ESCAPE_SEQUENCE_LENGTH];

	/* Column 0 */
	if(row_sel == 2 && col_sel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(row_sel == 2 && col_sel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_C_str,"Parameter 5: %s%d%s %sParameter 6: %s%d%s\n\r",color_left_str,gi8Parameter2_0,COLOR_DEFAULT,
			CURSOR_RIGHT_COLUMN, color_right_str,gi8Parameter2_1,COLOR_DEFAULT);

	VCOM_PushString((char*) line_C_str);

}

void draw_UI_parameter_line_3(int8_t row_sel, int8_t col_sel){

	char line_D_str[LINE_WIDTH];

	sprintf(line_D_str,"Info page: ");
	/* Column 0 */
	if(row_sel == 3 && col_sel == 0){
		strcat(line_D_str, COLOR_SELECTED);
	}else{
		strcat(line_D_str, COLOR_NOT_SELECTED);
	}

	switch(gInfoPage){

		case INFO_PAGE_SYS:
			strcat(line_D_str, "System");
			break;

		case INFO_PAGE_CAN:
			strcat(line_D_str, "CANbus");
			break;

		case INFO_PAGE_TEMP:
			strcat(line_D_str, "Temperature");
			break;

		case INFO_PAGE_HUM:
			strcat(line_D_str, "Humidity");
			break;

		case INFO_PAGE_CO2:
			strcat(line_D_str, "CO2");
			break;
	}

	strcat(line_D_str, COLOR_DEFAULT);
	strcat(line_D_str, CURSOR_RIGHT_COLUMN);
	strcat(line_D_str, "Parameter 7: ");

	/* Column 1 */
	if(row_sel == 3 && col_sel == 1){
		strcat(line_D_str, COLOR_SELECTED);
	}else{
		strcat(line_D_str, COLOR_NOT_SELECTED);
	}

	char right_param_value_str[ESCAPE_SEQUENCE_LENGTH];
	sprintf(right_param_value_str, "%d", gi8Parameter3_1);

	strcat(line_D_str, right_param_value_str);
	strcat(line_D_str, COLOR_DEFAULT);
	strcat(line_D_str, " \n\r");

	VCOM_PushString((char*) line_D_str);

}


//void UI_serialize_code(uint32_t* faults_code, uint8_t bit_number, bool flag){
//
//	if(flag){// If the flag is true
//		*faults_code |= (1<<bit_number);// Add a 1
//	}
//
//
//}
//
//uint32_t UI_get_faults_code(void){
//
//	uint32_t faults_code = 0;
//
//	/* Bit 00*/UI_serialize_code(&faults_code, 0, inverter_faults.PLL_sync_fault);
//	/* Bit 01*/UI_serialize_code(&faults_code, 1, inverter_faults.i_sync_fault);
//	/* Bit 02*/UI_serialize_code(&faults_code, 2, inverter_faults.OV_v_AC_fault);
//	/* Bit 03*/UI_serialize_code(&faults_code, 3, inverter_faults.OV_V_DC_plus_fault);
//	/* Bit 04*/UI_serialize_code(&faults_code, 4, inverter_faults.OV_V_DC_minus_fault);
//	/* Bit 05*/UI_serialize_code(&faults_code, 5, inverter_faults.UV_V_DC_plus_fault);
//	/* Bit 06*/UI_serialize_code(&faults_code, 6, inverter_faults.UV_V_DC_minus_fault);
//	/* Bit 07*/UI_serialize_code(&faults_code, 7, inverter_faults.UV2_V_DC_plus_fault);
//	/* Bit 08*/UI_serialize_code(&faults_code, 8, inverter_faults.UV2_V_DC_minus_fault);
//	/* Bit 09*/UI_serialize_code(&faults_code, 9, inverter_faults.OV_V_DC_diff_fault);
//	/* Bit 10*/UI_serialize_code(&faults_code, 10, inverter_faults.precharge_timeout_fault);
//	/* Bit 11*/UI_serialize_code(&faults_code, 11, inverter_faults.charge_timeout_fault);
//	/* Bit 12*/UI_serialize_code(&faults_code, 12, g_Interrupt_real_time_fault);
//
//	return faults_code;
//}


