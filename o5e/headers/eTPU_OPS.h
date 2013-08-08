/**
 * @file   eTPU_OPS.h
 * @author
 * @brief
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __etpu_ops_h
#define   __etpu_ops_h

#ifdef __cplusplus
extern "C"
{
#endif

/* TODO: figure out whats going on here. This code wasn't wrapped in a guard
         so I did it */

// defines
#define Total_Teeth (N_Teeth + Missing_Teeth)
#define Degrees_Per_Tooth_x100 (36000 / Total_Teeth)
// note: Cam_Lobe_Pos must always be in the latter half of the engine cycle - 360-720
#define Start_Tooth (1 + ((72000 - (uint32_t)Cam_Lobe_Pos ) / Degrees_Per_Tooth_x100) - Total_Teeth)

// See FreeScale documentation for selection of this value
#define PRESCALER 64UL
#define etpu_tcr1_freq (uint32_t)((CPU_CLOCK / PRESCALER)/2)

// Functions
int32_t init_eTPU(void);
int32_t fs_etpu_fuel_init_cylinder(  uint8_t  channel_1,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us);

int32_t fs_etpu_spark_init_cylinder(    uint8_t  spark_channel_1,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint8_t  priority,
                                        uint8_t  polarity,
                                        uint24_t min_dwell_time,
                                        uint24_t max_dwell_time,
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time,
                                        uint8_t  multi_num_pulses,
                                        uint24_t recalc_offset_angle,
                                        uint24_t init_dwell_time_1,
                                        uint24_t init_dwell_time_2,
                                        uint24_t init_end_angle_1,
                                        uint24_t init_end_angle_2 );


// Variables
extern uint8_t N_Injectors;
extern uint8_t N_Coils;

// What eTPU channel to use for each coil in an array form
extern uint8_t Spark_Channels[12];

// What eTPU channel to use for each injector in an array form
extern uint8_t Fuel_Channels[24];

// What eTPU channel to use fro each speed speed in an array form
extern uint8_t Wheel_Speed_Channels[4];

//Make tcr clocks available
extern uint32_t etpu_a_tcr1_freq;
extern uint32_t etpu_b_tcr1_freq;

#ifdef __cplusplus
}
#endif

#endif // __etpu_ops_h
