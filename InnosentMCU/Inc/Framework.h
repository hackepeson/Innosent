/**************************************************************************************

      II    N     N     N     N      OOOO      SSSSS     EEEEE    N     N    TTTTTTT
     II    NNN   N     NNN   N    OO    OO    S         E        NNN   N       T
    II    N NN  N     N NN  N    OO    OO    SSSSS     EEE      N NN  N       T
   II    N  NN N     N  NN N    OO    OO        S     E        N  NN N       T
  II    N    NN     N    NN      OOOO      SSSSS     EEEEE    N    NN       T
                         copyright (c) 2016, InnoSenT GmbH
                                 all rights reserved

***************************************************************************************

	filename:		Framework.h
	brief:			framework functionality
	creation:		19.01.2017
	author:			Sergej Gauerhof
	last editor:	.....


**************************************************************************************/


#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

/**************************************************************************************
   preprocessor includes
**************************************************************************************/

/**************************************************************************************
   preprocessor defines
**************************************************************************************/
typedef enum {
	ID_IDLE = 0,			/*	no command executing or pending */
	ID_STOP,				/*	sensor stops /is not in measurement mode */
	ID_RUN,					/*	sensor starts / is in measurement mode */
	ID_SAMPLE,				/*  sampling ADC data */
	ID_PROCESSING,			/*  process captured data */
	ID_TX_RAWDATA,			/*  transmit measurement data to PC */
	ID_WAIT_CYCLE_END,		/*  waiting till measurement cycle ends*/
	ID_FREQ_CALIB,
	/* add error codes before this element */
	ID_MAX
}active_id_t;


typedef enum {
	UART_OK,
	UART_ERROR_PROTOCOL_END_DELIMITER,
	UART_ERROR_PROTOCOL_START_DELIMITER,
	UART_ERROR_PROTOCOL_SOURCE_ADDRESS,
	UART_ERROR_PROTOCOL_DESTINATION_ADDRESS,
	UART_ERROR_PROTOCOL_NET_DATA_LENGTH,
	UART_ERROR_PROTOCOL_FRAME_CHECKSUM,
	UART_ERROR,
	UART_ERROR_END
}UART_Error_t;

/**************************************************************************************
   global constants
**************************************************************************************/

/**************************************************************************************
   global variables
**************************************************************************************/

/**************************************************************************************
   global function prototypes
**************************************************************************************/
void g_ActivateCommands(void);
void g_ExecuteCommands(void);

extern volatile active_id_t g_ui16_runId;		/* shows if new command available */
extern volatile active_id_t g_ui16_activeId;	/* shows running command*/

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


#endif /* FRAMEWORK_H_ */
