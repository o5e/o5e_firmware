/*********************************************************************************

    @file main.c                                                              
    @date   October, 2011
    @brief  Open5xxxECU - calls init routines, creates tasks, starts scheduler to run them
    @note www.Open5xxxECU.org
    @version 1.3
    @copyright 2011 Jon Zeeff
    @warning O5E is not for use in safety critical systems or pollution controlled vehicles.

**********************************************************************************/

/*
Copyright (c) 2011 Jon Zeeff  - All Rights Reserved
Copyright (c) 2012 Sean Stasiak <sstasiak at gmail dot com>
*/

#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "mpc563xm.h"
#include "cocoos.h"
#include "main.h"
#include "variables.h"
#include "Tuner.h"
#include "Engine_OPS.h"
#include "err.h"
#include "led.h"
#include "etpu_struct.h"
#include "etpu_util.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "Testing_OPS.h"
#include "Fake_Cam_Signal.h"
#include "Optional_Output_Ops.h"



static void Angle_Clock_Task(void);
static Evt_t event;             // dummy event for angle task
static void LED_Task(void);

// Note: CocoOS allows less critical tasks to run less frequently and prevents spagetti code caused by state machines
//       Currently it uses no interrupts and no preemption.  But it does use Duff's device - so no non-static local 
//       variables in a task (elsewhere is fine).
/* classic misunderstanding above TODO: remove */

void System_Init(void);                 // init routine

/*************************************************************************************/

void main( void )
{
  err_init(); /**< get code logger abilities up asap */
    // Initialize the O5E system
    System_Init();
    // the code that we want to run repeatedly
    // Note: all tasks must "task_wait(n)" at least every 1/2 msec, preferably more often
    // CRITICAL: tasks cannot have non-static local variables and cannot use switch statements!
    /* classic misunderstanding above TODO: remove */
    
    if (Flash_OK != 0) {        // don't run these with nonsense flash values 
        (void)task_create(Engine10_Task, 0 + 128, 0, 0, 0);      // create the task
        (void)task_create(Fuel_Pump_Task, 1 + 128, 0, 0, 0);     // create the task
        (void)task_create(Slow_Vars_Task, 2 + 128, 0, 0, 0);     // create the task
        (void)task_create(Eng_Pos_Status_BLINK_Task, 3 + 128, 0, 0, 0);      //create the task 
        if (Sync_Mode_Select == 1){   // user option to generate a cam signal
           (void)task_create(Cam_Pulse_Task, 4 + 128, 0, 0, 0);      // create the task
        }

        (void)task_create(Output_Task, 5 + 128, 0, 0, 0); // create the task	
      
        
        
    } 
   	(void)task_create(Test_RPM_Task, 9 + 128, 0, 0, 0);
    (void)task_create(Tuner_Task, 10 + 128, 0, 0, 0);           // create the task
    (void)task_create(LED_Task, 11 + 128, 0, 0, 0);             // create the task 

    //this should always be last
    (void)task_create(Angle_Clock_Task, 254, 0, 0, 0);         // task to update angle clock, always last

  os_start();
}

static void Angle_Clock_Task(void)
{
    static uint32_t prev_angle; // previous cam position in ticks
    static uint32_t i;   
    register uint64_t j =0;
    static int8_t status;
    static int8_t Previous_Status;
    static uint32_t Start_Time;     // time when start started
	static uint32_t Start_Degrees;  // engine position when start started

    
#define ANGLE_TICKS_PER_DEGREE ((Ticks_Per_Tooth * (N_Teeth + Missing_Teeth)) / 360)
    task_open();                // NOTE: must be first line


// previous cam position in ticks

    for (;;) {

        status = fs_etpu_eng_pos_get_engine_position_status ();
      	if  (Previous_Status != status || status != FS_ETPU_ENG_POS_FULL_SYNC){  //position known so fuel and spark have started
            Degree_Clock = 0;;
            prev_angle = angle_clock();
            Start_Time = systime;
            Start_Degrees = Degree_Clock;
            Post_Start_Time = 0;
            Post_Start_Cycles = 0;
            Previous_Status = status;
        }

        
        // update crank shaft degree/angle clock (free running, not synced to an absolute engine position)
//Match the degrees per tic to the eTPU settings
        i = (angle_clock() - prev_angle) & 0xffffff;    // 24 bit hw counter 
        j = i * (((uint64_t)1 << 32) / ANGLE_TICKS_PER_DEGREE);        // avoid a run time divide
        i = (uint32_t) (j >> 32);       // convert back to bin 0
        if (i > 0) {            // delta full degrees
            Degree_Clock += i;
            prev_angle = (prev_angle + i * ANGLE_TICKS_PER_DEGREE) & 0xffffff;
            os_task_tick(1, (uint16_t) i);      // increment os angle clock value
        } 
                      // maintain some timers for use by enrichment
         //update + make sure the timers don't overflow  - TODO eliminate divides
        if (Post_Start_Time < 10000)
           Post_Start_Time = (uint32_t)(systime - Start_Time) / 1000;
        if (Post_Start_Cycles < 10000)
            Post_Start_Cycles = (uint16_t)(Degree_Clock - Start_Degrees) / 720;
        event_signal(event);    // dummy to cause task change - all higher priority tasks will be run

        
     } // for ever
    
    task_close();
}
    
// Task to verify that OS is running - flash a LED, feed the watchdog, feed TunerStudio

static void LED_Task(void)
{
    task_open();                // NOTE:  no non-static local variables allowed
    task_wait(410);             /**< why the wait? */

    // flash every second and feed watchdog
    for (;;) {
        /* don't even bother hitting wd here, this is the wrong way to it */
        //Feed_Watchdog();        // will reset cpu if not fed


        // DEBUG - check for DMA errors
        /* if we have DMA errors, so what ? */
        if (EDMA.ESR.R != 0)
            err_push( CODE_OLDJUNK_D5 );

        led_invert( LED1 );
        if (Last_Error == 0)
            task_wait(999);     // delay about 1 sec
        else 
            task_wait(303);     // something is wrong, blink fast
        
    }                           // for ever

    task_close();               // never reached

}                               // LED_task()
