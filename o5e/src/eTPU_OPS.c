/*********************************************************************************

    @file      eTPU_OPS.c                                                              
    @date      December, 2011
    @brief     Open5xxxECU - This file contains functions to initialize AND operate the eTPU
    @note      www.Open5xxxECU.org
    @version   see below
    @copyright 2011, 2012, P. Schlein, M. Eberhardt, J. Zeeff

***********************************************************************************/

// Portions Copyright 2011         P. Schlein - MIT License
// Portions Copyright 2011,2012    M. Eberhardt - MIT License
// Portions Copyright 2011,2012    Jon Zeeff - All rights reserved

// enable generation of crank and cam pulses
#define SIMULATOR       

#include "config.h"
#include "etpu_util.h"
#include "variables.h"
#include "err.h"

/*  eTPU Function Set */
#include "etpu_set.h"

/*  eTPU APIs         */
#include "etpu_toothgen.h"
#include "etpu_app_eng_pos.h"
#include "etpu_fuel.h"
#include "etpu_spark.h"
#include "etpu_pwm.h"
#include "eTPU_OPS.h"
#include "etpu_fpm.h"
#include "main.h"   /**< pickup msec_clock */

uint8_t N_Injectors;
uint8_t N_Coils;
uint8_t Spark_Channels[12] = { SPARK_CHANNELS_1_6, SPARK_CHANNELS_7_12 } ;
uint8_t Fuel_Channels[24] = { FUEL_CHANNELS_1_6, FUEL_CHANNELS_7_12, FUEL_CHANNELS_13_18, FUEL_CHANNELS_19_24 };
uint8_t Wheel_Speed_Channels[4] = {WHEEL_SPEED_1_4};


static uint24_t Cyl_Angle_eTPU[24];
static int32_t error_code;

/* First Test value of tcr1, engine=4,125,000; Prescaler=32                    */
uint32_t etpu_a_tcr1_freq = etpu_tcr1_freq;     // used by FreeScale code
uint32_t etpu_b_tcr1_freq = etpu_tcr1_freq;

/* eTPU characteristics definition */
struct eTPU_struct *eTPU = (struct eTPU_struct *)0xC3FC0000;

uint32_t fs_etpu_code_start =     0xC3FD0000;
uint32_t fs_etpu_data_ram_start = 0xC3FC8000;
uint32_t fs_etpu_data_ram_end =   0xC3FC89FC;
uint32_t fs_etpu_data_ram_ext =   0xC3FCC000;

/*******************************************************************************/
/*                                                                             */
/* etpu_config structure filled according to the user settings                 */
/*                                                                             */
/*******************************************************************************/

#include "engine_etpu_gct.h"

struct etpu_config_t my_etpu_config = {
  /* etpu_config.mcr */
  FS_ETPU_VIS_OFF                /* SCM not visible (VIS=0) */
  | FS_ETPU_MISC_DISABLE,        /* SCM operation disabled (SCMMISEN=0) */

  /* etpu_config.misc */
  FS_ETPU_MISC,

  /* etpu_config.ecr_a */
  FS_ETPU_ENTRY_TABLE_ADDR      /* entry table base address = shifted FS_ETPU_ENTRY_TABLE */
  | FS_ETPU_CHAN_FILTER_3SAMPLE /* channel filter mode = three-sample mode (CDFC=2) */
  | FS_ETPU_ENGINE_ENABLE       /* engine is enabled (MDIS=0) */
  | FS_ETPU_FILTER_CLOCK_DIV8,   /* channel filter clock = etpuclk div 32 (FPSCK=4) */

  /* etpu_config.tbcr_a */
  FS_ETPU_ANGLE_MODE_ENABLE     /* TCR2 works in angle mode, driven by TCRCLK input and ch.0 (AM=1) */
  | FS_ETPU_TCR2_PRESCALER(8)   /* TCR2 prescaler = 1 (TCR2P='1-1') */
  | FS_ETPU_TCR1_PRESCALER(PRESCALER)   /* TCR1 prescaler = 8 (TCR1P='8-1') */
//  | FS_ETPU_TCRCLK_INPUT_CHANCLOCK      /* TCRCLK signal is filtered with filter clock = channel filter clock (TCRCF=x1) */
  | FS_ETPU_TCR2CTL_RISE        /* TCR2 source = fall TCRCLK transition (TCR2CTL=2) */
//  | FS_ETPU_TCRCLK_MODE_INTEGRATION     /* TCRCLK signal is filtered using integration mode (TCRCF=1x) */
  | FS_ETPU_TCRCLK_INPUT_DIV2CLOCK
  | FS_ETPU_TCRCLK_MODE_2SAMPLE
  | FS_ETPU_TCR1CTL_DIV2,        /* TCR1 source = etpuclk div 2 (see also TCR1CS if using eTPU2) */

  /* etpu_config.stacr_a */
  FS_ETPU_TCR1_STAC_SERVER      /* TCR1 resource operates as server (RSC1=1) */
  | FS_ETPU_TCR2_STAC_SERVER    /* TCR2 resource operates as server (RSC2=1) */
  | FS_ETPU_TCR1_STAC_DISABLE   /* TCR1 operation on STAC bus disabled (REN1=0) */
  | FS_ETPU_TCR2_STAC_DISABLE   /* TCR2 operation on STAC bus disabled (REN2=0) */
  | FS_ETPU_TCR1_STAC_SRVSLOT(0)        /* TCR1 resource server slot = 0 (SRV1=0) */
  | FS_ETPU_TCR2_STAC_SRVSLOT(0),        /* TCR2 resource server slot = 0 (SRV2=0) */

 /* etpu_config.ecr_b */
  FS_ETPU_ENTRY_TABLE_ADDR      /* entry table base address = shifted FS_ETPU_ENTRY_TABLE */
  | FS_ETPU_CHAN_FILTER_3SAMPLE /* channel filter mode = two-sample mode (CDFC=0) */
  | FS_ETPU_ENGINE_ENABLE       /* engine is enabled (MDIS=0) */
  | FS_ETPU_FILTER_CLOCK_DIV8,   /* channel filter clock = etpuclk div 2 (FPSCK=0), see also FCSS if using eTPU2 */

  /* etpu_config.tbcr_b */
  FS_ETPU_ANGLE_MODE_ENABLE     /* TCR2 works in angle mode, driven by TCRCLK input and ch.0 (AM=1) */
  | FS_ETPU_TCR2_PRESCALER(8)   /* TCR2 prescaler = 1 (TCR2P='1-1') */
  | FS_ETPU_TCR1_PRESCALER(PRESCALER)   /* TCR1 prescaler = 8 (TCR1P='8-1') */
//  | FS_ETPU_TCRCLK_INPUT_CHANCLOCK      /* TCRCLK signal is filtered with filter clock = channel filter clock (TCRCF=x1) */
  | FS_ETPU_TCR2CTL_RISE        /* TCR2 source = fall TCRCLK transition (TCR2CTL=2) */
//  | FS_ETPU_TCRCLK_MODE_INTEGRATION     /* TCRCLK signal is filtered using integration mode (TCRCF=1x) */
  | FS_ETPU_TCRCLK_INPUT_DIV2CLOCK
  | FS_ETPU_TCRCLK_MODE_2SAMPLE
  | FS_ETPU_TCR1CTL_DIV2,        /* TCR1 source = etpuclk div 2 (see also TCR1CS if using eTPU2) */

  /* etpu_config.stacr_b */
  FS_ETPU_TCR1_STAC_SERVER      /* TCR1 resource operates as server (RSC1=1) */
  | FS_ETPU_TCR2_STAC_SERVER    /* TCR2 resource operates as server (RSC2=1) */
  | FS_ETPU_TCR1_STAC_DISABLE   /* TCR1 operation on STAC bus disabled (REN1=0) */
  | FS_ETPU_TCR2_STAC_DISABLE   /* TCR2 operation on STAC bus disabled (REN2=0) */
  | FS_ETPU_TCR1_STAC_SRVSLOT(0)        /* TCR1 resource server slot = 0 (SRV1=0) */
  | FS_ETPU_TCR2_STAC_SRVSLOT(0),        /* TCR2 resource server slot = 0 (SRV2=0) */

  0,0

};


/*********************************************************************************

    @brief   Set up eTPU(s)
    @note    This routine is NOT speed critical

*************************************************************************************/

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/* the above is inserted until I can figure out how this code works
   and fix it properly */

int32_t init_eTPU()
{
    uint32_t i;

    int32_t Cam_Window_Width;
    int32_t Window_Start;
    uint24_t Drop_Dead_Angle_eTPU;    
    uint24_t Engine_Position_eTPU;
    uint24_t Cam_Lode_Position_eTPU;
    uint8_t Gen_Tooth_open;
    uint8_t Gen_Tooth_close;
    uint8_t Cam_Edge_Select_eTPU;
    
    #define Fake_Cam_Window_Width 72000 // Use 120*100 for the width of the cam window when Fake cam is used
	#define Fake_Cam_Lobe_Position 54000 // Use 540 8 100 as athe location of fake cam
    // Load firmware into eTPU
    error_code = (int32_t)
        fs_etpu_init(my_etpu_config, (uint32_t *) etpu_code, sizeof(etpu_code), (uint32_t *) etpu_globals, sizeof(etpu_globals));

    if (error_code != 0) 
        err_push( CODE_OLDJUNK_E0 );

    if (N_Cyl > 12)
        err_push( CODE_OLDJUNK_DF );
        // The goal here is to open a cam window that that will work with the cam position
    Engine_Position_eTPU = (72000 - ((uint32_t)Engine_Position << 2));   // adjust bin -2 to bin 0
    Cam_Lode_Position_eTPU = (72000-((uint32_t)Cam_Lobe_Pos<< 2));   // adjust bin -2 to bin 0
    Cam_Window_Width = Cam_Window_Width_Set <<2; //adjust bin -2 to bin 0
    Cam_Edge_Select_eTPU = Cam_Edge_Select; //for normal operation allow user setting

          // set the cam window correctly for semi-sequentail mode   
    if (Sync_Mode_Select ||(Engine_Type_Select && Sync_Mode_Select == 0)){
       Cam_Window_Width =  Fake_Cam_Window_Width;
       Cam_Lode_Position_eTPU = Fake_Cam_Lobe_Position;
       Cam_Edge_Select_eTPU = 0; // use rising edge to match generated tooth  
	}
    // cam window starts 1/2 of the window before the cam signal

     Window_Start =  (Cam_Lode_Position_eTPU - Cam_Window_Width / 2); // cam_angle_window_start 
    //removed gap stuff for testing
    /* 
    int32_t crank_gap_ratio = 0xafffff;
    
   
    if (Missing_Teeth > 1) {
    	crank_gap_ratio = 0xffffff;
    } 
    */   



    // check that these are always true for the tcr1 rate in use
    // trr = ((Tooth_Period << 3) / (Ticks_Per_Tooth)) << 6;
    // trr < 2^24
    // tcr2 Ticks_Per_Tooth < 200
    // Tooth_Period in tcr1 ticks  >  100 at the fastest rpm
    // Tooth_Period in tcr1 ticks  <  2000000 at the slowest rpm
    
// Links cause a stall to notify some other channels and turn them off - 4 packed into each 32 bit word
// TODO change to variables and some logic to handle > 8 configs (fuel has priority over spark)
// Always include the CAM channel (1)
#   define Link1  (uint32_t)((Fuel_Channels[0] << 12) |  (Fuel_Channels[1] << 8) | (Fuel_Channels[2] << 4) | (Fuel_Channels[3] << 0))
#   define Link2  (uint32_t)((Fuel_Channels[4] << 12) |  (Fuel_Channels[5] << 8) | (Fuel_Channels[6] << 4) | (Fuel_Channels[7] << 0))
#   define Link3  (uint32_t)((Spark_Channels[0] << 12) |  (Spark_Channels[1] << 8) | (Spark_Channels[2] << 4) | (Spark_Channels[3] << 0))
#   define Link4  (uint32_t)((Spark_Channels[4] << 12) |  (Spark_Channels[5] << 8) | (Spark_Channels[6] << 4) | (1 << 0))

    // Initialization of eTPU channel settings                        
    // eTPU API Function Init: 'Engine Position (CAM and CRANK channels)'
    // AN3769, pg16-18                                    
    // Note: crank on pin 0, cam on pin 1

    error_code = fs_etpu_app_eng_pos_init(1,                            /* CAM in engine: A; channel: 1 */
                                        FS_ETPU_CAM_PRIORITY_MIDDLE,    /* cam_priority: Middle */
                                        Cam_Edge_Select_eTPU,                /* cam_edge_polarity: rising(1) or falling edge(0) */
                                        Window_Start,                   /* cam_angle_window_start: cam_window_open*100   */
                                        Cam_Window_Width,               /* cam_angle_window_width: cam_window_width*100   */
                                        0,                              /* CRANK in engine: A; channel: 0 */
                                        FS_ETPU_CRANK_PRIORITY_MIDDLE,  /* crank_priority: Middle */
                                        Crank_Edge_Select,               /* crank_edge_polarity: rising(1) or falling edge(0) */
                                        N_Teeth,                        /* crank_number_of_physical_teeth */
                                        Missing_Teeth,                  /* crank_number_of_missing_teeth */
                                        Total_Teeth/3,                  /* crank_blank_tooth_count */
                                        Ticks_Per_Tooth,                /* crank_tcr2_ticks_per_tooth */

                                        crank_windowing_ratio_normal_set,   /* crank_windowing_ratio_normal: 0x199999 */
                                        crank_windowing_ratio_after_gap_set,/* crank_windowing_ratio_after_gap: 0x199999 */
                                        crank_windowing_ratio_across_gap_set,/* crank_windowing_ratio_across_gap: 0x199999 */
                                        crank_windowing_ratio_timeout_set,  /* crank_windowing_ratio_timeout: 0x299999 */
                                        crank_gap_ratio_set,                /* crank_gap_ratio: 0x9fffff */

                                        5,                              /* crank_blank_time_ms */
                                        700000/Total_Teeth,             /* crank_first_tooth_timeout_us */
                                        Link1, Link2, Link3, Link4,      /* a stall will notify these other channels */
                                        etpu_tcr1_freq);                /* tcr1_timebase_freq: etpu_a_tcr1_freq */
    if (error_code != 0) 
        err_push( CODE_OLDJUNK_DE );

#ifdef SIMULATOR
    // Engine crank/cam simulator for testing
    // Note: uses only rising edges
    // calculate a cam signal (in teeth) that agrees with the user's input for position
    // Note, users input the number of degrees that the rising edge of cam follows the rising edge of tooth 1 (the tooth after the gap)
    // User input is always within the range 0-71999
    // Example: 35-1 wheel with lobe position = 0 results in cam rising at the rising edge of tooth 1
    // TODO - When falling edge is used in testing the cam signal can be up to 1.5 teeth late becaue the math is base on the rising edge, issue #6

    if (Total_Teeth < 4 || Total_Teeth > 90 || Start_Tooth < 1 || Start_Tooth >= (Total_Teeth * 2))
        err_push( CODE_OLDJUNK_DD );

    error_code =
        fs_etpu_toothgen_init(TOOTHGEN_PIN1,              // crank channel
                              TOOTHGEN_PIN2,              // cam channel
                              FS_ETPU_PRIORITY_LOW,       // priority
                              N_Teeth,                    // number of physical teeth
                              Missing_Teeth,              // number of missing teeth
                              (ufract24_t)(.5 * (1<<24)), // tooth duty cycle 50%
                              1,                          // tooth number to generate first (not important)
                              500,                          // engine speed
                              etpu_a_tcr1_freq,           //
                              (uint8_t)Start_Tooth,       // tooth to start cam signal (tooth #s start with 1)
                              (uint8_t)Start_Tooth+1      // tooth to end cam signal
                              );
    if (error_code != 0) 
        err_push( CODE_OLDJUNK_DC );

    RPM = Test_RPM+1;     // set inital value

#endif

    // set up fuel and spark

    // might be using staged injection
    N_Injectors = N_Cyl;
    //if (Staged_Inj > 0) 
     //  N_Injectors *= 2;

    // might be using wasted spark
    N_Coils = N_Cyl;
    if (Ignition_Type == 1)                   // one pin drives two coils/plugs
       N_Coils = (N_Coils + 1) / 2;

    // Calculate the cylinder angles from user inputs
    Engine_Position_eTPU = (72000 - ((uint32_t)Engine_Position << 2));   // adjust bin -2 to bin 0

    for (i = 0; i < N_Cyl; ++i) {
        Cyl_Angle_eTPU[i] = (((int32_t)Cyl_Offset_Array[i] << 2 ) + Engine_Position_eTPU) % 72000;  // << to convert bin -2 to bin 0
        //if (Staged_Inj) 
        //   Cyl_Angle_eTPU[i+N_Cyl] = Cyl_Angle_eTPU[i];
    }

    Drop_Dead_Angle_eTPU = ((Drop_Dead_Angle  << 2 ) + Engine_Position_eTPU) % 72000;   // << to convert bin -2 x100 to bin 0 x100

    for (i = 0; i <  N_Injectors; ++i) {

        // eTPU API Function initialization: 'fuel'-see AN3770, pg7-9
        error_code = fs_etpu_fuel_init_cylinder(Fuel_Channels[i],
                                              1,                        // CAM in engine: A; channel: 1 
                                              FS_ETPU_PRIORITY_MIDDLE, 
                                              FS_ETPU_FUEL_FM0_ACTIVE_HIGH,       // 
                                              Cyl_Angle_eTPU[i],        // 
                                              Drop_Dead_Angle_eTPU,     // drop dead angle*100 
                                              70000,                    // normal end angle*100 relative to ??
                                              1500,                     // recalc offset ANGLE*100
                                              0,                        // injection time, usec
                                              100,                      // comp time, usec
                                              100,                      // min inject time, usec
                                              100                       // min injector off time, usec
            );
        if (error_code != 0) 
            err_push( CODE_OLDJUNK_DB );

    } // for

    // eTPU API Function initialization: 'spark'
    // in cylinder order not firing order

    for (i = 0; i <  N_Coils; ++i) {

        error_code = fs_etpu_spark_init_cylinder(Spark_Channels[i],     // 
                                               1,                       // CAM in engine: A; channel: 1
                                               Cyl_Angle_eTPU[i],       // offset angles*100
                                               FS_ETPU_PRIORITY_HIGH, 
                                               Ignition_Invert, 
                                               500,                     // min_coil_dur, usec 
                                               15000,                   // max_coil_dur, usec 
                                               0,                       // multi spark on time 
                                               1,                       // multi spark off time
                                               0,                       // multi nmbr pulses
                                               9000,                    // recalc offset angles*100
                                               3000,                    // init dwell time 1, usec 
                                               0,                       // init dwell time 2, usec, 0 if direct fire
                                               70000,                   // init end angle 1*100
                                               70000                    // init end angle 2*100                                           
        );
        if (error_code != 0) 
           err_push( CODE_OLDJUNK_DA );

    } // for
 
 /****************************************************************************

   @note Use the eTPU to Output an RPM signal.

*****************************************************************************/
    error_code = fs_etpu_pwm_init(TACH_CHANNEL, 
                                  FS_ETPU_PRIORITY_LOW, 
                                  1,                        //frequency in hz
                                  1000,                     //duty cycle
                                  FS_ETPU_PWM_ACTIVEHIGH, 
                                  FS_ETPU_TCR1, 
                                  etpu_tcr1_freq);
    if (error_code != 0)
        err_push( CODE_OLDJUNK_D7 ); 
  
/****************************************************************************

   @note Use the eTPU to maintain a single channel servo or PWM valve position.

   @note Standard servo motors use 50Hz and duty cycle can range from 5% to 10% (500-1000 or 1-2 msec)
   @note Ford PWM idle valves run best at a frequency of around 300-320 Hz with 0-100% duty cycle
   @note Bosch 3 wire PWM idle valves run best at a frequency of 200 Hz with 10-80% duty cycle
   @param Supply duty cycle % x 100

   @ note - it would probably be better to use the eMIOS for this

*****************************************************************************/


    // start with 0% DC
     error_code = fs_etpu_pwm_init(PWM1_CHANNEL, 
                                  FS_ETPU_PRIORITY_LOW, 
                                  1000,                     //frequency in hz
                                  1000,                     //duty cycle
                                  FS_ETPU_PWM_ACTIVEHIGH, 
                                  FS_ETPU_TCR1, 
                                  etpu_tcr1_freq);
    if (error_code != 0)
        err_push( CODE_OLDJUNK_D9 );
/*
static void update_PWM1(uint32_t duty_cycle)
{
    if (duty_cycle > 10000)     // clip to 100% DC
        duty_cycle = 10000;

    // update to new duty cycle
    error_code = fs_etpu_pwm_update(PWM1_CHANNEL, PWM1_frequency, (uint16_t)duty_cycle, etpu_tcr1_freq);
    if (error_code != 0)
        err_push( CODE_OLDJUNK_D8 );
}                               // update_PWM1()
*/  
 /****************************************************************************

   @note Use the eTPU to read frequency.

*****************************************************************************/  
for (i = 0; i <  4; ++i) {

	 error_code = fs_etpu_fpm_init (Wheel_Speed_Channels[i],
                         		    FS_ETPU_PRIORITY_LOW,
                         		    FS_ETPU_FPM_CONTINUOUS,
                         		    FS_ETPU_FPM_RISING_EDGE,    //or FS_ETPU_FPM_FALLING_EDGE
                          			FS_ETPU_TCR1, 
                          			0x700);						//# of tcr ticks to use in freq cal




}
 
 return 0;
}                               // init_eTPU()

  
#if __CWCC__
#pragma pop
#endif




/****************************************************************************

   @note Use the eTPU to maintain a single channel servo or PWM valve position.

   @note Standard servo motors use 50Hz and duty cycle can range from 5% to 10% (500-1000 or 1-2 msec)
   @note Ford PWM idle valves run best at a frequency of around 300-320 Hz with 0-100% duty cycle
   @note Bosch 3 wire PWM idle valves run best at a frequency of 200 Hz with 10-80% duty cycle
   @param Supply duty cycle % x 100

   @ note - it would probably be better to use the eMIOS for this

*****************************************************************************/
/*
static uint32_t PWM1_frequency;
static void init_PWM1(uint32_t frequency)
{

    // start with 0% DC
    error_code = fs_etpu_pwm_init(PWM1_CHANNEL, FS_ETPU_PRIORITY_LOW, frequency, 0, 
                 FS_ETPU_PWM_ACTIVEHIGH, FS_ETPU_TCR1, etpu_tcr1_freq);
    if (error_code != 0)
        err_push( CODE_OLDJUNK_D9 );
    PWM1_frequency = frequency;
}

static void update_PWM1(uint32_t duty_cycle)
{
    if (duty_cycle > 10000)     // clip to 100% DC
        duty_cycle = 10000;

    // update to new duty cycle
    error_code = fs_etpu_pwm_update(PWM1_CHANNEL, PWM1_frequency, (uint16_t)duty_cycle, etpu_tcr1_freq);
    if (error_code != 0)
        err_push( CODE_OLDJUNK_D8 );
}                               // update_PWM1()
*/


