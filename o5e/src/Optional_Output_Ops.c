/*********************************************************************************

    @file      Optional_Output_Ops.c                                                              
    @date      May 2013
    @brief     Open5xxxECU - this file contains functions for generating a fake cam signal
    @note      www.Open5xxxECU.org
    @version   1.0
    @copyright 2013 - M. Eberhardt

**********************************************************************************/
// Portions Copyright 2011, 2013 M. Eberhardt - BSD 3 clause  License


#include <stdint.h>
#include "mpc563xm.h"
#include "config.h"
#include "variables.h"
#include "typedefs.h"
#include "cocoos.h"
#include "etpu_util.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "bsp.h"   /**< pickup systime      */
#include "main.h"  /**< pickup Degree_Clock */
#include "Optional_Output_Ops.h"




#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/* the above is inserted until I can figure out how this code works
   and fix it properly */



// Generate a cam pulse every other rev for engines that don't have a cam signal
// Doesn't have to be very accurate - cam is not used for timing
// Note: this implies batch fuel which is done by tying pins together - each pin only fires once per 720 degree cycle
// The calibration must position an extra wide cam window (because the timing isn't exact)
// This code could use the eTPU Synchronized Pulse-Width Modulation Function instead.

void Cam_Pulse_Task(void)
    {
        task_open();                // standard OS entry
        task_wait(1);

        static uint_fast8_t tooth;
        static uint8_t start_tooth;
        static uint_fast8_t prev_tooth = 255;
        static uint_fast8_t alternate = 1;
        static uint_fast8_t sync_flag = 1;//normally zero when odd fire stuff is running
        static uint_fast8_t TDC_Tooth;
        static uint_fast8_t TDC_Minus_Position;
        static uint_fast8_t TDC_Plus_Position;
        static uint32_t TDC_Minus_Position_RPM = 0;
        static uint32_t Last_TDC_Minus_Position_RPM = 0;
        
        start_tooth = (typeof(start_tooth))Start_Tooth;     // position is based on user setting of cam position
        //position =  Total_Teeth / 2;    // doesn't matter where, but this is a good spot
        //these are needed for syncing a crank only odd fire engine
        TDC_Tooth = ((Engine_Position << 2) / Degrees_Per_Tooth_x100) % Total_Teeth; //adjust from bin-2 to bin 0
        //find teeth to compare rpm to test if compression stroke
        TDC_Minus_Position = (Total_Teeth  + TDC_Tooth - ((Odd_Fire_Sync_Angle <<2) / Degrees_Per_Tooth_x100)) % Total_Teeth;

        for (;;) {

                // output pulse once per 2 crank revs

                tooth = fs_etpu_eng_pos_get_tooth_number();     // runs number of teeth
         /* not safe to use with the cam window opened up much past 120 degrees.....currently setto 720               
                 //find cylinder #1 on odd fire engines
                // this works by comparing the rpm before #1TDC to rpm after #1TDC
                // if the RPM after is great than the minus rpm plus a sync_theshold #1TDC position is known

           if (Engine_Type_Select && Sync_Mode_Select == 0 && sync_flag == 0){
                   Get_Fast_Op_Vars(); // Read current RPM from eTPU
                   if (tooth == TDC_Minus_Position )
                       TDC_Minus_Position_RPM = RPM;
                   
                   if (TDC_Minus_Position_RPM > Odd_Fire_Sync_Threshold && Last_TDC_Minus_Position_RPM > Odd_Fire_Sync_Threshold && TDC_Minus_Position_RPM < (Last_TDC_Minus_Position_RPM - Odd_Fire_Sync_Threshold))
                       sync_flag = 1;
                   
                   if (tooth < prev_tooth) //detect missing tooth
                   Last_TDC_Minus_Position_RPM = TDC_Minus_Position_RPM;
                   
           }else{ 
           
                sync_flag = 1;  
           }*/
        
                // after odd fire home found or any time on even fire engines
           if (sync_flag == 1 && prev_tooth < start_tooth && tooth >= start_tooth  && (alternate ^= 1)){
              	Set_Pin(FAKE_CAM_PIN, 1);           // create rising edge 
                task_wait(1);                       // always 1 msec wide
                Set_Pin(FAKE_CAM_PIN, 0);           // falling edge 
                task_wait (3);                       // TODO-angle would be better
           } else
				task_wait(1);
                   
            prev_tooth = tooth;
            
        } // for

    task_close();     

} // Cam_Pulse_Task()



#if __CWCC__
#pragma pop
#endif
