
// Users should change this file to match their hardware and compiler

#ifndef CONFIG_H
#define CONFIG_H

#define CPU_CLOCK             80000000    // 80 Mhz
#define TOOTHGEN_PIN1         31          // which eTPU channel to use for crank simulator
#define TOOTHGEN_PIN2         30          // which eTPU channel to use for crank simulator
// TODO define eTPU channels to use here
// Note: different hardware may have to change these
// Note: what is here must match what is in SIU_OPS.c
#define FUEL_CHANNELS_1_6     21,9,10,11,12,13   //5 used instead of 10 to get LED on on deme board
#define FUEL_CHANNELS_7_12    18,20,22,23,24,25
#define FUEL_CHANNELS_13_18   25,25,25,25,25,25  /* not used for 5634 */
#define FUEL_CHANNELS_19_24   25,25,25,25,25,25  /* not used for 5634*/
//changed the channel 18 for testing
#define SPARK_CHANNELS_1_6    19,2,3,4,10,6        // 10 used instead of 5,to give LED to fuel 1
#define SPARK_CHANNELS_7_12   7,8,25,25,25,25   /* 9-12 not used for 5634*/ 
#define TACH_CHANNEL          27                  // Tach Output (pwm) eTPU chan
#define FUEL_PUMP_PIN         141                 // fuel pump
#define WHEEL_SPEED_1_4       14, 15, 16, 17      // read wheel speed
//for testing - Blink based on engine position status
#define MAP_WINDOW_CHANNEL    26    // eTPU channel to output MAP sample windows on - fixed, do not change
#define KNOCK_WINDOW_CHANNEL  28    // eTPU channel to output MAP sample windows on - fixed
#define FAKE_CAM_PIN          193   // GPIO used for semi-sequentail operation

// used for serial port A and tuner communications
#define BAUD_RATE 115200    // speed of serial port comm
#define SERIAL_BUFFER_SIZE (2048+10)

// servo motor or idle air control
#define PWM1_CHANNEL 29     // eTPU channel to use for servo motor output 
#define PWM1_PAD     218    // servo GPIO pad # for above
#define PWM1_HZ      50     // recommend 50 for servos, 200 or 300 for IAC

// increasing clock available for general use - runs at CPU_CLOCK/1000000 ticks per msec
#define MSEC_EMIOS_CHANNEL 9  // eMIOS channel to use for the msec timer (OS and many other uses)
#define hclock()  (volatile uint32_t)(EMIOS.CH[MSEC_EMIOS_CHANNEL].CCNTR.R)   

// angle clock in degrees
#define angle_clock() (volatile uint32_t)(eTPU->TB2R_A.R)

#endif
