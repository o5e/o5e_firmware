/*********************************************************************************

    @file main.c                                                              
    @date   October, 2011
    @brief  Open5xxxECU - calls init routines, creates tasks, starts scheduler to run them
    @note www.Open5xxxECU.org
    @version 1.1
    @copyright MIT License

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

// Note: CocoOS allows less critical tasks to run less frequently and prevents spagetti code caused by state machines
//       Currently it uses no interrupts and no preemption.  But it does use Duff's device - so no non-static local 
//	     variables in a task (elsewhere is fine).

// Declare tasks that you want to run here and add below
extern void engine_task (void);
extern void tuner_task (void);
extern void LED_task(void);

void System_Init(void); // init routine

// OS housekeeping
static void scheduler (void);
static uint8_t task_freq[N_TASKS];   	// how often to run, 1 = most often
static uint8_t task_id[N_TASKS];     	// tasks that have been created
static int num_tasks=0;
static Evt_t event;		    	// dummy event
static uint8_t max_task_time;        	// longest that any task took
static uint32_t task_runs;              // how many task runs

/*************************************************************************************/

int main(void)
{
	// Initialize the O5E system
	System_Init();
	send_serial_A("O5E start\r\n",11);   // sent without DMA

#ifdef JZ
	void generator(void);
	generator();
#endif
	
	// the code that we want to run repeatedly
	// Note: order by priority
	// Note: all tasks must "suspend()" or "wait()" at least every 1/2 msec, preferably more often
	// CRITICAL: tasks cannot have non-static local variables and cannot use switch statements!
	// pick task_freq from this list: 1,2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97

	// TODO - put back in - crashes my board
	//task_id[num_tasks] = task_create(engine_task,num_tasks+128,0,0,0);   	// create the task
	//task_freq[num_tasks++] = 1;	// set how often we want it to run (1 = most often), should be a prime #

	task_id[num_tasks] = task_create(tuner_task,num_tasks+128,0,0,0);   	// create the task
	task_freq[num_tasks++] = 3;	// set how often we want it to run (1 = most often), should be a prime #
	
	task_id[num_tasks] = task_create(LED_task,253,0,0,0);   		// create the task - second lowest priority
	task_freq[num_tasks++] = 47;	// set how often we want it to run (1 = most often), should be a prime #
	// =====  add more here ======

	// this task doesn't change -  always last and lowest priority
	task_create(scheduler,254,0,0,0); 	// task which decides which other tasks can run - lowest priority
	event = event_create();    		// dummy event
	
	os_start();   // never returns
	return 0;    
}

/*************************************************************************************/

// Move Along, Nothing To See Here
// Just trust that if you added the right two lines above, your task will be run
// 
// Determine which tasks are ready to run
// CocoOS is non-preemptive and uses no interrupts.  This implements round-robin, 1/n frequency, msec clock and events
// This task is always ready to run, but it has the lowest priority. 

static void scheduler(void) {
	static uint_fast8_t task_count[N_TASKS];    		// tracks when to run each task
	static int32_t prev_clock;
	static int32_t prev_angle;       // previous cam position in ticks
	static uint32_t i;
	
	task_open();					// NOTE:  no non-static local variables allowed

	// spread counters around so all x tasks don't run at the same time
	for (i = 0; i < N_TASKS; ++i)
	task_count[i] = 97*i;

	prev_clock = clock(); 	// init to current value

	for (;;) {
		// go through all tasks and resume those that should be run
		for (i = 0; i < num_tasks; ++i) {
			if (++task_count[i] % task_freq[i] == 0) {      // 1/n scheduling
				os_task_resume(task_id[i]);             // set task to run if not waiting on anything else
				event_signal(event);                    // dummy to cause task change 
			}                

			// update clocks and tell OS

			if ((((clock() -  prev_clock)) & 0xffffff) > TICKS_PER_MSEC) {  // it's a 24 bit clock
				++msec_clock;
				os_tick();		      
				prev_clock = (prev_clock + TICKS_PER_MSEC) & 0xffffff;   // increment previous
			    #if 0
				// update degree/angle clock
				i = ((angle_clock() -  prev_angle) & 0xffffff) / ANGLE_TICKS_PER_DEGREE;
				if (i > 0) {
      				Degree_Counter += i;
                                os_sub_nTick(1,i);    // tell os new clock value
      				prev_angle = (prev_angle + i * ANGLE_TICKS_PER_DEGREE) & 0xffffff;
				}
			    #endif
			}

			// TODO test and set other events here

			++task_runs;			// track this for debug
			// TODO - track %CPU for each task
		} // for
	} // for ever
	task_close();  // never reached
}  // scheduler()

/*************************************************************************************/

// Routine to record an error
void system_error(int16_t code, char *file, uint16_t line, char *note) 
{
	Last_Error = code;
	sprintf(Error_String,"%d %d %s %d %s\r\n",msec_clock, code, file, line, note);	// log error
}

/*************************************************************************************/

#include "cpu.h"
#include "FreeScale/FSutil.h"
#include "eSCI_OPS.h"
#include "mpc5500_ccdcfg.h"
#include <string.h>

unsigned char string[80];
 
// Task to verify that OS is running - flash a LED

void LED_task(void)
{
	task_open();     // NOTE:  no non-static local variables allowed

	task_wait(500);
	send_serial_A(signature,strlen(signature)); 
	sprintf(string,"\r\nCompiled %s %s\r\n",__DATE__, __TIME__);
	send_serial_A(string,strlen(string));
	//send_serial_B(string,strlen(string));
	
	// init LED pin
	init_LED(LED_PIN, 0);

	// flash every second
	for (;;) {
		invert_LED(LED_PIN);
		task_wait(1000);       // delay 1000 msec
	} // for ever

	task_close();  // never reached

} // LED_task()

