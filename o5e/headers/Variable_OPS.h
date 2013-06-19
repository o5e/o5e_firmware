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

#define Reference_Temp 293.15f  //use 293.15K (20C) as our refence state
#define Inv_Ref_Pres 1.0f/100.0f  //use 100kpa as a reference and invert to allow multiplication i real time code
#define Inverse100 1.0f/100.0f // conversion for getting 100 base % stuff into decimal based correction factor
#define Inverse1000 1.0f/1000.0f //Converion for usec to msec

extern float Ref_IAT;
extern float Ref_MAP;
extern float Ref_Baro;
extern float Ref_TPS;

void Get_Slow_Op_Vars(void);
void Get_Fast_Op_Vars(void);
uint16_t Filter_AD(vuint16_t * Value, uint16_t Strength);

#ifdef __cplusplus
}
#endif

#endif // __variable_ops_h
