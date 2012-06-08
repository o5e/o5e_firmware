/*********************************************************************************

    @file      Engine_OPS.c                                                              
    @date      December, 2011
    @brief     Open5xxxECU - this file contains functions for fuel pulse width 
               as well as spark timing 
    @note      www.Open5xxxECU.org
    @version   2.1
    @copyright MIT License

**********************************************************************************/

/*================================================================================
   ORIGINAL AUTHOR:  Paul Schlein                                                 
   REV      AUTHOR          DATE          DESCRIPTION OF CHANGE                   
   ---     -----------     ----------    --------------------- 
   2.5me   M. Eberhardt    31/May/12     added load model
   2.4me   M. Eberhardt    18/May/12     fixes, added pulse_Per_rev for tach                   
   2.3     M. Eberhardt    04/May/12     Simplified enrichment scheme
   2.2     J. Zeeff        01/May/12     Reorganized
   2.1     J. Zeeff        28/Dec/11     Cleanup                                  
   2.0     M. Eberhardt    22/Dec/11     Rewrote to better use OS features        
   1.0     P. Schlein      12/Sep/11     Initial version with Placeholders        
================================================================================*/

/*
Copyright (c) 2011 

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "config.h"
#include "system.h"
#include "main.h"
#include "variables.h"
#include "etpu_toothgen.h"
#include "eQADC_OPS.h"
#define EXTERN
#include "Engine_OPS.h"
#include "Enrichment_OPS.h"
#include "INPUT_OPS.h"
#include "Variable_OPS.h"
#include "OS.h"
#include "Table_Lookup_JZ.h"
#include "etpu_util.h"
#include "etpu_spark.h"
#include "etpu_fuel.h"
#include "etpu_pwm.h"
#include "etpu_app_eng_pos.h"
#include "etpu_crank_auto.h"
#include "eTPU_OPS.h"

uint32_t *fs_free_param;

/**********************************************************************************/
/*          Done while power on                                                   */
/**********************************************************************************/

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


// Generate a cam pulse for engines that don't have one.  
// Note: this implies semi-sequential fuel and wasted spark.
// Doesn't have to be very accurate - cam is not used for timing

    #define Sync_Threshold 30

    void Cam_Pulse_Task(void)
    {
        task_open();                // standard OS entry
        task_wait(1);

        register uint_fast8_t tooth;        // not saved across an OS call
        static uint_fast8_t prev_tooth;
        static uint_fast8_t position;
        static uint_fast8_t alternate = 0;
        static uint_fast8_t sync_flag = 0;
        static uint_fast8_t TDC_Tooth;
        static uint_fast8_t TDC_Minus_Position;
        static uint_fast8_t TDC_Plus_Position;
        static uint32_t TDC_Minus_Position_RPM = 0;
        static uint32_t TDC_Plus_Position_RPM;
        static uint_fast8_t position_flag = 0;
        static uint32_t Cam_Pulse_Wait_Angle;
        

        position =  N_Teeth / 2;    // doesn't matter where, but this is a good spot
        //these are needed for syncing a crank only odd fire engine
        TDC_Tooth = ((Engine_Position << 2) / Degrees_Per_Tooth_x100) % Total_Teeth; //adjust from bin-2 to bin 0
        //find teeth to compare rpm to test if compression stroke
        TDC_Minus_Position = (Total_Teeth  + TDC_Tooth - (3000 * Degrees_Per_Tooth_x100)) % Total_Teeth;
        TDC_Plus_Position = (TDC_Tooth + (3000 * Degrees_Per_Tooth_x100)) % Total_Teeth;
        Cam_Pulse_Wait_Angle = (((Total_Teeth - TDC_Minus_Position) + Total_Teeth / 2) * Degrees_Per_Tooth_x100 / 100);
        
        for (;;) {
        
           	// output pulse once per 2 crank revs

           	tooth = fs_etpu_eng_pos_get_tooth_number();     // runs number of teeth
           	
           	 //find cylinder #1 on odd fire engines
           	// this works by comparing the rpm before #1TDC to rpm after #1TDC
           	// if the RPM after is great than the minus rpm plus a sync_theshold #1TDC position is known

           if (Engine_Type_Select && Sync_Mode_Select == 0){
               if (sync_flag == 0){
           	       Get_Fast_Op_Vars();; // Read current RPM from eTPU
           	       if (tooth == TDC_Minus_Position )
           	           TDC_Minus_Position_RPM = RPM;
           	       if (tooth == TDC_Plus_Position )
           	           TDC_Plus_Position_RPM = RPM;
           	       if (TDC_Minus_Position_RPM != 0 && TDC_Plus_Position_RPM > (TDC_Minus_Position_RPM + Sync_Threshold)){
            	       sync_flag = 1;
            	        task_wait_id(1, Cam_Pulse_Wait_Angle); // delay to put cam pulse about 180 degress after the mising tooth
      			   }
               }
           }else{
                sync_flag = 1;	
           }
        
                // after odd fire home found or any time on even fire engines
           if (sync_flag == 1 && prev_tooth < position && tooth >= position && (alternate ^= 1))
               Set_Pin(FAKE_CAM_PIN,1);
    	   else
           	   Set_Pin(FAKE_CAM_PIN,0);
		   
            prev_tooth = tooth;

               task_wait(2);

        } // for

    task_close();     

} //if

// Debug
// Blink based on engine position status - for testing
// A sec blink means all is well, Fast blink or no blink means something's wrong

void Eng_Pos_Status_BLINK_Task(void)
{
    task_open();                // standard OS entry - NOTE: no non-static local variables! 
    task_wait(1);

    for (;;) {
        // reads the etpu crank position function status
        if (fs_etpu_eng_pos_get_engine_position_status() == FS_ETPU_ENG_POS_FULL_SYNC) {        // position known, so all is well slow blink
            Set_Pin(SYNC_STATUS_PIN, 0);        // Light On  Constant   
            task_wait(903);     // allow others tasks to run          
            Set_Pin(SYNC_STATUS_PIN, 1);        // Light Off
            task_wait(903);     // allow others tasks to run          
        } else {
            // if you are here the crank position is not known so fast blink
            Set_Pin(SYNC_STATUS_PIN, 0);        // Light ON
            task_wait(133);     // allow others tasks to run          
            Set_Pin(SYNC_STATUS_PIN, 1);        // Light Off     
            task_wait(133);     // allow others tasks to run          
        }
        Check_Engine();         // get status of eTPU routines for debugger
    }                           // for
    task_close();
}                               /* end of RPM_BLINK_task() */

// Decide if fuel pump should be on or off

void Fuel_Pump_Task(void)
{
    task_open();                // standard OS entry - NOTE: no non-static local variables! 
    task_wait(1);

    Set_Pin(FUEL_PUMP_PIN, 1);  // Prime fuel system, Fuel Pump ON 
    task_wait(10311);           // allow others tasks to run for 10 sec

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

void Engine10_Task(void)
{

    task_open();
    task_wait(1);

    for (;;) {
        static uint32_t Start_Time;     // time when start started
        static uint32_t Start_Degrees;  // engine position when start started
        static uint32_t Previous_RPM;

        // Read the sensors that can change quickly like RPM, TPS, MAP, ect
        Get_Fast_Op_Vars();

        // maintain some timers for use by enrichment
        // did we just start?
        if (RPM > 0 && Previous_RPM == 0) {
            Start_Time = msec_clock;
            Start_Degrees = Degree_Clock;
            Post_Start_Time = Post_Start_Cycles = 0;
        }
        Previous_RPM = RPM;

        // update + make sure the timers don't overflow  - TODO eliminate divides
        if (Post_Start_Time < 10000)
            Post_Start_Time = (msec_clock - Start_Time) / 1000;
        if (Post_Start_Cycles < 10000)
            Post_Start_Cycles = (Degree_Clock - Start_Degrees) / 720;

        // TODO  - add load sense method selection and calcs. This only works right with 1 bar MAP
        // Load = Get_Load();
        Load = (MAP[0] << 2);   // convert bin 12 to 14 and account for /100Kpa

        // set spark advance and dwell based on current conditions
        Set_Spark();

        // set fuel pulse width + position based on current conditions
        Set_Fuel();

        // Update Tach signal
        uint32_t frequency = ((RPM * Pulses_Per_Rev) * (uint32_t) ((1 << 14) / 60.) >> 14);
        //Update_Tach(frequency);

        // Update MAP windows (where to sample MAP value)
#include "etpu_knock_window.h"
#include "etpu_knock_window_auto.h"
        uint32_t MAP_Angle;
        uint32_t i;
        MAP_Angle = Table_Lookup_JZ(RPM, Load, MAP_Angle_Table);
        for (i = 0; i < N_MAP_windows; ++i) {
            // TODO - why is this disabled?
            //MAP_Angles[i * 2] = MAP_Angle + Cyl_Angle_eTPU[i];     // Adjust open point (leave width alone)
            // update eTPU 
            //fs_etpu_knock_window_update(MAP_WINDOW_CHANNEL, 1, i, MAP_Angles[i * 2], MAP_Angles[i * 2 + 1]);
        }                       // for

        task_wait(9);           // allow others tasks to run
    }                           // for      
    task_close();
}                               // Engine10_Task()

// All spark calcs go here

void Set_Spark()
{
    static uint32_t Spark_Advance_eTPU;
    static uint32_t Spark_Recalc_Angle_eTPU;
    static uint32_t Min_Dwell;
    static uint32_t Max_Dwell;
    uint32_t error_code;

    // if the engine is not turning or the engine position is not known, shut off the spark
    if (RPM == 0 || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC || Enable_Ignition == 0
        || (RPM > Rev_Limit && (Rev_Limit_Type == 2 || Rev_Limit_Type == 4))) {
        fs_etpu_spark_set_min_max_dwell_times(Spark_Channels[0], 0, 0); 
        Dwell = 0;
        Spark_Advance = 0;

    } else {
        // Looks up the desired spark advance in degrees before Top Dead Center (TDC)
        Spark_Advance = Table_Lookup_JZ(RPM, Load, Spark_Advance_Table) ;  // Bin shift tuner angles from -2 to 0 for eTPU use 
        Spark_Advance_eTPU = Spark_Advance <<2;

        // TODO Knock_Retard(); Issue #7

        // Update min/max dwell time
        Min_Dwell = (Dwell * 102) / 128;        // compiler will use a shift, >> 7
        Max_Dwell = (Dwell * 153) / 128;
        fs_etpu_spark_set_min_max_dwell_times(Spark_Channels[0], Min_Dwell, Max_Dwell);

        // Calculate an appropriate re-calculation angle for the current Spark_Angle so the update is as close to firing time as possible
        uint32_t Temp1 = (((RPM * Dwell) >> 14) * (uint32_t) (1.2 * (1 << 12)) >> 12);  // 1.2 is to give the processor time to do the math 
        uint32_t Temp2 = (uint32_t) (.0006 * (1 << 12));        // conversion factor to get Temp1 into deg x 100
        uint32_t Angle_Temp = 72000 - ((Temp1 * Temp2) >> 12);

        Spark_Recalc_Angle_eTPU = (Spark_Advance_eTPU + Angle_Temp);
        if (Spark_Recalc_Angle_eTPU >= 72000)
            Spark_Recalc_Angle_eTPU -= 72000;

        fs_etpu_spark_set_recalc_offset_angle(Spark_Channels[0], Spark_Recalc_Angle_eTPU);
        // Dwell
        Dwell = (Dwell_Set * Table_Lookup_JZ(V_Batt, 0, Dwell_Table)) >> 13;

    }                           // else      
    // set advance and dwell
    int i;
    for (i = 0; i < N_Coils; ++i) {

        // Advance
        error_code = 1;
        while (error_code != 0)
            error_code =
                fs_etpu_spark_set_end_angles(Spark_Channels[i], Spark_Advance_eTPU,
                                             ((Spark_Advance_eTPU + 36000) % 72000));

        // Dwell update - tries until the channel actual updated
        error_code = 1;
        while (error_code != 0)
            error_code = fs_etpu_spark_set_dwell_times(Spark_Channels[i], Dwell, (Dwell * Ignition_Type));

    }                           // for

}
//TODO - add to ini for setting in TS.  Issue #11
#define CRANK_VOLTAGE 11
#define Run_Threshold 250       // RPM below this then not running
#define Enrich_Threshold 6000
#define Prime_Cycles_Threshold 100
#define TPS_Dot_Dead 2000 

// Primary purpose is to set the fuel pulse width/injection time

void Set_Fuel(void)
{
    static int16_t Corr;
    static uint32_t error_code;
    static uint32_t Dead_Time;
    static uint32_t Dead_Time_Corr;
    static uint32_t Prime_Post_Start_Last = 1;
    static uint32_t Prime_Decay = (1 << 14);
    static uint32_t Prime_Decay_Last = (1 << 14);
    static uint32_t Prime_Corr;
    static uint32_t TPS_Last = 0;       // bin 14
    static uint32_t Degree_Clock_Last = 0;   // bin 0 
    static uint32_t TPS_Dot_Decay_Last = (1 <<14);  // bin 14
    static uint32_t TPS_Dot_Degree = 0;
    static uint32_t TPS_Dot_Decay_Rate = (1 << 14);
    static uint32_t Load_Ref_AFR;
    static int32_t TPS_Dot_Last = 0;    // bin 14
    static int32_t TPS_Dot_Limit;      // bin 14
    static int32_t TPS_Dot_Decay;       // bin 14
    static int32_t TPS_Dot_Corr;
    static int32_t TPS_Dot_Sign = 0;
    static int32_t TPS_Dot_Temp;

                
    // if the engine is not turning or the engine position is not known, shut off the fuel channels
    if (RPM == 0 || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC || Enable_Inj == 0
        || (RPM > Rev_Limit && (Rev_Limit_Type == 1 || Rev_Limit_Type == 3))) {
 
        int i;
        for (i = 0; i < N_Cyl; ++i) {
            fs_etpu_fuel_switch_off(Fuel_Channels[i]);
        Injection_Time = 0;
            

        }
        //set the accel/deccel varibles to current conditions
        TPS_Last = TPS;
        TPS_Dot_Limit = 1 <<14;
        TPS_Dot_Corr = 0;
        TPS_Dot = 0;
        TPS_Dot_Last = 0;
        TPS_Dot_Sign = 0;
        
        Degree_Clock_Last = Degree_Clock;
    } else {

        // calc fuel pulse width

        // base (max) pulse width
        Pulse_Width = Max_Inj_Time;     // base value in microseconds bin 0

        // apply various multiplier adjustments

        // RPM correction based on engine model - this is the primary tuning
        if Model_Tuning_Enable{
           Corr = Table_Lookup_JZ(RPM, 0, Eng_Model_Table);
           Pulse_Width = (Pulse_Width * Corr) >> 14;
        // Adjust according to load
           //This is where the user tells the ecu what to do mixture wise at different loads
           // Normally low power is run leaner than high power.
           // this corrention is set up so it does NOT alter full power mixture, that point is the reference condition 
           Load_Ref_AFR = Table_Lookup_JZ((1<<14), 0, Load_Model_Table); //get the 100% load AFR
           Corr = Table_Lookup_JZ(Load, 0, Load_Model_Table);
           Corr = (Load_Ref_AFR << 10)/Corr;
           Pulse_Width = (Pulse_Width * Corr) >> 10;
        }
        //Load correction - assumes fuel required is proportional to load
        Pulse_Width = (Pulse_Width * Load) >> 14;

        // Main fuel table corection - this is used to fine tuning
        Corr = Table_Lookup_JZ(RPM, Load, Inj_Time_Corr_Table);
        Pulse_Width = (Pulse_Width * Corr) >> 14;

        // air temperature correction....I can't figure out how to not make this a divide at the moment
        Pulse_Width = ((Pulse_Width << 14) / IAT);

        // Coolant temp coorection from enrichment_ops
        Corr = Table_Lookup_JZ(CLT, 0, Fuel_Temp_Corr_Table);
        Pulse_Width = (Pulse_Width * Corr) >> 13;

        // check if enrichment cals shold be done - this might want to be a % of redline
        // maintain some timers for use by enrichment
        // did we just start?
        if ((RPM < Enrich_Threshold) && (Enable_Accel_Decel == 1)) {
            // Prime pulse - extra fuel to wet the manifold on start-up   
            // check if in prime needed conditions   
            if (Post_Start_Cycles < Prime_Cycles_Threshold) {

                Prime_Corr = Table_Lookup_JZ(CLT, 0, Dummy_Corr_Table);
                // scale the correction to the pusle width
                Prime_Corr = (((Pulse_Width * Prime_Corr) >> 13) - Pulse_Width);

                // Update Prime decay each cycle - this is a log decay of the prime pulse
                if (Post_Start_Cycles > Prime_Post_Start_Last) {
                    // reset cycle number
                    Prime_Post_Start_Last = Post_Start_Cycles;
                    // Get the decay rate for current conditions
                    Prime_Decay = Table_Lookup_JZ(RPM, 0, Prime_Decay_Table);
                    // decrease decay by the new value
                    Prime_Decay = (Prime_Decay_Last * Prime_Decay) >> 14;
                    // reset last
                    Prime_Decay_Last = Prime_Decay;
                }
                // apply the decay
                Prime_Corr = (Prime_Corr * Prime_Decay) >> 14;
                // Reduce the Prime correction by the decay rate and add to pulse_width            
                Pulse_Width = (Pulse_Width + Prime_Corr);
            }
            /**********************************************************************************/
            /*                           accel/decel enrichment                               */
            /* This working by looking at the rate the throttle is moving  and  calculating   */
            /* an enrichment or a derichment to compensate for fuel that  condenses on the    */
            /* manifold wall due to the pressure increase when the throttle opens. This is    */
            /* done by watching the throttle change rate since throttle is the first variable */
            /* to change.                                                                     */
            /*                                                                                */
            /* The accel/decel variables are set to base values above at the start of the     */
            /* fuel routine so they get current sensor values to work with.                   */
            /*                                                                                */ 
            /* Get the TPS change. This simply compares the throttle position each pass       */
            /* through.  It probbly should be an actual rate by dividing by  the change in    */
            /* crank position but that added too much noise to the  calculation when I tried  */
            /*                                                                                */
            /* The throttle change rate is compared to a dead band.  The deadband helps       */
            /* clean up noise but more importantly no throttle enrichment is required for     */
            /* slow throttle change rates.                                                    */
            /*                                                                                */
            /* When TPS_Dot is above the deadband, the sensativity value is used to calculate */
            /* howw much enrichment is required.  The faster the throttle is moving the more  */
            /* enrifchment should be added.                                                   */
            /*                                                                                */
            /* When the TPS_Dot stops increasing a decay is applied which deceases the        */
            /* enrichment by the specified % each engine cycle.  It's done by cycle because   */
            /* each cylinder has it's own manifold runner and port so each cylinder require   */
            /* the enrichment.                                                                */ 
            /*                                                                                */
            /* If TPS_Dot goes negative, ie the throttle is closing, accel enrichment ends    */
            /* imediately and a calculation is done to determine if decel derichment is       */
            /* required.  Decel derichment works exactly the same a acel enrichment, only     */
            /* using negative TPS_Dot rates an dpusle width reductions to compensate for fuel */
            /* being remover from the port and manifold walls due to pressure drop            */
            /*                                                                                */
            /**********************************************************************************/
             
              //get a TPS change         
            TPS_Dot_Temp = (TPS_Last - TPS);
            TPS_Last = (3 * TPS_Last + TPS) >>2;
              //get           
            TPS_Dot = TPS_Dot_Temp <<3;                 
            TPS_Dot_Degree = (Degree_Clock - Degree_Clock_Last);            
            // check if acceleration enrich required
            if (TPS_Dot >= TPS_Dot_Dead && TPS_Dot > TPS_Dot_Last){
                    TPS_Dot_Limit = Table_Lookup_JZ(RPM, 0, Accel_Limit_Table);
                    TPS_Dot_Corr = Table_Lookup_JZ(RPM, 0, Accel_Sensativity_Table);
                    TPS_Dot_Decay_Rate = Table_Lookup_JZ(RPM, 0, Accel_Decay_Table);  
                    TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot- TPS_Dot_Dead)) >> 14;                   

                      // update the last clock
                    Degree_Clock_Last = Degree_Clock;
                    TPS_Dot_Degree = 0;
                    TPS_Dot_Decay_Last = 1<<14;
                    TPS_Dot_Sign = 1;
                // decel required 
            } else if (TPS_Dot <= (-TPS_Dot_Dead) && TPS_Dot < TPS_Dot_Last){
                    TPS_Dot_Limit = Table_Lookup_JZ(RPM, 0, Decel_Limit_Table);
                    TPS_Dot_Corr = Table_Lookup_JZ(RPM, 0, Decel_Sensativity_Table);
                    TPS_Dot_Decay_Rate = Table_Lookup_JZ(RPM, 0, Decel_Decay_Table);
                    TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot_Dead -TPS_Dot)) >> 14;
                    // update the last clock
                    Degree_Clock_Last = Degree_Clock;
                    TPS_Dot_Degree = 0;
                    TPS_Dot_Decay_Last = 1<<14;
                    TPS_Dot_Sign = -1;
            }
            TPS_Dot_Last = TPS_Dot;
            // calculate the required decay
            if  (TPS_Dot_Degree >= 720){
                 Degree_Clock_Last = Degree_Clock_Last + 720;
                 TPS_Dot_Decay = (TPS_Dot_Decay_Last * TPS_Dot_Decay_Rate) >> 14;
                 TPS_Dot_Decay_Last = TPS_Dot_Decay;
                 TPS_Dot_Corr = (TPS_Dot_Corr * TPS_Dot_Decay) >> 14;
            } 
            if (TPS_Dot_Corr > TPS_Dot_Limit)
                TPS_Dot_Corr = TPS_Dot_Limit;
            if (TPS_Dot_Sign > -1){
                Pulse_Width = (Pulse_Width + ((Pulse_Width * TPS_Dot_Corr) >> 14));	
            }else{
                Pulse_Width = (Pulse_Width - ((Pulse_Width * TPS_Dot_Corr) >> 14));
                if  (Pulse_Width < 0)
                     Pulse_Width = 0; 	
            }
            
        }
        // TODO adjust based on O2 sensor data Issue #8
        // Corr = O2_Fuel();
        // Pulse_Width = (Pulse_Width * Corr) >> 14;

        // Assume fuel pressure is constant
        

        // fuel dead time - extra pulse needed to open the injector
        // take user value and adjust based on battery voltage
        Dead_Time = (Dead_Time_Set * Table_Lookup_JZ(V_Batt, 0, Inj_Dead_Time_Table)) >> 13;

        Injection_Time = Pulse_Width + Dead_Time;

        // Sanity check - greater than 100% duty cycle?
        if (Injection_Time > ((1000000 * 60 * 2) / RPM)) {
            system_error(4832, __FILE__, __LINE__, "");
        }
        // Fuel pulse width calc is done

        // where should pulse end (injection timing)?
        uint32_t Inj_End_Angle_eTPU = (Table_Lookup_JZ(RPM, Load, Inj_End_Angle_Table)) << 2;  // Bin shift tuner angles from -2 to 0 for eTPU use 

        // Calculate angles for eTPU use, must reference the missing tooth not TDC

        // Calculate an approprite re-calculation angle for the current injection Inj_End_Angle 
        uint32_t Temp1 = ((RPM * Pulse_Width) >> 14);   //  * 1.2;       // 1.2 is to give the processor time to do the math
        uint32_t Temp2 = (uint32_t) (.0006 * (1 << 12));        // conversion factor to get Temp1 into deg x 100
        uint32_t Angle_Temp = 72000 - ((Temp1 * Temp2) >> 12);
        uint32_t Fuel_Recalc_Angle_eTPU = (Inj_End_Angle_eTPU + Angle_Temp);
        if (Fuel_Recalc_Angle_eTPU > 72000)
            Fuel_Recalc_Angle_eTPU -= 72000;

        // TODO - Cylinder Trim math and updates.  Issue #9
        // TODO - Staged injection math and updates Issue #10

        // tell eTPU to use new fuel injection pulse values (same for all cylinders)
        uint32_t j;
        for (j = 0; j < N_Injectors; ++j) {
            fs_etpu_fuel_switch_on(Fuel_Channels[j]);   // Turn on fuel channels

            error_code = 1;
            while (error_code != 0)     // This tries until the channel is actually updated
                error_code = fs_etpu_fuel_set_injection_time(Fuel_Channels[j], Injection_Time);

        }                       // for

        // These are global across channels, so only do it once
        fs_etpu_fuel_set_compensation_time(Fuel_Channels[0], Dead_Time);
        fs_etpu_fuel_set_normal_end_angle(Fuel_Channels[0], Inj_End_Angle_eTPU);        // degrees * 100
        fs_etpu_fuel_set_recalc_offset_angle(Fuel_Channels[0], Fuel_Recalc_Angle_eTPU); // degrees * 100
    }                           // else
}                               // Set_Fuel()