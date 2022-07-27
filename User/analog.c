/*
 * analog.c
 *
 *  Created on: Jul 5, 2022
 *      Author: Hugo Boyce
 */

#include <math.h>
#include "analog.h"
#include "vcom_serial.h"
#include "errors.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/* Used in EADC interrupt */
volatile uint32_t ADC_acq_buff[EADC_TOTAL_CHANNELS];
volatile uint8_t ADC_acq_count;
volatile bool ADC_first_acq;

/* Used in EADC interrupt and in "runtime" */
volatile bool ADC_acq_done;


/*---------------------------------------------------------------------------------------------------------*/
/* Function definitions                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
// TODO: Add conversion to celsius  for the temperature channels

void ADC_Init(void){

    /* Set input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

    /* Initialize general purpose channels, they'll use interrupt 0 */
    uint8_t channel;
    for(channel = 0; channel <= EADC_LAST_GP_CHANNEL; channel++){
    	/* Configure the sample module 0 for analog input channel 0 and enable ADINT0 trigger source */
    	EADC_ConfigSampleModule(EADC, channel, EADC_ADINT0_TRIGGER, channel);
    	EADC_SetExtendSampleTime(EADC, channel, 0x3F); /* 0x3F Is what's present in the example code*/
    }

    /* Temperature sensor (Sample module 17) */
    SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk; /* Enable temperature sensor */
    /* Set sample module 17 external sampling time to 0x3F */
    EADC_SetExtendSampleTime(EADC, 17, 0x3F); /* That's present in the example code*/



    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk|EADC_STATUS2_ADIF1_Msk); /* Clear the A/D ADINT0 and ADINT1 interrupt flag for safety */
    EADC_ENABLE_INT(EADC, BIT0|BIT1);//Enable sample module  A/D ADINT0 and ADINT1 interrupt.

    // We won't bother setting the interrupt priority for now.
    //NVIC_SetPriority(EADC00_IRQn, ADC_INT_PRIORITY);

}



void ADC_StartAcquisition(void){

	/******************** Calibration ************************************/
	// Re-calibrating every time increases measurement noise but probably reduces the thermal variations
	//volatile uint32_t cycles_cnt = 0;
	CLK->CLKDIV0 &= ~CLK_CLKDIV0_EADCDIV_Msk; /* Clear EADCDIV to zero for calibration as per TRM. */
	EADC->CALCTL |= EADC_CALCTL_CALSEL_Msk; /* Sel CALSEL bit so calibration is performed when we start it. */
	EADC->CALCTL |= EADC_CALCTL_CALSTART_Msk; /* Set the CALSTART to begin calibration*/
	while(!(EADC->CALCTL & EADC_CALCTL_CALDONE_Msk)){/* While the CALDONE bit is not set*/
		//Wait
		//cycles_cnt++;/* Wait and increment a counter. Usually takes 25 cycles*/
	}
	/******************** Calibration end ********************************/

	//CLK->CLKDIV0 |= CLK_CLKDIV0_EADC(10); /* Set clkdiv to 10 for acquisition */

	/* Start ADC acquisition batch*/
	ADC_acq_count = EADC_OVERSAMPLING_NUMBER;
	ADC_acq_done = false; /* Clear the acquisition done flag cause we'ere starting a new one, in case it was never serviced */
	ADC_first_acq = true;

	EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT11);/*Enable sample module 11 interrupt. It will be the last to acquire, so it will be the one
	    												that generates the interrupt */
	//EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);      /* Clear the A/D ADINT0 interrupt flag */
	NVIC_EnableIRQ(EADC00_IRQn);
	EADC_START_CONV(EADC, BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT10|BIT11);


}

void ADC_StartAcquisition_ChipTemp(void){

	EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 1, BIT17);//Enable sample module 17 ADINT1 interrupt.
	NVIC_EnableIRQ(EADC01_IRQn);
	EADC_START_CONV(EADC, BIT17);
}

void ADC_SaveToChannelAndClearBuffer(void){



}

void Analog_Acquire(void){

	ADC_StartAcquisition(); /* Start acquisition */

	/* Caution, blocking task: */
	while(!ADC_acq_done){/* As long as ADC_acq_done is false (acquisition has not finished) */
		// Wait
	}
	ADC_acq_done = false; /* Clear the flag*/

	Analog_ConvertToFieldValue();
	Analog_FilterFieldValues();
	Analog_FilterNoise();
	Analog_ConvertToProcessValues();

}

void Analog_ConvertToFieldValue(void){

	uint8_t i;
	for(i = 0; i <= EADC_LAST_GP_CHANNEL; i++){/* For all general purpose channels*/

		switch(analog_channels[i].fieldUnit){/* If the field (sensor output) unit is */

			case UNIT_OHM: /* ohms, then */
				/* convert using this formula */
				analog_channels[i].fieldValue = ((-((float)analog_channels[i].rawValue) * analog_channels[i].biasResistor)/(analog_channels[i].rawValue - EADC_OVERSAMPLING_NUMBER*EADC_12BIT_MAX_COUNT));
				break;

			case UNIT_MILLIVOLT:/* millivolts, then  */
				/* convert using this formula */
				analog_channels[i].fieldValue = (((float)analog_channels[i].rawValue)/(EADC_OVERSAMPLING_NUMBER*EADC_12BIT_MAX_COUNT))*EADC_VREF_MILLIVOLTS*(1/analog_channels[i].voltageGain);
				break;

			case UNIT_VOLT:/* volts, then  */
				/* convert using this formula */
				analog_channels[i].fieldValue = (((float)analog_channels[i].rawValue)/(EADC_OVERSAMPLING_NUMBER*EADC_12BIT_MAX_COUNT))*(EADC_VREF_MILLIVOLTS/1000)*(1/analog_channels[i].voltageGain);
				break;

			default:
				break;

		}

	}

}

void Analog_FilterFieldValues(void){

	static bool firstRun = true; /* Flag initialized to true */
	static float fieldValue_prev[EADC_TOTAL_CHANNELS] = {0}; /* Explicitly initialized to 0 even if its guaranteed by the C standard */
	static float fieldValue_filtered_prev[EADC_TOTAL_CHANNELS] = {0};

	uint8_t i;/* i will be the channel number */
	for(i = 0; i <= EADC_LAST_GP_CHANNEL; i++){/* For every channel */

		//if(analog_channels[i].fieldValue <= 0){
		//	printf("Negative\n\r");
		//}

		if(firstRun){/* If the function has never been run before */
			analog_channels[i].fieldValue_filtered = analog_channels[i].fieldValue; /* Just copy the values*/
		}else{/* For every execution thereafter */
			/* Run filter */

			/* The algorithm used is a Trapezoidal (Tustin) 1st order low-pass filter *
			 *                                                                        *
			 *             u*z^1 + u*z^0 - Y*z^0 + Y*(2 * TAU / T)*z^0                *
			 *    Y*z^1 = ---------------------------------------------               *
			 *                       1 + (2 * TAU / T)                                *
			 *                                                                        *
			 * u is fieldValue, Y is fieldValueFiltered, T is time step,              *
			 * TAU = R*C = 1/(2*pi*f_c)                                               */

			float ct;
			ct = (2 * ANALOG_LPF_TAU / ANALOG_SAMPLE_T);
			analog_channels[i].fieldValue_filtered = (analog_channels[i].fieldValue + fieldValue_prev[i] - fieldValue_filtered_prev[i] + (fieldValue_filtered_prev[i] * ct)) / (1 + ct);

		}

		fieldValue_prev[i] = analog_channels[i].fieldValue; /* Current values become previous values, for the next iteration. */
		fieldValue_filtered_prev[i] = analog_channels[i].fieldValue_filtered;

	}
	firstRun = false; /* After the function has been run at least once, this flag goes false*/

}

void Analog_FilterNoise(void){

	static bool firstRun = true; /* Flag initialized to true */
	static float squaredNoise_inst[EADC_TOTAL_CHANNELS] = {0};
	static float squaredNoise_inst_prev[EADC_TOTAL_CHANNELS] = {0}; /* Explicitly initialized to 0 even if its guaranteed by the C standard */
	static float squaredNoise_filtered[EADC_TOTAL_CHANNELS] = {0};
	static float squaredNoise_filtered_prev[EADC_TOTAL_CHANNELS] = {0};

	uint8_t i;/* i will be the channel number */
	for(i = 0; i <= EADC_LAST_GP_CHANNEL; i++){/* For every channel */


		/* Calculate noise */

		float err;
		err = (analog_channels[i].fieldValue - analog_channels[i].fieldValue_filtered);/* The difference between the averaged and instantaneous value is the error. */
		squaredNoise_inst[i] = err * err; /* noise is the error squared. */

		if(firstRun){/* If the function has never been run before */
			squaredNoise_filtered[i] = squaredNoise_inst[i]; /* Just copy the values*/
		}else{/* For every execution thereafter */
			/* Run filter */

			/* The algorithm used is a Trapezoidal (Tustin) 1st order low-pass filter *
			 *                                                                        *
			 *             u*z^1 + u*z^0 - Y*z^0 + Y*(2 * TAU / T)*z^0                *
			 *    Y*z^1 = ---------------------------------------------               *
			 *                       1 + (2 * TAU / T)                                *
			 *                                                                        *
			 * u is fieldValue, Y is fieldValueFiltered, T is time step,              *
			 * TAU = R*C = 1/(2*pi*f_c)                                               */

			float ct;
			ct = (2 * 1.0 / ANALOG_SAMPLE_T); /* Tau = 1.0 */
			squaredNoise_filtered[i] = (squaredNoise_inst[i] + squaredNoise_inst_prev[i] - squaredNoise_filtered_prev[i] + (squaredNoise_filtered_prev[i] * ct)) / (1 + ct);

		}

		analog_channels[i].RMSNoise = sqrtf(squaredNoise_filtered[i]);
		
		squaredNoise_inst_prev[i] = squaredNoise_inst[i]; /* Current values become previous values, for the next iteration. */
		squaredNoise_filtered_prev[i] = squaredNoise_filtered[i];

	}
	firstRun = false; /* After the function has been run at least once, this flag goes false*/

}


void Analog_ConvertToProcessValues(void){


	uint8_t i;/* i will be the channel number */
	for(i = 0; i <= EADC_LAST_GP_CHANNEL; i++){/* For every channel */

		switch(analog_channels[i].sensorType){/* According to the sensor type: */

			case ANALOG_SENSOR_NTC: /* If the sensor is an NTC thermistor */

				if(analog_channels[i].processUnit == UNIT_KELVIN || analog_channels[i].processUnit == UNIT_CELSIUS){/* If Kelvin or Celsius */

					/* Using the Steinhart-Hart equation as per Wikipedia, get the Kelvin value */
					float r_inf;
					r_inf = analog_channels[i].NTCRZero * expf(-analog_channels[i].NTCBeta / ANALOG_NTC_T_ZERO);
					analog_channels[i].processValue = analog_channels[i].NTCBeta / logf(analog_channels[i].fieldValue_filtered/r_inf);

					if(analog_channels[i].processUnit == UNIT_CELSIUS){/* If celsius */
						analog_channels[i].processValue -= 273.15; /* Convert by substracting absolute zero */
					}

				}else{
					VCOM_PushString("Process unit not supported on channel ");
					char str[SHORT_STRING_LENGTH];
					sprintf(str, "%u\n\r", i); /* Print error message */
					VCOM_PushString(str);
				}
				break;

			case ANALOG_SENSOR_NONE: /* If no sensor */
				if(analog_channels[i].fieldUnit == analog_channels[i].processUnit){ /* the units have to match because */
					analog_channels[i].processValue = analog_channels[i].fieldValue_filtered; /* the process value is just copied to the field value */
				}else{/* Else */
					Error_Set(ERROR_INVALID_CONFIG);
					static bool sent = false;
					if(!sent){
						printf("Unit mismatch on channel %u\n+", i); /* Print error message */
						sent = true;
					}


				}
				break;

			default:
				break;

		}

	}

}

