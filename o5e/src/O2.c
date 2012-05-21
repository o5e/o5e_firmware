/*********************************************************************************

        @file   O2.c                                                              
        @author Jon Zeeff 
        @date   Dec 21, 2011
        @brief  Open5xxxECU - manage a WB O2 sensor
        @note   Not completed pending clarifications....
        @version 1.2
        @copyright MIT License

*************************************************************************************/

/* 
Copyright(c) 2011 Jon Zeeff
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include "config.h"
#include "cpu.h"
#include "system.h"
#include "variables.h"
#include "main.h"

// O2 heater control
// After the engine is known to be running, ramp heat up slowly
// Use bang/bang control every 10 msec.

#define O2_1_TEMP_PIN AN24      // feedback signal
#define O2_HEAT_TARGET  ???     // desired voltage on feedback signal
#define O2_1_HEATER_PIN         // on 33800

/* Notes about wide band using the Bosch CJ125 controller chip

The ECU reads VUR (cell resistance or temp) and VUA (lambda) from CJ125

VUR = VCCS/17 + vRi x IRM x RiLSU4.x (sensor)
VUR = VCCS/17 + vRi x IRM x Rical (calibration)

VCCS = 5V typical
vRi = 15.5
IRM = 500μA (LSU4.2) or 158μA (LSU4.9)
Ri=82.5Ω (LSU4.2) or Ri=200Ω (LSU4.9)  (note, check circuit board for the right value)

.933V and .784V???

----------

VUA = 1.5 + 17 * Ipmeas * 61.9
Ipmeas = (VAU - 1.5) / (17 * 61.9)

Use this table to convert:

LSU4.2
Ipmeas(mA)       λ   	VAU
-1.85		0.7	-0.446755
-1.08		0.8	0.363516
-0.76		0.85	0.700252
-0.47		0.9	1.005419
0		1.009	1.5
0.34		1.18	1.857782
0.68		1.43	2.215564
0.95		1.7	2.499685
1.4		2.42	2.97322
2.55		207	4.183365

LSU4.9
Ip      VAU    λ
-1.243 0.192 0.750
-0.927 0.525 0.800
-0.800 0.658 0.822
-0.652 0.814 0.850
-0.405 1.074 0.900
-0.183 1.307 0.950
-0.106 1.388 0.970
-0.040 1.458 0.990
0      1.500 1.003
0.015  1.515 1.010
0.097  1.602 1.050
0.193  1.703 1.100
0.250  1.763 1.132
0.329  1.846 1.179
0.671  2.206 1.429
0.938  2.487 1.701
1.150  2.710 1.990
1.385  2.958 2.434
1.700  3.289 3.413
2.000  3.605 5.391
2.150  3.762 7.506
2.250  3.868 10.11

Note: exhaust pressure will make lambda readings report richer than actual

IP(p) = Ip(p0) * p/(k+p) * (k+p0)/p0

p = pressure in bars (example: 1.2)
p0 = normal pressure in bars? (=1)
k = ~.4

Notes about not using the CJ125 (probably a good idea)

consider Vs/Ip ground
adjust pump cell current through Ip to keep Vs at .450 VDC
pump cell current is negative ~2.0 milliamps when air/fuel ratio is rich
pump cell current = 0 at 14.7:1
pump cell current is ~2 ma positive for free air (see above tables)
above takes +/- .16 VDC to Ip

target for heater is cell resistance Ri=80 Ohms
measure cell resistance with a +/-2.5V, 3Khz AC square wave applied through a 10K ohm resistor to Ip

JZ design
Op amp to provide 2.5V virtual ground
PWM source running at 100Khz to provide +- Ip
Capacitor sufficient to filter 100 Khz
To measure resistance:
   Stop 100Khz and apply full + voltage for .1 msec
   Measure voltage at voltage divider between current pump and a series resistor
   (supply and ground are known, 20x op amp to boost)
   Repeat for full negative voltage
   LMV824 op amp

*/
void O2_Task(void)
{
    static uint32_t target;     // bin 16, varies 0 to HEAT_TARGET, in A/D counts
    static uint32_t temp;       // in A/D counts, bin 16
    static uint_fast8_t i;

    task_open();                // standard OS entry - required on all tasks

    for (;;) {

        // check for not running
        while (RPM == 0) {      	// turn completely off when not running
            // turn off heaters
            O2_1_HEATER_PIN = 0;    	// turn off
            task_wait(1010);    	// 1  second
        }

        // do warmup - ramp up temp over 50 sec
        target = 0; 			// bin 16
        while (target < HEAT_TARGET) {
            target += (uint32_t) ((HEAT_TARGET * .002) << 16);  // allow .002 more every 10 msec loop
            temp = Read_AD(O2_1_TEMP_PIN) << 16;        //  current value, bin 16

            if (temp > target)
                O2_1_HEATER_PIN = 0;    // turn off
            else
                O2_1_HEATER_PIN = 1;    // turn off

            task_wait(11);      // 50+ sec before we exit this loop
        }

        // control heat - bang/bang (not PWM) for fastest response
        // 100hz control 
        while (RPM > 0) {
            if (Read_AD(O2_1_TEMP_PIN > HEAT_TARGET))
                O2_1_HEATER_PIN = 0;    // turn off
            else
                O2_1_HEATER_PIN = 1;    // turn on

            // adjust PWM to cell as needed to maintain Vs
            // stop PWM to cell
            // read cell resistance
            // restart PWM to cell

            task_wait(9);
        }                       // for

    }                           // for ever

    task_close();
}                               // O2_task()

// Turn off heater and cell voltage

void 
O2_Shutdown(void)
{
	O2_1_HEATER_PIN = 0;    	// turn off

}
