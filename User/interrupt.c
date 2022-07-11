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
#include "analog.h"
#include "I2C_sensors.h"


/* SysTick */
volatile uint64_t gu32SysTickIntCnt = 0;
volatile bool gbSecondsFlag = false;

/* UI */
volatile bool gbTerminalActive = false;
volatile uint8_t gu8PcTerminalActive_TimeoutCounter = 0;

volatile bool gbDrawNewUIFrame = false;

volatile bool gbSampleAnalog = false;


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

    static uint16_t u16AnalogSampleCounter = ANALOG_SAMPLE_INTERVAL_MS;
	if(!u16AnalogSampleCounter){// If counter has elapsed (equals zero)
		u16AnalogSampleCounter = ANALOG_SAMPLE_INTERVAL_MS;//Reload counter.
		gbSampleAnalog = true;
	}
	u16AnalogSampleCounter--; // Decrease the counter every time SysTick_Handler is called

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

        if (u32IntSts & USBD_INTSTS_EP2)/* Ceci g�re le endpoint HARDWARE #2, ce qui �quivaut au BULK_IN_EP_NUM (1)*/
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

        if (u32IntSts & USBD_INTSTS_EP3) /* Ceci g�re le endpoint HARDWARE #3, ce qui �quivaut au BULK_OUT_EP_NUM (2)*/
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // Bulk Out
            EP3_Handler();
        }

        if (u32IntSts & USBD_INTSTS_EP4) /*Ceci g�re le endpoint HARDWARE #4, ce qui �quivaut au INT_IN_EP_NUM (3) (Inactif)*/
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

void EADC00_IRQHandler(void){ /* Interrupt for general channels */

	extern volatile uint16_t ADC_acq_buff[EADC_TOTAL_CHANNELS];
	extern volatile uint8_t ADC_acq_count;
	extern volatile bool ADC_acq_done;
	extern volatile bool ADC_first_acq;

    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);      /* Clear the A/D ADINT0 interrupt flag */

    uint8_t channel;

    if(ADC_first_acq){ /* If it's the first series of acquisition after the start*/
    	ADC_first_acq = false;/* Do nothing, the data is bad, we must discard it. */
    }else{/* Else the data is good, store it in the acq buff */

    	for(channel = 0; channel <= EADC_LAST_GP_CHANNEL;channel++){/* For every channel */
			ADC_acq_buff[channel] += (uint16_t)(EADC->DAT[channel]);/* Only keep the lowest 16 bits of the register*/
		}
		ADC_acq_count--;/* Decrease the number of acquisitions left to make*/
    }

    if(!ADC_acq_count){/* If zero acquisitions left to do */

		for(channel = 0; channel <= EADC_LAST_GP_CHANNEL;channel++){/* For every channel */

			analog_channels[channel].rawValue = ADC_acq_buff[channel]; /* Copy the acquisition buffer, dividing by the number of oversamples. */

			ADC_acq_buff[channel] = 0; /* Clear the acquisition buffer */
		}

		ADC_acq_done = true; /* Raise the flag signaling that the acquisition is finished. */

    	EADC_STOP_CONV(EADC, BIT11);
    	EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, BIT11);/*Disable sample module 11 interrupt. It will be the last to acquire, so it will be the one
    		    												that generates the interrupt */
    	NVIC_DisableIRQ(EADC00_IRQn); /* Stop the interrupt */
    	EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);      /* Clear the A/D ADINT0 interrupt flag */
    }

}

void EADC01_IRQHandler(void){ /* Interrupt for temperature sensor channel */

	EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF1_Msk);      /* Clear the A/D ADINT0 interrupt flag */


}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
	extern volatile uint8_t g_u8DeviceAddr;

	extern volatile uint8_t g_au8Data[I2C_DATA_BUFF_LEN];/* The first two bytes are always byte to be sent (a 16 bit command) */
	/* Actual data starts at g_au8Data[2] */

	extern volatile uint8_t g_u8DataLen;
	extern volatile uint8_t g_u8Index;
	extern volatile bool g_bEndFlag;
	extern volatile bool g_bReceiving;

	uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C0);

    if (I2C_GET_TIMEOUT_FLAG(I2C0)){

        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);

    }else{

    	if (u32Status == 0x08){                      /* START has been transmitted */

			if(g_u8Index){ /* If the start condition was sent with index not at zero */
				/* We're doing a plain read without a command sent first. The next thing we have to do is a read. Prepare SLA+R (slave read) */
				/* This is when repeated start is not used. */
				I2C_SET_DATA(I2C0, (g_u8DeviceAddr << 1) | 0x01);  /* Write SLA+R (Slave with read mask) to Register I2CDAT */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);

			}else{ /* Else the start condition was sent with the index at zero */
				/* The next thing we have to do is a write. Prepare SLA+W (slave write) */
				I2C_SET_DATA(I2C0, (g_u8DeviceAddr << 1)); /* Write SLA+W to Register I2CDAT */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
			}

		}

    	else if (u32Status == 0x10){                 /* Repeat START has been transmitted and prepare SLA+R */
    		/* Repeated start only comes after a write */

    		I2C_SET_DATA(I2C0, (g_u8DeviceAddr << 1) | 0x01);  /* Write SLA+R (Slave with read mask) to Register I2CDAT */
			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);

		}

		else if (u32Status == 0x18){                 /* SLA+W has been transmitted and ACK has been received */

			/* Begin data transmission */
			I2C_SET_DATA(I2C0, g_au8Data[g_u8Index++]); /* Send the next byte of data */
			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI); /* Clear I. F. */
		}

		else if (u32Status == 0x20){                 /* SLA+W has been transmitted and NACK has been received */

			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA | I2C_CTL_STO | I2C_CTL_SI); /* clear the interrupt flag, send START and STOP to reset bus. */
		}

		else if (u32Status == 0x28){                 /* DATA has been transmitted and ACK has been received */

			if(g_bReceiving){/* if in receiving mode */

				if (g_u8Index <= 2){ /* As long as we're not finished sending the first 2 command bytes */

						I2C_SET_DATA(I2C0, g_au8Data[g_u8Index++]); /* Send the next byte of data */
						I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);

				}else{/* else we've sent the first 2 command bytes and we'll switch to actually receiving... */

					I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA | I2C_CTL_SI);/* ...by sending a repeated start (and clearing the int. flag). */
					/* Next status should be 0x10, repeated start */
				}

			}else{ /* Else we're in sending mode */


				if(g_u8Index <= g_u8DataLen){ /* As long as the end of the buffer is not reached */

						I2C_SET_DATA(I2C0, g_au8Data[g_u8Index++]); /* Send the next byte of data */
						I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI); /* Clear I.F. */

				}else{/* else we've sent the whole buffer */
					I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO | I2C_CTL_SI); /* Send STOP and clear interrupt flag. */
					g_bEndFlag = true; /* Flag the operation as finished */
				}

			}

		}

		else if (u32Status == 0x30){                 /* DATA has been transmitted and NACK has been received */

			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA | I2C_CTL_STO | I2C_CTL_SI); /* clear the interrupt flag, send START and STOP to reset bus. */
		}

		else if (u32Status == 0x40){                 /* SLA+R has been transmitted and ACK has been received */

			if(g_u8Index + 1 < g_u8DataLen){ /* If the byte we'll receive wont'be the last */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI|I2C_CTL_AA); /* Clear interrupt flag and ACK next byte */
			}else{/* else */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI); /*  Else, just clear interrupt flag, the byte we'll receive will be NACked cause it's the last */
			}

		}
		else if (u32Status == 0x50){                 /* DATA has been received and ACK has been returned */

			g_au8Data[g_u8Index++] = I2C_GET_DATA(I2C0); /* Store the received data in the buffer */

			if(g_u8Index + 1 < g_u8DataLen){ /* If the next byte we'll receive wont'be the last */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI|I2C_CTL_AA); /* Clear interrupt flag and ACK next byte */
			}else{/* Else */
				I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI); /* just clear the interrupt flag the byte we'll receive will be NACked since it's the last */
			}

			//g_u8RxData = I2C_GET_DATA(I2C0);
			//I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO | I2C_CTL_SI);
			//g_bEndFlag = true; /* Set the flag indicating we're finished */
		}

		else if (u32Status == 0x58){                 /* DATA has been received and NACK has been returned */

			g_au8Data[g_u8Index++] = I2C_GET_DATA(I2C0);
			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO | I2C_CTL_SI);/* Send a STOP and clear interrupt flag */
			g_bEndFlag = true; /* Set the flag indicating we're finished */
		}

		else{

			/* TO DO */
			printf("ERROR: I2C status 0x%x received and NOT processed.\n", u32Status);
		}
    }
}
