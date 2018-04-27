/**************************************************************************************

      II    N     N     N     N      OOOO      SSSSS     EEEEE    N     N    TTTTTTT
     II    NNN   N     NNN   N    OO    OO    S         E        NNN   N       T
    II    N NN  N     N NN  N    OO    OO    SSSSS     EEE      N NN  N       T
   II    N  NN N     N  NN N    OO    OO        S     E        N  NN N       T
  II    N    NN     N    NN      OOOO      SSSSS     EEEEE    N    NN       T
                         copyright (c) 2016, InnoSenT GmbH
                                 all rights reserved

***************************************************************************************

	filename:		Protocol.c
	brief:			communication protocol functionality
	creation:		20.01.2017
	author:			Sergej Gauerhof
	last editor:	.....


**************************************************************************************/

/**************************************************************************************
   preprocessor includes
**************************************************************************************/
#include "Protocol.h"
#include "usart.h"
#include "Initialization.h"
/**************************************************************************************
   preprocessor defines
**************************************************************************************/

/**************************************************************************************
   global constants
**************************************************************************************/

/**************************************************************************************
   global variables
**************************************************************************************/

/**************************************************************************************
   global function prototypes
**************************************************************************************/


/*************************************************************************************
	function name:		g_send_measurement_data
	purpose:			this function returns measurement data to host

	return value:		none
	input parameters:	function code, returned data1, returned data2
	output parameters:	none

	author:				Duy Bien Le
	creation:			14.12.2016
**************************************************************************************/
void g_send_measurement_data(uint8_t fc)
{
	int32_t fcs;
	uint16_t offsetIndex = 0;

	fcs = 0;
	Uart.txData[0] = SD;	/* start delimiter */
	Uart.txData[1] = fc;		/* function code */
	fcs += Uart.txData[1];
	Uart.txData[2] = Sensor.modulation;	/* sensor modulation */
	fcs += Uart.txData[2];
	Uart.txData[3] = (FFT_SIZE >> 8) & 0xFF;	/* signal length */
	fcs += Uart.txData[3];
	Uart.txData[4] = (FFT_SIZE >> 0) & 0xFF;
	fcs += Uart.txData[4];
	offsetIndex = 5;

	/* return raw I */
	for(uint16_t i=0; i<FFT_SIZE; i++)
	{
		Uart.txData[offsetIndex+2*i] = ((rawI[i]) >> 8) & 0x00FF;
		fcs += Uart.txData[offsetIndex+2*i];
		Uart.txData[offsetIndex+2*i+1] = ((rawI[i]) >> 0) & 0x00FF;
		fcs += Uart.txData[offsetIndex+2*i+1];
	}

	/* return raw Q */
	offsetIndex = 5 + 2*FFT_SIZE;
	for(uint16_t i=0; i<FFT_SIZE; i++)
	{
		Uart.txData[offsetIndex+2*i] = ((rawQ[i]) >> 8) & 0x00FF;
		fcs += Uart.txData[offsetIndex+2*i];
		Uart.txData[offsetIndex+2*i+1] = ((rawQ[i]) >> 0) & 0x00FF;
		fcs += Uart.txData[offsetIndex+2*i+1];
	}

	/* return FFT-mag */
	offsetIndex = 5 + 4*FFT_SIZE;
	for(uint16_t i=0; i<FFT_SIZE; i++)
	{
		Uart.txData[offsetIndex+4*i] = ((FFTmagnitude[i]) >> 24) & 0x00FF;
		fcs += Uart.txData[offsetIndex+4*i];
		Uart.txData[offsetIndex+4*i+1] = ((FFTmagnitude[i]) >> 16) & 0x00FF;
		fcs += Uart.txData[offsetIndex+4*i+1];
		Uart.txData[offsetIndex+4*i+2] = ((FFTmagnitude[i]) >> 8) & 0x00FF;
		fcs += Uart.txData[offsetIndex+4*i+2];
		Uart.txData[offsetIndex+4*i+3] = ((FFTmagnitude[i]) >> 0) & 0x00FF;
		fcs += Uart.txData[offsetIndex+4*i+3];
}

	offsetIndex = 5 + 8*FFT_SIZE;

	fcs &= 0x000000FF;		/* Checksum: only take 8 LSB bits of check sum */
	Uart.txData[offsetIndex] = fcs;
	Uart.txData[offsetIndex+1] = ED;
	HAL_UART_Transmit(&huart2, (uint8_t *)Uart.txData, TX_DATA_LENGTH, 200);

}
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
