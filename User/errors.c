/*
 * errors.c
 *
 *  Created on: Jul 26, 2022
 *      Author: Hugo Boyce
 */

#include "errors.h"


/* g_ErrorCode is zero when there are no errors
 * Each error has one bit assigned to it, through masks defined in the header file.
 *
 *
 * */


uint32_t g_ErrorCode = ERROR_CORRUPTED_CONFIG /*| ERROR_SOME_OTHER_ERROR */; // Put errors that are initialized to 1 here

uint32_t Error_GetCode(void){

	return g_ErrorCode;

}

void Error_Set(uint32_t mask){

	g_ErrorCode |= mask;
}

bool Error_Get(uint32_t mask){

	return (bool)(g_ErrorCode & mask);
}

void Error_Clear(uint32_t mask){

	g_ErrorCode &= ~(mask);

}

void Error_ClearCode(void){

	g_ErrorCode = 0;

}
