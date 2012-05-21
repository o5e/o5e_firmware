/********************************************************************************************/
     
/* DESCRIPTION:                                                                             */ 
/* This file contains functions for the MPC5554 to access the Main_Table and calculate      */ 
/* working values form the 4 subwords in the 32bit table words                              */ 
/*                                                                                          */ 
/*==========================================================================================*/ 
/* REV      AUTHOR         DATE          DESCRIPTION OF CHANGE                              */ 
/* ---      -----------    ----------    ---------------------                              */ 
/* 1.1      P. Schlein     12/Sep/11     Four 8-bit [16][32] Tables                         */ 
/* 1.1      P. Schlein     4/Sep/11      Remove some global Variables                       */ 
/* 1.0      P. Schlein     7/July/11     Initial version                                    */ 
/********************************************************************************************/ 
    
// commented out until variables are fixed
     
#include "config.h"
#include "Table_OPS.h"
#include "variables.h"
     
#if 0
    
/********************************************************************************************/ 
/*                      Global Declarations                                                 */ 
/********************************************************************************************/ 
//   extern uint8_t MAP_value;                     //Implicit Defn.in Variable_OPS.c
//   extern uint32_t speed_rpm;                    //Implicit Defn.in Variable_OPS.c
extern const uint8_t Fuel_Table[16][32];       //Implicit Defn.in INPUT_OPS.c
extern const uint8_t Spark_Table[16][32];       //Implicit Defn.in INPUT_OPS.c
extern const uint8_t AFR_Table[16][32]; //Implicit Defn.in INPUT_OPS.c
extern const uint8_t Inj_End_Table[16][32];     //Implicit Defn.in INPUT_OPS.c
uint8_t RPM_value, RPM_index, MAP_index;
uint8_t Base_Pulsewidth, Spark_Advancex;
 
/********************************************************************************************/ 
/*                       C Code Functions                                                   */ 
/********************************************************************************************/ 
    
/********************************************************************************************/ 
/* FUNCTION     : Table_OPS                                                                 */ 
/* PURPOSE      : Read Main_Table and Interpolate for two 8bit variables                    */ 
/*                Note:  Variables need not be limited to 8 bits-see Masking in Interpolate */ 
/* INPUT NOTES  :                                                                           */ 
/* RETURN NOTES : In this version Engine_Model_Corr and Spark Advance are obained           */ 
/* WARNING      : None                                                                      */ 
/********************************************************************************************/ 
 void Table_OPS(void)
{
     
/*                                                                                          */ 
/*  First, the indices into the MAP rowed and RPM columned Main_Table                       */ 
/*  Shifts (1 machine cyle) are used instead of division (6-16 machine cyles)               */ 
        
/*  Ensure Table lower limits are observed                                                  */ 
        if (speed_rpm > 400)
        RPM_value = (speed_rpm / 25) - 9;
    
    else
        RPM_value = 7;
    if (MAP_value < 15)
        MAP_value = 15;
     
        /* Calculate Table Lookup Indices                                                          */ 
        RPM_index = (RPM_value - 7) >> 3;
    MAP_index = (MAP_value - 15) >> 4;
     
/* Interpolate                                                                              */ 
        Base_Pulsewidth = Interpolate(Fuel_Table);
     Spark_Advance = Interpolate(Spark_Table);
}

 
/********************************************************************************************/ 
/* FUNCTION     : Interpolate 2-D                                                           */ 
/* PURPOSE      :                                                                           */ 
/* INPUTS NOTES :                                                                           */ 
/* RETURNS NOTES: none.                                                                     */ 
/* WARNING      :                                                                           */ 
/********************************************************************************************/ 
uint8_t Interpolate(Table)  const uint8_t Table[16][32];

{
     uint8_t A, B, C, D;
    uint8_t Intrm_Calc1, Intrm_Calc2, Value;
     uint8_t MAP_diff = (MAP_value - 15) - 16 * MAP_index;
    uint8_t RPM_diff = (RPM_value - 7) - 8 * RPM_index;
     
/* A, B, C, and D are the values bracketing the (MAP_value,speed_rpm) point                 */ 
        A = Table[MAP_index][RPM_index];
    B = Table[MAP_index + 1][RPM_index];
    C = Table[MAP_index][RPM_index + 1];
    D = Table[MAP_index + 1][RPM_index + 1];
     
/* Perform Interpolation                                                                    */ 
        Intrm_Calc1 = (((B - A) * MAP_diff) >> 4) + A;
    Intrm_Calc2 = (((D - C) * MAP_diff) >> 4) + C;
    Value = (((Intrm_Calc2 - Intrm_Calc1) * RPM_diff) >> 3) + Intrm_Calc1;
     return (Value);
}

 
#endif  /*  */
