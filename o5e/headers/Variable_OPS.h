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

extern uint32_t RPM_In;
extern uint8_t Density_Corr, Vbatt_Corr;

void Get_Op_Vars(void);
void Get_Slow_Op_Vars(void);
void Get_Fast_Op_Vars(void);
uint16_t Filter_AD(vuint16_t * Value, uint16_t Strength);

