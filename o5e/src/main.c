/*********************************************************************************

    @file main.c                                                              
    @date   October, 2011
    @brief  Open5xxxECU - calls init routines, creates tasks, starts scheduler to run them
    @note www.Open5xxxECU.org
    @version 1.3
    @copyright MIT License
    @warning O5E is not for use in safety critical systems or pollution controlled vehicles.

**********************************************************************************/

/*
Copyright (c) 2011 Jon Zeeff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "config.h"
#include "system.h"
#include <stdint.h>
#define EXTERN
#include "main.h"
#include "cpu.h"
#include "variables.h"
#include "OS.h"
#include <stdio.h>
#include "eSCI_DMA.h"
#include "etpu_struct.h"
#include "etpu_util.h"
#include "FreeScale/FSutil.h"
#include "eSCI_OPS.h"
#include "mpc5500_ccdcfg.h"
#include "Functions.h"

// Note: CocoOS allows less critical tasks to run less frequently and prevents spagetti code caused by state machines
//       Currently it uses no interrupts and no preemption.  But it does use Duff's device - so no non-static local 
//       variables in a task (elsewhere is fine).

void System_Init(void);         // init routine

// OS housekeeping
static void scheduler(void);
static uint8_t task_id[N_TASKS];        // tasks that have been created
static uint8_t num_tasks = 0;
static Evt_t event;             // dummy event
static uint32_t max_delay = 0;  // worse case OS delay

/*************************************************************************************/

int main(void)
{
    // Initialize the O5E system
    System_Init();

    // the code that we want to run repeatedly
    // Note: order by priority
    // Note: all tasks must "task_wait(n)" at least every 1/2 msec, preferably more often
    // CRITICAL: tasks cannot have non-static local variables and cannot use switch statements!

    if (Flash_OK != 0) {        // don't run these with nonsense flash values 
        //task_id[num_tasks] = task_create(Enrichment_Task,num_tasks+128,0,0,0);       	// create the task
        //++num_tasks;
        task_id[num_tasks] = task_create(Engine10_Task, num_tasks + 128, 0, 0, 0);      // create the task
        ++num_tasks;
        task_id[num_tasks] = task_create(Fuel_Pump_Task, num_tasks + 128, 0, 0, 0);     // create the task
        ++num_tasks;
        task_id[num_tasks] = task_create(Slow_Vars_Task, num_tasks + 128, 0, 0, 0);     // create the task
        ++num_tasks;
        //task_id[num_tasks] = task_create(O2_Task,num_tasks + 128,0,0,0);         	// create the task
        //++num_tasks;
        task_id[num_tasks] = task_create(Eng_Pos_Status_BLINK_Task, num_tasks + 128, 0, 0, 0);      //create the task 
        ++num_tasks;
        #ifdef FAKE_CAM_PIN
           task_id[num_tasks] = task_create(Cam_Pulse_Task, num_tasks + 128, 0, 0, 0);      // create the task
           ++num_tasks;
        #endif
    }
    task_id[num_tasks] = task_create(tuner_task, num_tasks + 128, 0, 0, 0);     // create the task
    ++num_tasks;
    task_id[num_tasks] = task_create(LED_task, num_tasks + 128, 0, 0, 0);       // create the task 
    ++num_tasks;
    // =====  add more here ======

    // must be last
    task_id[num_tasks] = task_create(scheduler, 254, 0, 0, 0);  		// task to update clocks - always last
    ++num_tasks;
    event = event_create();     // dummy event

    os_start();                 // never returns
    return 0;
}

/*************************************************************************************/

// CocoOS is non-preemptive and uses no interrupts.  
// This task is always ready to run, but it has the lowest priority. 
// Similar to an idle routine.
// It also updates the OS clocks

static void scheduler(void)
{
    static uint32_t prev_clock; // for msec clock
    static uint32_t prev_angle; // previous cam position in ticks
    static uint32_t i;
    register uint64_t j;

    task_open();                // NOTE: must be first line

    prev_clock = hclock();      // for msec clock
    prev_angle = angle_clock(); // previous cam position in ticks
    max_delay = 0;              // diagnostic

    for (;;) {

        // update msec clock

        i = (hclock() - prev_clock) & 0xffffff; // 24 bit hw counter
        j = i * (((uint64_t)1 << 32) / TICKS_PER_MSEC);        // avoid a run time divide
        i = (uint32_t) (j >> 32);               // convert back to bin 0
        if (i > 0) {                            // delta full msec
            msec_clock += i;                    // firmware maintained clock
            prev_clock = (prev_clock + i * TICKS_PER_MSEC) & 0xffffff;  // increment previous
            os_task_tick(0, (uint16_t) i);      // increment os msec clock value
        }

        // update crank shaft degree/angle clock (free running, not synced to an absolute engine position)

#       define ANGLE_TICKS_PER_DEGREE 10
        i = (angle_clock() - prev_angle) & 0xffffff;    // 24 bit hw counter 
        j = i * (((uint64_t)1 << 32) / ANGLE_TICKS_PER_DEGREE);        // avoid a run time divide
        i = (uint32_t) (j >> 32);                       // convert back to bin 0
        if (i > 0) {                                    // delta full degrees
            Degree_Clock += i;
            prev_angle = (prev_angle + i * ANGLE_TICKS_PER_DEGREE) & 0xffffff;
            os_task_tick(1, (uint16_t) i);              // increment os angle clock value
        }
        // TODO test and set other fast events here

        // TODO - track %CPU for each task, max delay and a "load average".  Use STM clock.

        // do diagnostics and cause task switch
        static uint32_t save_clock;                             // this should be available to the tuner
        save_clock = hclock();  // current time

        event_signal(event);    // dummy to cause task change - all higher priority tasks will be run

        save_clock = (hclock() - save_clock) & 0xffffff;        // how long all other tasks took
        if (save_clock > max_delay) {                           // track max OS delay ever seen
            max_delay = save_clock;
            //if (max_delay > TICKS_PER_MSEC * 10)              // 10 msec is too long
            //   Last_Error = 3894;
        }

    }                           // for ever

    task_close();               // never reached

}                               // scheduler()

/*************************************************************************************/

#include <string.h>

// Routine to record an error
void system_error(uint16_t code, char *file, uint16_t line, char *note)
{
    Last_Error = code;          // output that will be sent to tuner
    sprintf((char *)Error_String, "%d %u %s %u %s\r\n", msec_clock, code, file, line, note);    // log error
}

/*************************************************************************************/

char string[80];

// Task to verify that OS is running - flash a LED and feed the watchdog

void LED_task(void)
{
    task_open();                // NOTE:  no non-static local variables allowed
    task_wait(410);

    sprintf(string, "\r\nO5E main.c %s %s\r\n", __DATE__, __TIME__);
    write_serial((unsigned char *)string, (unsigned short)strlen(string));

    // init LED pin
    init_LED(LED_PIN, 0);

    // flash every second and feed watchdog
    for (;;) {
        //Feed_Watchdog();        // will reset cpu if not fed

        // Check for DMA errors
        if (EDMA.ESR.R != 0)
            system_error(31629, __FILE__, __LINE__, "");

        invert_LED(LED_PIN);
        if (Last_Error == 0)
            task_wait(999);     // delay about 1 sec
        else 
            task_wait(303);     // something is wrong, blink fast
        
    }                           // for ever

    task_close();               // never reached

}                               // LED_task()
