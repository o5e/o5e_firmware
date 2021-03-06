
// Users should change this file to match their hardware and compiler

#ifndef CONFIG_H
#define CONFIG_H

#define CPU_CLOCK             80000000    // 80 Mhz
#define TOOTHGEN_PIN1         31          // which eTPU channel to use for crank simulator
#define TOOTHGEN_PIN2         30          // which eTPU channel to use for crank simulator
// TODO define eTPU channels to use here
// Note: different hardware may have to change these
// Note: what is here must match what is in SIU_OPS.c
#define FUEL_CHANNELS_1_6     6,7,8,9,9,10   //5 used instead of 10 to get LED on on deme board
#define FUEL_CHANNELS_7_12    11,12,24,24,20,20
#define FUEL_CHANNELS_13_18   20,20,20,20,20,20  /* not used for 5634 */
#define FUEL_CHANNELS_19_24   20,20,20,20,20,20  /* not used for 5634*/
//changed the channel 18 for testing
#define SPARK_CHANNELS_1_6    2,3,4,5,24,24        // 10 used instead of 5,to give LED to fuel 1
#define SPARK_CHANNELS_7_12   24,24,24,24,24,24   /* 9-12 not used for 5634*/ 
#define TACH_CHANNEL          14                  // Tach Output (pwm) eTPU chan
#define FUEL_PUMP_PIN         129                 // fuel pump
#define WHEEL_SPEED_1_4       16,17,18,19      // read wheel speed
//for testing - Blink based on engine position status
#define MAP_WINDOW_CHANNEL    26    // eTPU channel to output MAP sample windows on - fixed, do not change
#define KNOCK_WINDOW_CHANNEL  28    // eTPU channel to output MAP sample windows on - fixed
#define FAKE_CAM_PIN          137   // GPIO used for semi-sequentail operation

// used for serial port A and tuner communications
#define BAUD_RATE 115200    // speed of serial port comm
#define SERIAL_BUFFER_SIZE (2048+10)

// servo motor or idle air control
#define PWM1_CHANNEL 27     // eTPU channel to use for servo motor output 
#define PWM1_PAD     143    // servo GPIO pad # for above
#define PWM1_HZ      50     // recommend 50 for servos, 200 or 300 for IAC

//Outptional outputs
#define GENERIC_OUPUT_1 135 //assign pad #
#define GENERIC_OUPUT_2 136 //assign pad #
//#define GENERIC_OUPUT_3 217 //assign pad #
//#define GENERIC_OUPUT_4 218 //assign pad #


// increasing clock available for general use - runs at CPU_CLOCK/1000000 ticks per msec
//not currently used anywhere
//#define MSEC_EMIOS_CHANNEL 9  // eMIOS channel to use for the msec timer (OS and many other uses will roll!!)
//#define hclock()  (volatile uint32_t)(EMIOS.CH[MSEC_EMIOS_CHANNEL].CCNTR.R)//!!

// angle clock in degrees
#define angle_clock() (volatile uint32_t)(eTPU->TB2R_A.R)
// used by thr eTPU angle clock
#define Ticks_Per_Tooth  120        // Max 200

// how to turn a pin on/off
#define Set_Pin(pin,value)  SIU.GPDO[pin].R = value
#define Read_Pin(pin,value)  (SIU.GPDI[pin].R & 1)


#endif
