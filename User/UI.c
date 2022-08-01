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
#include "analog.h"
#include "user_sys.h"
#include "CAN_message.h"
#include "persistent_data.h"
#include "errors.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

info_page_state_t gInfoPage = 0; /* Track which page we're on*/
param_page_state_t gParamPage = 0; /* Track which page we're on*/

int8_t g_i8AnalogChannelSel = 0;


int8_t g_i8RowSel, g_i8ColumnSel, g_i8NumRows, g_i8AnalogChannelSel;

/*---------------------------------------------------------------------------------------------------------*/
/* Function definitions                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/


/* XXXus in Debug, XXXus in Release ( -O2 ) */
void UI_draw(void){/* 60-ish characters width*/

	/* Cursor home : \x1B[H */

    /* Current state display */
	UI_draw_title();

	UI_draw_page_select();

	switch(gParamPage){ // Depending on which one is selected

		case PARAM_PAGE_SYS:
			UI_draw_param_page_sys();
			break;

		case PARAM_PAGE_CAN:
			UI_draw_param_page_CAN();
			break;

		case PARAM_PAGE_ANALOG:
			UI_draw_param_page_analog();
			break;

		case PARAM_PAGE_I2C:
			UI_draw_param_page_I2C();
			break;

		default:
			UI_draw_param_page_sys();
			break;
	}


	UI_draw_line_separator();

	/* Draw the info page*/
	switch(gInfoPage){ // Depending on which one is selected

		case INFO_PAGE_SYS:
			UI_draw_info_page_sys();
			break;

		case INFO_PAGE_CAN:
			break;

		case INFO_PAGE_PROCESS:
			UI_draw_info_page_PV();
			break;

		case INFO_PAGE_FIELD:
			UI_draw_info_page_FV();
			break;

		case INFO_PAGE_NOISE:
			UI_draw_info_page_noise();
			break;

		case INFO_PAGE_HUM:
			break;

		case INFO_PAGE_CO2:
			break;

		default:
			UI_draw_info_page_sys();
			break;
	}

	UI_draw_line_separator();

	/* Last line */
	/* Move cursor to position 1;1 command */
	VCOM_PushString("\x1B[1;1H");
	/* \x1B[3J is an escape sequence that clears scrollback */
	VCOM_PushString("\x1B[3J");
	/* End of last line */

}

void UI_read_user_input(void){

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
				(g_i8RowSel)--;/*Move the selection up*/
				if(g_i8RowSel<0){/* If the selection goes past the lowest row*/
					g_i8RowSel = 0;/* Stay at the lowest row, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else if (user_char == 'B'){ /* Down*/
				(g_i8RowSel)++;/*Move the selection down*/
				if(g_i8RowSel >= g_i8NumRows){/* If the selection goes past the highest row*/
					g_i8RowSel = (g_i8NumRows-1);/* Stay at the highest row, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else if (user_char == 'C'){ /* Right */
				(g_i8ColumnSel)++;/*Move the selection right*/
				if(g_i8ColumnSel >= UI_MENU_NB_COLUMNS){/* If the selection goes past the highest column*/
					g_i8ColumnSel = (UI_MENU_NB_COLUMNS-1);/* Stay at the highest column, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else if (user_char == 'D'){ /* Left*/
				(g_i8ColumnSel)--;/*Move the selection left*/
				if(g_i8ColumnSel<0){/* If the selection goes past the lowest column*/
					g_i8ColumnSel = 0;/* Stay at the lowest column, don't wrap around*/
				}
				state = 0;/* Finally, reset the state machine*/

			}else{
				state = 0;/* Else reset the state machine*/
			}

		}

		if(user_char == '+'){/* If a '+' is received, the selected value is incremented*/
			UI_increment_value();
		}else if (user_char == '-'){/* If a '-' is received, the selected value is decremented*/
			UI_decrement_value();
		}
		//printf("\nReceived character '0x%x' \n", user_char); /* for debug */

	}
}

void UI_increment_value(void){


	if(g_i8RowSel == 0){

		if(g_i8ColumnSel == 0){
			if(gInfoPage < UI_HIGHEST_INFO_PAGE){
				gInfoPage++;
			}
		}else if (g_i8ColumnSel == 1){
			if(gParamPage < UI_HIGHEST_PARAM_PAGE){
				gParamPage++;
			}
		}
	}else{

		switch(gParamPage){

			case PARAM_PAGE_SYS:
				switch(g_i8RowSel){
					case 1:
						if(g_i8ColumnSel == 0){
							PD_SaveConfig();
						}else if (g_i8ColumnSel == 1){
							/*inverter_setpoints.V_DC_diff_setpoint += FLOAT_INCREMENT;*/
							PD_SaveConfig();
						}
						break;

				}
				break;


			case PARAM_PAGE_CAN:

				switch(g_i8RowSel){
					case 1:
						g_CANSpeed *= 2;
						break;
					case 2:
						g_CANNodeID++;
						if(g_CANNodeID > CAN_NODE_ID_MAX){
							g_CANNodeID = CAN_NODE_ID_MAX;
						}
				}
				break;

			case PARAM_PAGE_ANALOG:
				switch(g_i8RowSel){
					case 1:
						if(g_i8ColumnSel == 0){
							g_i8AnalogChannelSel++;
							if(g_i8AnalogChannelSel > EADC_LAST_GP_CHANNEL){
								g_i8AnalogChannelSel = EADC_LAST_GP_CHANNEL;
							}

						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].sensorType++;
							if(analog_channels[g_i8AnalogChannelSel].sensorType > ANALOG_SENSOR_THERMOCOUPLE){/* If greater than last type */
								analog_channels[g_i8AnalogChannelSel].sensorType = ANALOG_SENSOR_THERMOCOUPLE;/* Don't go any further, stay at last type. */
							}
						}
						break;
					case 2:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].isEnabled = true;
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].fieldUnit++;
							if(analog_channels[g_i8AnalogChannelSel].fieldUnit > UNIT_PPM){
								analog_channels[g_i8AnalogChannelSel].fieldUnit = UNIT_PPM;
							}
						}
						break;

					case 3:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].processUnit++;
							if(analog_channels[g_i8AnalogChannelSel].processUnit > UNIT_PPM){
								analog_channels[g_i8AnalogChannelSel].processUnit = UNIT_PPM;
							}
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].biasResistor += UI_FLOAT_ADD_INC;
						}
						break;

					case 4:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].NTCRZero += UI_FLOAT_ADD_INC;
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].NTCBeta += UI_FLOAT_ADD_INC;
						}
						break;

					case 5:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].voltageGain *= 1 + UI_FLOAT_MULT_INC;
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].sensorGain *= 1 + UI_FLOAT_MULT_INC;
						}
						break;
				}
				break;

			case PARAM_PAGE_I2C:
				break;

		}

	}

}

void UI_decrement_value(void){

	if(g_i8RowSel == 0){

		if(g_i8ColumnSel == 0){
			if(gInfoPage > 0){
				gInfoPage--;
			}
		}else if (g_i8ColumnSel == 1){
			if(gParamPage > 0){
				gParamPage--;
			}
		}
	}else{

		switch(gParamPage){

			case PARAM_PAGE_SYS:
				switch(g_i8RowSel){
					case 1:
						if(g_i8ColumnSel == 0){
							PD_SaveConfig();
						}else if (g_i8ColumnSel == 1){
							/*inverter_setpoints.V_DC_diff_setpoint += FLOAT_INCREMENT;*/
							PD_SaveConfig();
						}
						break;

				}
				break;


			case PARAM_PAGE_CAN:

				switch(g_i8RowSel){
					case 1:
						g_CANSpeed /= 2;
						break;
					case 2:
						if(g_CANNodeID > CAN_NODE_ID_MIN){
							g_CANNodeID--;
						}
				}
				break;

			case PARAM_PAGE_ANALOG:
				switch(g_i8RowSel){
					case 1:
						if(g_i8ColumnSel == 0){
							if(g_i8AnalogChannelSel > 0){
								g_i8AnalogChannelSel--;
							}
						}else if (g_i8ColumnSel == 1){
							if(analog_channels[g_i8AnalogChannelSel].sensorType > 0){/* If greater than last type */
								analog_channels[g_i8AnalogChannelSel].sensorType--;/* Don't go any further, stay at last type. */
							}
						}
						break;
					case 2:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].isEnabled = false;
						}else if (g_i8ColumnSel == 1){
							if(analog_channels[g_i8AnalogChannelSel].fieldUnit > 0){
								analog_channels[g_i8AnalogChannelSel].fieldUnit--;
							}
						}
						break;

					case 3:
						if(g_i8ColumnSel == 0){
							if(analog_channels[g_i8AnalogChannelSel].processUnit > 0){
								analog_channels[g_i8AnalogChannelSel].processUnit--;
							}
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].biasResistor -= UI_FLOAT_ADD_INC;
						}
						break;

					case 4:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].NTCRZero -= UI_FLOAT_ADD_INC;
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].NTCBeta -= UI_FLOAT_ADD_INC;
						}
						break;

					case 5:
						if(g_i8ColumnSel == 0){
							analog_channels[g_i8AnalogChannelSel].voltageGain *= 1 - UI_FLOAT_MULT_INC;
						}else if (g_i8ColumnSel == 1){
							analog_channels[g_i8AnalogChannelSel].sensorGain *= 1 - UI_FLOAT_MULT_INC;
						}
						break;
				}
				break;

			case PARAM_PAGE_I2C:
				break;

		}

	}


}

void UI_draw_title(void){


	/* \x1B[3J is an escape sequence that clears the screen */
	VCOM_PushString("\x1B[0J************* CAN Acquisition Module *************\n\r");
	VCOM_PushString("Use arrows to select, + and - to edit.\n\r");

}

void UI_draw_page_select(void){

	/*********************** First Line ***********************************/
	char line_str[LINE_WIDTH];

	sprintf(line_str,"Info page: ");
	/* Column 0 */
	if(g_i8RowSel == 0 && g_i8ColumnSel == 0){
		strcat(line_str, COLOR_SELECTED);
	}else{
		strcat(line_str, COLOR_NOT_SELECTED);
	}

	switch(gInfoPage){

		case INFO_PAGE_SYS:
			strcat(line_str, "System");
			break;

		case INFO_PAGE_CAN:
			strcat(line_str, "CANbus");
			break;

		case INFO_PAGE_PROCESS:
			strcat(line_str, "Process values");
			break;

		case INFO_PAGE_FIELD:
			strcat(line_str, "Field values unf.");
			break;

		case INFO_PAGE_NOISE:
			strcat(line_str, "RMS Noise");
			break;


		case INFO_PAGE_HUM:
			strcat(line_str, "Humidity");
			break;

		case INFO_PAGE_CO2:
			strcat(line_str, "CO2");
			break;
	}

	strcat(line_str, COLOR_DEFAULT);
	strcat(line_str, CURSOR_RIGHT_COLUMN);
	strcat(line_str, "Param. page: ");

	/* Column 1 */
	if(g_i8RowSel == 0 && g_i8ColumnSel == 1){
		strcat(line_str, COLOR_SELECTED);
	}else{
		strcat(line_str, COLOR_NOT_SELECTED);
	}

	switch(gParamPage){

		case PARAM_PAGE_SYS:
			strcat(line_str, "System");
			break;

		case PARAM_PAGE_CAN:
			strcat(line_str, "CANbus");
			break;

		case PARAM_PAGE_ANALOG:
			strcat(line_str, "Analog Channels");
			break;

		case PARAM_PAGE_I2C:
			strcat(line_str, "I2C Environmental Sensor");
			break;

	}

	strcat(line_str, COLOR_DEFAULT);
	strcat(line_str, " \n\r");

	VCOM_PushString((char*) line_str);

}


void UI_draw_param_page_sys(void){


	/*********************** Second Line ***********************************/
	char line_str[LINE_WIDTH];
	char color_str[SHORT_STRING_LENGTH];

	/* Column 0 */
	if(g_i8RowSel == 1){
		strcpy(color_str, COLOR_SELECTED);
	}else{
		strcpy(color_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_str,"Config save: %sPress + or - to save%s\n\r",color_str,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	UI_set_num_rows(2);/* 2 rows total including page select */

}


void UI_draw_param_page_CAN(void){


	/*********************** Second Line ***********************************/

	char line_str[LINE_WIDTH];
	char color_str[SHORT_STRING_LENGTH];


	/* Column 0 */
	if(g_i8RowSel == 1){
		strcpy(color_str, COLOR_SELECTED);
	}else{
		strcpy(color_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_str,"CAN Speed: %s%i bps%s\n\r",color_str, g_CANSpeed,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	/*********************** Third Line ***********************************/
	/* Column 1 */
	if(g_i8RowSel == 2){
		strcpy(color_str, COLOR_SELECTED);
	}else{
		strcpy(color_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_str,"CAN Node ID: %s%i%s\n\r",color_str, g_CANNodeID,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	UI_set_num_rows(3);/* 3 rows total including page select */

}
void UI_draw_param_page_analog(void){


	/*********************** Second Line ***********************************/
	char line_str[LINE_WIDTH];
	char color_left_str[SHORT_STRING_LENGTH];
	char color_right_str[SHORT_STRING_LENGTH];
	char temp_l_str[SHORT_STRING_LENGTH];
	char temp_r_str[SHORT_STRING_LENGTH];

	/* Column 0 */
	if(g_i8RowSel == 1 && g_i8ColumnSel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(g_i8RowSel == 1 && g_i8ColumnSel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	UI_get_analog_sensor_string(analog_channels[g_i8AnalogChannelSel].sensorType, temp_l_str);

	sprintf(line_str,"Analog Channel: %s%i%s %sSensor type: %s%s%s\n\r",color_left_str,g_i8AnalogChannelSel,COLOR_DEFAULT,
			CURSOR_RIGHT_COLUMN, color_right_str,temp_l_str,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	/*********************** Third Line ***********************************/

	/* Column 0 */
	if(g_i8RowSel == 2 && g_i8ColumnSel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(g_i8RowSel == 2 && g_i8ColumnSel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	if(analog_channels[g_i8AnalogChannelSel].isEnabled){
		strcpy(temp_l_str, "Yes");
	}else{
		strcpy(temp_l_str, "No");
	}

	sys_get_unit_string(analog_channels[g_i8AnalogChannelSel].fieldUnit,temp_r_str);

	sprintf(line_str,"Enabled: %s%s%s %sField unit: %s%s%s\n\r",color_left_str,temp_l_str,COLOR_DEFAULT,
	CURSOR_RIGHT_COLUMN, color_right_str, temp_r_str,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	/*********************** Fourth Line ***********************************/

	/* Column 0 */
	if(g_i8RowSel == 3 && g_i8ColumnSel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(g_i8RowSel == 3 && g_i8ColumnSel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	sys_get_unit_string(analog_channels[g_i8AnalogChannelSel].processUnit,temp_l_str);

	sprintf(line_str,"Process unit: %s%s%s %sBias resistor: %s%f Ohm%s\n\r",color_left_str,temp_l_str,COLOR_DEFAULT,
			CURSOR_RIGHT_COLUMN, color_right_str, analog_channels[g_i8AnalogChannelSel].biasResistor,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	/*********************** Fifth Line ***********************************/

	/* Column 0 */
	if(g_i8RowSel == 4 && g_i8ColumnSel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(g_i8RowSel == 4 && g_i8ColumnSel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_str,"R @ 25 degC: %s%f Ohm%s %sBeta coef.: %s%f K%s\n\r",color_left_str, analog_channels[g_i8AnalogChannelSel].NTCRZero, COLOR_DEFAULT,
			CURSOR_RIGHT_COLUMN, color_right_str, analog_channels[g_i8AnalogChannelSel].NTCBeta,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	/*********************** Sixth Line ***********************************/
	/* Column 0 */
	if(g_i8RowSel == 5 && g_i8ColumnSel == 0){
		strcpy(color_left_str, COLOR_SELECTED);
	}else{
		strcpy(color_left_str, COLOR_NOT_SELECTED);
	}

	/* Column 1 */
	if(g_i8RowSel == 5 && g_i8ColumnSel == 1){
		strcpy(color_right_str, COLOR_SELECTED);
	}else{
		strcpy(color_right_str, COLOR_NOT_SELECTED);
	}

	sprintf(line_str,"Voltage gain: %s%f%s %sSensor gain: %s%f%s\n\r",color_left_str, analog_channels[g_i8AnalogChannelSel].voltageGain, COLOR_DEFAULT,
			CURSOR_RIGHT_COLUMN, color_right_str, analog_channels[g_i8AnalogChannelSel].sensorGain,COLOR_DEFAULT);

	VCOM_PushString((char*) line_str);

	UI_set_num_rows(6);/* 6 rows total including page select */

}
void UI_draw_param_page_I2C(void){

	UI_set_num_rows(1);/* 1 rows total including page select */

}

void UI_draw_info_page_sys(void){

	/***************************************** First Line ********************************************/
	char line_str[LINE_WIDTH];

	uint32_t uptime = (uint32_t)(gu64SysTickIntCnt/1000); /* Uptime in seconds. Overflows after 127 years */

	sprintf(line_str, "Max. VCOM Tx Buf.: %i/%i %sUptime: %u s\n\r", comTbytesMax, TXBUFSIZE, CURSOR_RIGHT_COLUMN, uptime);

	VCOM_PushString((char*) line_str);

	/***************************************** Second Line ********************************************/

	sprintf(line_str, "Max. VCOM Rx Buf.: %i/%i %sADC cal. word: %u\n\r", comRbytesMax, RXBUFSIZE, CURSOR_RIGHT_COLUMN, EADC->CALDWRD);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	uint32_t u32LEDStatus = (PB->PIN & (BIT13|BIT14|BIT15));
	u32LEDStatus >>= 13;
	sprintf(line_str, "LED status: %u %sDebug UART Speed: %i bps\n\r", u32LEDStatus, CURSOR_RIGHT_COLUMN, DEBUG_UART_SPEED);

	VCOM_PushString((char*) line_str);


	/*************************************Environmental sensor type ************************************/
	char I2C_sensor_str[SHORT_STRING_LENGTH];
	UI_get_I2C_sensor_string(env_sensor.sensorType, I2C_sensor_str);
	sprintf(line_str, "I2C sensor: %s\n\r", I2C_sensor_str);
	VCOM_PushString((char*) line_str);


	/******************************** Unique ID *********************************************************/
	sprintf(line_str, "Unique ID: %08X %08X %08X\n\r", g_u32UID[2], g_u32UID[1], g_u32UID[0]);
	VCOM_PushString((char*) line_str);

	/******************************** Error code *********************************************************/
	sprintf(line_str, "Error code: %08X (zero is no error)\n\r", Error_GetCode());
	VCOM_PushString((char*) line_str);

	/******************************** Version ************************************************************/
	sprintf(line_str, "Version: %s %s\n\r", __DATE__, __TIME__); /* current date and time when the preprocessor is run, using standard predefined macros */
	VCOM_PushString((char*) line_str);

}
void UI_draw_info_page_PV(void){ /* Process values */



	char line_str[LINE_WIDTH];
	char left_unit_str[SHORT_STRING_LENGTH];
	char right_unit_str[SHORT_STRING_LENGTH];

	VCOM_PushString("			ANALOG\n\r");

	/***************************************** First Line ********************************************/
	sys_get_unit_string(analog_channels[0].processUnit, left_unit_str);
	sys_get_unit_string(analog_channels[1].processUnit, right_unit_str);
	sprintf(line_str, "CH0: %f %s%sCH1: %f %s\n\r", analog_channels[0].processValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[1].processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Second Line ********************************************/
	sys_get_unit_string(analog_channels[2].processUnit, left_unit_str);
	sys_get_unit_string(analog_channels[3].processUnit, right_unit_str);
	sprintf(line_str, "CH2: %f %s%sCH3: %f %s\n\r", analog_channels[2].processValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[3].processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	sys_get_unit_string(analog_channels[4].processUnit, left_unit_str);
	sys_get_unit_string(analog_channels[5].processUnit, right_unit_str);
	sprintf(line_str, "CH4: %f %s%sCH5: %f %s\n\r", analog_channels[4].processValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[5].processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Fourth Line ********************************************/
	sys_get_unit_string(analog_channels[6].processUnit, left_unit_str);
	sys_get_unit_string(analog_channels[7].processUnit, right_unit_str);
	sprintf(line_str, "CH6: %f %s%sCH7: %f %s\n\r", analog_channels[6].processValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[7].processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Fifth Line ********************************************/
	sys_get_unit_string(analog_channels[8].processUnit, left_unit_str);
	sys_get_unit_string(analog_channels[9].processUnit, right_unit_str);
	sprintf(line_str, "CH8: %f %s%sCH8: %f %s\n\r", analog_channels[8].processValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[9].processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Sixth Line ********************************************/
	sys_get_unit_string(analog_channels[10].processUnit, left_unit_str);
	sys_get_unit_string(analog_channels[11].processUnit, right_unit_str);
	sprintf(line_str, "CH10: %f %s%sCH11: %f %s\n\r", analog_channels[10].processValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[11].processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	VCOM_PushString(ENVIRONMENTAL_SENSOR_HEADER);

	/***************************************** seventh Line ********************************************/
	sys_get_unit_string(env_sensor.temperature_processUnit, left_unit_str);
	sys_get_unit_string(env_sensor.humidity_processUnit, right_unit_str);
	sprintf(line_str, "Temperature: %f %s%sHumidity: %f%s\n\r", env_sensor.temperature_processValue, left_unit_str, CURSOR_RIGHT_COLUMN, env_sensor.humidity_processValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Eight Line ********************************************/
	sys_get_unit_string(env_sensor.CO2_processUnit, left_unit_str);
	//sys_get_unit_string(analog_channels[11].processUnit, right_unit_str);
	sprintf(line_str, "CO2: %f %s\n\r", env_sensor.CO2_processValue, left_unit_str);

	VCOM_PushString((char*) line_str);


}
void UI_draw_info_page_FV(void){ /* Field values */


	char line_str[LINE_WIDTH];
	char left_unit_str[SHORT_STRING_LENGTH];
	char right_unit_str[SHORT_STRING_LENGTH];

	VCOM_PushString(ANALOG_HEADER);

	/***************************************** First Line ********************************************/
	sys_get_unit_string(analog_channels[0].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[1].fieldUnit, right_unit_str);
	sprintf(line_str, "CH0: %f %s%sCH1: %f %s\n\r", analog_channels[0].fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[1].fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Second Line ********************************************/
	sys_get_unit_string(analog_channels[2].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[3].fieldUnit, right_unit_str);
	sprintf(line_str, "CH2: %f %s%sCH3: %f %s\n\r", analog_channels[2].fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[3].fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	sys_get_unit_string(analog_channels[4].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[5].fieldUnit, right_unit_str);
	sprintf(line_str, "CH4: %f %s%sCH5: %f %s\n\r", analog_channels[4].fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[5].fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Fourth Line ********************************************/
	sys_get_unit_string(analog_channels[6].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[7].fieldUnit, right_unit_str);
	sprintf(line_str, "CH6: %f %s%sCH7: %f %s\n\r", analog_channels[6].fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[7].fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	sys_get_unit_string(analog_channels[8].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[9].fieldUnit, right_unit_str);
	sprintf(line_str, "CH8: %f %s%sCH9: %f %s\n\r", analog_channels[8].fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[9].fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Fourth Line ********************************************/
	sys_get_unit_string(analog_channels[10].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[11].fieldUnit, right_unit_str);
	sprintf(line_str, "CH10: %f %s%sCH11: %f %s\n\r", analog_channels[10].fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[11].fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	VCOM_PushString(ENVIRONMENTAL_SENSOR_HEADER);

	/***************************************** seventh Line ********************************************/
	sys_get_unit_string(env_sensor.temperature_fieldUnit, left_unit_str);
	sys_get_unit_string(env_sensor.humidity_fieldUnit, right_unit_str);
	sprintf(line_str, "Temperature: %u %s%sHumidity: %u %s\n\r", env_sensor.temperature_fieldValue, left_unit_str, CURSOR_RIGHT_COLUMN, env_sensor.humidity_fieldValue, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Eight Line ********************************************/
	sys_get_unit_string(env_sensor.CO2_fieldUnit, left_unit_str);
	//sys_get_unit_string(analog_channels[11].processUnit, right_unit_str);
	sprintf(line_str, "CO2: %u %s\n\r", env_sensor.CO2_fieldValue, left_unit_str);

	VCOM_PushString((char*) line_str);

}
void UI_draw_info_page_noise(void){ /* Field values */


	char line_str[LINE_WIDTH];
	char left_unit_str[SHORT_STRING_LENGTH];
	char right_unit_str[SHORT_STRING_LENGTH];

	VCOM_PushString(ANALOG_HEADER);

	/***************************************** First Line ********************************************/
	sys_get_unit_string(analog_channels[0].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[1].fieldUnit, right_unit_str);
	sprintf(line_str, "CH0: %f %s%sCH1: %f %s\n\r", analog_channels[0].RMSNoise, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[1].RMSNoise, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Second Line ********************************************/
	sys_get_unit_string(analog_channels[2].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[3].fieldUnit, right_unit_str);
	sprintf(line_str, "CH2: %f %s%sCH3: %f %s\n\r", analog_channels[2].RMSNoise, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[3].RMSNoise, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	sys_get_unit_string(analog_channels[4].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[5].fieldUnit, right_unit_str);
	sprintf(line_str, "CH4: %f %s%sCH5: %f %s\n\r", analog_channels[4].RMSNoise, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[5].RMSNoise, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Fourth Line ********************************************/
	sys_get_unit_string(analog_channels[6].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[7].fieldUnit, right_unit_str);
	sprintf(line_str, "CH6: %f %s%sCH7: %f %s\n\r", analog_channels[6].RMSNoise, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[7].RMSNoise, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Third Line ********************************************/
	sys_get_unit_string(analog_channels[8].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[9].fieldUnit, right_unit_str);
	sprintf(line_str, "CH8: %f %s%sCH9: %f %s\n\r", analog_channels[8].RMSNoise, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[9].RMSNoise, right_unit_str);

	VCOM_PushString((char*) line_str);

	/***************************************** Fourth Line ********************************************/
	sys_get_unit_string(analog_channels[10].fieldUnit, left_unit_str);
	sys_get_unit_string(analog_channels[11].fieldUnit, right_unit_str);
	sprintf(line_str, "CH10: %f %s%sCH11: %f %s\n\r", analog_channels[10].RMSNoise, left_unit_str, CURSOR_RIGHT_COLUMN, analog_channels[11].RMSNoise, right_unit_str);

	VCOM_PushString((char*) line_str);

}

void UI_draw_line_separator(void){

	VCOM_PushString("**************************************************\n\r");
}


void UI_get_I2C_sensor_string(I2C_sensor_t type, char* string){

	switch(type){

		case I2C_SENSOR_NONE:
			strcpy(string, "None");
			break;

		case I2C_SENSOR_SHT3x:
			strcpy(string, "SHT3x");
			break;

		case I2C_SENSOR_SCD30:
			strcpy(string, "SCD30");
			break;

		default: /*If the unit is not known */
			strcpy(string, "Unknown");
			break;

	}

}

void UI_get_analog_sensor_string(analog_sensor_t type, char* string){

	switch(type){

		case ANALOG_SENSOR_NONE:
			strcpy(string, "None");
			break;

		case ANALOG_SENSOR_NTC:
			strcpy(string, "NTC Thermistor");
			break;

		case ANALOG_SENSOR_PYRANOMETER:
			strcpy(string, "Pyranometer");
			break;

		case ANALOG_SENSOR_THERMOCOUPLE:
			strcpy(string, "Thermocouple");
			break;

		default: /*If the unit is not known */
			strcpy(string, "Unknown");
			break;

	}

}

void UI_set_num_rows(uint8_t nb_rows){

	g_i8NumRows = nb_rows;
	if(g_i8RowSel >= nb_rows){
		g_i8RowSel = nb_rows - 1;
	}

}
