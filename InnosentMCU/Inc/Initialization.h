/**************************************************************************************

      II    N     N     N     N      OOOO      SSSSS     EEEEE    N     N    TTTTTTT
     II    NNN   N     NNN   N    OO    OO    S         E        NNN   N       T
    II    N NN  N     N NN  N    OO    OO    SSSSS     EEE      N NN  N       T
   II    N  NN N     N  NN N    OO    OO        S     E        N  NN N       T
  II    N    NN     N    NN      OOOO      SSSSS     EEEEE    N    NN       T
                         copyright (c) 2016, InnoSenT GmbH
                                 all rights reserved

***************************************************************************************

	filename:		Initialization.h
	brief:			initialize application
	creation:		19.01.2017
	author:			Sergej Gauerhof
	last editor:	.....


**************************************************************************************/


#ifndef INITIALIZATION_H_
#define INITIALIZATION_H_

/**************************************************************************************
   preprocessor includes
**************************************************************************************/
#include "stm32f4xx_hal.h"
#include "CONFIG.h"
#include "arm_math.h"
#include "tim.h"

/**************************************************************************************
   preprocessor defines
**************************************************************************************/
/**/
#define FFT_SIZE			(128u)	/* number of samples a measurement */
#define NBR_ADC_CHANNEL		(2)		/*	number of the ADC channels	*/
#define TX_DATA_LENGTH		(7+FFT_SIZE*8)	/*	length of the send data via UART to PC	*/

#define TRUE		(1)
#define FALSE		(0)
#define CW			(0)	/* CW mode modulation value */
#define FMCW		(1)	/* FMCW mode modulation value */





/**************************************************************************************
   global constants
**************************************************************************************/

/**************************************************************************************
   global variables
**************************************************************************************/
struct SENSOR_REGS {
	uint8_t		modulation;				/*	current modulation: CW or FMCW */
	uint16_t	sampleTime;				/*	application sample time */
	float32_t	startFreq;				/*	start frequency of the CW- or FMCW-modulation in MHz*/
	int32_t		dwnCnvStartFreq;		/*  down converted start frequency (factor 8192) */
	uint16_t	startFreqDigits;		/*	start frequency of the CW- or FMCW-modulation in digits for the VCO of the SMR	*/
	float32_t	stopFreq;				/*	stop frequency of the FMCW-modulation in MHz*/
	int32_t		dwnCnvStopFreq;			/*  down converted stop frequency (factor 8192) */
	uint16_t	stopFreqDigits;			/*	stop frequency of the FMCW-modulation in digits for the VCO of the SMR	*/
	int16_t		bandwidth;				/*	frequency sweep of the FMCW-modulation */
	uint16_t	bandwidthDigits;		/*	frequency sweep of the FMCW-modulation in digits */
	int16_t		freqStep;				/*	single frequency step of the FMCW-modulation	*/
	uint16_t	freqStepDigits;			/*	single frequency step of the FMCW-modulation in digits s*/
	uint16_t	currentFreqDigits;		/*	the current transmit frequency of the FMCW-Modulation	*/

	int32_t		measFreq;				/* measured frequency in Hz */
	int32_t		freqDiff;				/* frequency difference between measured frequency and wanted frequency */

};
extern struct SENSOR_REGS Sensor;

struct COUNTER_REGS {
	int32_t		systemCnt;				/* system counter */
	uint32_t	cycleCntMAX;			/* maximum cycle count	*/
	uint32_t	sampleCnt;				/* ADC sample counter */
	uint32_t	edgeCnt;				/* number of detected edges due to SMR-frequency  */
	uint32_t	freqCalibIntvl;			/* count timing interval for performing frequency calibration */
};
extern struct COUNTER_REGS Counter;

struct THRESHOLD_REGS {
	uint32_t	freqCalibNbr;			/* indicate when transmit frequency is read */
	int32_t		deltaFreq;				/* threshold for difference between transmit frequency and wanted frequency */
};
extern struct THRESHOLD_REGS Threshold;

struct UART_REGS {
	uint8_t 	txData[TX_DATA_LENGTH];			/* transmit data */
};
extern struct UART_REGS Uart;

struct FLAG_REGS {
	uint8_t		freqCalibration;		/* indicate whether frequency auto calibration process is used */
	uint8_t		wait1Cycle;				/* wait 1 cycle for transmit frequency to be stable */
	uint8_t		calibStartStopFreq;		/* indicate whether start or stop frequency will be calibrated. 0: start frequency, 1: stop frequency */
	uint8_t		freqInitCalib;			/* indicate whether initial calibration for transmitting frequencies is performed */
	uint8_t		startFreqCalibCmplt;	/* indicate whether initial calibration for start frequency completed */
	uint8_t		stopFreqCalibCmplt;		/* indicate whether initial calibration for stop frequency completed */
};
extern struct FLAG_REGS Flags;

extern int16_t I[FFT_SIZE];		/* I-signal buffer */
extern int16_t Q[FFT_SIZE];		/* Q-signal buffer */
extern float32_t IQ[2*FFT_SIZE];	/* I/Q interleave buffer */
extern int32_t	FFTmagnitude[FFT_SIZE];	/* FFT magnitude buffer */
extern int16_t rawI[FFT_SIZE];	/* original raw I signal buffer */
extern int16_t rawQ[FFT_SIZE];	/* original raw Q signal buffer */

/**************************************************************************************
   global function prototypes
**************************************************************************************/
void g_InitApplication(void);
void g_InitFreqCalib();
void g_freqAutoCalib();
void g_ActivateSMR(uint16_t enable);

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


#endif /* INITIALIZATION_H_ */
