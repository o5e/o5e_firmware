/*********************************************************************************

        @file   O2.c                                                              
        @author Jon Zeeff 
        @date   October 25, 2011
        @brief  Open5xxxECU - manage a WB O2 sensor
        @note   Not completed pending clarifications....
        @version 1.1
        @copyright MIT License

*************************************************************************************/

Copyright (c) 2011 Jon Zeeff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.


#include "config.h"
#include "cpu.h"
#include "system.h"
#include "variables.h"
#include "main.h"

// O2 heater control
// after the engine is known to be running, start at 60% and ramp to 100% (if needed) over 20 seconds (2% per second).  
// Use PID and PWM (100hz) to control temperature of O2 sensor.

#define O2_1_TEMP_PIN AN24    // feedback signal
#define O2_HEAT_TARGET  ???   // desired voltage on feedback signal
#define O2_1_HEATER_PIN       // on 33800

/*

VUR = VCCS/17 + vRi x IRM x RiLSU4.x (sensor)
VUR = VCCS/17 + vRi x IRM x Rical (calibration)

amplifier gain: vRi = 15.5
measurement current:
IRM = 500μA (LSU4.2) or 158μA (LSU4.9)
Ri=82.5Ω (LSU4.2) or Ri=200Ω (LSU4.9)

*/

void O2_task(void)
{
static uint_fast8_t slow_heat=0;

task_open();            // standard OS entry - required on all tasks

pid_struct pid;

pid.desired = O2_HEAT_TARGET;   // desired output value
pid.Kp =  (.3 << 8);            // proportional gain  - bin8, example: .3
pid.Ki = (.1 << 8);             // integral gain  - bin8, example: .1
pid.Kd = (.1 << 8);             // derivative gain  - bin8, example: .1

for (;;) {
    if (speed_rpm == 0)     	// turn completely off when not running
       slow_heat = 0;
    else if (speed_rpm < Run_Threshold && slow_heat < 100)
       slow_heat = 0;  		//  limit % max output allowed

    if (slow_heat < 100)
       slow_heat += 2;    	// allow 2% more every loop

    // calc PID control of heat
    pid.current = Read_AD(O2_1_TEMP_PIN);
    PID(&pid);      // call routine to calc new values

    // clip to warmup limit
    if (pid.control > slow_heat)
       pid.control = slow_heat;

    // set PWM value
    Set_PWM_33800(O2_1_HEATER_PIN, pid.control);

    // repeat for second O2 sensor

    task_wait(1000);  // sleep 1000 msec - 1 sec
} // for ever

task_close();

}  // O2_task()
