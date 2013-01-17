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

#ifndef   __variable_ops_h
#define   __variable_ops_h

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************/
/* FILE NAME: Variable_OPS.h                                              */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for Initializing OPENECU */
/*                                                                        */
/*========================================================================*/
/*ORIGINAL AUTHOR:  Paul Schlein                                          */
/* REV    AUTHOR        DATE          DESCRIPTION OF CHANGE               */
/* ---    -----------   -----------   ---------------------               */
/* 1.0    P. Schlein    12/Sep/11     Initial version.                    */
/**************************************************************************/

void Get_Slow_Op_Vars(void);
void Get_Fast_Op_Vars(void);
uint16_t Filter_AD(vuint16_t * Value, uint16_t Strength);

#ifdef __cplusplus
}
#endif

#endif // __variable_ops_h
