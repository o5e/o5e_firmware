/*********************************************************************************

    @file      Engine_OPS.c                                                              
    @date      December, 2011
    @brief     Open5xxxECU - this file contains functions for fuel pulse width 
               as well as spark timing 
    @note      www.Open5xxxECU.org
    @version   2.2
    @copyright 2011, 2012, 2013 - P. Schlein, M. Eberhardt, J. Zeeff

**********************************************************************************/

// Portions Copyright 2011 P. Schlein - BSD 3 clause License
// Portions Copyright 2011,2012, 2013 M. Eberhardt - BSD 3 clause License
// Portions Copyright 2011, 2012  Jon Zeeff - All rights reserved

#include <stdint.h>
//#include <math.h>
#include "mpc563xm.h"
#include "config.h"
#include "err.h"
#include "variables.h"
#include "typedefs.h"
#include "Engine_OPS.h"
#include "Enrichment_OPS.h"
#include "Variable_OPS.h"
#include "cocoos.h"
#include "Table_Lookup.h"
#include "etpu_util.h"
#include "etpu_spark.h"
#include "etpu_fuel.h"
#include "etpu_pwm.h"
#include "etpu_fpm.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "Load_OPS.h"
#include "Base_Values_OPS.h"



uint32_t *fs_free_param;
float Pulse_Width;
uint32_t etpu_Pulse_Width;
//uint32_t Injector_Flow;
static void Check_Engine(void);

static void Set_Spark(void);
static void Set_Fuel(void);


#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/* the above is inserted until I can figure out how this code works
   and fix it properly */


//
// Engine stuff that don't change very fast ~100 msec

void Slow_Vars_Task(void)
{
    task_open();                // standard OS entry - NOTE: no non-static local variables!
    task_wait(1);

    for (;;) {
        Get_Slow_Op_Vars();     // Read the slow reacting sensors like CLT, IAT, Barometer, ect
        task_wait(103);         // allow others tasks to run
    }
    task_close();
}                               /* end of slow_vars_task() */



// Decide if fuel pump should be on or off

void Fuel_Pump_Task(void)
{
    task_open();                // standard OS entry - NOTE: no non-static local variables! 
    task_wait(1);

    Set_Pin(FUEL_PUMP_PIN, 1);  // Prime fuel system, Fuel Pump ON 
    task_wait(Fuel_Pump_Prime_Time); //          // allow others tasks to run during pump prime time

    for (;;) {
        if (RPM == 0)
            Set_Pin(FUEL_PUMP_PIN, 0);  // turn pump off if engine not turning 
        else
            Set_Pin(FUEL_PUMP_PIN, 1);  // Engine is turning so turn Fuel Pump ON 

        task_wait(500);         // allow others tasks to run then come back to see what the pump should be doing
    }
    task_close();
}                               /* end of fuel_pump_task() */

/**********************************************************************************/
/*  Fuel and Spark task - run every 10msec 
/**********************************************************************************/



#define Delta_V_Crank 2.0f
#define Run_Threshold 250.0f      		// RPM below this then not running
#define Warmup_Threshold 10000.0f  		// let warmup stuff go 10k cycles for now.  todo  - change this to end when the correction is zero


				// todo real number maybe used variable


void Engine10_Task(void)
{
    task_open();
    task_wait(1);
    

	static float V_Battery_Stored;
	
	

	//read sensors to get basline values
	Get_Slow_Op_Vars();
	Get_Fast_Op_Vars();
	Get_Base_Pulse_Width();
	

	//store a baseline Battery Voltage
	V_Battery_Stored = V_Batt;

    for (;;) {    
        // Read the sensors that can change quickly like RPM, TPS, MAP, ect
        Get_Fast_Op_Vars();
        // go calculate the %Reference VE that should be used for current conditions
        Get_Reference_VE();

        // set spark advance and dwell based on current conditions
        Set_Spark();

        // set fuel pulse width + position based on current conditions
        Set_Fuel();

//
//MOVE THIS
//
        //Update_Tach(RPM)
        // Update Tach signal
        uint32_t frequency = (RPM * Pulses_Per_Rev) / 60 ;
           // maybe there should be 1 Update_eTPU() ???
        //Update_Tach(frequency);
		fs_etpu_pwm_update(TACH_CHANNEL, frequency, 1000, etpu_tcr1_freq);

		
        
        task_wait(9);           // allow others tasks to run
    }                           // for      
    task_close();
}                               // Engine10_Task()
/***************************************************************************************/
// All spark calcs go here

static void Set_Spark()
{
    static uint32_t Spark_Advance_eTPU;
    static uint32_t Spark_Recalc_Angle_eTPU;
    static uint32_t Spark_Advance_eTPU_2;
    int i;


       
        // Looks up the desired spark advance in degrees before Top Dead Center (TDC)
        Spark_Advance = table_lookup(RPM, Reference_VE, Spark_Advance_Table);
        
        // TODO Knock_Retard(); Issue #7
        // TODO  Air Temp retard                

        Spark_Advance_eTPU = 72000 - ((int24_t)(Spark_Advance *100));
        Spark_Advance_eTPU_2 = Spark_Advance_eTPU + 36000; // needed for waste spark, harmless otherwise
          if (Spark_Advance_eTPU_2 >= 72000) // roll it over at 720 degrees
              Spark_Advance_eTPU_2 -= 72000;
          

        if (Spark_Advance_eTPU < (72000 - 6000) && Spark_Advance_eTPU > 2000) {      // error checking, -60 to +20 is OK
              err_push( CODE_OLDJUNK_E3 );
              Spark_Advance_eTPU = 0;
        }
     
        // Calculate an appropriate re-calculation angle for the current Spark_Angle so the update is as close to firing time as possible
        float Angle_Temp = RPM *  0.006;        // convert rpm to angle/msec
              Angle_Temp = Angle_Temp * Dwell * 1.2;  // 1.2 is to give the processor time to do the math 
              Angle_Temp = 720.0 - Angle_Temp;

        Spark_Recalc_Angle_eTPU = (Spark_Advance_eTPU + ((uint32_t)(Angle_Temp * 100)));//convert to deg*100 for etpu
           if (Spark_Recalc_Angle_eTPU >= 72000) // roll it over at 720 degrees
               Spark_Recalc_Angle_eTPU -= 72000;
          //Update re-calculation angle in eTPU
        fs_etpu_spark_set_recalc_offset_angle(Spark_Channels[0], Spark_Recalc_Angle_eTPU); // global value despite the channel param

        // Dwell
           Dwell = (table_lookup(V_Batt, 1, Dwell_Table)) * Inverse100;  //
           Dwell = Dwell_Set * (1+ Dwell);
           
             //the engine position is not known, of over rev limit, shut off the spark
              if (Enable_Ignition == 0 //spark disabled
                 || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC //crank position unknow
                 || (RPM > Rev_Limit && (Rev_Limit_Type == 2 || Rev_Limit_Type == 4))) //rev limit engaged
                    //Turn spark off
                    Dwell = 0;



        if (Dwell > 15 || Dwell < 0.5) {               // error checking
              err_push( CODE_OLDJUNK_E2 );
              Dwell = 3.0;
        }
    // convert to int type and send values to eTPU
    uint24_t Dwell_etpu = (uint24_t)(Dwell *1000);//the user sees msec, the etpu wants usec
    uint24_t Dwell_etpu_2 = Dwell_etpu * Ignition_Type; //used for waste spark, set to zero otherwise
    
    for (i = 0; i < N_Coils; ++i) {
        fs_etpu_spark_set_end_angles(Spark_Channels[i], Spark_Advance_eTPU, Spark_Advance_eTPU_2);
        
        fs_etpu_spark_set_dwell_times(Spark_Channels[i], Dwell_etpu, Dwell_etpu_2);
    }                           // for


} // Set_Spark()

//TODO - add to ini for setting in TS.  Issue #11
#define CRANK_VOLTAGE 11
//#define Run_Threshold 250       // RPM below this then not running



// Primary purpose is to set the fuel pulse width/injection time

static void Set_Fuel(void)
{
    static float Corr;
    static uint32_t error_code;
    static float Dead_Time;
    static uint32_t Dead_Time_etpu;

    static uint32_t Load_Ref_AFR;

    // if the engine is not turning, the engine position is not known, or over reving, shut off the fuel channels




        // calc fuel pulse width
        //Set to a base value
        Pulse_Width = Base_Pulse_Width;


        // apply various multiplier adjustments


        // Reference_VE correction - assumes fuel required is roughly proportional to Reference_VE
        Pulse_Width = Pulse_Width * Reference_VE * Inverse100;


        // Main fuel table correction - this is used to adjust for RPM effects
        Corr = table_lookup(RPM, Reference_VE, Inj_Time_Corr_Table);
        Corr = 1.0f + (Corr * Inverse100);
        Pulse_Width = Pulse_Width * Corr;


        // Coolant temp correction from enrichment_ops
        if (Enable_Coolant_Temp_Corr == 1){
           Fuel_Temp_Corr = table_lookup(CLT, 1, Fuel_Temp_Corr_Table);
           Fuel_Temp_Corr = 1.0f + (Fuel_Temp_Corr * Inverse100);
           Pulse_Width = Pulse_Width * Fuel_Temp_Corr;
        }

                // Coolant temp correction from enrichment_ops
        if (Enable_Air_Temp_Corr == 1){
           Air_Temp_Fuel_Corr = table_lookup(IAT, 1, IAT_Fuel_Corr_Table);
           Air_Temp_Fuel_Corr = 1.0f + (Air_Temp_Fuel_Corr * Inverse100);
           Pulse_Width = Pulse_Width * Air_Temp_Fuel_Corr;
        }
                
        // Prime/warmup correction
        Get_Prime_Corr();
        Pulse_Width = Pulse_Width * Prime_Corr;
         
        // Acel/decel correction
        Get_Accel_Decel_Corr();
        
        Pulse_Width = Pulse_Width * Accel_Decel_Corr;

                 
        // TODO adjust based on O2 sensor data Issue #8
        // Corr = O2_Fuel();
        // Pulse_Width = (Pulse_Width * Corr) >> 14;
        
        // Assume fuel pressure is constant

        // fuel dead time - extra pulse needed to open the injector
        // take user value and adjust based on battery voltage
        Dead_Time = table_lookup(V_Batt, 1, Inj_Dead_Time_Table);
        Dead_Time = Dead_Time_Set *  (1+ (Dead_Time * Inverse100));     
        Dead_Time_etpu = (uint32_t)(Dead_Time * 1000);//etpu wants usec
         
         //this give the tuner the current pulse width
       Injection_Time = (Pulse_Width + Dead_Time);
        
        
        // TODO - add code for semi-seq fuel
       
        // Sanity check - greater than 99% duty cycle?
        if (Injection_Time > ((990000 * 60 * 2) / RPM)) {
            err_push( CODE_OLDJUNK_E1 );
        }

        // Fuel pulse width calc is done convert to etpu type and to usec
        etpu_Pulse_Width = (uint32_t) (Pulse_Width * 1000);
        
//Injection_angle()
        // where should pulse end (injection timing)?
         uint24_t Inj_End_Angle_eTPU =(uint24_t) (100 * (table_lookup(RPM, Reference_VE, Inj_End_Angle_Table)));//etpu needs deg * 100

		//I'm not sure where this came from but I think it's wrong - me 7/24/2013
        //if (Inj_End_Angle_eTPU >= Drop_Dead_Angle )            // clip to 1 degree before Drop_Dead
        //    Inj_End_Angle_eTPU = (Drop_Dead_Angle ) - (1 * 100);

        // Calculate angles for eTPU use, must reference the missing tooth not TDC

        // Calculate an approprite re-calculation angle for the current injection Inj_End_Angle 
        float Angle_Temp = RPM * 0.006; //convert to deg/msec
        	  Angle_Temp = Angle_Temp * Pulse_Width;
              Angle_Temp = 720 - Angle_Temp ;
        uint24_t Fuel_Recalc_Angle_eTPU = (Inj_End_Angle_eTPU + ((uint24_t)Angle_Temp * 100));
        if (Fuel_Recalc_Angle_eTPU > 72000) //roll over at 720 degrees
            Fuel_Recalc_Angle_eTPU -= 72000;
 

        // TODO - Cylinder Trim math and updates.  Issue #9 
        // TODO - Staged injection math and updates Issue #10

        // tell eTPU to use new fuel injection pulse values (same for all cylinders)
        uint32_t j;
        for (j = 0; j < N_Injectors; ++j) {
            fs_etpu_fuel_switch_on(Fuel_Channels[j]);   // Turn on fuel channels

            error_code = 1;
            while (error_code != 0)     // This tries until the channel is actually updated
     //look up trim values
         //need to make table use the "j" value before it will work
            //Corr = table_lookup(RPM, Reference_VE, Cyl_Trim_1_Table);
            //Cyl_Pulse_Width=  (Pulse_Width * Corr) >> 14;
           
                //error_code = fs_etpu_fuel_set_injection_time(Fuel_Channels[j], Cyl_Pulse_Width);
                //this goes away once cyl trim is working
                error_code = fs_etpu_fuel_set_injection_time(Fuel_Channels[j], etpu_Pulse_Width);

        }                       // for

        // These are global across channels, so only do it once
        fs_etpu_fuel_set_compensation_time(Fuel_Channels[0], Dead_Time_etpu);
        fs_etpu_fuel_set_normal_end_angle(Fuel_Channels[0], Inj_End_Angle_eTPU);        // degrees * 100
        fs_etpu_fuel_set_recalc_offset_angle(Fuel_Channels[0], Fuel_Recalc_Angle_eTPU); // degrees * 100
    
   
        if (fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC 
           || Enable_Inj == 0
           || (RPM > Rev_Limit && (Rev_Limit_Type == 1 || Rev_Limit_Type == 3))) {

           int i;
           for (i = 0; i < N_Cyl; ++i) {
             fs_etpu_fuel_switch_off(Fuel_Channels[i]);
             Injection_Time = 0;
           } // for

         }//if
         
}                              // Set_Fuel()

/***************************************************************************************/ 

// read status - returns can be viewed in the debugger or sent to the tuner
// see etpu_crank_auto.h and AN3769
//
//MOVE THIS
//
static void Check_Engine(void)
{
    int8_t s4;
    int8_t s5;
    static int8_t prev_s4;
    static int8_t prev_s5;
    
    s4 = fs_etpu_eng_pos_get_cam_error_status();        // returns the error status of the CAM function
    s5 = fs_etpu_eng_pos_get_crank_error_status();      // returns the error status of the CRANK function

    // count and record time of last error
    if (s4 & !prev_s4) {
       ++Cam_Errors;
       fs_etpu_eng_pos_clear_cam_error_status();
       Last_Error_Time = systime;
    }

    if (s5 && !prev_s5) {
       ++Crank_Errors;
       fs_etpu_eng_pos_clear_crank_error_status();
       Last_Error_Time = systime;
    }

    prev_s4 = s4;
    prev_s5 = s5;

} // Check_Engine()

#if __CWCC__
#pragma pop
#endif
