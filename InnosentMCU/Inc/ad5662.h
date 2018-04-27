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

#ifndef INCLUSION_GUARD_AD5662_H
#define INCLUSION_GUARD_AD5662_H

/**************************************************************************************
   preprocessor includes
**************************************************************************************/
//#include "InnoSenT_guidelines.h"
#include "stm32f4xx_hal.h"

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
void g_ad5662_set(uint16_t ui16_data);

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

#endif /* INCLUSION_GUARD_AD5662_H */

