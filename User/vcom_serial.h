/**************************************************************************//**
 * @file     cdc_serial.h
 * @brief    M480 series USB driver header file
 * @version  V1.00
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __USBD_CDC_H__
#define __USBD_CDC_H__

/* Define the vendor id and product id */
#define USBD_VID        0x0416
#define USBD_PID        0x5011

/*!<Define CDC Class Specific Request */
#define SET_LINE_CODE           0x20
#define GET_LINE_CODE           0x21
#define SET_CONTROL_LINE_STATE  0x22

/*-------------------------------------------------------------*/
/* Define EP maximum packet size */
#define EP0_MAX_PKT_SIZE    64
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
#define EP2_MAX_PKT_SIZE    64
#define EP3_MAX_PKT_SIZE    64
#define EP4_MAX_PKT_SIZE    8

#define SETUP_BUF_BASE      0
#define SETUP_BUF_LEN       8
#define EP0_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN         EP0_MAX_PKT_SIZE
#define EP1_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN         EP1_MAX_PKT_SIZE
#define EP2_BUF_BASE        (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN         EP2_MAX_PKT_SIZE
#define EP3_BUF_BASE        (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN         EP3_MAX_PKT_SIZE
#define EP4_BUF_BASE        (EP3_BUF_BASE + EP3_BUF_LEN)
#define EP4_BUF_LEN         EP4_MAX_PKT_SIZE

/* Define the LOGICAL endpoint numbers (as seen on the bus). Do not confuse them with the HARDWARE endpoint numbers, which are NOT seen on the bus and are internal to the M480 */
#define BULK_IN_EP_NUM      0x01
#define BULK_OUT_EP_NUM     0x02
#define INT_IN_EP_NUM       0x03


/* Define Descriptor information */
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  50  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

#define RXBUFSIZE           2048 /* RX buffer size. (Bytes received by the USB driver and transmitted to the host) Max 64k. */
#define TXBUFSIZE           32 /* TX buffer size. (Bytes received from the host and transmitted by the USB driver) Max 64k. */

/* Define the timeout of PC terminal */
#define PC_TERMINAL_TIMEOUT_MS 2 /* Number of milliseconds after which we consider the terminal has been closed if we stop receiving IN packets. Max. 255*/


/************************************************/
/* for CDC class */
/* Line coding structure
  0-3 dwDTERate    Data terminal rate (baudrate), in bits per second
  4   bCharFormat  Stop bits: 0 - 1 Stop bit, 1 - 1.5 Stop bits, 2 - 2 Stop bits
  5   bParityType  Parity:    0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
  6   bDataBits    Data bits: 5, 6, 7, 8, 16  */

typedef struct
{
    uint32_t  u32DTERate;     /* Baud rate    */
    uint8_t   u8CharFormat;   /* stop bit     */
    uint8_t   u8ParityType;   /* parity       */
    uint8_t   u8DataBits;     /* data bits    */
} STR_VCOM_LINE_CODING;

/*-------------------------------------------------------------*/

extern volatile uint16_t comRbytesMax;
extern volatile uint16_t comTbytesMax;


/*-------------------------------------------------------------*/
void VCOM_Init(void);
void VCOM_ClassRequest(void);/* Routine that answers the USB Class Request */

void EP2_Handler(void);
void EP3_Handler(void);
void VCOM_TransferData(void);
int8_t VCOM_PushToBuf(uint8_t bInChar); // Send to USB host (a.k.a PC)
int8_t VCOM_PopFromBuf(uint8_t* bInChar); // Received from USB host (a.k.a. PC)

void VCOM_PushString(char* string); // Send string to USB host (a.k.a PC)
#endif  /* __USBD_CDC_H_ */

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
