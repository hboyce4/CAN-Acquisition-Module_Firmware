/**************************************************************************//**
 * @file     main.c
 * @brief    Demonstrate how to implement a USB virtual com port device.
 * @version  2.0.0
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "NuMicro.h"
#include "vcom_serial.h"
#include "sys.h"


/* SysTick */
volatile uint32_t g_SysTickIntCnt = 0;
volatile bool gbSecondsFlag = false;
extern volatile bool gbPcTerminalActive;
extern volatile uint8_t gu8PcTerminalActive_TimeoutCounter;


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT,CLK_CLKDIV0_HCLK(1));

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2;


    /* Select IP clock source for USB */
    CLK->CLKSEL0 |= CLK_CLKSEL0_USBSEL_Msk;
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBDIV_Msk) | CLK_CLKDIV0_USB(4);/* USB peripheral requires 48 MHz clk. 192MHz/4 = 48 MHz*/


    /* Select USBD */
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk;

    /* Enable IP clock */
    CLK_EnableModuleClock(USBD_MODULE);

    /* Select UART clock source from HXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART0(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* SysTick*/
    /* Update System Core Clock */
    SystemCoreClockUpdate();
    /* SysTick 1m second interrupts  */
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_SetPriority(SysTick_IRQn, 1);// Set the interrupt priority.

    /* Set PA.12 ~ PA.14 (Full Speed USB pins)to input mode */
    PA->MODE &= ~(GPIO_MODE_MODE12_Msk | GPIO_MODE_MODE13_Msk | GPIO_MODE_MODE14_Msk);
    SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA12MFP_Msk|SYS_GPA_MFPH_PA13MFP_Msk|SYS_GPA_MFPH_PA14MFP_Msk|SYS_GPA_MFPH_PA15MFP_Msk);
    SYS->GPA_MFPH |= (SYS_GPA_MFPH_PA12MFP_USB_VBUS|SYS_GPA_MFPH_PA13MFP_USB_D_N|SYS_GPA_MFPH_PA14MFP_USB_D_P|SYS_GPA_MFPH_PA15MFP_USB_OTG_ID);

    /* Set GPA multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_UART0_RXD | SYS_GPA_MFPL_PA1MFP_UART0_TXD);

    /* Set Port B multi-function pins (MFP) for LEDs */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB13MFP_Msk | SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB13MFP_GPIO | SYS_GPB_MFPH_PB14MFP_GPIO | SYS_GPB_MFPH_PB15MFP_GPIO);// Set Multifunction pin (MFP) as GPIO
	GPIO_SetMode(PB, BIT13|BIT14|BIT15, GPIO_MODE_OUTPUT); // Set LED pins as outputs.


    /* Lock protected registers */
    SYS_LockReg();
}

/* Brown-out detector to be set to 2.4V */

int main (void)
{

    SYS_Init();

    UART_Open(UART0, 115200); // Debug port. printf() is piped to this.
    printf("CAN Acquisition Module - Debug port\n");



    /* Initialize virtual COM port (over USB) */
    VCOM_Init();

    //NVIC_EnableIRQ(UART0_IRQn);

    NVIC_EnableIRQ(USBD_IRQn);

    uint8_t u8LEDState = 0;

    while(1)
    {
        VCOM_TransferData();

        uint8_t byte;
        if(VCOM_PopFromBuf(&byte) == EXIT_SUCCESS){
        	printf("Received character %i\n", byte);
        }

        if(gbSecondsFlag){// Every second
        	gbSecondsFlag = false;// Clear the flag
        	// Do things that need to be done every second

        	VCOM_PushString("Tick\n\r");

        	PB->DOUT &= ~(BIT13|BIT14|BIT15);
        	PB->DOUT |= (u8LEDState << 13);
        	printf("LED State is %i\n", u8LEDState);
        	u8LEDState++;
        	if(u8LEDState > 0b111){
        		u8LEDState =0;
        	}
        }
    }


}

void SysTick_Handler(void)	// Every millisecond (Medium frequency).
{
    g_SysTickIntCnt++;

    /* USB terminal on host PC timeout*/
    if(gu8PcTerminalActive_TimeoutCounter){/* If the inactivity counter has not elapsed*/
    	gu8PcTerminalActive_TimeoutCounter--;/* Decrement*/
    	gbPcTerminalActive = true; /* Is means the terminal is active*/
    }else{
    	gbPcTerminalActive = false; /* Else the terminal is not active*/

    }

    static uint16_t u16SecondsCounter = 1000; /* 1000 ms per second*/
    if(!u16SecondsCounter){// If counter has elapsed (equals zero)
    	u16SecondsCounter = 1000;//Reload counter. 1000 ms per second.
    	gbSecondsFlag = true;
    }
    u16SecondsCounter--; // Decrease the counter every time SysTick_Handler is called

}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
