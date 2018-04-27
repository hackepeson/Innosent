/**************************************************************************************

      II    N     N     N     N      OOOO      SSSSS     EEEEE    N     N    TTTTTTT
     II    NNN   N     NNN   N    OO    OO    S         E        NNN   N       T
    II    N NN  N     N NN  N    OO    OO    SSSSS     EEE      N NN  N       T
   II    N  NN N     N  NN N    OO    OO        S     E        N  NN N       T
  II    N    NN     N    NN      OOOO      SSSSS     EEEEE    N    NN       T
                         copyright (c) 2011, InnoSenT GmbH
                                 all rights reserved

***************************************************************************************

	device:			AD5662
	filename:		ad5662.c
	brief:			16-bit digital to analog converter
	creation:		31.07.2014
	author:			Thomas Popp
	version:		v1.0
	last edit:		.....
	last editor:	.....
	change:			.....

**************************************************************************************/

/**************************************************************************************
   preprocessor includes
**************************************************************************************/
//#include <DAVE3.h>						/* Declarations from DAVE3 Code Generation	*/
//#include "InnoSenT_guidelines.h"
//#include "iSYS4001_spi.h"
//#include "spi_tranceive.h"
#include "ad5662.h"
#include "spi.h"

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
	function name:		g_ad5662_set
	purpose:			sets the dac value

	return value:		none
	input parameters:	data in digits
	output parameters:	none

	creation:			19.01.2017
	author:				Sergej Gauerhof
	version:			V1.0
	last edit:			...
	last editor:		...
	change:				...

**************************************************************************************/
void g_ad5662_set(uint16_t ui16_data)
{
/* register	DB15-DB0	data bits			0 ... 65535							*/

	(void)g_Spi1Tranceive(ui16_data, LENGTH_24BIT, CS_AD5662);
}


