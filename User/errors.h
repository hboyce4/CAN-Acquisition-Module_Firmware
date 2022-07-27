/*
 * errors.h
 *
 *  Created on: Jul 26, 2022
 *      Author: Hugo Boyce
 */

#ifndef ERRORS_H_
#define ERRORS_H_

#include <stdbool.h>
#include <stdint.h>

#define ERROR_CORRUPTED_CONFIG 		0x00000001 /* Module configuration is has CRC32 mismatch */
#define ERROR_SOME_ERROR			0x00000002 /* TBD */
#define ERROR_SOME_OTHER_ERROR		0x00000004 /* TBD */
#define ERROR_INVALID_CONFIG		0x00000008 /* Inappropriate config settings */

uint32_t Error_GetCode(void);
void Error_Set(uint32_t);
bool Error_Get(uint32_t);
void Error_Clear(uint32_t);
void Error_ClearCode(void);

#endif /* ERRORS_H_ */
