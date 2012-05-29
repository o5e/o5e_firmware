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

extern const uint8_t Spark_Table[16][32];       //Implicit Defn.in INPUT_OPS.c
extern const uint8_t AFR_Table[16][32]; //Implicit Defn.in INPUT_OPS.c
extern const uint8_t Inj_End_Table[16][32];     //Implicit Defn.in INPUT_OPS.c



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

{
    
/*                                                                                          */ 
/*  First, the indices into the MAP rowed and RPM columned Main_Table                       */ 
/*  Shifts (1 machine cyle) are used instead of division (6-16 machine cyles)               */ 
        
/*  Ensure Table lower limits are observed                                                  */ 
        if (speed_rpm > 400)
        RPM_value = (speed_rpm / 25) - 9;
    
    else
        RPM_value = 7;
    
        MAP_value = 15;
    
        /* Calculate Table Lookup Indices                                                          */ 
        RPM_index = (RPM_value - 7) >> 3;
    
    
/* Interpolate                                                                              */ 
        Base_Pulsewidth = Interpolate(Fuel_Table);
    



/********************************************************************************************/ 
/* FUNCTION     : Interpolate 2-D                                                           */ 
/* PURPOSE      :                                                                           */ 
/* INPUTS NOTES :                                                                           */ 
/* RETURNS NOTES: none.                                                                     */ 
/* WARNING      :                                                                           */ 
/********************************************************************************************/ 


{
    
    
    
    
    
/* A, B, C, and D are the values bracketing the (MAP_value,speed_rpm) point                 */ 
        A = Table[MAP_index][RPM_index];
    
    
    
    
/* Perform Interpolation                                                                    */ 
        Intrm_Calc1 = (((B - A) * MAP_diff) >> 4) + A;
    
    
    



#endif  /* 