/**************************************************************************/
/* FILE NAME: Table_OPS.h                                                 */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for Table_OPS.c          */
/*========================================================================*/
/* REV      AUTHOR         DATE          DESCRIPTION OF CHANGE            */
/* ---      -----------    -----------   ---------------------            */
/* 1.1      P. Schlein     12/Sep/10     Four 8-bit [16][32] Tables       */
/* 1.1      P. Schlein     4/Sep/10      Mod for Parameter Passing        */
/* 1.0      P. Schlein     7/July/10     Initial version.                 */
/**************************************************************************/

// commented out until variables are fixed

#ifdef MPC5634
#include "mpc563xm.h"
#endif
#ifdef MPC5554
#include "mpc5554.h"
#endif

// #define __PEGNU__

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/


/**************************************************************************/
/*                       C Code Prototypes                                */
/**************************************************************************/

extern uint8_t RPM_value, RPM_index, MAP_index;
extern uint8_t Base_Pulsewidth, Spark_Advancex;


/* This function calls for Reading and Interpolation of Main_Table        */

void Table_OPS(void);

/* This function performs the Interpolation                                */

uint8_t Interpolate (const uint8_t Table[16][32]);

/**************************************************************************/


