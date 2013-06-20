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
Portions Copyright 2012, Sean Stasiak <sstasiak at gmail dot com> - BSD 3 Clause License
*/

/*  General                                                                    */
#include <stdint.h>
#include "typedefs.h" /**< pickup vuint_xxx */
#include "config.h"
#include "variables.h"
#include "Variable_OPS.h"
#include "etpu_util.h"
#include "Table_Lookup.h"
#include "eQADC_OPS.h"
#include "eTPU_OPS.h"
#include "bsp.h" //pickup systime for the clock to work


/*  eTPU APIs                                                                  */
#include "etpu_toothgen.h"
#include "etpu_app_eng_pos.h"


/* Global Declarations  */
#   define MAX_AD_COUNTS  16384.0f                /* not 4096 as you might expect */
#   define MAX_AD_VOLTAGE 5.0f

#   define VBATT_VOLTAGE_DIVIDER 49.0f/10.0f
#   define V_Batt_AD    ADC_RsltQ0[25]
#   define CLT_VOLTAGE_DIVIDER 1.0f
#   define V_CLT_AD    ADC_RsltQ0[39]
#   define IAT_VOLTAGE_DIVIDER 1.0f
#   define V_IAT_AD    ADC_RsltQ0[38]
#   define TPS_VOLTAGE_DIVIDER 1.0f
#   define V_TPS_AD    ADC_RsltQ0[31]
#   define MAP_1_VOLTAGE_DIVIDER 1.0f
#   define V_MAP_1_AD    ADC_RsltQ5 [0]       
#   define MAP_2_VOLTAGE_DIVIDER 1.0f
#   define V_MAP_2_AD    ADC_RsltQ0 [23]	/* TODO */
#   define MAF_1_VOLTAGE_DIVIDER 1.0f
#   define V_MAF_1_AD    ADC_RsltQ0 [35]
#   define P1_VOLTAGE_DIVIDER 1.0f
#   define V_P1_AD    ADC_RsltQ0 [17]
#   define P2_VOLTAGE_DIVIDER 1.0f
#   define V_P2_AD    ADC_RsltQ0 [31]
#   define P3_VOLTAGE_DIVIDER 1.0f
#   define V_P3_AD    ADC_RsltQ0 [32]
#   define P4_VOLTAGE_DIVIDER 1.0f
#   define V_P4_AD    ADC_RsltQ0 [33]
#   define O2_1_VOLTAGE_DIVIDER 1.0f
#   define V_O2_1_AD    ADC_RsltQ0 [23]
#   define O2_2_VOLTAGE_DIVIDER 1.0f
#   define V_O2_2_AD    ADC_RsltQ0 [28]
#   define Knock_1_VOLTAGE_DIVIDER 1.0f
#   define V_Knock_1_AD    ADC_RsltQ0 [0]   // TODO
#   define Knock_2_VOLTAGE_DIVIDER 1.0f
#   define V_Knock_2_AD    ADC_RsltQ0 [2]   // TODO

float Ref_IAT;
float Ref_MAP;
float Ref_Baro;
float Ref_TPS;

extern uint32_t etpu_a_tcr1_freq;       //Implicit Defn.in eTPU_OPS.c
extern uint32_t etpu_b_tcr1_freq;       //Implicit Defn.in eTPU_OPS.c

int8_t crank_position_status;

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


        } else {

            // Test_Value = 1 allows values simulating the ADC to be input 
            V_CLT = Test_V_CLT;
            CLT = table_lookup(V_CLT, 0, CLT_Table);

            V_IAT = Test_V_IAT;
            IAT = table_lookup(V_IAT, 0, IAT_Table);


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
        V_CLT = (V_CLT_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * CLT_VOLTAGE_DIVIDER));
        CLT = table_lookup(V_CLT, 0, CLT_Table);

        // intake air temp
        Filter_AD(&V_IAT_AD,3);  // smooth by 8
        V_IAT = (V_IAT_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * IAT_VOLTAGE_DIVIDER));
        IAT = table_lookup(V_IAT, 0, IAT_Table);

        // manifold absolute pressure 

        //V_MAP[2] = (float)(V_MAP_3_AD ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_3_VOLTAGE_DIVIDER));
        //MAP[2] = table_lookup(V_MAP[2], 0, MAP_3_Table);

        // O2 sensors 
        V_O2[0] = (V_O2_1_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * O2_1_VOLTAGE_DIVIDER));
        Lambda[0] = table_lookup (V_O2[0],0, Lambda_1_Table); 	// convert volts to lambda
        V_O2[1] = (V_O2_2_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * O2_2_VOLTAGE_DIVIDER));
        Lambda[1] = table_lookup (V_O2[0],0, Lambda_2_Table); 	// convert volts to lambda

    }  // if normal run mode
    //Convert sensor reading to a form more easily used in the corrections code
	Ref_IAT = Reference_Temp / (IAT + 273.15f);
    IAT1 = (int32_t)(IAT * 4096);
    CLT1 = (int32_t)(CLT * 4096);
    Lambda1[0] = (int32_t)(Lambda[0] * 4096);
    Lambda1[1] = (int32_t)(Lambda[1] * 4096);
    

}

// calc variables that need updating around every 10 msec

void Get_Fast_Op_Vars(void)
{
	crank_position_status = fs_etpu_eng_pos_get_engine_position_status ();
	// = fs_etpu_eng_pos_get_crank_error_status();
	//TS looks at "seconds" to know ift he OS is running....we're giving it msec but that will do
	seconds = systime;//(EMIOS.CH[MSEC_EMIOS_CHANNEL].CCNTR.R);
	seconds1 = seconds1;
    // Code for testing
    // Test_Enable allows real time variables to be set in TunerStudio to test code

    if (Test_Enable == 1) {

        // Test_Value = 0 allows the actual value to be input bypassing reading the ADC and the table lookup       
        if (Test_Value == 0) {
            V_Batt = Test_V_Batt;
            RPM = Test_RPM_Array[0]; 
            TPS = Test_TPS;
            MAP[0] = Test_MAP_Array[0];
            MAP[1] = Test_MAP_Array[1];
            MAF[0] = Test_MAF_Array[0];

        } else {                // Test_Value = 1 allows values simulating the ADC to be input 
            V_Batt = Test_V_Batt;
			if (crank_position_status == 0) //if status = 0 the TCR2 clock in not valid so set rpm to 0
			    RPM = 0;
			else
				RPM =  (float)fs_etpu_eng_pos_get_engine_speed(etpu_a_tcr1_freq);   // Read RPM from eTPU


            V_TPS = Test_V_TPS;
            TPS = table_lookup(V_TPS, 0, TPS_Table);
            V_MAP[1] = Test_V_MAP_Array[1];
            MAP[1] = table_lookup(V_MAP[1], 0, MAP_2_Table);
            /* Angle based stuff */
            V_MAP[0] = Test_V_MAP_Array[0];
            MAP[0] = table_lookup(V_MAP[0], 0, MAP_1_Table);
        }

    } else {                    //Run Mode, normal operation

        /* On fast for now, but should be medium speed ...100hz or so */
        Filter_AD(&V_Batt_AD,3);  // smooth by 8
        V_Batt = (V_Batt_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * VBATT_VOLTAGE_DIVIDER));
		if(crank_position_status == 0) //if status = 0 the TCR2 clock in not valid so set rpm to 0
			RPM = 0;
		else
        	RPM = (float)fs_etpu_eng_pos_get_engine_speed(etpu_a_tcr1_freq);       // Read RPM from eTPU

        /* Fast speed stuff...1000hz or so */
        Filter_AD(&V_TPS_AD,3);  // smooth by 8
        V_TPS = (V_TPS_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * TPS_VOLTAGE_DIVIDER));
        TPS = table_lookup(V_TPS, 0, TPS_Table);
        
        Filter_AD(&V_MAP_2_AD,3);  // smooth by 8
        V_MAP[1] = (V_MAP_2_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_2_VOLTAGE_DIVIDER));
        MAP[1] = table_lookup(V_MAP[1], 0, MAP_2_Table);
        
        Filter_AD(&V_MAF_1_AD,3);  // smooth by 8
        V_MAF[0] = (V_MAF_1_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAF_1_VOLTAGE_DIVIDER));
        MAF[0] = table_lookup(V_MAF[0], 0, MAF_1_Table);

        /* Angle based stuff */
        Filter_AD(&V_MAP_1_AD,3);  // smooth by 8
        V_MAP[0] = (V_MAP_1_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * MAP_1_VOLTAGE_DIVIDER));
        MAP[0] = table_lookup(V_MAP[0], 0, MAP_1_Table);
        
        
        /* convert P1*/
        Filter_AD(&V_P1_AD,3);  // smooth by 8
        Pot_RPM = (V_P1_AD * ((MAX_AD_VOLTAGE / MAX_AD_COUNTS) * P1_VOLTAGE_DIVIDER ) ); 
        Pot_RPM=  3000* Pot_RPM;
        
    }
    
    Ref_MAP = MAP[0] * Inv_Ref_Pres;
    Ref_Baro = MAP[2] *  Inv_Ref_Pres;
    Ref_TPS = TPS * Inverse100;
    RPM1 = (int32_t)(RPM * 4096);
    TPS1 = (int32_t)(TPS * 4096);
    MAP1[0] = (int32_t)(MAP[0] * 4096);
    MAP1[1] = (int32_t)(MAP[1] * 4096);

}                               // Get_Fast_Op_Vars()
