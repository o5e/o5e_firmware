/*********************************************************************************

    @file      eTPU_OPS.c                                                              
    @date      December, 2011
    @brief     Open5xxxECU - This file contains functions to initialize AND operate the eTPU
    @note      www.Open5xxxECU.org
    @version   see below
    @copyright MIT License

*/
/*=============================================================================*/
/* REV      AUTHOR         DATE          DESCRIPTION OF                        */
/* ---      -----------    ----------    ---------------------                 */
/* 4.8me    M.Eberhardt    31/May/12     added more used variables for eng pos */ 
/* 4.7jz    J. Zeeff       22/May/12     add mke comment changes               */
/* 4.6jz    J. Zeeff       18/May/12     open window to get sync               */
/* 4.5jz    J. Zeeff       18/May/12     cleanup                               */
/* 4.4me    M.Eberhardt    18/May/12     fixed cam windowing                   */
/* 4.3jz    J. Zeeff       18/May/12     allow sim to use large cam angles     */
/* 4.2jz    J. Zeeff       17/May/12     fixed sync issue                      */
/* 4.0me    M.Eberhardt    16/May/12     fix drop dead angle                   */
/* 3.9jz    J. Zeeff       16/May/12     cleanup + fixes                       */
/* 3.8me    M.Eberhardt    16/May/12     fix Cyl_Angle_eTPU math, remove unused*/
/* 3.7me    M.Eberhardt    16/May/12     user variables in eng pos &tooth gen  */
/* 3.61     J. Zeeff       16/May/12     Various cleanup + fixes               */
/* 3.4      M.Eberhardt    1/Jan/12      Added Used Config stuff               */
/* 3.3      J. Zeeff       29/Dec/11     Various cleanup                       */
/* 3.2      J. Zeeff       29/Oct/11     Add servo function                    */
/* 3.0      P. Schlein     22/Sep/11     Add Update_eTPU function              */
/* 2.1      P. Schlein     20/Aug/11     Remove SPARK from Compilation         */
/* 2.0      P. Schlein     20/June/11    Add 6 Channels of FUEL and SPARK      */
/* 1.0      P. Schlein     8/June/10     Initial ver.-eTPU Struct, init_eTPU   */
/*******************************************************************************/

#include "config.h"
#include "system.h"
#include "etpu_util.h"
#include "variables.h"
#include "main.h"

/*  eTPU Function Set */
#include "etpu_set.h"

/*  eTPU APIs         */
#include "etpu_toothgen.h"
#include "etpu_app_eng_pos.h"
#include "etpu_fuel.h"
#include "etpu_spark.h"
#include "etpu_pwm.h"
#include "Functions.h"
#define EXTERN
#include "eTPU_OPS.h"

#ifdef MPC5634
#   include "mpc563m_vars.h"
#endif

#ifdef MPC5554
#   include "mpc5554_vars.h"
#endif

/*******************************************************************************/
/* Global Variables                                                            */
static int32_t error_code;

/* First Test value of tcr1, engine=4,125,000; Prescaler=32                    */
#define etpu_tcr1_freq (uint32_t)((CPU_CLOCK / 32)/2)
uint32_t etpu_a_tcr1_freq = etpu_tcr1_freq;     // used by FreeScale code
uint32_t etpu_b_tcr1_freq = etpu_tcr1_freq;

/*******************************************************************************/
/*                                                                             */
/* etpu_config structure filled according to the user settings                 */
/*                                                                             */
/*******************************************************************************/

/*This structure follows the definition in eTPU_util.h that sets up the        */
/*series-mcr, misc, ecr_a, tbcr_a, stacr_a, ecr_b, tbcr_b and stcr_b           */
/*and defines the variables used below.                                        */

struct etpu_config_t my_etpu_config = {
    /*eTPU MCR Register */
    FS_ETPU_MISC_DISABLE,
    /*MISC from compiler link file */
    FS_ETPU_MISC,
    /*Configure eTPU Engine A ECR-see RM, pg18-23, 707 */
    FS_ETPU_FILTER_CLOCK_DIV8 + FS_ETPU_CHAN_FILTER_3SAMPLE + FS_ETPU_ENTRY_TABLE,
    /*Configure eTPU Engine A Timebases-see RM, Chapter 4  */
    FS_ETPU_TCR1CTL_DIV2 +      // TCR1 clock control from sysclk/2
        31 +                    // TCR1 prescaler 32
        FS_ETPU_TCR2CTL_RISE + FS_ETPU_ANGLE_MODE_ENABLE +      // TCR2 clock control (angle clock)
        (0 << 16) +             // TCR2 prescaler 1
        FS_ETPU_TCRCLK_INPUT_DIV2CLOCK + FS_ETPU_TCRCLK_MODE_2SAMPLE,   // TCRCLK input filter control

    /*Configure eTPU Engine A STAC CR - not used */
    0,
    /*Configure eTPU Engine B ECR - ignored on 5634 */
    FS_ETPU_FILTER_CLOCK_DIV4 + FS_ETPU_CHAN_FILTER_3SAMPLE + FS_ETPU_ENTRY_TABLE,
    /*Configure eTPU Engine B timebases */
    FS_ETPU_TCR2CTL_DIV8 + (7 << 16) +  /*TCR2 prescaler of 8 (7+1) */
        FS_ETPU_TCR1CTL_DIV2 +  /*TCR1 PRESCALER OF 32 (31+1) APPLIED TO SYSCLK/2 */
        31,
    /*Configure eTPU Engine B STAC CR */
    0,
    /*this is SRV2--try to find details in documentation.. */
    0,0                     // ???? TODO */
};

/*********************************************************************************

    @brief   Set up eTPU(s)
    @note    This routine is NOT speed critical

*************************************************************************************/

int32_t init_eTPU()
{
    uint32_t i;
    uint24_t Cam_Window_Width;
    uint24_t Engine_Position_eTPU;
    uint24_t Cam_Window_Open;
    uint24_t Cam_Lobe_Pos_eTPU;
    uint8_t Gen_Tooth_open;
    uint8_t Gen_Tooth_close;
    uint24_t Gen_Tooth_Pos;
    uint24_t Drop_Dead_Angle_eTPU;
    uint8_t Cam_Edge_Select_eTPU;

    // Load firmware into eTPU
    error_code = (int32_t)
        fs_etpu_init(my_etpu_config, (uint32_t *) etpu_code, sizeof(etpu_code), (uint32_t *) etpu_globals, sizeof(etpu_globals));

    if (error_code != 0) 
        system_error(12379, __FILE__, __LINE__, "");

    if (N_Cyl > 12)
        return -1;              // check for flash misconfig

    
    // The goal here is to open a cam window that that will work with the cam position
    Engine_Position_eTPU = (72000 - ((uint32_t)Engine_Position << 2));   // adjust bin -2 to bin 0
    Cam_Lobe_Pos_eTPU = (72000 - ((uint32_t)Cam_Lobe_Pos << 2)) ;      // adjust bin -2 to bin 0
    Cam_Window_Open = (72000 + Cam_Lobe_Pos_eTPU - (Cam_Window_Open_Set << 2) ) % 72000; //adjust bin -2 to bin 0
    Cam_Window_Width = Cam_Window_Width_Set <<2; //adjust bin -2 to bin 0
    Cam_Edge_Select_eTPU = Cam_Edge_Select; //for normal operation allow user setting
    // set the cam window correctly for semi-sequentail mode
    if (Sync_Mode_Select ||(Engine_Type_Select && Sync_Mode_Select == 0)){
       Cam_Window_Open = 36000;
       Cam_Window_Width = 35999;
       Cam_Edge_Select_eTPU = 0; // use rising edge to match generated tooth
    }

// Links cause a stall to notify some other channels and turn them off - 4 packed into each 32 bit word
// TODO change to variables and some logic to handle different configs (fuel has priority over spark)
#   define Link1  0x01010101    /* cam - repeated as filler */
#   define Link2  (Fuel_Channels[0] << 12) |  (Fuel_Channels[1] << 8) | (Fuel_Channels[2] << 4) | (Fuel_Channels[3] << 0)
#   define Link3  (Fuel_Channels[4] << 12) |  (Fuel_Channels[5] << 8) | (Fuel_Channels[6] << 4) | (Fuel_Channels[7] << 0)
#   define Link4  (Fuel_Channels[8] << 12) |  (Fuel_Channels[9] << 8) | (Fuel_Channels[10] << 4) | (Fuel_Channels[11] << 0)


    // Initialization of eTPU channel settings                        
    // eTPU API Function Init: 'Engine Position (CAM and CRANK channels)'
    // AN3769, pg16-18                                    
    // Note: crank on pin 0, cam on pin 1
    // Windowing ratios are from FreeScale code

    error_code = fs_etpu_app_eng_pos_init(1,                            /* CAM in engine: A; channel: 1 */
                                        FS_ETPU_CAM_PRIORITY_MIDDLE,    /* cam_priority: Middle */
                                        Cam_Edge_Select_eTPU,           /* cam_edge_polarity: falling edge = 0, rising = 1 */
                                        Cam_Window_Open,                /* cam_angle_window_start: cam_window_open*100   */
                                        Cam_Window_Width,               /* cam_angle_window_width: cam_window_width*100   */
                                        0,                              /* CRANK in engine: A; channel: 0 */
                                        FS_ETPU_CRANK_PRIORITY_MIDDLE,  /* crank_priority: Middle */
                                        Crank_Edge_Select,              /* crank_edge_polarity: falling edge = 0, rising = 1 */
                                        N_Teeth,                        /* crank_number_of_physical_teeth: crank_number_of_physical_teeth */
                                        Missing_Teeth,                  /* crank_number_of_missing_teeth: crank_number_of_missing_teeth */
                                        2,                             /* crank_blank_tooth_count: crank_blank_tooth_count */
                                        60,                             /* crank_tcr2_ticks_per_tooth: crank_tcr2_ticks_per_tooth */
                                        0x199999,                       /* crank_windowing_ratio_normal: 0x199999 */
                                        0x199999,                       /* crank_windowing_ratio_after_gap: 0x199999 */
                                        0x199999,                       /* crank_windowing_ratio_across_gap: 0x199999 */
                                        0x2fffff,                       /* crank_windowing_ratio_timeout: 0x299999 */
                                        0x9fffff,                       /* crank_gap_ratio: 0x9fffff */
                                        5,                              /* crank_blank_time_ms: 5 */
                                        200000,                          /* crank_first_tooth_timeout_us: 40000 */
                                        Link1,Link2, Link3, Link4,      /* a stall will notify these other channels */
                                        etpu_tcr1_freq);                /* tcr1_timebase_freq: etpu_a_tcr1_freq */
    if (error_code != 0) 
        system_error(43379, __FILE__, __LINE__, "");

#if 1
    // Engine crank/cam simulator for testing
    /* Initialize TOOTHGEN function */
    // calculate a cam signal (in teeth) that agrees with the user's input for position

    // Find position for cam signal based on user input.  
    // Note, users input the number of degrees that the rising edge of cam _precedes_ the rising edge of the next tooth 1
    // User input is always within 0-359 (or less) 
    // Example: 35-1 wheel with lobe position = 0 results in cam rising at the rising edge of tooth 37 (aka tooth 1)
    // TODO - When falling edge is used in testing the cam signal can be up to 1.5 teeth late becaue the math is base on the rising edge, issue #6
#define Start_Tooth (1 + Total_Teeth - (((uint32_t)Cam_Lobe_Pos << 2) / Degrees_Per_Tooth_x100))  // adjust x100 bin -2 value to x100 bin 0 before using


    error_code =
        fs_etpu_toothgen_init(TOOTHGEN_PIN1,              // crank channel
                              TOOTHGEN_PIN2,              // cam channel
                              FS_ETPU_PRIORITY_LOW,       // priority
                              N_Teeth,                    // number of physical teeth
                              Missing_Teeth,              // number of missing teeth
                              (ufract24_t)(.5 * (1<<24)), // tooth duty cycle 50%
                              1,                          // tooth number to generate first (not important)
                              Test_RPM,                   // engine speed
                              etpu_a_tcr1_freq,           //
                              (uint8_t)Start_Tooth,       // tooth to start cam signal (tooth #s start with 1)
                              (uint8_t)Start_Tooth+2      // tooth to end cam signal
                              );
    if (error_code != 0) 
        system_error(13379, __FILE__, __LINE__, "");
#endif

    // set up fuel and spark

    // might be using staged injection
    N_Injectors = N_Cyl;
    if (Staged_Inj > 0) 
       N_Injectors *= 2;

    // might be using wasted spark
    N_Coils = N_Cyl;
    if ( N_Coils == 1)//N_Coils_Per_Cylinder
       N_Coils /= 2;

    // Calculate the cylinder angles from user inputs
    for (i = 0; i < N_Cyl; ++i) {
        Cyl_Angle_eTPU[i] = (((int32_t)Cyl_Offset_Array[i] << 2 ) + Engine_Position_eTPU) % 72000;  // << to convert bin -2 to bin 0
        if (Staged_Inj) 
           Cyl_Angle_eTPU[i+N_Cyl] = Cyl_Angle_eTPU[i];
    }

    Drop_Dead_Angle_eTPU = ((Drop_Dead_Angle  << 2 ) + Engine_Position_eTPU) % 72000;   // << to convert bin -2 x100 to bin 0 x100

    // eTPU API Function initialization: 'fuel'-see AN3770, pg7-9
    error_code = fs_etpu_fuel_init_6cylinders(FUEL_CHANNELS_1_6,        // 
                                              1,                        // CAM in engine: A; channel: 1 
                                              FS_ETPU_PRIORITY_LOW, 
                                              FS_ETPU_FUEL_FM0_ACTIVE_HIGH,       // 
                                              Cyl_Angle_eTPU[0],        // 
                                              Cyl_Angle_eTPU[1],        // 
                                              Cyl_Angle_eTPU[2],        // 
                                              Cyl_Angle_eTPU[3],        // 
                                              Cyl_Angle_eTPU[4],        // 
                                              Cyl_Angle_eTPU[5],        // offset angles * 100
                                              Drop_Dead_Angle_eTPU,     // drop dead angle*100 
                                              0,                        // normal end angle*100
                                              1500,                     // recalc offset ANGLE*100
                                              0, 0, 0, 0, 0, 0,         // injection time, usec
                                              0,                        // comp time, usec
                                              100,                      // min inject time, usec
                                              100                       // min injector off time, usec
        );
    if (error_code != 0) 
        system_error(12379, __FILE__, __LINE__, "");

    if (N_Injectors > 6) { //TODO this should work but doesn't, OS locks when user changes Cyl_Count <6 to >6
        error_code = fs_etpu_fuel_init_6cylinders(FUEL_CHANNELS_7_12,   // channels  
                                                  1,                    // CAM in engine: A; channel: 1 
                                                  FS_ETPU_PRIORITY_LOW, 
                                                  FS_ETPU_FUEL_FM0_ACTIVE_HIGH,   // 
                                                  Cyl_Angle_eTPU[6],    // offset angles * 100
                                                  Cyl_Angle_eTPU[7], 
                                                  Cyl_Angle_eTPU[8], 
                                                  Cyl_Angle_eTPU[9], 
                                                  Cyl_Angle_eTPU[10], 
                                                  Cyl_Angle_eTPU[11],   
                                                  Drop_Dead_Angle_eTPU, // drop dead angle*100
                                                  0,                    // normal end angle*100
                                                  1500,                 // recalc offset ANGLE*100
                                                  0, 0, 0, 0, 0, 0,     // injection time, usec
                                                  0,                    // comp time, usec
                                                  100,                  // min inject time, usec
                                                  100                   // min injector off time, usec
            );
        if (error_code != 0) 
            system_error(12479, __FILE__, __LINE__, "");
    } 
// MPC5554 can have more than 12 fuel
#   ifdef MPC5554
    if (N_Injectors > 12) {
        error_code = fs_etpu_fuel_init_6cylinders(FUEL_CHANNELS_13_18,  // channels  
                                                  1,                    // CAM in engine: A; channel: 1 
                                                  FS_ETPU_PRIORITY_LOW, 
                                                  FS_ETPU_FUEL_FM0_ACTIVE_HIGH, 
                                                  Cyl_Angle_eTPU[12],   // offset angles * 100
                                                  Cyl_Angle_eTPU[13], 
                                                  Cyl_Angle_eTPU[14], 
                                                  Cyl_Angle_eTPU[15], 
                                                  Cyl_Angle_eTPU[16], 
                                                  Cyl_Angle_eTPU[17],   
                                                  Drop_Dead_Angle_eTPU,         // drop dead angle*100
                                                  0,                    // normal end angle*100
                                                  1500,                 // recalc offset ANGLE*100
                                                  0, 0, 0, 0, 0, 0,     // injection time, usec
                                                  0,                    // comp time, usec
                                                  100,                  // min inject time, usec
                                                  100                   // min injector off time, usec
            );
        if (error_code != 0) 
            system_error(12579, __FILE__, __LINE__, "");
    }

    if (N_Injectors > 18) {
        error_code = fs_etpu_fuel_init_6cylinders(FUEL_CHANNELS_19_24, 
                                                  1,                    // CAM in engine: A; channel: 1 
                                                  FS_ETPU_PRIORITY_LOW, 
                                                  FS_ETPU_FUEL_FM0_ACTIVE_HIGH,   
                                                  Cyl_Angle_eTPU[18],   // offset angles*100
                                                  Cyl_Angle_eTPU[19], 
                                                  Cyl_Angle_eTPU[20], 
                                                  Cyl_Angle_eTPU[21], 
                                                  Cyl_Angle_eTPU[22], 
                                                  Cyl_Angle_eTPU[23],   // offset angles * 100
                                                  Drop_Dead_Angle_eTPU, // drop dead angle*100
                                                  0,                    // normal end angle*100
                                                  1500,                 // recalc offset ANGLE*100
                                                  0, 0, 0, 0, 0, 0,     // injection time, usec
                                                  0,                    // comp time, usec
                                                  100,                  // min inject time, usec
                                                  100                   // min injector off time, usec
            );
        if (error_code != 0) 
            system_error(12679, __FILE__, __LINE__, "");
    }
#   endif

    // eTPU API Function initialization: 'spark'
    // in cylinder order not firing order

    error_code = fs_etpu_spark_init_6cylinders(SPARK_CHANNELS_1_6,      // 
                                               1,                       // CAM in engine: A; channel: 1
                                               // offset angles*100
                                               Cyl_Angle_eTPU[0],       // 
                                               Cyl_Angle_eTPU[1],       // 
                                               Cyl_Angle_eTPU[2],       // 
                                               Cyl_Angle_eTPU[3],       // 
                                               Cyl_Angle_eTPU[4],       // 
                                               Cyl_Angle_eTPU[5],       // offset angles * 100
                                               FS_ETPU_PRIORITY_MIDDLE,  
                                               Ignition_Invert,         // Fire high or low
                                               0,                       // min_coil_dur, usec 
                                               4000,                    // max_coil_dur, usec 
                                               0,                       // multi spark on time 
                                               0,                       // multi spark on time
                                               0,                       // multi nmbr pulses
                                               1500,                    // recalc offset angles*100
                                               3000,                    // init dwell time 1, usec 
                                               (uint32_t)(3000 * Ignition_Type),  // init dwell time 2, usec, 0 if direct fire
                                               5000,                    // init end angle 1*100
                                               41000                    // init end angle 2*100                                           
        );
    if (error_code != 0) 
        system_error(12789, __FILE__, __LINE__, "");

// 5554 can have more
#   ifdef MPC5554
    if ((N_Coils > 6) && (N_Injectors < 19)) {
        error_code = fs_etpu_spark_init_6cylinders(SPARK_CHANNELS_7_12, 
                                                   1,                   // CAM in engine: A; channel: 1
                                                   Cyl_Angle_eTPU[6], 
                                                   Cyl_Angle_eTPU[7], 
                                                   Cyl_Angle_eTPU[8], 
                                                   Cyl_Angle_eTPU[9], 
                                                   Cyl_Angle_eTPU[10], 
                                                   Cyl_Angle_eTPU[11],  // offset angles * 100
                                                   FS_ETPU_PRIORITY_MIDDLE, 
                                                   Ignition_Invert,     // Fire high or low
                                                   0,                   // min_coil_dur, usec 
                                                   4000,                // max_coil_dur, usec
                                                   500,                 // multi spark on time 
                                                   0,                   // multi spark on time
                                                   0,                   // multi nmbr pulses
                                                   1500,                // recalc offset angles*100
                                                   3000,                // init dwell time 1, usec 
                                                   (3000 * Ignition_Type),      // init dwell time 2, usec, 0 if direct fire
                                                   5000,                // init end angle 1*100
                                                   41000                // init end angle 2*100                                           
            );
        if (error_code != 0) 
            system_error(12889, __FILE__, __LINE__, "");
        return 0;
    }
#   endif

    return 0;
}                               // init_eTPU()

// read status - returns can be viewed in the debugger
// see etpu_crank_auto.h and AN3769

volatile int8_t s1, s2, s3, s4, s5;
int16_t Cam_Errors;
int16_t Crank_Errors;

void Check_Engine(void)
{
    s1 = fs_etpu_eng_pos_get_engine_position_status();  // returns the overall engine position status
    s2 = fs_etpu_eng_pos_get_cam_status();              // returns the status of the CAM function
    s3 = fs_etpu_eng_pos_get_crank_status();            // returns the status of the CRANK function
    s4 = fs_etpu_eng_pos_get_cam_error_status();        // returns the error status of the CAM function
    s5 = fs_etpu_eng_pos_get_crank_error_status();      // returns the error status of the CRANK function

// TODO turn this on
#if 0
    if (s4) {
       ++Cam_Errors;
       fs_etpu_eng_pos_clear_cam_error_status();
    }

    if (s5) {
       ++Crank_Errors;
       fs_etpu_eng_pos_clear_crank_error_status();
    }
#endif

} // Check_Engine()


/****************************************************************************

   @note Use the eTPU to maintain a single channel servo or PWM valve position.

   @note Standard servo motors use 50Hz and duty cycle can range from 5% to 10% (500-1000 or 1-2 msec)
   @note Ford PWM idle valves run best at a frequency of around 300-320 Hz with 0-100% duty cycle
   @note Bosch 3 wire PWM idle valves run best at a frequency of 200 Hz with 10-80% duty cycle
   @param Supply duty cycle % x 100

   @ note - it would probably be better to use the eMIOS for this

*****************************************************************************/
static uint32_t PWM1_frequency;
void init_PWM1(uint32_t frequency)
{

    // start with 0% DC
    error_code =
        fs_etpu_pwm_init(PWM1_CHANNEL, FS_ETPU_PRIORITY_LOW, frequency, 0, FS_ETPU_PWM_ACTIVEHIGH, FS_ETPU_TCR1,
                         etpu_tcr1_freq);
    if (error_code != 0)
        system_error(24579, __FILE__, __LINE__, "");
    PWM1_frequency = frequency;
}

void update_PWM1(uint32_t duty_cycle)
{
    if (duty_cycle > 10000)     // clip to 100% DC
        duty_cycle = 10000;

    // update to new duty cycle
    error_code = fs_etpu_pwm_update(PWM1_CHANNEL, PWM1_frequency, (uint16_t)duty_cycle, etpu_tcr1_freq);
    if (error_code != 0)
        system_error(34379, __FILE__, __LINE__, "");
}                               // update_PWM1()

/****************************************************************************

   @note Use the eTPU to Output an RPM signal.

*****************************************************************************/
void Init_Tach(void)
{
    error_code =
        fs_etpu_pwm_init(TACH_CHANNEL, FS_ETPU_PRIORITY_LOW, 1, 1000, FS_ETPU_PWM_ACTIVEHIGH, FS_ETPU_TCR1, etpu_tcr1_freq);
    if (error_code != 0)
        system_error(24579, __FILE__, __LINE__, "");
}

void Update_Tach(uint32_t frequency)
{
    // update to new freq
    error_code = fs_etpu_pwm_update(TACH_CHANNEL, frequency, 1000, etpu_tcr1_freq);
    if (error_code != 0)
        system_error(34379, __FILE__, __LINE__, "");
}