/*********************************************************************************

        @file   Variable_OPS.c                                                              
        @author Paul Schlein, M. Eberhardt, Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU - knock detection
        @note   This file contains functions for Reading and "Smoothing" Operating Variables   
        @version  1.1
        @copyright 2011 P. Schlein, M. Eberhardt, J. Zeeff

*************************************************************************************/

/* 
Portions Copyright 2011 P. Schlein - MIT License
Portions Copyright 2011,2012  M. Eberhardt - MIT License
Portions Copyright 2011,2012  Jon Zeeff - All rights reserved
Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
*/

/*  General                                                                    */
#include <stdint.h>
#include "typedefs.h" /**< pickup vuint_xxx */
#include "config.h"
#include "variables.h"
#include "Variable_OPS.h"
#include "etpu_util.h"
#include "Table_Lookup_JZ.h"
#include "eQADC_OPS.h"
#include "eTPU_OPS.h"
#include "mpc563xm.h" //pickup EMIOS for the clock to work

/*  eTPU APIs                                                                  */
#include "etpu_toothgen.h"
#include "etpu_app_eng_pos.h"


/* Global Declarations  */
#   define MAX_AD_COUNTS  16384.                /* not 4096 as you might expect */
#   define MAX_AD_VOLTAGE 5.

#   define VBATT_VOLTAGE_DIVIDER (49.0/10.0)
#   define V_Batt_AD    ADC_RsltQ0[25]
#   define CLT_VOLTAGE_DIVIDER 1.0
#   define V_CLT_AD    ADC_RsltQ0[39]
#   define IAT_VOLTAGE_DIVIDER 1.0
#   define V_IAT_AD    ADC_RsltQ0[38]
#   define TPS_VOLTAGE_DIVIDER 1.0
#   define V_TPS_AD    ADC_RsltQ0[31]
#   define MAP_1_VOLTAGE_DIVIDER 1.0
#   define V_MAP_1_AD    ADC_RsltQ5 [0]       
#   define MAP_2_VOLTAGE_DIVIDER 1.0
#   define V_MAP_2_AD    ADC_RsltQ0 [18]	/* TODO */
#   define MAP_3_VOLTAGE_DIVIDER 1.0
#   define V_MAP_3_AD    ADC_RsltQ0 [23]
#   define P1_VOLTAGE_DIVIDER 1.0
#   define V_P1_AD    ADC_RsltQ0 [17]
#   define P2_VOLTAGE_DIVIDER 1.0
#   define V_P2_AD    ADC_RsltQ0 [31]
#   define P3_VOLTAGE_DIVIDER 1.0
#   define V_P3_AD    ADC_RsltQ0 [32]
#   define P4_VOLTAGE_DIVIDER 1.0
#   define V_P4_AD    ADC_RsltQ0 [33]
#   define P5_VOLTAGE_DIVIDER 1.0
#   define V_P5_AD    ADC_RsltQ0 [34]
#   define P6_VOLTAGE_DIVIDER 1.0
#   define V_P6_AD    ADC_RsltQ0 [35]
#   define P7_VOLTAGE_DIVIDER 1.0
#   define V_P7_AD    ADC_RsltQ0 [35]
#   define P8_VOLTAGE_DIVIDER 1.0
#   define V_P8_AD    ADC_RsltQ0 [35]
#   define P9_VOLTAGE_DIVIDER 1.0
#   define V_P9_AD    ADC_RsltQ0 [35]
#   define P10_VOLTAGE_DIVIDER 1.0
#   define V_P10_AD    ADC_RsltQ0 [35]
#   define P11_VOLTAGE_DIVIDER 1.0
#   define V_P11_AD    ADC_RsltQ0 [35]
#   define P12_VOLTAGE_DIVIDER 1.0
#   define V_P12_AD    ADC_RsltQ0 [35]
#   define P13_VOLTAGE_DIVIDER 1.0
#   define V_P13_AD    ADC_RsltQ0 [35]
#   define P14_VOLTAGE_DIVIDER 1.0
#   define V_P14_AD    ADC_RsltQ0 [35]
#   define O2_1_UA_VOLTAGE_DIVIDER 1.0
#   define V_O2_1_UA_AD    ADC_RsltQ0 [23]
#   define O2_1_UR_VOLTAGE_DIVIDER 1.0
#   define V_O2_1_UR_AD    ADC_RsltQ0 [24]
#   define O2_2_UA_VOLTAGE_DIVIDER 1.0
#   define V_O2_2_UA_AD    ADC_RsltQ0 [28]
#   define O2_2_UR_VOLTAGE_DIVIDER 1.0
#   define V_O2_2_UR_AD    ADC_RsltQ0 [30]
#   define Knock_1_VOLTAGE_DIVIDER 1.0
#   define V_Knock_1_AD    ADC_RsltQ0 [0]   // TODO
#   define Knock_2_VOLTAGE_DIVIDER 1.0
#   define V_Knock_2_AD    ADC_RsltQ0 [2]   // TODO

extern uint32_t etpu_a_tcr1_freq;       //Implicit Defn.in eTPU_OPS.c
extern uint32_t etpu_b_tcr1_freq;       //Implicit Defn.in eTPU_OPS.c

// filter a raw A/D value in ADC_RsltQ0 - done "in place"
// strength can be 1,2,3,.. for 1/2, 1/4, 1/8, etc
// WARNING: will not work on values in any other Q 
// WARNING: effect depends on how often you call it
static uint16_t prev_value[sizeof(ADC_RsltQ0) / sizeof(uint16_t)];

inline uint16_t
Filter_AD(vuint16_t * Value, uint16_t Strength)
{
register unsigned int index = (unsigned int)(Value - &ADC_RsltQ0[0]);   // 0 to 39 normally

// use, for example (strength = 3), 7/8 of old value and 1/8 of new value

// check that it is in the memory range of RsltQ0, otherwise do nothing
if (index < (unsigned int)(sizeof(ADC_RsltQ0) / sizeof(uint16_t)))
   prev_value[index] = *Value = (*Value + (prev_value[index] * ((1<<Strength) - 1))) >> Strength;

return prev_value[index];
}

//**********************************************************************************
// FUNCTION     : Get_Operational_Variables                                       
// PURPOSE      : This function Gets Operational Variables from the eQADCResult   
// INPUT NOTES  : none                                                            
// RETURN NOTES : None                                                            
// WARNING      : 
//**********************************************************************************

void Get_Slow_Op_Vars(void)
{

/* Slow stuff...10hz or so*/

    // Code for testing
    // Test_Enable allows real time variables to be set in TunerStudio to test code.

    if (Test_Enable == 1) {

        // speed doesn't matter here

        // Test_Value = 0 allows the actual value to be input bypassing reading the ADC and the table lookup
        if (Test_Value == 0) {
            CLT = Test_CLT;
            IAT = Test_IAT;
            MAP[0] = MAP[1] = Test_MAP_2;

        } else {

            // Test_Value = 1 allows values simulating the ADC to be input 
            V_CLT = Test_V_CLT;
            CLT = (int16_t) table_lookup_jz(V_CLT, 0, CLT_Table);

            V_IAT = Test_V_IAT;
            IAT = (int16_t) table_lookup_jz(V_IAT, 0, IAT_Table);

            V_MAP[0] = V_MAP[1] = Test_V_MAP_2;
            MAP[0] = MAP[1] = (int16_t) table_lookup_jz(V_MAP[1], 0, MAP_2_Table);

        } // if

        /* TODO: this needs to be selectively enabled, what happens when firmware is
           built without SIMULATOR defined in etpu_ops.c ? - this code shouldn't run in
           that case */
        /* might be best to alias this functionality under its own thread that is
           compiled out in one place instead of spreading this functionality
           across multiple files */
        // Updates toothgen with the desired test RPM
//#ifndef SIMULATOR
//        fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xEFFFFF, Test_RPM, etpu_tcr1_freq);
//#endif

    } else {                    // Run Mode, normal operation

        // coolant temperature
        Filter_AD(&V_CLT_AD,3);  // smooth by 8
        V_CLT = (int16_t) ((V_CLT_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * CLT_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);        // V_CLT is bin 12
        CLT = (int16_t) table_lookup_jz(V_CLT, 0, CLT_Table);

        // intake air temp
        Filter_AD(&V_IAT_AD,3);  // smooth by 8
        V_IAT = (int16_t) ((V_IAT_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * IAT_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);        // V_IAT is bin 12
        IAT = (int16_t) table_lookup_jz(V_IAT, 0, IAT_Table);

        // manifold absolute pressure - TODO - Mark, make it clear what the 3 are
        V_MAP[0] = (int16_t) ((V_MAP_1_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_1_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);        // V_MAP_1 is bin 12
        MAP[0] = (int16_t) table_lookup_jz(V_MAP[0], 0, MAP_1_Table);

        V_MAP[1] = (int16_t) ((V_MAP_2_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_2_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);        // V_MAP_2 is bin 12
        MAP[1] = (int16_t) table_lookup_jz(V_MAP[1], 0, MAP_2_Table);

        V_MAP[2] = (int16_t) ((V_MAP_3_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_3_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);        // V_MAP_3 is bin 12
        MAP[2] = (int16_t) table_lookup_jz(V_MAP[2], 0, MAP_3_Table);

        // O2 sensors - only for pass through to tuner
        V_O2_UA[0] = (int16_t) ((V_O2_1_UA_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * O2_1_UA_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);     // V_O2 is bin 12
        V_O2_UR[0] = (int16_t) ((V_O2_1_UR_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * O2_1_UR_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);     // V_O2 is bin 12
        AFR[0] = (int16_t) table_lookup_jz (V_O2_UA[0],0, AFR_1_Table); 	// convert volts to AFR
        V_O2_UA[1] = (int16_t) ((V_O2_2_UA_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * O2_2_UA_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);     // V_O2 is bin 12
        V_O2_UR[1] = (int16_t) ((V_O2_2_UR_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * O2_2_UR_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);     // V_O2 is bin 12
        AFR[1] = (int16_t) table_lookup_jz (V_O2_UA[0],0, AFR_2_Table); 	// convert volts to AFR

    }  // if normal run mode

}

// calc variables that need updating around every 10 msec

void Get_Fast_Op_Vars(void)
{
        //TS looks at "seconds" to know ift he OS is running....we're giving it msec but that will do
       seconds = (EMIOS.CH[MSEC_EMIOS_CHANNEL].CCNTR.R);
    // Code for testing
    // Test_Enable allows real time variables to be set in TunerStudio to test code

    if (Test_Enable == 1) {

        // Test_Value = 0 allows the actual value to be input bypassing reading the ADC and the table lookup       
        if (Test_Value == 0) {
            V_Batt = Test_V_Batt;
            RPM = Test_RPM_1;
            TPS = Test_TPS;
            MAP[0] = Test_MAP_1;

        } else {                // Test_Value = 1 allows values simulating the ADC to be input 
            V_Batt = Test_V_Batt;

            RPM = (int16_t) fs_etpu_eng_pos_get_engine_speed(etpu_a_tcr1_freq);   // Read RPM from eTPU

            // TODO: if using a double gap wheel, divide rpm by 2

            V_TPS = Test_V_TPS;
            V_TPS = (int16_t) ((V_TPS_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * TPS_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);       // V_TPS is bin 12
            TPS = (int16_t) table_lookup_jz(V_TPS, 0, TPS_Table);
            /* Angle based stuff */
            V_MAP[0] = Test_V_MAP_1;
            MAP[0] = (int16_t) table_lookup_jz(V_MAP[0], 0, MAP_1_Table);
        }

    } else {                    //Run Mode, normal operation

        /* On fast for now, but should be medium speed ...100hz or so */
        V_Batt = (int16_t) ((V_Batt_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * VBATT_VOLTAGE_DIVIDER) * (1 << 20))) >> 10);  // V_Batt is bin 10

        RPM = (int16_t) fs_etpu_eng_pos_get_engine_speed(etpu_a_tcr1_freq);       // Read RPM from eTPU

        /* Fast speed stuff...1000hz or so */
        V_TPS = (int16_t) ((V_TPS_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * TPS_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);       // V_TPS is bin 12
        TPS = (int16_t) table_lookup_jz(V_TPS, 0, TPS_Table);

        /* Angle based stuff */
        V_MAP[0] = (int16_t) ((V_MAP_1_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_1_VOLTAGE_DIVIDER) * (1 << 20))) >> 8);        // V_MAP is bin 12
        MAP[0] = (int16_t) table_lookup_jz(V_MAP[0], 0, MAP_1_Table);
        /* convert P1*/
        Pot_RPM = (int16_t) ((V_P1_AD * (uint32_t) (((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * P1_VOLTAGE_DIVIDER ) * (1 << 20))) >> 8);       // V_P1_AD is bin 12
        Pot_RPM=  (3000* Pot_RPM) >>12;
        
    }

}                               // Get_Fast_Op_Vars()
