/**************************************************************************************

      II    N     N     N     N      OOOO      SSSSS     EEEEE    N     N    TTTTTTT
     II    NNN   N     NNN   N    OO    OO    S         E        NNN   N       T
    II    N NN  N     N NN  N    OO    OO    SSSSS     EEE      N NN  N       T
   II    N  NN N     N  NN N    OO    OO        S     E        N  NN N       T
  II    N    NN     N    NN      OOOO      SSSSS     EEEEE    N    NN       T
                         copyright (c) 2016, InnoSenT GmbH
                                 all rights reserved

***************************************************************************************

	filename:		Initialization.c
	brief:			initialize application
	creation:		19.01.2017
	author:			Sergej Gauerhof
	last editor:	.....


**************************************************************************************/

/**************************************************************************************
   preprocessor includes
**************************************************************************************/
#include "Initialization.h"
#include "CONFIG.h"
#include "tim.h"
#include "usart.h"
#include "ad5662.h"
#include "Framework.h"
#include "arm_math.h"
/**************************************************************************************
   preprocessor defines
**************************************************************************************/

/**************************************************************************************
   global constants
**************************************************************************************/

/**************************************************************************************
   global variables
**************************************************************************************/
struct SENSOR_REGS Sensor;
struct COUNTER_REGS Counter;
struct THRESHOLD_REGS Threshold;
struct UART_REGS Uart;
struct FLAG_REGS Flags;

int16_t I[FFT_SIZE];		/* I-signal buffer */
int16_t Q[FFT_SIZE];		/* Q-signal buffer */
float32_t IQ[2*FFT_SIZE];	/* I/Q interleave buffer */
int32_t	FFTmagnitude[FFT_SIZE];	/* FFT magnitude buffer */

int16_t rawI[FFT_SIZE];		/* original raw I signal buffer */
int16_t rawQ[FFT_SIZE];		/* original raw Q signal buffer */

/**************************************************************************************
   global function prototypes
**************************************************************************************/

/**************************************************************************************
   local constants
**************************************************************************************/

/**************************************************************************************
   local macros
**************************************************************************************/

/**************************************************************************************
   local types
**************************************************************************************/

/**************************************************************************************
   local variables
**************************************************************************************/

/**************************************************************************************
   local function prototypes
**************************************************************************************/

/**************************************************************************************
   local function definitions
**************************************************************************************/

/**************************************************************************************
	function name:		g_InitApplication
	purpose:			this routine initializes the application parameters

	return value:		none
	input parameters:	none
	output parameters:	none

	author:				Sergej Gauerhof
	version:			V1.0
	last edit:			...
	last editor:		...
	change:				...

**************************************************************************************/
void g_InitApplication(void)
{

	/*-------------------- Check CONFIG.h parameters --------------------------------*/
	/* the frequency should be between 24.000GHz and 24.250GHz because of Radio Regulations! */
	if( (FREQ_START < 24000) || (FREQ_STOP > 24250) )
	{
		Error_Handler();
	}
	if(FREQ_START > FREQ_STOP)	/*	the start frequency should be less than the stop frequency	*/
	{
		Error_Handler();
	}

	/* Check MODULATION mode */
	if((MODULATION != FMCW) && (MODULATION != CW))
	{
		Error_Handler();
	}


	/*---------------------------------------------------------------------------------------------------------*/


	/*-------------------- Sensor register --------------------------------*/
	Sensor.modulation 			= 	MODULATION;		/*	set the current modulation	*/
	Sensor.startFreq 			= 	FREQ_START;		/* set the start frequency	*/
	Sensor.dwnCnvStartFreq 		= 	(Sensor.startFreq/8192)*1e6;	/* down-converted start frequency by factor 8192 in [MHz] */
	Sensor.stopFreq 			= 	FREQ_STOP;		/* set the stop frequency	*/
	Sensor.dwnCnvStopFreq 		= 	(Sensor.stopFreq/8192)*1e6;		/* down-converted stop frequency by factor 8192 in [MHz] */
	Sensor.bandwidth 			= 	Sensor.stopFreq - Sensor.startFreq;	/*	calculates the FMCW sweep in [MHz]	*/

	Sensor.startFreqDigits 		= 	25775;	/* default DAC start frequency in digits */
	Sensor.stopFreqDigits 		= 	30225;	/* default DAC stop frequency in digits */
	Sensor.bandwidthDigits 		= 	Sensor.stopFreqDigits - Sensor.startFreqDigits;		/* bandwidth in digits */
	Sensor.freqStepDigits 		= 	Sensor.bandwidthDigits/FFT_SIZE;	/* frequency step in digits */
	Sensor.currentFreqDigits 	= 	Sensor.startFreqDigits;			/* current transmitted frequency */

	Sensor.measFreq				= 	0;	/* measured down-converted SMR transmitting frequency */


	/*---------------------------------------------------------------------------------------------------------*/

	/*-------------------- Usart register --------------------------------*/

	/*---------------------------------------------------------------------------------------------------------*/

	/*-------------------- Flags register --------------------------------*/
	Flags.freqCalibration			=	FREQ_CALIB_EN;	/**/
	Flags.wait1Cycle 				= 	TRUE;			/* indicate first run after starting application */
	Flags.calibStartStopFreq 		= 	0;				/* indicate whether start or stop frequency is calibrated. 0: start frequency, 1: stop frequency */
	Flags.freqInitCalib				=	TRUE;			/* indicate whether initial frequency calibration is carried out */
	Flags.startFreqCalibCmplt		=	FALSE;			/* indicate whether initial start frequency has been calibrated */
	if(Sensor.modulation == FMCW)
	{
		Flags.stopFreqCalibCmplt	= 	FALSE;		/* indicate whether initial stop frequency has been calibrated */
	}
	else	/* no need to perform frequency calibration for stop frequency in CW mode */
	{
		Flags.stopFreqCalibCmplt	= 	TRUE;		/* indicate whether initial start frequency has been calibrated */
	}
	/*---------------------------------------------------------------------------------------------------------*/

	/*-------------------- Counter register --------------------------------*/
	Counter.systemCnt 			= 	0;		/* application time tick */
	Counter.sampleCnt 			= 	0;		/* count number of captured samples */
	Counter.edgeCnt	 			= 	0;		/* count number of detected edges of sensor frequency input signal */
	Counter.freqCalibIntvl 		= 	0;		/* count number of measurements between two frequency calibrations */

	Counter.cycleCntMAX 		= 	(CYCLE_DURATION/SAMPLING_PERIOD);	/* measurement cycle duration	in [ADC sampling interval] */
	/*---------------------------------------------------------------------------------------------------------*/

	/*-------------------- Threshold register --------------------------------*/
	Threshold.freqCalibNbr 		= 	FREQ_CALIB_INTVL;		/* indicate when frequency auto calibration is carried out in [number of measurements] */
	Threshold.deltaFreq			= 	120;	/* threshold for adjusting transmit frequency in [MHz]: 120*8192 approx. 0.983MHz */
	/*---------------------------------------------------------------------------------------------------------*/

	/* INIT UART BAUDRATE -------------------------------------------------------------------------------------*/
	huart2.Init.BaudRate = BAUDRATE;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/*---------------------------------------------------------------------------------------------------------*/

	/*-------------------- Init frequency auto-calibration mode --------------------------------*/
	/* Check Frequency auto calibration mode */
	if(Flags.freqCalibration == TRUE)
	{
		g_ui16_runId = ID_FREQ_CALIB;	/* go to frequency calibration mode and perform initial frequency calibration */
	}
	else
	{
		g_ui16_runId = ID_RUN;	/* go directly to run mode */
	}
	/*---------------------------------------------------------------------------------------------------------*/

}


/**************************************************************************************
	function name:		g_ActivateSMR
	purpose:			this routine switches on/off the SMR

	return value:		none
	input parameters:	enable
	output parameters:	none

	creation:			19.01.2017
	author:				Sergej Gauerhof
	version:			V1.0
	last edit:			...
	last editor:		...
	change:				...

**************************************************************************************/
void g_ActivateSMR(uint16_t enable)
{
	if(enable == 1)
	{
		HAL_GPIO_WritePin(SMR_Vcc_EN_GPIO_Port, SMR_Vcc_EN_Pin, GPIO_PIN_SET);			/* switch on SMR radar module */
		HAL_Delay(1);	/*	wait 1ms for defined sequential switching on of SMR	*/
		HAL_GPIO_WritePin(SMR_TX_out_EN_GPIO_Port, SMR_TX_out_EN_Pin, GPIO_PIN_SET);	/* switch on Tx amplifier */
		HAL_Delay(1);	/*	wait 1ms for defined sequential switching on of SMR	*/
		HAL_GPIO_WritePin(Vcc_div_EN_GPIO_Port, Vcc_div_EN_Pin, GPIO_PIN_SET);			/* switch on transmit frequency down-conversion */
	}
	else
	{
		HAL_GPIO_WritePin(Vcc_div_EN_GPIO_Port, Vcc_div_EN_Pin, GPIO_PIN_RESET);		/* switch off SMR radar module */
		HAL_Delay(1);	/*	wait 1ms for defined sequential switching off of SMR */
		HAL_GPIO_WritePin(SMR_TX_out_EN_GPIO_Port, SMR_TX_out_EN_Pin, GPIO_PIN_RESET);	/* switch off Tx amplifier */
		HAL_Delay(1);	/*	wait 1ms for defined sequential switching off of SMR */
		HAL_GPIO_WritePin(SMR_Vcc_EN_GPIO_Port, SMR_Vcc_EN_Pin, GPIO_PIN_RESET);		/* switch off transmit frequency down-conversion */
	}
}





/**************************************************************************************
	function name:		g_InitFreqCalib
	purpose:			this routine perform initial frequency calibration after starting program

	return value:		none
	input parameters:	enable
	output parameters:	none

	creation:			09.02.2017
	author:				Duy Bien Le
	version:			V1.0
	last edit:			...
	last editor:		...
	change:				...
**************************************************************************************/
void g_InitFreqCalib()
{

	/* Correct frequency */
	if(Flags.calibStartStopFreq == 0)	/* correcting for start frequency */
	{
		/* Correct transmit frequency */
		Sensor.freqDiff = Sensor.measFreq - Sensor.dwnCnvStartFreq;	/* difference between transmitting frequency and wanted frequency */
		if(Sensor.freqDiff > Threshold.deltaFreq)					/* compare frequency shift to a tolerance */
		{
			/* reduce transmit frequency */
			Sensor.startFreqDigits = Sensor.startFreqDigits - 25;	/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
		else if(Sensor.freqDiff < -Threshold.deltaFreq)
		{
			/* increase transmit frequency */
			Sensor.startFreqDigits = Sensor.startFreqDigits + 25;	/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
	}
	else	/* correcting for start frequency */
	{
		/* Correct transmit frequency */
		Sensor.freqDiff = Sensor.measFreq - Sensor.dwnCnvStopFreq;	/* difference between transmitting frequency and wanted frequency */
		if(Sensor.freqDiff > Threshold.deltaFreq)
		{
			/* reduce transmit frequency */
			Sensor.stopFreqDigits = Sensor.stopFreqDigits - 25;		/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
		else if(Sensor.freqDiff < -Threshold.deltaFreq)
		{
			/* increase transmit frequency */
			Sensor.stopFreqDigits = Sensor.stopFreqDigits + 25;		/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
	}

	/* finding start frequency is completed if difference is smaller than threshold delta */
	if(((Sensor.measFreq-Sensor.dwnCnvStartFreq) < Threshold.deltaFreq) && ((Sensor.measFreq-Sensor.dwnCnvStartFreq) > -Threshold.deltaFreq) )
	{
		Flags.startFreqCalibCmplt = TRUE;						/* set flag to indicate start frequency found */
		Flags.calibStartStopFreq = !Flags.calibStartStopFreq;	/* switch to finding stop frequency  */
	}

	/* finding stop frequency is completed if difference is smaller than threshold delta */
	if(((Sensor.measFreq-Sensor.dwnCnvStopFreq) < Threshold.deltaFreq) && ((Sensor.measFreq-Sensor.dwnCnvStopFreq) > -Threshold.deltaFreq) )
	{
		Flags.stopFreqCalibCmplt = TRUE;	/* set flag to indicate stop frequency found */
	}

	if((Flags.startFreqCalibCmplt == TRUE) && (Flags.stopFreqCalibCmplt == TRUE))
	{
		Flags.freqInitCalib 		= FALSE;	/* initial calibration is finished */
		Flags.calibStartStopFreq 	= 0;		/* switch back to start frequency calibration for normal calibration */
		Threshold.deltaFreq			= 365;		/* increase threshold delta for correcting transmit frequency in normal calibration in [MHz]: 365*8192 approx. 2.99MHz */

		if(Sensor.modulation == FMCW)	/* re-calculate new bandwidth and frequency step after calibration */
		{
			Sensor.bandwidthDigits 		= 	Sensor.stopFreqDigits - Sensor.startFreqDigits;		/* bandwidth in digits */
			Sensor.freqStepDigits 		= 	Sensor.bandwidthDigits/FFT_SIZE;	/* frequency step in digits */
		}
		Sensor.currentFreqDigits 	= 	Sensor.startFreqDigits;			/* current transmitted frequency */

		/* go back to perform measurements */
		g_ui16_runId = ID_RUN;			/* go back to perform measurements */
		HAL_TIM_Base_Start_IT(&htim4);	/* restart timer for sampling and generating FMCW ramp */
	}
	else
	{
		g_ui16_runId = ID_FREQ_CALIB;	/* keep calibrating transmit frequencies */
	}
}




/*****************************g_autoFreqCalib********************************************************
	function name:		g_freqAutoCalib
	purpose:			this routine perform frequency auto calibration every pre-define interval

	return value:		none
	input parameters:	enable
	output parameters:	none

	creation:			09.02.2017
	author:				Duy Bien Le
	version:			V1.0
	last edit:			...
	last editor:		...
	change:				...
**************************************************************************************/
void g_freqAutoCalib()
{
	/* Correct transmit frequency */
	if(Flags.calibStartStopFreq == 0)	/* correcting for start frequency */
	{
		/* Correct transmit frequency */
		Sensor.freqDiff = Sensor.measFreq - Sensor.dwnCnvStartFreq;	/* difference between transmitting frequency and wanted frequency */
		if(Sensor.freqDiff > Threshold.deltaFreq)					/* compare frequency shift to a tolerance */
		{
			/* reduce transmit frequency */
			Sensor.startFreqDigits = Sensor.startFreqDigits - 25;	/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
		else if(Sensor.freqDiff < -Threshold.deltaFreq)
		{
			/* increase transmit frequency */
			Sensor.startFreqDigits = Sensor.startFreqDigits + 25;	/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
	}
	else	/* correcting for start frequency */
	{
		/* Correct transmit frequency */
		Sensor.freqDiff = Sensor.measFreq - Sensor.dwnCnvStopFreq;	/* difference between transmitting frequency and wanted frequency */
		if(Sensor.freqDiff > Threshold.deltaFreq)
		{
			/* reduce transmit frequency */
			Sensor.stopFreqDigits = Sensor.stopFreqDigits - 25;		/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
		else if(Sensor.freqDiff < -Threshold.deltaFreq)
		{
			/* increase transmit frequency */
			Sensor.stopFreqDigits = Sensor.stopFreqDigits + 25;		/* 1 digit approx. 34kHz. 50digits approx. 1.7MHz */
		}
	}

	if(Sensor.modulation == FMCW)	/* FMCW mode: perform calibration for both Start and Stop frequencies interleave */
	{
		Flags.calibStartStopFreq = !Flags.calibStartStopFreq;
	}
	else	/* CW mode: perform calibration only for Start frequency */
	{
	}

	/* Update new values for bandwidth and frequency step in digits */
	Sensor.bandwidthDigits	= 	Sensor.stopFreqDigits - Sensor.startFreqDigits;		/* bandwidth in digits */
	Sensor.freqStepDigits 	= 	Sensor.bandwidthDigits/FFT_SIZE;	/* frequency step in digits */

	g_ad5662_set(Sensor.startFreqDigits);	/* set new start frequency for next measurement */

	/* go back to perform measurements */
	g_ui16_runId 		= ID_RUN;			/* go back to perform measurements */
	Flags.wait1Cycle 	= TRUE;			/* wait 1 measurement for new transmit frequency stable */
	HAL_TIM_Base_Start_IT(&htim4);	/* restart timer for sampling and generating FMCW ramp */
}


