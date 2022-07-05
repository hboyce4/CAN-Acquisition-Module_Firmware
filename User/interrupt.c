/*
 * interrupt.c
 *
 *  Created on: Jul 4, 2022
 *      Author: Hugo Boyce
 */

#include "interrupt.h"

#include "NuMicro.h"
#include "UI.h"
#include "vcom_serial.h"


/* SysTick */
volatile uint64_t gu32SysTickIntCnt = 0;
volatile bool gbSecondsFlag = false;

/* UI */
volatile bool gbTerminalActive = false;
volatile uint8_t gu8PcTerminalActive_TimeoutCounter = 0;

volatile bool gbDrawNewUIFrame = false;


/* Most important interrupt */
void SysTick_Handler(void)	// Every millisecond (Medium frequency).
{
    gu32SysTickIntCnt++;

    /* USB terminal on host PC timeout*/
    if(gu8PcTerminalActive_TimeoutCounter){/* If the inactivity counter has not elapsed*/
    	gu8PcTerminalActive_TimeoutCounter--;/* Decrement*/
    	gbTerminalActive = true; /* Is means the terminal is active*/
    }else{
    	gbTerminalActive = false; /* Else the terminal is not active*/

    }

    static uint8_t u8NewUIFrameTimer = UI_FRAME_INTERVAL_MS; /* 1000 ms per second*/
	if(!u8NewUIFrameTimer){// If counter has elapsed (equals zero)
		u8NewUIFrameTimer = UI_FRAME_INTERVAL_MS;//Reload counter. 1000 ms per second.
		gbDrawNewUIFrame = true;
	}
	u8NewUIFrameTimer--; // Decrease the counter every time SysTick_Handler is called

    static uint16_t u16SecondsCounter = 1000; /* 1000 ms per second*/
    if(!u16SecondsCounter){// If counter has elapsed (equals zero)
    	u16SecondsCounter = 1000;//Reload counter. 1000 ms per second.
    	gbSecondsFlag = true;
    }
    u16SecondsCounter--; // Decrease the counter every time SysTick_Handler is called

}

void USBD_IRQHandler(void)
{

	uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_FLDET)
    {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if (USBD_IS_ATTACHED())
        {
            /* USB Plug In */
            USBD_ENABLE_USB();
        }
        else
        {
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }

//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_BUS)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if (u32State & USBD_STATE_USBRST)
        {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
            extern volatile uint32_t g_u32OutToggle;
            g_u32OutToggle = 0;
        }
        if (u32State & USBD_STATE_SUSPEND)
        {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
        }
        if (u32State & USBD_STATE_RESUME)
        {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
        }
    }

//------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_WAKEUP)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }

    if (u32IntSts & USBD_INTSTS_USB)
    {
        extern uint8_t g_usbd_SetupPacket[];
        // USB event
        if (u32IntSts & USBD_INTSTS_SETUP)
        {
            // Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

        // EP events
        if (u32IntSts & USBD_INTSTS_EP0)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);

            if(((USBD->EPSTS0 & USBD_EPSTS0_EPSTS0_Msk) >> USBD_EPSTS0_EPSTS0_Pos) != 0b0001){// If the status is not "In NACK", that is, if the interrupt was not the result of a NACK. See page 1826 of Tech. Ref. Man. V3.02
				// control IN
            	USBD_CtrlIn();
			}

        }

        if (u32IntSts & USBD_INTSTS_EP1)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);

            // control OUT
            USBD_CtrlOut();

            // In ACK of SET_LINE_CODE
            if(g_usbd_SetupPacket[1] == SET_LINE_CODE)
            {
                /* This is where the settings of the physical UART would be updated. */
            	/* However there is no physical UART so we do nothing. */
            	//if(g_usbd_SetupPacket[4] == 0)  /* VCOM-1 */
                //    VCOM_LineCoding(0); /* Apply UART settings */
            }
        }

        if (u32IntSts & USBD_INTSTS_EP2)/* Ceci gère le endpoint HARDWARE #2, ce qui équivaut au BULK_IN_EP_NUM (1)*/
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);

            /* This block will be trigerred everytime the host PC requests an IN packet */
            /* The host PC will request an IN packet every 1ms when a terminal is open */
            /* Therefore everytime an IN packet is received we reset the inactivity counter */
            gu8PcTerminalActive_TimeoutCounter = PC_TERMINAL_TIMEOUT_MS;

            if(((USBD->EPSTS0 & USBD_EPSTS0_EPSTS2_Msk) >> USBD_EPSTS0_EPSTS2_Pos) == 0b0000){// If the status is "In ACK", that is, an in packet was successfully transmitted on HW endpoint 2
            	// Bulk IN
            	EP2_Handler();
            }


        }

        if (u32IntSts & USBD_INTSTS_EP3) /* Ceci gère le endpoint HARDWARE #3, ce qui équivaut au BULK_OUT_EP_NUM (2)*/
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // Bulk Out
            EP3_Handler();
        }

        if (u32IntSts & USBD_INTSTS_EP4) /*Ceci gère le endpoint HARDWARE #4, ce qui équivaut au INT_IN_EP_NUM (3) (Inactif)*/
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }

        if (u32IntSts & USBD_INTSTS_EP5)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        if (u32IntSts & USBD_INTSTS_EP6)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }

        if (u32IntSts & USBD_INTSTS_EP7)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }
    }

}
