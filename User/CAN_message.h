/*
 * can_message.h
 *
 *  Created on: Jul 18, 2022
 *      Author: Hugo Boyce
 */

#ifndef CAN_MESSAGE_H_
#define CAN_MESSAGE_H_

/* TODO: Implement a proper CANOpen stack: https://github.com/embedded-office/canopen-stack */
/* This would allow plug-and-play interconnection to other CANOpen devices  */
/* However, for now, a simpler custom protocol will be used. Plus, we get to use extended identifiers. */

/* Custom CAN Protocol used :
 *
 *
 * ------- Node IDs ------------
 *
 * Node IDs 1 to 127 (0x01 to 0x7F) can be used
 *
 * Node ID 0 is reserved for broadcast
 *
 *
 * -------- Communication objects (COBs) ------------
 *
 * 29-bit (extended) IDs and 8 bytes data length are used throughout
 * (Reminder: lower COB-IDs have higher priority)
 *
 * SYNC Frame				// Triggers the generation of CAN measurement messages. From host PC to acquisition modules.
 * 		Extended ID:		0x00000080
 *		Data bytes [0-7]:	Counter increased by one every time the message is sent
 *
 * TIME Frame				// Contains current time, if the nodes ever need to know the current time.
 * 		Extended ID:		0x00000100
 *		Data bytes [0-7]:	TBD
 *
 * PD0s (Process Data Objects)
 *
 * TPDO1					// Contains temperature data from the environmental sensor
 * 		Extended ID:		0x00000180 + NodeID
 *		Data bytes [0-3]:	Field value of temperature from environmental sensor as uint32 in unitless
 *		Data bytes [4-7]:	Process value of temperature from environmental sensor channel as float in Celsius
 *
 *
 * TPDO2					// Contains humidity data from the environmental sensor
 * 		Extended ID:		0x00000280 + NodeID
 *		Data bytes [0-3]:	Field value of humidity from environmental sensor as uint32 in unitless
 *		Data bytes [4-7]:	Process value of humidity from environmental sensor as float in % R.H.
 *
 *
 * TPDO3					// Contains CO2 data from the environmental sensor
 * 		Extended ID:		0x00000380 + NodeID
 *		Data bytes [0-3]:	Padding (to be ignored)
 *		Data bytes [4-7]:	Process value of CO2 from environmental sensor as float in ppm
 *
 *
 * TPDO4					// Contains data from the first Analog channel (CH0)
 * 		Extended ID:		0x00001080 + NodeID
 *		Data bytes [0-3]:	Field value from analog channel 0 as float
 *		Data bytes [4-7]:	Process value from analog channel 0 as float
 *
 *
 * TPDO5					// Contains data from the second Analog channel (CH1)
 * 		Extended ID:		0x00001180 + NodeID
 *		Data bytes [0-3]:	Field value from analog channel 1 as float
 *		Data bytes [4-7]:	Process value from analog channel 1 as float
 *
 *
 * TPDO6					// Contains data from the third Analog channel (CH2)
 * 		Extended ID:		0x00001280 + NodeID
 *		Data bytes [0-3]:	Field value from analog channel 2 as float
 *		Data bytes [4-7]:	Process value from analog channel 2 as float
 *
 *
 * TPDO7 to TPDO11
 * 		// Incremented from TPDO6 (CH3 at ID 0x00001380 to CH7 at ID 0x00001780)
 *
 *
 * TPDO12					// Contains data from the eighth Analog channel (CH8)
 * 		Extended ID:		0x00001880 + NodeID
 *		Data bytes [0-3]:	Field value from analog channel 0 as float
 *		Data bytes [4-7]:	Process value from analog channel 0 as float
 *
 *
 * TPDO13 to TPDO15
 * 		// Incremented from TPDO12 (CH9 at ID 0x00001980 to CH11 at ID 0x00001B80)
 *
 * TPDO16					// Contains data from the eighth Analog channel (CH8)
 * 		Extended ID:		0x00002080 + NodeID
 *		Data bytes [0-3]:	Module error code as uint32
 *		Data bytes [4-7]:	Module uptime as uint32 in seconds
 *
 * So far, no mechanism exists to send or automatically configure the units.
 * They will have to be matched manually on the host PC and the acquisition modules.
 *
 * Implementing the CANOpen stack would solve this problem.
 *
 * */

#define CAN_SPEED_DEFAULT 125000
#define CAN_NODE_ID_DEFAULT 1

#define CAN_NODE_ID_MIN 2 /* Define minimum node ID as 2, so that 1 is usually free.
 	 	 	 	 	 	 	 That way if a node looses calibration and goes to node ID 1, there's no conflict. */
#define CAN_NODE_ID_MAX 127 /* Largest permitted by standard */

#define CAN_TX_SUCCES_LED_ON_MS 100 /* Milliseconds the LED is kept on after CAN TX success */

extern uint32_t g_CANSpeed;
extern uint8_t g_CANNodeID;

void CAN_MsgInterrupt(uint32_t u32IIDR);
void CAN_Init(void);

/* Main CAN service routine, to be executed as frequently as possible */
void CAN_Service(void);


/* number is the object (message) number */
/* id is the message's COB-ID (CAN-ID) */
/* lower is a pointer to the lower 32 bits of data (Bytes 0 to 3) */
/* upper is a pointer to the upper 32 bits of data (Bytes 4 to 7) */
void CAN_BuildAndSendMessage_32(uint8_t number, uint32_t id, void* lower, void* upper);

#endif /* CAN_MESSAGE_H_ */
