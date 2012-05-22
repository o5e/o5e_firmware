
// This file defines various low level items related to the exact CPU and pinout used
// Users should not normally change this file.  See config.h for higher level info.
// Also see: mpc5500_usrdefs.inc

#ifndef SYSTEM_H
#define SYSTEM_H

#include "stdint.h"

// low cost development CPU
#ifdef MPC5634
    #define CPU_CLOCK 		        80000000     // 80 Mhz
    #define RPM_SIGNAL_PIN  	        32      // pin to indicate RPM TODO FIX!
    #define TOOTHGEN_PIN1  		31      // which eTPU channel to use for crank simulator
    #define TOOTHGEN_PIN2  		30      // which eTPU channel to use for crank simulator
    #define LED_PIN PIN_EMIOS10    		// LED to flash for testing
    // TODO define eTPU channels to use here
    // Note: different hardware may have to change these
    // Note: what is here must match what is in SIU_OPS.c
    #define FUEL_CHANNELS_1_6           5,11,12,13,14,15   //5 used instead of 10 to get LED on on deme board
    #define FUEL_CHANNELS_7_12  	21,22,8,9,27,29
    #define FUEL_CHANNELS_13_18  	24,24,24,24,24,24  /* not used */
    #define FUEL_CHANNELS_19_24  	25,25,25,25,25,25  /* not used */
    //changed the channel 18 for testing
    #define SPARK_CHANNELS_1_6  	2,3,4,8,6,7 // 22, 8 used instead of 5,to give LED to fuel 1
    #define SPARK_CHANNELS_7_12  	25,25,25,25,25,25  /* not used */ 
    #define TACH_CHANNEL 		16 	// Tach Output (pwm) eTPU chan
    #define FUEL_PUMP_PIN 		179 // fuel pump
    //for testing - Blink based on engine position status
    #define SYNC_STATUS_PIN             190  // RPM Blink task
    #define MAP_WINDOW_CHANNEL          26    // eTPU channel to output MAP sample windows on - fixed, do not change
    #define KNOCK_WINDOW_CHANNEL        28    // eTPU channel to output MAP sample windows on - fixed
#endif

// standard CPU used
#ifdef MPC5554
    #define CPU_CLOCK 132000000     // 132 Mhz
    #define RPM_SIGNAL_PIN  	32      // pin to indicate RPM
    #define TOOTHGEN_PIN1  31     	// which eTPU channel to use for crank simulator
    #define TOOTHGEN_PIN2  30     	// which eTPU channel to use for crank simulator
    #define LED_PIN 179	        // LED to flash for testing
    // TODO define eTPU channels to use here
    // Note: different hardware may have to change these
    // Note: what is here must match what is in SIU_OPS.c
    #define FUEL_CHANNELS_1_6  		73,74,75,76,77,78
    #define FUEL_CHANNELS_7_12  	79,80,81,82,83,84
    #define FUEL_CHANNELS_13_18  	85,86,87,88,64,65
    #define FUEL_CHANNELS_19_24  	2,3,4,5,6,7
    #define SPARK_CHANNELS_1_6  	8,9,10,11,12,13
    #define SPARK_CHANNELS_7_12  	2,3,4,5,6,7
    #define TACH_CHANNEL                67 	// Tach Output (pwm) eTPU chan
    #define FUEL_PUMP_PIN               197 	// fuel pump
    //for testing - Blink based on engine position status
    #define SYNC_STATUS_PIN             190  // RPM Blink task
    #define MAP_WINDOW_CHANNEL          26    // eTPU channel to output MAP sample windows on - fixed, do not change
    #define KNOCK_WINDOW_CHANNEL        28    // eTPU channel to output MAP sample windows on - fixed
#endif

// used for serial port A and tuner communications
#define BAUD_RATE	115200		// speed of serial port comm
#define SERIAL_BUFFER_SIZE (2048+10)

// servo motor or idle air control
#define PWM1_CHANNEL 17 		// eTPU channel to use for servo motor output 
#define PWM1_PAD     131 		// servo GPIO pad # for above
#define PWM1_HZ	     50			// recommend 50 for servos, 200 or 300 for IAC

// increasing clock available for general use - runs at CPU_CLOCK/1000000 ticks per msec
#define MSEC_EMIOS_CHANNEL 9 	// eMIOS channel to use for the msec timer (OS and many other uses)
#define hclock()  (volatile uint32_t)(EMIOS.CH[MSEC_EMIOS_CHANNEL].CCNTR.R)   
// how fast the "msec" counter counts - rolls over every 2 minutes or so
#define TICKS_PER_MSEC (CPU_CLOCK / 1000000)

// angle clock in degrees
#define angle_clock() (volatile uint32_t)(eTPU->TB2R_A.R)

// how to turn a pin on/off
#define Set_Pin(pin,value)  (SIU.GPDO[pin].R = value)

#define PartNumber SIU.MIDR.B.PARTNUM    // CPU ID - is in hex, example: 0x5634

// handy macros - caution, use only with variables
#ifndef max
        #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef min
        #define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif
#ifndef xabs
        #define xabs( a ) ( ((a) > 0) ? (a) : (0-(a)) )
#endif

// unclear why these are needed
#define __MOTO__ 0
#define __MRC__ 0

#endif

