/**
 * @file   <filename.h>
 * @author <author>
 * @brief  <one liner description>
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __esci_ops_h
#define   __esci_ops_h

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************/
/* FILE NAME: eSCI_OPS.h                                                  */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for Initializing OPENECU */
/*                                                                        */
/*========================================================================*/
/*ORIGINAL AUTHOR:  Paul Schlein                                          */
/* REV    AUTHOR        DATE          DESCRIPTION OF CHANGE               */
/* ---    -----------   -----------   ---------------------               */
/* 1.0    P. Schlein    22/Sep/11     Initial version.                    */
/**************************************************************************/

/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/

/**************************************************************************/

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

void init_eSCI(void);

/**************************************************************************/
/*                       C Code Prototypes                                */
/**************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // __esci_ops_h