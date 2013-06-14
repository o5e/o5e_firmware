/*********************************************************************************

    @file      Testing_OPS.c                                                              
    @date      April, 2013
    @brief     Open5xxxECU - this file contains functions for firmware testing
    @note      www.Open5xxxECU.org
    @version   1.0
    @copyright 2013 - M. Eberhardt

**********************************************************************************/

// Portions Copyright 2011 P. Schlein - BSD 3 Clause License
// Portions Copyright 2011 M. Eberhardt - BSD 3 Clause License
// Portions Copyright 2011, 2012  Jon Zeeff - All rights reserved

#include <stdint.h>
#include "config.h"
#include "led.h"
#include "variables.h"
#include "typedefs.h"
#include "Engine_OPS.h"
#include "Variable_OPS.h"
#include "cocoos.h"
#include "etpu_util.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "bsp.h"   /**< pickup systime      */
#include "main.h"  /**< pickup Degree_Clock */
#include "Testing_OPS.h"
#include "etpu_toothgen.h"

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/* the above is inserted until I can figure out how this code works
   and fix it properly */


// This code is for testing only
// It isused to simulate jitter in the crank signal by altering the test rpm, which alters the tooth period
// The tooth width is a % of tooth period, so this will cause the tooth size to alternate 
// small/big/......., while keeping the average rpm constant

void Test_RPM_Task(void)// test routine, only run ifdef SIMULATOR (see main.c)

{
    task_open();                // standard OS entry
    task_wait(1);

    static int8_t tooth;                 // current position
    static int32_t degrees_to_wait;
    static int24_t jitter_rpm;
    static int24_t jitter_previous;
    static int24_t set_RPM;
    

    

    // do these once for speed
    //degrees_to_wait = 360 /(N_Teeth + Missing_Teeth); degree based wait not working
    
    
    for (;;) {
    
  	if (Test_RPM_Type == 0){	// simply set the RPM
            fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xEFFFFF, Test_RPM[0], etpu_tcr1_freq); //set a base RPM to get started
            task_wait (11);                           	 
    		}


	else if(Test_RPM_Type == 1){ //use constant rpm with jitter
    
        if(jitter_previous != Jitter){
           jitter_rpm = Test_RPM[0] * Jitter / 100;
           jitter_previous = Jitter; 
        }
         set_RPM = Test_RPM_1 + jitter_rpm;
         fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xFFFFFF, set_RPM, etpu_tcr1_freq);
         task_wait (1); //TODO this should be angle based
          
         set_RPM = Test_RPM_1 - jitter_rpm;
         fs_etpu_toothgen_adj(TOOTHGEN_PIN1, 0xFFFFFF, set_RPM, etpu_tcr1_freq);
         task_wait (1); //TODO this should be angle based               
        }//else if
      else if (Test_RPM_Type == 2){ //Run RPM cycle

    	fs_etpu_toothgen_adj(TOOTHGEN_PIN1, RPM_Change_Rate_1, Test_RPM_1, etpu_tcr1_freq); //set a base RPM to get started 
    	task_wait (Test_RPM_Dwell_1);
    			 
    	fs_etpu_toothgen_adj(TOOTHGEN_PIN1, RPM_Change_Rate_2, Test_RPM_2, etpu_tcr1_freq); //set a base RPM to get started 
    	task_wait (Test_RPM_Dwell_2);
    	
    	fs_etpu_toothgen_adj(TOOTHGEN_PIN1, RPM_Change_Rate_3, Test_RPM_3, etpu_tcr1_freq); //set a base RPM to get started 
    	task_wait (Test_RPM_Dwell_3);
    	
    	fs_etpu_toothgen_adj(TOOTHGEN_PIN1, RPM_Change_Rate_4, Test_RPM_4, etpu_tcr1_freq); //set a base RPM to get started 
    	task_wait (Test_RPM_Dwell_4);
      }//else if
      
      else{// use signal from POT to set RPM
              // Read the Pot
        Get_Fast_Op_Vars();
      
      fs_etpu_toothgen_adj(TOOTHGEN_PIN1,0xFFFFFF,Pot_RPM , etpu_tcr1_freq); //set a base RPM to get started
            task_wait (11);   	
      }
        
    }                           // for
    task_close();
}//Test_RPM_Task()

// Debug
// Blink based on engine position status - for testing
// A sec blink means all is well, Fast blink or no blink means something's wrong
//
//MOVE THIS
//
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

    }                           // for
    task_close();
}                               /* end of RPM_BLINK_task() */



#if __CWCC__
#pragma pop
#endif
