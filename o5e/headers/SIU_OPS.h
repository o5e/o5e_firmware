/**************************************************************************/
/* FILE NAME: SIU_OPS.h                                                   */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for the MPC5554          */
/* initialization of  the System Integration Unit.                        */
/*========================================================================*/
/*                                                                        */
/* REV    AUTHOR         DATE          DESCRIPTION OF CHANGE              */
/* ---    -----------    -----------   ---------------------              */
/* 1.0    P. Schlein     11/June/11    Initial version.                   */
/*                                                                        */
/**************************************************************************/

#ifdef MPC5634
#include "mpc563xm.h"
#endif
#ifdef MPC5554
#include "mpc5500_usrccdcfg.h"
#include "mpc5554.h"
#endif

#define __PEGNU__

#ifdef  __cplusplus
extern "C" {
#endif

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/**************************************************************************/
/*                       C Code Prototypes                                */
/**************************************************************************/

/* This function calls for configuration of the SIU                       */
void init_SIU(void);

/**************************************************************************/

#ifdef  __cplusplus
}
#endif

