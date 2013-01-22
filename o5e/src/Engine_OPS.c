/*********************************************************************************

    @file      Engine_OPS.c                                                              
    @date      December, 2011
    @brief     Open5xxxECU - this file contains functions for fuel pulse width 
               as well as spark timing 
    @note      www.Open5xxxECU.org
    @version   2.1
    @copyright 2011, 2012 - P. Schlein, M. Eberhardt, J. Zeeff

**********************************************************************************/

// Portions Copyright 2011 P. Schlein - MIT License
// Portions Copyright 2011 M. Eberhardt - MIT License
// Portions Copyright 2011, 2012  Jon Zeeff - All rights reserved

#include <stdint.h>
#include "mpc563xm.h"
#include "config.h"
#include "err.h"
#include "led.h"
#include "variables.h"
#include "typedefs.h"
#include "Engine_OPS.h"
#include "Variable_OPS.h"
#include "cocoos.h"
#include "Table_Lookup_JZ.h"
#include "etpu_util.h"
#include "etpu_spark.h"
#include "etpu_fuel.h"
#include "etpu_pwm.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "bsp.h"   /**< pickup systime      */
#include "main.h"  /**< pickup Degree_Clock */

#include "etpu_toothgen.h"

uint32_t *fs_free_param;
static uint32_t Pulse_Width;
static void Check_Engine(void);

static void Set_Spark(void);
static void Set_Fuel(void);

// how to turn a pin on/off
#define Set_Pin(pin,value)  SIU.GPDO[pin].R = value
#define Read_Pin(pin,value)  (SIU.GPDI[pin].R & 1)

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/* the above is inserted until I can figure out how this code works
   and fix it properly */

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

// Generate a cam pulse every other rev for engines that don't have a cam signal
// Doesn't have to be very accurate - cam is not used for timing
// Note: this implies batch fuel which is done by tying pins together - each pin only fires once per 720 degree cycle
// The calibration must position an extra wide cam window (because the timing isn't exact)
// This code could use the eTPU Synchronized Pulse-Width Modulation Function instead.

void Cam_Pulse_Task(void)
{
    task_open();                // standard OS entry
    task_wait(1);

    static int8_t tooth;                 // current position
    static int8_t prev_tooth = 99;
    static uint8_t start_tooth;

    // do these once for speed
    start_tooth = (typeof(start_tooth))Start_Tooth;     // position is based on user setting of cam position
                                               
   // note: cam position must be in the range 361 - 719.  540 is recommended.
   // note: use "rising edge" cam with fake cam signals and a larger than normal window

    for (;;) {
    

        tooth = fs_etpu_eng_pos_get_tooth_number();     // runs 1 to number of teeth

        if (prev_tooth < start_tooth && tooth >= start_tooth) {            // this works even with 1/2 sync
            Set_Pin(FAKE_CAM_PIN, 1);           // create rising edge 
            task_wait(1);                       // always 1 msec wide
            Set_Pin(FAKE_CAM_PIN, 0);           // falling edge 
            uint16_t period;
            period = (fs_etpu_get_chan_local_24(0,  FS_ETPU_CRANK_TOOTH_PERIOD_A_OFFSET) * Total_Teeth) / (etpu_tcr1_freq/1000);
            task_wait(period + period/2 - 1);   // skip 1.5 rotations (in msec)
        } else 
            task_wait(1);                       // waiting for rising edge point

        prev_tooth = tooth;
    }                           // for
    task_close();

}  // Cam_Pulse_Task()

// This code is for testing only
// It isused to simulate jitter in the crank signal by altering the test rpm, which alters the tooth period
// The tooth width is a % of tooth period, so this will cause the tooth size to alternate 
// small/big/......., while keeping the average rpm constant

void Crank_Tooth_Jitter_Task(void)
{
    task_open();                // standard OS entry
    task_wait(1);

    static int8_t tooth;                 // current position
    static int32_t degrees_to_wait;
    static int24_t jitter_rpm;
    static int24_t set_RPM;

    

    // do these once for speed
    jitter_rpm = Test_RPM * Jitter / 100;
    degrees_to_wait = 360 /(N_Teeth + Missing_Teeth);
  
     fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xEFFFFF, Test_RPM, etpu_tcr1_freq); //set a base RPM to get started                                          


    for (;;) {
    
       
         set_RPM = Test_RPM + jitter_rpm;
         fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xFFFFFF, set_RPM, etpu_tcr1_freq);
         //task_wait_id(1, degrees_to_wait);
         task_wait (1);  
         set_RPM = Test_RPM - jitter_rpm;
         fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xFFFFFF, set_RPM, etpu_tcr1_freq);      
         //task_wait_id(1, degrees_to_wait);                       // waiting for rising edge point
		 task_wait (1); 
        
    }                           // for
    task_close();
}//Crank_Tooth_Jitter_Task()

// Debug
// Blink based on engine position status - for testing
// A sec blink means all is well, Fast blink or no blink means something's wrong

void Eng_Pos_Status_BLINK_Task(void)
{
    task_open();                // standard OS entry - NOTE: no non-static local variables! 
    task_wait(1);

    for (;;) {
        static int8_t status;
 
        // reads the etpu crank position function status
        status = fs_etpu_eng_pos_get_engine_position_status();

        if (status == FS_ETPU_ENG_POS_FULL_SYNC) {        // position known, so all is well slow blink
            led_on( LED2 );
            task_wait(903);     // allow others tasks to run          
            led_off( LED2 );
            task_wait(903);   // allow others tasks to run          
        } else if (status == FS_ETPU_ENG_POS_HALF_SYNC) {   
            // fast and slow blink
            led_on( LED2 );
            task_wait(133);     // allow others tasks to run          
            led_off( LED2 );
            task_wait(133);     // allow others tasks to run          
            led_on( LED2 );
            task_wait(903);     // allow others tasks to run          
            led_off( LED2 );
            task_wait(903);     // allow others tasks to run          
        } else {
            // if you are here the crank position is not known so fast blink
            led_on( LED2 );
            task_wait(133);     // allow others tasks to run          
            led_off( LED2 );
            task_wait(133);     // allow others tasks to run          
        }

        Sync_Status = (int16_t)status;   // send to tuner
        Check_Engine();         // get status of eTPU routines for debugger and tuner

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
            Start_Time = systime;
            Start_Degrees = Degree_Clock;
            Post_Start_Time = Post_Start_Cycles = 0;
        }
        Previous_RPM = RPM;

        // update + make sure the timers don't overflow  - TODO eliminate divides
        if (Post_Start_Time < 10000)
            Post_Start_Time = (systime - Start_Time) / 1000;
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

        // consider replacing MAP window with the minimum MAP value seen
        task_wait(9);           // allow others tasks to run
    }                           // for      
    task_close();
}                               // Engine10_Task()

// All spark calcs go here

static void Set_Spark()
{
    static uint32_t Spark_Advance_eTPU;
    static uint32_t Spark_Recalc_Angle_eTPU;
    static uint32_t Prev_Dwell=99;
    int i;

    // if the engine is not turning or the engine position is not known, shut off the spark
    if (RPM == 0 || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC || Enable_Ignition == 0
        || (RPM > Rev_Limit && (Rev_Limit_Type == 2 || Rev_Limit_Type == 4))) {
        for (i = 0; i < N_Coils; ++i) 
            fs_etpu_spark_set_dwell_times(Spark_Channels[i],0,0);
        Prev_Dwell = 0;
        Spark_Advance = 0;
    } else {
        // Looks up the desired spark advance in degrees before Top Dead Center (TDC)
        Spark_Advance = (int16_t) table_lookup_jz(RPM, Load, Spark_Advance_Table);        

        Spark_Advance_eTPU = (uint24_t) (72000 - (Spark_Advance << 2));    // bin -2 to 0 for eTPU use 

        // TODO Knock_Retard(); Issue #7

        if (Spark_Advance_eTPU < (72000 - 4000) && Spark_Advance_eTPU > 2000) {      // error checking, -40 to +20 is OK
              err_push( CODE_OLDJUNK_E3 );
              Spark_Advance_eTPU = 0;
        }

        // Dwell
        Dwell = Dwell_Set;                      // user specified value
        if (V_Batt < (13 << 10))                // use longer dwell when battery is low (< 13V bin 10)
           Dwell = (typeof(Dwell))((Dwell * table_lookup_jz(V_Batt, 0, Dwell_Table)) >> 13);  // dwell is in usec, bin 0

        if (Dwell > 15000 || Dwell < 500) {               // error checking
              err_push( CODE_OLDJUNK_E2 );
              Dwell = 3000;
        }

        // Calculate an appropriate re-calculation angle for the current Spark_Angle so the update is as close to firing time as possible
        uint32_t Temp1 = (((RPM * Dwell) >> 14) * (uint32_t) (1.2 * (1 << 12)) >> 12);  // 1.2 is to give the processor time to do the math 
        uint32_t Temp2 = (uint32_t) (.0006 * (1 << 12));        // conversion factor to get Temp1 into deg x 100
        uint32_t Angle_Temp = 72000 - ((Temp1 * Temp2) >> 12);

        Spark_Recalc_Angle_eTPU = (Spark_Advance_eTPU + Angle_Temp);
        if (Spark_Recalc_Angle_eTPU >= 72000)
            Spark_Recalc_Angle_eTPU -= 72000;

        fs_etpu_spark_set_recalc_offset_angle(Spark_Channels[0], Spark_Recalc_Angle_eTPU); // global value despite the channel param

    }  // if

    // set advance and dwell

    uint32_t Dwell_2;
    uint32_t Spark_Advance_eTPU_2;      // for second pulse

    if (Ignition_Type == 1) {           // wasted spark mode - fire twice, 360 degrees apart
       Dwell_2 = Dwell;
       Spark_Advance_eTPU_2 = Spark_Advance_eTPU + 36000;
       if (Spark_Advance_eTPU_2 >= 72000) 
          Spark_Advance_eTPU_2 -= 72000;
    } else {
       Dwell_2 = 0;                     // disable second spark
       Spark_Advance_eTPU_2=0;  
    } // if

    // send values to eTPU
    for (i = 0; i < N_Coils; ++i) {
        fs_etpu_spark_set_end_angles(Spark_Channels[i], Spark_Advance_eTPU, Spark_Advance_eTPU_2);
        if (Dwell != Prev_Dwell) 
           fs_etpu_spark_set_dwell_times(Spark_Channels[i], Dwell, Dwell_2);
    }                           // for

    Prev_Dwell = Dwell;         // used to avoid unnecessary updates

} // Set_Spark()

//TODO - add to ini for setting in TS.  Issue #11
#define CRANK_VOLTAGE 11
#define Run_Threshold 250       // RPM below this then not running
#define Enrich_Threshold 6000
#define Prime_Cycles_Threshold 100
#define TPS_Dot_Dead 2000

// Primary purpose is to set the fuel pulse width/injection time

static void Set_Fuel(void)
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
    static uint32_t Degree_Clock_Last = 0;      // bin 0 
    static uint32_t TPS_Dot_Decay_Last = (1 << 14);     // bin 14
    static uint32_t TPS_Dot_Degree = 0;
    static uint32_t TPS_Dot_Decay_Rate = (1 << 14);
    static uint32_t Load_Ref_AFR;
    static int32_t TPS_Dot_Last = 0;    // bin 14
    static int32_t TPS_Dot_Limit;       // bin 14
    static int32_t TPS_Dot_Decay;       // bin 14
    static int32_t TPS_Dot_Corr;
    static int32_t TPS_Dot_Sign = 0;
    static int32_t TPS_Dot_Temp;

    // if the engine is not turning, the engine position is not known, or over reving, shut off the fuel channels
    if (RPM == 0 
        || fs_etpu_eng_pos_get_engine_position_status() != FS_ETPU_ENG_POS_FULL_SYNC 
        || Enable_Inj == 0
        || (RPM > Rev_Limit && (Rev_Limit_Type == 1 || Rev_Limit_Type == 3))) {

        int i;
        for (i = 0; i < N_Cyl; ++i) {
            fs_etpu_fuel_switch_off(Fuel_Channels[i]);
            Injection_Time = 0;

        } // for

        // set the accel/deccel variables to current conditions
        TPS_Last = TPS;
        TPS_Dot_Limit = 1 << 14;
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

        // check if enrichment cals shold be done - this might want to be a % of redline
        // maintain some timers for use by enrichment
        // did we just start?
        if ((RPM < Enrich_Threshold) && (Enable_Accel_Decel == 1)) {
            // Prime pulse - extra fuel to wet the manifold on start-up   
            // check if in prime needed conditions   
            if (Post_Start_Cycles < Prime_Cycles_Threshold) {

                Prime_Corr = table_lookup_jz(CLT, 0, Dummy_Corr_Table);
                // scale the correction to the pusle width
                Prime_Corr = (((Pulse_Width * Prime_Corr) >> 13) - Pulse_Width);

                // Update Prime decay each cycle - this is a log decay of the prime pulse
                if (Post_Start_Cycles > Prime_Post_Start_Last) {
                    // reset cycle number
                    Prime_Post_Start_Last = Post_Start_Cycles;
                    // Get the decay rate for current conditions
                    Prime_Decay = table_lookup_jz(RPM, 0, Prime_Decay_Table);
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
            TPS_Last = (3 * TPS_Last + TPS) >> 2;
            //get           
            TPS_Dot = TPS_Dot_Temp << 3;
            TPS_Dot_Degree = (Degree_Clock - Degree_Clock_Last);
            // check if acceleration enrich required
            if (TPS_Dot >= TPS_Dot_Dead && TPS_Dot > TPS_Dot_Last) {
                TPS_Dot_Limit = table_lookup_jz(RPM, 0, Accel_Limit_Table);
                TPS_Dot_Corr = table_lookup_jz(RPM, 0, Accel_Sensativity_Table);
                TPS_Dot_Decay_Rate = table_lookup_jz(RPM, 0, Accel_Decay_Table);
                TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot - TPS_Dot_Dead)) >> 14;

                // update the last clock
                Degree_Clock_Last = Degree_Clock;
                TPS_Dot_Degree = 0;
                TPS_Dot_Decay_Last = 1 << 14;
                TPS_Dot_Sign = 1;
                // decel required 
            } else if (TPS_Dot <= (-TPS_Dot_Dead) && TPS_Dot < TPS_Dot_Last) {
                TPS_Dot_Limit = table_lookup_jz(RPM, 0, Decel_Limit_Table);
                TPS_Dot_Corr = table_lookup_jz(RPM, 0, Decel_Sensativity_Table);
                TPS_Dot_Decay_Rate = table_lookup_jz(RPM, 0, Decel_Decay_Table);
                TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot_Dead - TPS_Dot)) >> 14;
                // update the last clock
                Degree_Clock_Last = Degree_Clock;
                TPS_Dot_Degree = 0;
                TPS_Dot_Decay_Last = 1 << 14;
                TPS_Dot_Sign = -1;
            }
            TPS_Dot_Last = TPS_Dot;
            // calculate the required decay
            if (TPS_Dot_Degree >= 720) {
                Degree_Clock_Last = Degree_Clock_Last + 720;
                TPS_Dot_Decay = (TPS_Dot_Decay_Last * TPS_Dot_Decay_Rate) >> 14;
                TPS_Dot_Decay_Last = TPS_Dot_Decay;
                TPS_Dot_Corr = (TPS_Dot_Corr * TPS_Dot_Decay) >> 14;
            }
            if (TPS_Dot_Corr > TPS_Dot_Limit)
                TPS_Dot_Corr = TPS_Dot_Limit;
            if (TPS_Dot_Sign > -1) {
                Pulse_Width = (Pulse_Width + ((Pulse_Width * TPS_Dot_Corr) >> 14));
            } else {
                Pulse_Width = (Pulse_Width - ((Pulse_Width * TPS_Dot_Corr) >> 14));
                if (Pulse_Width < 0)
                    Pulse_Width = 0;
            }

        }
        // TODO adjust based on O2 sensor data Issue #8
        // Corr = O2_Fuel();
        // Pulse_Width = (Pulse_Width * Corr) >> 14;

        // Assume fuel pressure is constant

        // fuel dead time - extra pulse needed to open the injector
        // take user value and adjust based on battery voltage
        Dead_Time = (Dead_Time_Set * table_lookup_jz(V_Batt, 0, Inj_Dead_Time_Table)) >> 13;

        Injection_Time = Pulse_Width + Dead_Time;

        // TODO - add code for semi-seq fuel 
        // Sanity check - greater than 99% duty cycle?
        if (Injection_Time > ((990000 * 60 * 2) / RPM)) {
            err_push( CODE_OLDJUNK_E1 );
        }

        // Fuel pulse width calc is done

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

        // TODO - Cylinder Trim math and updates.  Issue #9
        // TODO - Staged injection math and updates Issue #10

        // tell eTPU to use new fuel injection pulse values (same for all cylinders)
        uint32_t j;
        for (j = 0; j < N_Injectors; ++j) {
            fs_etpu_fuel_switch_on(Fuel_Channels[j]);   // Turn on fuel channels

            error_code = 1;
            while (error_code != 0)     // This tries until the channel is actually updated
                error_code = fs_etpu_fuel_set_injection_time(Fuel_Channels[j], Pulse_Width);

        }                       // for

        // These are global across channels, so only do it once
        fs_etpu_fuel_set_compensation_time(Fuel_Channels[0], Dead_Time);
        fs_etpu_fuel_set_normal_end_angle(Fuel_Channels[0], Inj_End_Angle_eTPU);        // degrees * 100
        fs_etpu_fuel_set_recalc_offset_angle(Fuel_Channels[0], Fuel_Recalc_Angle_eTPU); // degrees * 100
    }                           // else
}                               // Set_Fuel()

// read status - returns can be viewed in the debugger or sent to the tuner
// see etpu_crank_auto.h and AN3769

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
