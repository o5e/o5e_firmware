 
/**************************************************************************************************

        @file           variables.h
        @author         Jon Zeeff
        @date           December, 2011
        @brief          System variables that the tuner will read or write
        @copyright      MIT license
        @warning        needs more testing - send results to jon at zeeff.com
        @version        1.2
        
        @note 		parts of this are created from a TunerStudio .ini file with the program format_ini.c
        
****************************************************************************************************/

/* Copyright (c) 2011 Jon Zeeff 
   Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
   Copyright 2011, 21012, 2013, Mark Eberhardt*/

#ifndef variables_h
#define variables_h

// these are what the tuner uses
#define U08 uint8_t
#define S08 int8_t
#define U16 uint16_t
#define S16 int16_t
#define U32 uint32_t
#define S32 int32_t
#define F32 float

// subroutines related to this file
void init_variables(void);
void Set_Page_Locations(uint8_t block);

//************************************************************************************

// **** below is manually mantained from the .ini file
// !!!! YES, change these things if you change the .ini file !!!!

// Change this every time you modify the .ini
#define VERSION  "O5e1.0.0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
//               "12345678 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9"

#define SIGNATURE "MShift 2.111       "
//                "1234567890123456789"

#define PROTOCOL  "001"
//

#define  NPAGES  	13

extern uint16_t const pageSize[NPAGES];

//*************************************

// All variables that may be sent to the tuner must be defined here and in the tuner.ini file with the exact same layout

// Output_Channels is special in that it is always in a fixed location in ram.  This allows us to define variables differently.
// Note, we trust that the compiler will keep these in order and not do any padding.  
// If not, then we will have to use a block of ram instead of a struct

// This list is maintained *manually* and must match the .ini file exactly - same size, number and order

//[Output_Channels]
struct Outputs {

float RPM;
float Reference_VE;
float TPS;
float TPS_Dot;
float CLT;
float IAT;
float Lambda[2];
float MAP[2];
float MAF[2];
float V_Batt;
float Injection_Time;
float Spark_Advance;
float Dwell;
float Inj_End_Angle;
float Lambda_Correction;
float Accel_Decel_Corr;
float Prime_Corr;
float Fuel_Temp_Corr;
float seconds;
uint32_t Post_Start_Time;
uint16_t Post_Start_Cycles;
uint16_t Post_Start_Cylinders;
uint16_t Sync_Status;            // see Freescale code for values 
uint16_t Cam_Errors;             // count of errors seen
uint16_t Crank_Errors;           // count of errors seen
uint16_t spare1;
uint32_t Last_Error;            // error code for last system error
uint32_t Last_Error_Time;       // time that Last_Error or Crank or Cam error occured
float Pot_RPM;
float V_MAP[2];
float V_CLT; 
float V_IAT; 
float V_TPS;
float V_MAF[2]; 
float V_O2[2]; 
float V_P[4];
};

// this must match the offsets in the .ini file AND must be a multiple of 4
#define OUTPUT_CHANNELS_SIZE  160        // don't use sizeof() here


// these are for convenience and more readable code - must match above
#define RPM Output_Channels.RPM
#define Reference_VE Output_Channels.Reference_VE
#define TPS Output_Channels.TPS
#define TPS_Dot Output_Channels.TPS_Dot
#define CLT Output_Channels.CLT
#define IAT Output_Channels.IAT
#define Lambda Output_Channels.Lambda
#define MAP Output_Channels.MAP
#define MAF Output_Channels.MAF
#define V_Batt Output_Channels.V_Batt
#define Injection_Time Output_Channels.Injection_Time
#define Spark_Advance Output_Channels.Spark_Advance
#define Dwell Output_Channels.Dwell
#define Inj_End_Angle Output_Channels.Inj_End_Angle
#define Lambda_Correction Output_Channels.Lambda_Correction
#define Accel_Decel_Corr Output_Channels.Accel_Decel_Corr
#define Prime_Corr Output_Channels.Prime_Corr
#define Fuel_Temp_Corr Output_Channels.Fuel_Temp_Corr
#define seconds Output_Channels.seconds
#define Post_Start_Time Output_Channels.Post_Start_Time
#define Post_Start_Cycles Output_Channels.Post_Start_Cycles
#define Post_Start_Cylinders Output_Channels.Post_Start_Cylinders
#define Sync_Status Output_Channels.Sync_Status
#define Cam_Errors Output_Channels.Cam_Errors
#define Crank_Errors Output_Channels.Crank_Errors
#define Last_Error Output_Channels.Last_Error
#define Last_Error_Time Output_Channels.Last_Error_Time
#define Pot_RPM Output_Channels.Pot_RPM
#define V_MAP Output_Channels.V_MAP
#define V_CLT Output_Channels.V_CLT 
#define V_IAT Output_Channels.V_IAT 
#define V_TPS  Output_Channels.V_TPS
#define V_MAF Output_Channels.V_MAF 
#define V_O2 Output_Channels.V_O2 
#define V_P Output_Channels.V_P 

//*******************************************************
//stuff stored in ram for general use but removed from the output block

#define Reference_VE_Dot 0.0f 
#define RPM_Dot 0.0f
#define MAP_Dot 0.0f
#define MAF_Dot 0.0f 

//*******************************************************
// initialized to all zeros
extern struct Outputs Output_Channels;

// allow writing to config variables for testing - see variables.c
#define CONST   /**< TODO: rip out FAST! - this needs to be checked though */
//#define CONST const

// Cast the flash memory pages into variable names
// Note: some of these could have ram copies for speed reasons
//--------------------------------------------------------
#define Password (*(CONST U32 * )(&Page_Ptr[0][0]))
#define Version_Array ((CONST U08 * )(&Page_Ptr[0][4]))
#define N_Cyl (*(CONST U08 * )(&Page_Ptr[0][24]) & ((2<<3)-1))
#define N_Teeth (*(CONST U08 * )(&Page_Ptr[0][25]))
#define Missing_Teeth (*(CONST U08 * )(&Page_Ptr[0][26]) & ((2<<2)-1))
#define Load_Sense (*(CONST U08 * )(&Page_Ptr[0][27]) & ((2<<2)-1))
#define Cam_Lobe_Pos (*(CONST U32 * )(&Page_Ptr[0][28]))

#define Cyl_Offset_Array ((CONST U32 * )(&Page_Ptr[0][32]))
#define Cyl_Offset_1 (*(CONST U32 * )(&Page_Ptr[0][32]))
#define Cyl_Offset_2 (*(CONST U32 * )(&Page_Ptr[0][36]))
#define Cyl_Offset_3 (*(CONST U32 * )(&Page_Ptr[0][40]))
#define Cyl_Offset_4 (*(CONST U32 * )(&Page_Ptr[0][44]))
#define Cyl_Offset_5 (*(CONST U32 * )(&Page_Ptr[0][48]))
#define Cyl_Offset_6 (*(CONST U32 * )(&Page_Ptr[0][52]))
#define Cyl_Offset_7 (*(CONST U32 * )(&Page_Ptr[0][56]))
#define Cyl_Offset_8 (*(CONST U32 * )(&Page_Ptr[0][60]))

#define Engine_Position (*(CONST S32 * )(&Page_Ptr[0][80]))
#define Drop_Dead_Angle (*(CONST S32 * )(&Page_Ptr[0][84]))
#define Ignition_Type (*(CONST U08 * )(&Page_Ptr[0][88]) & ((2<<0)-1))

#define Pulses_Per_Rev (*(CONST U08 * )(&Page_Ptr[0][89]) & ((2<<3)-1))
#define Crank_Edge_Select (*(CONST U08 * )(&Page_Ptr[0][90]) & ((2<<0)-1))
#define Cam_Edge_Select (*(CONST U08 * )(&Page_Ptr[0][91]) & ((2<<0)-1))
#define Sync_Mode_Select (*(CONST U08 * )(&Page_Ptr[0][92]) & ((2<<0)-1))
#define Engine_Type_Select (*(CONST U08 * )(&Page_Ptr[0][93]) & ((2<<1)-1))
#define Ignition_Invert (*(CONST U08 * )(&Page_Ptr[0][94]) & ((2<<0)-1))
#define N_Coils_Per_Cylinder (*(CONST U08 * )(&Page_Ptr[0][95]) & ((2<<0)-1))
#define Cam_Window_Open_Set (*(CONST S32 * )(&Page_Ptr[0][96]))
#define Cam_Window_Width_Set (*(CONST S32 * )(&Page_Ptr[0][100]))
#define Odd_Fire_Sync_Threshold (*(CONST F32 * )(&Page_Ptr[0][104]))
#define Odd_Fire_Sync_Angle (*(CONST S32 * )(&Page_Ptr[0][108]))



#define Displacement  (*(CONST F32 * )(&Page_Ptr[0][112])) 
#define Injector_Size (*(CONST F32 * )(&Page_Ptr[0][116]))
#define Rating_Fuel_Presure  (*(CONST F32 * )(&Page_Ptr[0][120]))
#define Fuel_Presure   (*(CONST F32 * )(&Page_Ptr[0][124]))

#define Inj_Dead_Array ((CONST F32 * )(&Page_Ptr[0][128]))
#define Inj_Dead_1 (*(CONST F32 * )(&Page_Ptr[0][128]))
#define Inj_Dead_2 (*(CONST F32 * )(&Page_Ptr[0][132]))
#define Inj_Dead_3 (*(CONST F32 * )(&Page_Ptr[0][136]))
#define Inj_Dead_4 (*(CONST F32 * )(&Page_Ptr[0][140]))
#define Inj_Dead_5 (*(CONST F32 * )(&Page_Ptr[0][144]))
#define Inj_Dead_6 (*(CONST F32 * )(&Page_Ptr[0][148]))
#define Inj_Dead_7 (*(CONST F32 * )(&Page_Ptr[0][152]))
#define Inj_Dead_8 (*(CONST F32 * )(&Page_Ptr[0][156]))
#define Inj_Dead_9 (*(CONST F32 * )(&Page_Ptr[0][160]))
#define Inj_Dead_10 (*(CONST F32 * )(&Page_Ptr[0][164]))
#define Inj_Dead_11 (*(CONST F32 * )(&Page_Ptr[0][168]))
#define Inj_Dead_12 (*(CONST F32 * )(&Page_Ptr[0][172]))

#define Config_Ign_Array ((CONST U08 * )(&Page_Ptr[0][224]))
#define Config_Ign_1 (*(CONST U08 * )(&Page_Ptr[0][224]) & ((2<<3)-1))
#define Config_Ign_2 (*(CONST U08 * )(&Page_Ptr[0][225]) & ((2<<3)-1))
#define Config_Ign_3 (*(CONST U08 * )(&Page_Ptr[0][226]) & ((2<<3)-1))
#define Config_Ign_4 (*(CONST U08 * )(&Page_Ptr[0][227]) & ((2<<3)-1))

#define Config_Inj_Array ((CONST U08 * )(&Page_Ptr[0][236]))
#define Config_Inj_1 (*(CONST U08 * )(&Page_Ptr[0][236]) & ((2<<3)-1))
#define Config_Inj_2 (*(CONST U08 * )(&Page_Ptr[0][237]) & ((2<<3)-1))
#define Config_Inj_3 (*(CONST U08 * )(&Page_Ptr[0][238]) & ((2<<3)-1))
#define Config_Inj_4 (*(CONST U08 * )(&Page_Ptr[0][239]) & ((2<<3)-1))
#define Config_Inj_5 (*(CONST U08 * )(&Page_Ptr[0][240]) & ((2<<3)-1))
#define Config_Inj_6 (*(CONST U08 * )(&Page_Ptr[0][241]) & ((2<<3)-1))
#define Config_Inj_7 (*(CONST U08 * )(&Page_Ptr[0][242]) & ((2<<3)-1))
#define Config_Inj_8 (*(CONST U08 * )(&Page_Ptr[0][243]) & ((2<<3)-1))

#define Config_LS_Array ((CONST U08 * )(&Page_Ptr[0][260]))
#define Config_LS_1 (*(CONST U08 * )(&Page_Ptr[0][260]) & ((2<<0)-1))
#define Config_LS_2 (*(CONST U08 * )(&Page_Ptr[0][261]) & ((2<<0)-1))
#define Config_LS_3 (*(CONST U08 * )(&Page_Ptr[0][262]) & ((2<<1)-1))
#define Config_LS_4 (*(CONST U08 * )(&Page_Ptr[0][263]) & ((2<<1)-1))
#define Config_LS_5 (*(CONST U08 * )(&Page_Ptr[0][264]) & ((2<<1)-1))
#define Config_LS_6 (*(CONST U08 * )(&Page_Ptr[0][265]) & ((2<<1)-1))
#define Config_LS_7 (*(CONST U08 * )(&Page_Ptr[0][266]) & ((2<<0)-1))
#define Config_LS_8 (*(CONST U08 * )(&Page_Ptr[0][267]) & ((2<<0)-1))

#define Config_Input_Array ((CONST U08 * )(&Page_Ptr[0][280]))
#define Config_Input_1 (*(CONST U08 * )(&Page_Ptr[0][280]) & ((2<<0)-1))
#define Config_Input_2 (*(CONST U08 * )(&Page_Ptr[0][281]) & ((2<<0)-1))
#define Config_Input_3 (*(CONST U08 * )(&Page_Ptr[0][282]) & ((2<<1)-1))
#define Config_Input_4 (*(CONST U08 * )(&Page_Ptr[0][283]) & ((2<<1)-1))

#define sqrt_Table ((CONST struct table * )(&Page_Ptr[0][308]))

#define Enable_Prime (*(CONST U08 * )(&Page_Ptr[0][696]) & ((2<<0)-1))
#define Prime_Corr_Table ((CONST struct table * )(&Page_Ptr[0][700]))
#define Prime_Decay_Table ((CONST struct table * )(&Page_Ptr[0][1024]))

#define Man_Crank_Corr_Table ((CONST struct table * )(&Page_Ptr[0][1316]))

 
#define Test_Enable (*(CONST U08 * )(&Page_Ptr[0][1616]) & ((2<<0)-1))
#define Test_Value (*(CONST U08 * )(&Page_Ptr[0][1617]) & ((2<<0)-1))
#define Test_RPM (*(CONST F32 * )(&Page_Ptr[0][1620]))
#define Test_TPS (*(CONST F32 * )(&Page_Ptr[0][1624]))

#define Test_Lambda_Array ((CONST F32 * )(&Page_Ptr[0][1628]))
#define Test_Lambda_1 (*(CONST F32 * )(&Page_Ptr[0][1628]))
#define Test_Lambda_2 (*(CONST F32 * )(&Page_Ptr[0][1632]))

#define Test_MAP_Array ((CONST F32 * )(&Page_Ptr[0][1636]))
#define Test_MAP_1 (*(CONST F32 * )(&Page_Ptr[0][1636]))
#define Test_MAP_2 (*(CONST F32 * )(&Page_Ptr[0][1640]))

#define Test_MAF_Array ((CONST F32 * )(&Page_Ptr[0][1644]))
#define Test_MAF_1 (*(CONST F32 * )(&Page_Ptr[0][1644]))
#define Test_MAF_2 (*(CONST F32 * )(&Page_Ptr[0][1648]))

#define Test_CLT (*(CONST F32 * )(&Page_Ptr[0][1652]))
#define Test_IAT (*(CONST F32 * )(&Page_Ptr[0][1656]))

#define Test_V_MAP_Array ((CONST F32 * )(&Page_Ptr[0][1660]))
#define Test_V_MAP_1 (*(CONST F32 * )(&Page_Ptr[0][1660]))
#define Test_V_MAP_2 (*(CONST F32 * )(&Page_Ptr[0][1664]))

#define Test_V_Batt (*(CONST F32 * )(&Page_Ptr[0][1668]))
#define Test_V_CLT (*(CONST F32 * )(&Page_Ptr[0][1672]))
#define Test_V_IAT (*(CONST F32 * )(&Page_Ptr[0][1676]))
#define Test_V_TPS (*(CONST F32 * )(&Page_Ptr[0][1680]))

#define Test_V_MAF_Array ((CONST F32 * )(&Page_Ptr[0][1684]))
#define Test_V_MAF_1 (*(CONST F32 * )(&Page_Ptr[0][1684]))
#define Test_V_MAF_2 (*(CONST F32 * )(&Page_Ptr[0][1688]))

#define Test_V_O2_Array ((CONST F32 * )(&Page_Ptr[0][1692]))
#define Test_V_O2_1 (*(CONST F32 * )(&Page_Ptr[0][1692]))
#define Test_V_O2_2 (*(CONST F32 * )(&Page_Ptr[0][1696]))

#define Jitter (*(CONST S16 * )(&Page_Ptr[0][1700]))
#define RPM_Pot (*(CONST S16 * )(&Page_Ptr[0][1702]))
#define crank_windowing_ratio_normal_set (*(CONST U32 * )(&Page_Ptr[0][1704]))
#define crank_windowing_ratio_after_gap_set (*(CONST U32 * )(&Page_Ptr[0][1708])) 
#define crank_windowing_ratio_across_gap_set (*(CONST U32 * )(&Page_Ptr[0][1712]))
#define crank_windowing_ratio_timeout_set (*(CONST U32 * )(&Page_Ptr[0][1716]))
#define crank_gap_ratio_set  (*(CONST U32 * )(&Page_Ptr[0][1720]))

#define RPM_Change_Rate_Array ((CONST U32 * )(&Page_Ptr[0][1724]))
#define RPM_Change_Rate_1 (*(CONST U32 * )(&Page_Ptr[0][1724]))
#define RPM_Change_Rate_2 (*(CONST U32 * )(&Page_Ptr[0][1728]))
#define RPM_Change_Rate_3 (*(CONST U32 * )(&Page_Ptr[0][1732]))
#define RPM_Change_Rate_4 (*(CONST U32 * )(&Page_Ptr[0][1736]))

#define Test_RPM_Array  ((CONST F32 * )(&Page_Ptr[0][1740]))
#define Test_RPM_1  (*(CONST F32 * )(&Page_Ptr[0][1740]))
#define Test_RPM_2  (*(CONST F32 * )(&Page_Ptr[0][1744]))
#define Test_RPM_3  (*(CONST F32 * )(&Page_Ptr[0][1748]))
#define Test_RPM_4  (*(CONST F32 * )(&Page_Ptr[0][1752]))

#define Test_RPM_Dwell_Array  ((CONST S16 * )(&Page_Ptr[0][1756]))
#define Test_RPM_Dwell_1  (*(CONST S16 * )(&Page_Ptr[0][1756]))
#define Test_RPM_Dwell_2  (*(CONST S16 * )(&Page_Ptr[0][1758]))
#define Test_RPM_Dwell_3  (*(CONST S16 * )(&Page_Ptr[0][1760]))
#define Test_RPM_Dwell_4  (*(CONST S16 * )(&Page_Ptr[0][1762]))

#define Test_RPM_Type (*(CONST U08 * )(&Page_Ptr[0][1764]) & ((2<<1)-1))
 


// Page 2
#define Enable_Coolant_Temp_Corr (*(CONST U08 * )(&Page_Ptr[1][0]) & ((2<<0)-1))
#define Fuel_Temp_Corr_Table ((CONST struct table * )(&Page_Ptr[1][4]))

#define Enable_Air_Temp_Corr (*(CONST U08 * )(&Page_Ptr[1][328]) & ((2<<0)-1))
#define IAT_Fuel_Corr_Table ((CONST struct table * )(&Page_Ptr[1][332]))

#define Enable_Ignition (*(CONST U08 * )(&Page_Ptr[1][656]) & ((2<<0)-1))
#define Enable_Map_Control (*(CONST U08 * )(&Page_Ptr[1][657]) & ((2<<0)-1))
#define Enable_Inj (*(CONST U08 * )(&Page_Ptr[1][658]) & ((2<<0)-1))

#define Dwell_Table ((CONST struct table * )(&Page_Ptr[1][660]))
#define Dwell_Set (*(CONST F32 * )(&Page_Ptr[1][952]))
#define Dwell_Min (*(CONST U16 * )(&Page_Ptr[1][956]))
#define Dwell_Max (*(CONST U16 * )(&Page_Ptr[1][958]))

#define Dead_Time_Set (*(CONST F32 * )(&Page_Ptr[1][960]))

#define Rev_Limit (*(CONST F32 * )(&Page_Ptr[1][964]))
#define Soft_Rev_Limit (*(CONST F32 * )(&Page_Ptr[1][968]))
#define Rev_Limit_Type (*(CONST U08 * )(&Page_Ptr[1][972]) & ((2<<2)-1))

#define Inj_Dead_Time_Table ((CONST struct table * )(&Page_Ptr[1][976]))


#define IAC_Enable (*(CONST U08 * )(&Page_Ptr[1][1268]) & ((2<<1)-1))
#define Warm_Idle_RPM (*(CONST F32 * )(&Page_Ptr[1][1272]))
#define Cold_Idle_RPM (*(CONST F32 * )(&Page_Ptr[1][1276]))
#define Warm_Start_IAC (*(CONST F32 * )(&Page_Ptr[1][1280]))
#define Cold_start_IAC (*(CONST F32 * )(&Page_Ptr[1][1284]))
#define Warm_IAC (*(CONST F32 * )(&Page_Ptr[1][1288]))
#define Cold_IAC (*(CONST F32 * )(&Page_Ptr[1][1292]))
#define Warm_Idle_Temp (*(CONST F32 * )(&Page_Ptr[1][1296]))
#define Warm_Start_Temp (*(CONST F32 * )(&Page_Ptr[1][1300]))

#define MAP_Angle_Table ((CONST struct table * )(&Page_Ptr[1][1304]))

#define Fuel_Pump_Prime_Time (*(CONST U32 * )(&Page_Ptr[1][1692])) 


// Page 3
#define Enable_Accel_Decel (*(CONST U08 * )(&Page_Ptr[2][0]) & ((2<<0)-1))
#define Accel_Limit_Table ((CONST struct table * )(&Page_Ptr[2][4]))
#define Accel_Decay_Table ((CONST struct table * )(&Page_Ptr[2][296]))
#define Accel_Sensativity_Table ((CONST struct table * )(&Page_Ptr[2][588]))
#define Decel_Limit_Table ((CONST struct table * )(&Page_Ptr[2][880]))
#define Decel_Decay_Table ((CONST struct table * )(&Page_Ptr[2][1172]))
#define Decel_Sensativity_Table ((CONST struct table * )(&Page_Ptr[2][1464]))

// Page 4
#define TPS_Flow_Cal_On (*(CONST U08 * )(&Page_Ptr[3][0]) & ((2<<0)-1))
#define TPS_Flow_Table ((CONST struct table * )(&Page_Ptr[3][4]))

#define CLT_Table ((CONST struct table * )(&Page_Ptr[3][520]))

#define IAT_Table ((CONST struct table * )(&Page_Ptr[3][844]))

#define TPS_Table ((CONST struct table * )(&Page_Ptr[3][1168]))

#define Lambda_1_Table ((CONST struct table * )(&Page_Ptr[3][1436]))

// Page 5
#define Lambda_2_Table ((CONST struct table * )(&Page_Ptr[4][0]))

#define MAF_1_Table ((CONST struct table * )(&Page_Ptr[4][324]))

#define MAF_2_Table ((CONST struct table * )(&Page_Ptr[4][648]))

#define MAP_1_Table ((CONST struct table * )(&Page_Ptr[4][972]))

#define MAP_2_Table ((CONST struct table * )(&Page_Ptr[4][1296]))


// Page 6
#define Inj_Time_Corr_Table ((CONST struct table * )(&Page_Ptr[5][0]))
// Page 7
#define Inj_End_Angle_Table ((CONST struct table * )(&Page_Ptr[6][0]))
// Page 8
#define Spark_Advance_Table ((CONST struct table * )(&Page_Ptr[7][0]))
// Page 9
#define Lambda_Set_Point_Table ((CONST struct table * )(&Page_Ptr[8][0]))
// Page 10
#define Cyl_Trim_1_Table ((CONST struct table * )(&Page_Ptr[9][0]))
#define Cyl_Trim_2_Table ((CONST struct table * )(&Page_Ptr[9][452]))
#define Cyl_Trim_3_Table ((CONST struct table * )(&Page_Ptr[9][904]))
#define Cyl_Trim_4_Table ((CONST struct table * )(&Page_Ptr[9][1356]))
// Page 11
#define Cyl_Trim_5_Table ((CONST struct table * )(&Page_Ptr[10][0]))
#define Cyl_Trim_6_Table ((CONST struct table * )(&Page_Ptr[10][452]))
#define Cyl_Trim_7_Table ((CONST struct table * )(&Page_Ptr[10][904]))
#define Cyl_Trim_8_Table ((CONST struct table * )(&Page_Ptr[10][1356]))
// Page 12
#define Cyl_Trim_9_Table ((CONST struct table * )(&Page_Ptr[11][0]))
#define Cyl_Trim_10_Table ((CONST struct table * )(&Page_Ptr[11][452]))
#define Cyl_Trim_11_Table ((CONST struct table * )(&Page_Ptr[11][904]))
#define Cyl_Trim_12_Table ((CONST struct table * )(&Page_Ptr[11][1356]))
// Page 13
#define Coil_Trim_1_Table ((CONST struct table * )(&Page_Ptr[12][0]))
#define Coil_Trim_2_Table ((CONST struct table * )(&Page_Ptr[12][452]))
#define Coil_Trim_3_Table ((CONST struct table * )(&Page_Ptr[12][904]))
#define Coil_Trim_4_Table ((CONST struct table * )(&Page_Ptr[12][1356]))


// ---------------------------------------------------
// Below here does not come from the .ini file

// A page is 2048 bytes, a block is 128K.  
// Only one block is in use at a time. This allows a flip/flop use/erase & burn strategy

#define MAX_PAGE_SIZE 2048    /**< self imposed. tuner has no say in this */

// Current flash or ram location of each page
extern volatile uint8_t *Page_Ptr[NPAGES];
extern int8_t Ram_Page_Buffer_Page;	// which page # is in the buffer (-1 means none)
extern int Flash_OK;		// is flash empty or has values
extern uint8_t Burn_Count;		// how many flash burns 

// This 8 byte (not counting directory) structure is written as a header to the beginning of a used flash block
struct Flash_Header {
    uint8_t Cookie[4];          // set to ABCD to indicate block is not blank
    uint8_t Burn_Count;        	// how many flash burns, used to pick 'newest' on powerup
} ;

#define BLOCK_HEADER_SIZE       1024U    // bigger than needed

extern uint8_t Flash_Block;		// flash block currently being used
extern uint8_t *Flash_Addr[2];

// ------------------

#endif

