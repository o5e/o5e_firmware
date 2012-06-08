/**************************************************************************/
/* FILE NAME: eTPU_OPS.h                                                  */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for Initializing OPENECU */
/*                                                                        */
/*========================================================================*/
/* ORIGINAL AUTHOR:  Luca Colombini                                       */
/* REV    AUTHOR        DATE          DESCRIPTION OF CHANGE               */
/* ---    -----------   -----------   ---------------------               */
/* 1.2jz  J. Zeeff      16/May/12     Cyl_Angle size increase       	  */
/* 1.1    J. Zeeff      05/Jan/12     Added Spark and Fuel channels       */
/* 1.0    L. Colombini  02/Jan/12     Initial version.                    */
/**************************************************************************/

#include "cpu.h"
#include "system.h"

#ifndef EXTERN
#define NOINIT
#define EXTERN extern
#endif
    #define Total_Teeth (N_Teeth + Missing_Teeth) 
    #define Degrees_Per_Tooth_x100 (36000 / Total_Teeth)
    #define Start_Tooth (1 + Total_Teeth - (((uint32_t)Cam_Lobe_Pos << 2) / Degrees_Per_Tooth_x100))  // adjust x100 bin -2 value to x100 bin 0 before using
// Functions

int32_t init_eTPU(void);
void init_PWM1(uint32_t frequency);
void update_PWM1(uint32_t duty_cycle);
void Init_Tach(void);
void Update_Tach(uint32_t frequency);
void Check_Engine(void);

// Variables

EXTERN uint8_t N_Injectors;
EXTERN uint8_t N_Coils;
EXTERN uint32_t N_MAP_windows;
EXTERN uint32_t MAP_Angles[8*2];      //  eTPU knock window limit is 8
EXTERN uint24_t Cyl_Angle_eTPU[24];

// What eTPU channel to use for each coil in an array form
EXTERN uint8_t Spark_Channels[12]
#ifdef NOINIT
;
#else
 = { SPARK_CHANNELS_1_6, SPARK_CHANNELS_7_12 } ;
#endif

// What eTPU channel to use for each injector in an array form
EXTERN uint8_t Fuel_Channels[24] 
#ifdef NOINIT
;
#else
= { FUEL_CHANNELS_1_6, FUEL_CHANNELS_7_12, FUEL_CHANNELS_13_18, FUEL_CHANNELS_19_24 };
#endif

#undef EXTERN   
#undef NOINIT
