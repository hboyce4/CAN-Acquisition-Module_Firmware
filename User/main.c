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
#include "interrupt.h"
#include "vcom_serial.h"
#include "UI.h"
#include "analog.h"
#include "I2C_sensors.h"
#include "user_sys.h"
#include "CAN_message.h"




void SYS_Init(void)
{
	/* Start of protected registers */
	/* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to HXT. HXT = HCLK = 12 MHz */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT,CLK_CLKDIV0_HCLK(1)); /* Most likely superfluous, see next line.*/

    /* Switch HCLK to PLL, set PLL to 192 MHz. PLL = HCLK = 192 MHz */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/4. PCLK0 = PCLK 1 = HCLK/4 = 192 MHz/4 = 48 MHz   */
    /* "PCLK must be less than 96 MHz." TRM Rev 3.02 page 614 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV4 | CLK_PCLKDIV_APB1DIV_DIV4;


    /*********************** USB Device *****************************************/
    /* Select IP clock source for USB */
    CLK->CLKSEL0 |= CLK_CLKSEL0_USBSEL_Msk; /* Select PLL as source (192 MHz) */
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBDIV_Msk) | CLK_CLKDIV0_USB(4);/* USB peripheral requires 48 MHz clk. 192MHz/4 = 48 MHz*/
    /* Select USBD */
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk;
    /* Enable IP clock */
    CLK_EnableModuleClock(USBD_MODULE);
    /*********************** USB Device End **************************************/

    /***************************** UART0 *****************************************/
    /* Select UART clock source from HXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART0(1));
    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    /***************************** UART0 End *************************************/

    /***************************** EADC *****************************************/
    /* EADC clock has only one source: PCLK1, so no need to select it.*/
    CLK->CLKDIV0 &= ~CLK_CLKDIV0_EADCDIV_Msk;/* Clear the divider for EADC */
    /* The divider set to 0 means the clock is divided by 1. It also means the ADC can be recalibrated*/
    /* EADC_CLK = PCLK/1 = 48MHz/1 = 48 MHz. (72 MHz max as per TRM) */
    /* Enable IP clock */
    CLK_EnableModuleClock(EADC_MODULE);
    SYS_SetVRef(SYS_VREFCTL_VREF_PIN); /* Set the voltage reference as the external pin. */
    /***************************** EADC End *************************************/

    /***************************** I2C0 *****************************************/
    /* Enable I2C0 clock */
    CLK_EnableModuleClock(I2C0_MODULE);
    /***************************** I2C0 End *************************************/

    /***************************** CAN0 *****************************************/
    // Enable IP clock
    CLK->APBCLK0 |= CLK_APBCLK0_CAN0CKEN_Msk;
    //CLK_EnableModuleClock(CAN0); // Probably works too
    /***************************** CAN0 end *************************************/

    /***************************** SysTick ***************************************/
    /* Update System Core Clock */
    SystemCoreClockUpdate();
    /* SysTick 1 millisecond interrupts  */
    SysTick_Config(SystemCoreClock / 1000); /* SysTick is driven by CPU Clock */
    NVIC_SetPriority(SysTick_IRQn, 1);// Set the interrupt priority.
    /***************************** SysTick End ***********************************/

    /* End of protected registers */
    /* Lock protected registers */
    SYS_LockReg();

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

	/***************************** EADC *****************************************/
	 /* EADC: Set PB.0 ~ PB.11 to input mode */
	GPIO_SetMode(PB, BIT11|BIT10|BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0, GPIO_MODE_INPUT);/* Cleared to 0b00 is input */
	GPIO_DISABLE_DIGITAL_PATH(PB, BIT11|BIT10|BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0);/* Disable digital input paths to reduce leakage currents. */

	/* EADC: Configure the GPB0 - GPB11 ADC analog input pins. (Multi Function Pins) */
	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk
				 |SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB7MFP_Msk);
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB0MFP_EADC0_CH0 | SYS_GPB_MFPL_PB1MFP_EADC0_CH1 | SYS_GPB_MFPL_PB2MFP_EADC0_CH2 |
				 SYS_GPB_MFPL_PB3MFP_EADC0_CH3 | SYS_GPB_MFPL_PB4MFP_EADC0_CH4 | SYS_GPB_MFPL_PB5MFP_EADC0_CH5 | SYS_GPB_MFPL_PB6MFP_EADC0_CH6 |
				 SYS_GPB_MFPL_PB7MFP_EADC0_CH7);
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk | SYS_GPB_MFPH_PB10MFP_Msk | SYS_GPB_MFPH_PB11MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_EADC0_CH8 | SYS_GPB_MFPH_PB9MFP_EADC0_CH9 | SYS_GPB_MFPH_PB10MFP_EADC0_CH10 | SYS_GPB_MFPH_PB11MFP_EADC0_CH11);
	/***************************** EADC End *************************************/

	/***************************** I2C0 *****************************************/
	/* Set I2C0 multi-function pins */
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk)) |
					(SYS_GPC_MFPL_PC0MFP_I2C0_SDA | SYS_GPC_MFPL_PC1MFP_I2C0_SCL);
	/* I2C pin enable schmitt trigger */
	PC->SMTEN |= GPIO_SMTEN_SMTEN0_Msk | GPIO_SMTEN_SMTEN1_Msk;
	/* Enable Pull-ups so NACKs can be observed of I2C components are not populated */
	GPIO_SetPullCtl(PC, BIT1|BIT0, GPIO_PUSEL_PULL_UP);
	/***************************** I2C0 End *************************************/

	/***************************** CAN0 *****************************************/
	/* Set PA multi-function pins for CAN0 RXD(PA.4) and TXD(PA.5) */
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk)) |
	                  (SYS_GPA_MFPL_PA4MFP_CAN0_RXD | SYS_GPA_MFPL_PA5MFP_CAN0_TXD);

	// if CAN transceiver is SN65HVD235, set the loopback pin as low:
	GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT);
	PA2 = false;
	/***************************** CAN0 end *************************************/
}


int main (void)
{

    SYS_Init();

    UART_Open(UART0, DEBUG_UART_SPEED); // Debug port. printf() is piped to this. See retarget.c file
    printf("CAN Acquisition Module - Debug port\n");

    ADC_Init();

    /* Initialize virtual COM port (over USB) */
    VCOM_Init();

    /* Init I2C0 */
    I2C0_Init();

    CAN_Init();


    interrupt_setPriorities();


    I2C_sensorAutodetect();
    I2C_sensorConfigure();

    while(1)
    {
    	// All the time
        VCOM_TransferData();

        int8_t i8RowSel, i8ColumnSel;
        UI_read_user_input(&i8RowSel,&i8ColumnSel);

        I2C_sensorCheckIfNewDataAndConvert();

        CAN_Service();




        if(gbDrawNewUIFrame){// Every UI_FRAME_INTERVAL_MS milliseconds
        	gbDrawNewUIFrame = false;

        	if(gbTerminalActive){
        		UI_draw(i8RowSel, i8ColumnSel);

        	}

        }

        if(gbSecondsFlag){// Every second
        	gbSecondsFlag = false;// Clear the flag
        	// Do things that need to be done every second

        	//ADC_StartAcquisition(); /* Run an acquisition every second */

        	cycleLED();

        	I2C_sensorOncePerSecondRoutine();/* This easily takes 10 ms*/

        }

        if(gbSampleAnalog){
        	gbSampleAnalog = false;

        	Analog_Acquire(); /* Sample analog inputs */

        }


    }


}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
