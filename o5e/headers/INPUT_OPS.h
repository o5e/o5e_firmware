/**************************************************************************/
/* FILE NAME: INPUT_OPS.h                                                 */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for the INPUT_OPS        */
/*                                                                        */
/*========================================================================*/
/*ORIGINAL AUTHOR:  Paul Schlein                                          */
/* REV      AUTHOR          DATE          DESCRIPTION OF CHANGE           */
/* ---      -----------     -----------   ---------------------           */
/* 2.1      P. Schlein      26/Sep/11    Add User Variables               */
/* 2.0      P. Schlein      12/Sep/11     Four 8-bit [16][32] Tables      */
/* 1.0      P. Schlein      2/July/11     Initial version.                */
/*                                                                        */
/**************************************************************************/

#include "cpu.h"

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

/**************************************************************************/

//User Inputs
#ifndef EXTERN
#define EXTERN extern
#endif

extern uint32_t Run_Threshold;
extern uint8_t Channel_cnt;
extern uint32_t Crank_Spark;
extern uint8_t One_Second_Crank_Fuel_Fctr;   
extern uint32_t Temp_Based_Crank_Fuel;
extern uint8_t Crank_Multiplier; 
	 
extern const uint8_t Fuel_Table[16][32];
extern const uint8_t Spark_Table[16][32];
extern const uint8_t AFR_Table[16][32];
extern const uint8_t Inj_End_Table[16][32];

#undef EXTERN

#ifdef  __cplusplus
}
#endif

