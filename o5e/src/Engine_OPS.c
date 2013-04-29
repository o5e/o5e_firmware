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
#include "mpc563xm.h"
#include "config.h"
#include "err.h"
#include "variables.h"
#include "typedefs.h"
#include "Engine_OPS.h"
#include "Enrichment_OPS.h"
#include "Variable_OPS.h"
#include "cocoos.h"
#include "Table_Lookup_JZ.h"
#include "etpu_util.h"
#include "etpu_spark.h"
#include "etpu_fuel.h"
#include "etpu_pwm.h"
#include "etpu_fpm.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "Load_OPS.h"



uint32_t *fs_free_param;
uint32_t Pulse_Width;
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



#define Delta_V_Crank 2 <<10 			//V_Batt is bin 10
#define Run_Threshold 250       		// RPM below this then not running
#define Warmup_Threshold 10000  		// let warmup stuff go 10k cycles for now.  todo  - change this to end when the correction is zero
#define Wheel_Slip_Threshold 1000		// todo user variable
#define Acceleration_Threshold 1000		// todo real number maybe used variable
#define Deceleration_Threshold 1000		// todo real number maybe used variable
#define Idle_Threshold 1000				// todo real number maybe used variable


void Engine10_Task(void)
{
    task_open();
    task_wait(1);
    

	static uint16_t V_Battery_Stored;

	//read sensors to get basline values
	Get_Slow_Op_Vars();
	Get_Fast_Op_Vars();
        
	//store a baseline Battery Voltage
	V_Battery_Stored = V_Batt;

    for (;;) {    
        // Read the sensors that can change quickly like RPM, TPS, MAP, ect
        Get_Fast_Op_Vars();
        
        // TODO  - add load sense method selection and calcs. This only works right with 1 bar MAP
        Get_Load();

        // set spark advance and dwell based on current conditions
        Set_Spark();

        // set fuel pulse width + position based on current conditions
        Set_Fuel();

//
//MOVE THIS
//
        //Update_Tach(RPM)
        // Update Tach signal
        uint32_t frequency = ((RPM * Pulses_Per_Rev) * (uint32_t) ((1 << 14) / 60 ) >> 14);
           // maybe there should be 1 Update_eTPU() ???
        //Update_Tach(frequency);
		fs_etpu_pwm_update(TACH_CHANNEL, frequency, 1000, etpu_tcr1_freq);
/***************************************************************************************/
		
        // consider replacing MAP window with the minimum MAP value seen
        task_wait(9);           // allow others tasks to run
    }                           // for      
    task_close();
}                               // Engine10_Task()

// All spark calcs go here
//
//MOVE THIS
//
static void Set_Spark()
{
    static uint32_t Spark_Advance_eTPU;
    static uint32_t Spark_Recalc_Angle_eTPU;
    static uint32_t Prev_Dwell=99;
    static uint32_t Dwell_2;
    static uint32_t Spark_Advance_eTPU_2;
    int i;

/***************************************************************************************/
 // TODO - This should go away
    // if the engine is not turning or the engine position is not known, shut off the spark
 //   if (RPM == 0 || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC || Enable_Ignition == 0
 //       || (RPM > Rev_Limit && (Rev_Limit_Type == 2 || Rev_Limit_Type == 4))) {
 //       for (i = 0; i < N_Coils; ++i) 
 //           fs_etpu_spark_set_dwell_times(Spark_Channels[i],0,0);
 //       Prev_Dwell = 0;
 //       Spark_Advance = 0;

    
/***************************************************************************************/    
        // Looks up the desired spark advance in degrees before Top Dead Center (TDC)
        Spark_Advance = (int16_t) table_lookup_jz(RPM, Load, Spark_Advance_Table);        

        Spark_Advance_eTPU = (uint24_t) (72000 - (Spark_Advance << 2));    // bin -2 to 0 for eTPU use 
        Spark_Advance_eTPU_2 = Spark_Advance_eTPU + 36000; // needed for waste spark, harmless otherwise
          if (Spark_Advance_eTPU_2 >= 72000) // roll it over at 720 degrees
              Spark_Advance_eTPU_2 -= 72000;
          
/***************************************************************************************/          
        // TODO Knock_Retard(); Issue #7
/***************************************************************************************/ 

        if (Spark_Advance_eTPU < (72000 - 4000) && Spark_Advance_eTPU > 2000) {      // error checking, -40 to +20 is OK
              err_push( CODE_OLDJUNK_E3 );
              Spark_Advance_eTPU = 0;
        }

/***************************************************************************************/
      

        // Calculate an appropriate re-calculation angle for the current Spark_Angle so the update is as close to firing time as possible
        uint32_t Temp1 = (((RPM * Dwell) >> 14) * (uint32_t) (1.2 * (1 << 12)) >> 12);  // 1.2 is to give the processor time to do the math 
        uint32_t Temp2 = (uint32_t) (.0006 * (1 << 12));        // conversion factor to get Temp1 into deg x 100
        uint32_t Angle_Temp = 72000 - ((Temp1 * Temp2) >> 12);

        Spark_Recalc_Angle_eTPU = (Spark_Advance_eTPU + Angle_Temp);
           if (Spark_Recalc_Angle_eTPU >= 72000) // roll it over at 720 degrees
               Spark_Recalc_Angle_eTPU -= 72000;
          //Update re-calculation angle in eTPU
        fs_etpu_spark_set_recalc_offset_angle(Spark_Channels[0], Spark_Recalc_Angle_eTPU); // global value despite the channel param

/***************************************************************************************/ 

        // Dwell
           Dwell = (typeof(Dwell))((Dwell_Set * table_lookup_jz(V_Batt, 0, Dwell_Table)) >> 13);  // dwell is in usec, bin 0
             //the engine position is not known, of over rev limit, shut off the spark
              if (Enable_Ignition == 0 //spark disabled
                 || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC //crank position unknow
                 || (RPM > Rev_Limit && (Rev_Limit_Type == 2 || Rev_Limit_Type == 4))) //rev limit engaged
                    //Turn spark off
                    Dwell = 0;



        if (Dwell > 15000 || Dwell < 500) {               // error checking
              err_push( CODE_OLDJUNK_E2 );
              Dwell = 3000;
        }

/***************************************************************************************/ 
        // Dwell_2 - used for waste spark
        
           Dwell_2 = Dwell * Ignition_Type; //used for waste spark, set to zero otherwise


        
/***************************************************************************************/  
//
//THIS PART SHOULD STAY
//
    // send values to eTPU
    for (i = 0; i < N_Coils; ++i) {
        fs_etpu_spark_set_end_angles(Spark_Channels[i], Spark_Advance_eTPU, Spark_Advance_eTPU_2);
        
        fs_etpu_spark_set_dwell_times(Spark_Channels[i], Dwell, Dwell_2);
    }                           // for


} // Set_Spark()

//TODO - add to ini for setting in TS.  Issue #11
#define CRANK_VOLTAGE 11
//#define Run_Threshold 250       // RPM below this then not running



// Primary purpose is to set the fuel pulse width/injection time
//
//MOVE THIS
//
static void Set_Fuel(void)
{
    static int16_t Corr;
    static uint32_t error_code;
    static uint32_t Dead_Time;
    static uint32_t Dead_Time_Corr;

    static uint32_t Load_Ref_AFR;

    // if the engine is not turning, the engine position is not known, or over reving, shut off the fuel channels




        // calc fuel pulse width

        // base (max) pulse width
        Pulse_Width = Max_Inj_Time;     // base value in microseconds bin 0

        // apply various multiplier adjustments

        // RPM correction based on engine model - this is the primary tuning
        if (Model_Tuning_Enable) {       
            Corr = table_lookup_jz(RPM, 0, Eng_Model_Table);
            Pulse_Width = (Pulse_Width * Corr) >> 14;
            // Adjust according to load
            //This is where the user tells the ecu what to do mixture wise at different loads
            // Normally low power is run leaner than high power.
            // this corrention is set up so it does NOT alter full power mixture, that point is the reference condition 
            Load_Ref_AFR = table_lookup_jz((1 << 14), 0, Load_Model_Table);     //get the 100% load AFR
            Corr = table_lookup_jz(Load, 0, Load_Model_Table);
            Corr = (Load_Ref_AFR << 10) / Corr;
            Pulse_Width = (Pulse_Width * Corr) >> 10;
        }
        // Load correction - assumes fuel required is roughly proportional to load
        Pulse_Width = (Pulse_Width * Load) >> 14;

        // Main fuel table correction - this is used to adjust for RPM effects
        Corr = table_lookup_jz(RPM, Load, Inj_Time_Corr_Table);
        Pulse_Width = (Pulse_Width * Corr) >> 14;

        // Air temperature correction....I can't figure out how to not make this a divide at the moment
        Pulse_Width = (Pulse_Width << 14) / IAT;

        // Coolant temp correction from enrichment_ops
        Corr = table_lookup_jz(CLT, 0, Fuel_Temp_Corr_Table);
        Pulse_Width = (Pulse_Width * Corr) >> 13;


        // Prime/warmup correction
        Get_Prime_Corr();
        Pulse_Width = (Pulse_Width + Prime_Corr);
            
        // Acel/decel correction
        Get_Accel_Decel_Corr();
        
        Pulse_Width = (Pulse_Width + ((Pulse_Width * Accel_Decel_Corr) >> 14));
        
/***************************************************************************************/         
        // TODO adjust based on O2 sensor data Issue #8
        // Corr = O2_Fuel();
        // Pulse_Width = (Pulse_Width * Corr) >> 14;
/***************************************************************************************/         

        // Assume fuel pressure is constant

        // fuel dead time - extra pulse needed to open the injector
        // take user value and adjust based on battery voltage
        Dead_Time = (Dead_Time_Set * table_lookup_jz(V_Batt, 0, Inj_Dead_Time_Table)) >> 13;

/***************************************************************************************/          
         //this give the tuner the current pulse width
        Injection_Time = Pulse_Width + Dead_Time;
        
/***************************************************************************************/ 
        // TODO - add code for semi-seq fuel
/***************************************************************************************/          
        // Sanity check - greater than 99% duty cycle?
        if (Injection_Time > ((990000 * 60 * 2) / RPM)) {
            err_push( CODE_OLDJUNK_E1 );
        }

        // Fuel pulse width calc is done
        
 /***************************************************************************************/        
//
//MOVE THIS
//
//Injection_angle()
        // where should pulse end (injection timing)?
        uint32_t Inj_End_Angle_eTPU = (table_lookup_jz(RPM, Load, Inj_End_Angle_Table)) << 2;   // Bin shift tuner angles from -2 to 0 for eTPU use 

        if (Inj_End_Angle_eTPU >= Drop_Dead_Angle << 2)            // clip to 1 degree before Drop_Dead
            Inj_End_Angle_eTPU = (Drop_Dead_Angle << 2) - (1 * 100);

        // Calculate angles for eTPU use, must reference the missing tooth not TDC

        // Calculate an approprite re-calculation angle for the current injection Inj_End_Angle 
        uint32_t Temp1 = ((RPM * Pulse_Width) >> 14);   //  * 1.2;       // 1.2 is to give the processor time to do the math
        uint32_t Temp2 = (uint32_t) (.0006 * (1 << 12));        // conversion factor to get Temp1 into deg x 100
        uint32_t Angle_Temp = 72000 - ((Temp1 * Temp2) >> 12);
        uint32_t Fuel_Recalc_Angle_eTPU = (Inj_End_Angle_eTPU + Angle_Temp);
        if (Fuel_Recalc_Angle_eTPU > 72000)
            Fuel_Recalc_Angle_eTPU -= 72000;
        
/***************************************************************************************/ 

        // TODO - Cylinder Trim math and updates.  Issue #9
        
/***************************************************************************************/         
        
        // TODO - Staged injection math and updates Issue #10
        
/***************************************************************************************/         
//
//THIS PART SHOULD STAY
//
        // tell eTPU to use new fuel injection pulse values (same for all cylinders)
        uint32_t j;
        for (j = 0; j < N_Injectors; ++j) {
            fs_etpu_fuel_switch_on(Fuel_Channels[j]);   // Turn on fuel channels

            error_code = 1;
            while (error_code != 0)     // This tries until the channel is actually updated
     //look up trim values
         //need to make table use the "j" value before it will work
            //Corr = table_lookup_jz(RPM, Load, Cyl_Trim_1_Table);
            //Cyl_Pulse_Width=  (Pulse_Width * Corr) >> 14;
            
                //error_code = fs_etpu_fuel_set_injection_time(Fuel_Channels[j], Cyl_Pulse_Width);
                //this goes away once cyl trim is working
                error_code = fs_etpu_fuel_set_injection_time(Fuel_Channels[j], Pulse_Width);

        }                       // for

        // These are global across channels, so only do it once
        fs_etpu_fuel_set_compensation_time(Fuel_Channels[0], Dead_Time);
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
}                               // Set_Fuel()

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
