/**************************************************************************************

      II    N     N     N     N      OOOO      SSSSS     EEEEE    N     N    TTTTTTT
     II    NNN   N     NNN   N    OO    OO    S         E        NNN   N       T
    II    N NN  N     N NN  N    OO    OO    SSSSS     EEE      N NN  N       T
   II    N  NN N     N  NN N    OO    OO        S     E        N  NN N       T
  II    N    NN     N    NN      OOOO      SSSSS     EEEEE    N    NN       T
                         copyright (c) 2016, InnoSenT GmbH
                                 all rights reserved

***************************************************************************************

	filename:		Framework.c
	brief:			framework functionality
	creation:		19.01.2017
	author:			Sergej Gauerhof
	last editor:	.....


**************************************************************************************/

/**************************************************************************************
   preprocessor includes
**************************************************************************************/
#include "Framework.h"
#include "CONFIG.h"
#include "Initialization.h"
#include "ad5662.h"
#include "Processing.h"
#include "usart.h"
#include "Protocol.h"
#include "tim.h"
#include "gpio.h"
/**************************************************************************************
   preprocessor defines
**************************************************************************************/

/**************************************************************************************
   global constants
**************************************************************************************/

/**************************************************************************************
   global variables
**************************************************************************************/
volatile active_id_t g_ui16_runId = ID_IDLE;				/* shows if new command available */
volatile active_id_t g_ui16_activeId = ID_IDLE;				/* shows running commands */
GPIO_InitTypeDef GPIO_InitStruct;

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
	function name:		g_ActivateCommands
	purpose:			this functions manages the activation of the commands

	return value:		none
	input parameters:	none
	output parameters:	none

	author:				Sergej Gauerhof
	creation:			19.01.2017
	last editor:		Duy Bien Le

**************************************************************************************/
void g_ActivateCommands(void)
{
	if(g_ui16_runId != ID_IDLE)
	{
		switch(g_ui16_runId)
		{
			case ID_STOP:	/* Application software stays idle */
			{
				/* add routine "Stop Mode" */
				g_ui16_activeId = ID_IDLE;
				g_ui16_runId = ID_IDLE;
			break;
			}

			case ID_RUN:
			{
				if(Flags.wait1Cycle == TRUE)	/* wait 1 cycle for sensor to be stable in the first run after starting program */
				{
					Flags.wait1Cycle = FALSE;				/* set first run flag to false */
					HAL_TIM_Base_Start_IT(&htim4);		/* start timer 4 for ADC sampling and ramp generating */
					g_ui16_activeId = ID_WAIT_CYCLE_END;	/* go to measurement cycle waiting stage */
					g_ui16_runId = ID_IDLE;
				}
				else
				{
					g_ui16_activeId = ID_SAMPLE;	/* go to ADC sampling stage and perform measurements */
					g_ui16_runId = ID_IDLE;
				}

			break;
			}

			case ID_FREQ_CALIB:		/* frequency auto calibration */
			{
				g_ui16_activeId = ID_IDLE;		/* set application software to be idle and wait until an interrupt occurs */
				g_ui16_runId = ID_IDLE;

				/* stop timer 4 for pausing generating FMCW ramp */
				HAL_TIM_Base_Stop_IT(&htim4);
				/* clear flags and reset timer counter for next run  */
				__HAL_TIM_CLEAR_FLAG(&htim4, TIM_SR_TIF|TIM_SR_UIF);
				__HAL_TIM_SET_COUNTER(&htim4, 0);
				/* Set SMR to wanted frequency for calibrating */
				HAL_Delay(10);
				if(Flags.calibStartStopFreq == 0)	/* start frequency calibration */
				{
					g_ad5662_set(Sensor.startFreqDigits);	/* set transmit frequency to start frequency for performing calibration */
				}
				else
				{
					g_ad5662_set(Sensor.stopFreqDigits);	/* set transmit frequency to stop frequency for performing calibration */
				}
				HAL_Delay(20);	/* wait 20ms for transmit frequency to be stable */


				/* start timer 2 for measuring transmitting frequency */
				__HAL_TIM_SET_COUNTER(&htim2, 0);	/* reset timer 2 counter */
				__HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_TIF|TIM_SR_UIF|TIM_SR_CC1IF|TIM_SR_CC1OF);	/* clear timer 2 flags */
				HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);		/* start detecting rising edge from SMR down-converted transmitting frequency signal */
				Counter.freqCalibIntvl = 0;		/* reset timing counter for next calibration */
				HAL_TIM_Base_Start_IT(&htim2);	/* start timer 2 for measuring SMR down-converted transmitting frequency */
				Counter.edgeCnt = 0;	/* reset number of detected edges */

				break;
			}

			default:
			{
			break;
			}
		}
	}
}

/**************************************************************************************
	function name:		g_ExecuteCommands
	purpose:			this functions manages the execution of the commands

	return value:		none
	input parameters:	none
	output parameters:	none

	author:				Sergej Gauerhof
	version:			V1.0
	last edit:			...
	last editor:		Duy Bien Le
	change:				...

**************************************************************************************/
void g_ExecuteCommands(void)
{
	if(g_ui16_activeId != ID_IDLE)
	{
		switch(g_ui16_activeId)
		{
			case ID_SAMPLE:
			{
				if(Counter.sampleCnt >= FFT_SIZE)	/* sampling is finished, proceed to ID_PROCESSING  */
				{
					g_ui16_activeId = ID_PROCESSING;	/* go to signal processing stage */
					g_ad5662_set(Sensor.startFreqDigits);	/* reset transmit frequency to start frequency for next ramp */
				}
			break;
			}

			case ID_PROCESSING:
			{
				g_SignalProcessing();	/* perform signal processing */
				g_ui16_activeId = ID_TX_RAWDATA;	/* go to data transmission stage */
			break;
			}

			case ID_TX_RAWDATA:
			{
				/* Re-configure GPIO_PIN_5 to blink LED */
				GPIO_InitStruct.Pin = GPIO_PIN_5;
				GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
				HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);	/* switch on LED to indicate transmission ongoing */
				g_send_measurement_data(FC);	/* transmit data to PC */
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);	/* switch off LED to indicate transmission is completed */

				/* Re-configure GPIO_PIN_5 for SPI */
			    GPIO_InitStruct.Pin = GPIO_PIN_5;
			    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			    GPIO_InitStruct.Pull = GPIO_NOPULL;
			    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
			    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

				g_ui16_activeId = ID_WAIT_CYCLE_END;	/* go waiting stage */
			break;
			}

			case ID_WAIT_CYCLE_END:		/* wait until measurement cycle ends */
			{
				if(Counter.systemCnt >= Counter.cycleCntMAX)	/* Go back to IDLE mode after completing measurement */
				{
					/* Reset counters to prepare for next measurement */
					Counter.sampleCnt = 0;
					Counter.systemCnt = 0;
					Sensor.currentFreqDigits = Sensor.startFreqDigits;
					g_ui16_activeId = ID_SAMPLE;	/* measurements are always performed*/

					/* check frequency calibration is activated */
					if(Flags.freqCalibration == TRUE)
					{
						Counter.freqCalibIntvl++;
						/* Check if it's time to perform frequency calibration */
						if(Counter.freqCalibIntvl == Threshold.freqCalibNbr)
						{
							Counter.freqCalibIntvl = 0;
							g_ui16_runId = ID_FREQ_CALIB;	/* carry out frequency calibration */
						}
					}
				}
			break;
			}

			default:
			{
			break;
			}
		}
	}
}
