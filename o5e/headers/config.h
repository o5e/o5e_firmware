
// Users should change this file to match their hardware and compiler

#ifndef CONFIG_H
#define CONFIG_H

#define CPU_CLOCK             80000000    // 80 Mhz
#define TOOTHGEN_PIN1         31          // which eTPU channel to use for crank simulator
#define TOOTHGEN_PIN2         30          // which eTPU channel to use for crank simulator
// TODO define eTPU channels to use here
// Note: different hardware may have to change these
// Note: what is here must match what is in SIU_OPS.c
#define FUEL_CHANNELS_1_6     5,11,12,13,14,15   //5 used instead of 10 to get LED on on deme board
#define FUEL_CHANNELS_7_12    21,22,8,9,24,25
#define FUEL_CHANNELS_13_18   24,24,24,24,24,24  /* not used */
#define FUEL_CHANNELS_19_24   25,25,25,25,25,25  /* not used */
//changed the channel 18 for testing
#define SPARK_CHANNELS_1_6    2,3,4,10,6,7        // 10 used instead of 5,to give LED to fuel 1
#define SPARK_CHANNELS_7_12   25,25,25,25,25,25   /* not used */ 
#define TACH_CHANNEL          16                  // Tach Output (pwm) eTPU chan
#define FUEL_PUMP_PIN         179                 // fuel pump
//for testing - Blink based on engine position status
#define MAP_WINDOW_CHANNEL    26    // eTPU channel to output MAP sample windows on - fixed, do not change
#define KNOCK_WINDOW_CHANNEL  28    // eTPU channel to output MAP sample windows on - fixed
#define FAKE_CAM_PIN          137   // GPIO used for semi-sequentail operation

// used for serial port A and tuner communications
#define BAUD_RATE 115200    // speed of serial port comm
#define SERIAL_BUFFER_SIZE (2048+10)

// servo motor or idle air control
#define PWM1_CHANNEL 17     // eTPU channel to use for servo motor output 
#define PWM1_PAD     131    // servo GPIO pad # for above
#define PWM1_HZ      50     // recommend 50 for servos, 200 or 300 for IAC

// increasing clock available for general use - runs at CPU_CLOCK/1000000 ticks per msec
#define MSEC_EMIOS_CHANNEL 9  // eMIOS channel to use for the msec timer (OS and many other uses)
#define hclock()  (volatile uint32_t)(EMIOS.CH[MSEC_EMIOS_CHANNEL].CCNTR.R)   

// angle clock in degrees
#define angle_clock() (volatile uint32_t)(eTPU->TB2R_A.R)

#endif
