 
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

float RPM; 					//engine speed in revs/min
float Reference_VE;     		//Engine load as a % of a  reference condition, basically 100% cylinder fill under the current conditions
float TPS;					//Throttle postion as a % of full open
float TPS_Dot;				//Throttle position rate of change
float CLT;					//Coolant Temp in C
float IAT;					//Inlet Air Temp in C
float Lambda[2];				//Actual Air/fuel ratio  divided by stoich Air/Fuel ratio
float MAP[2];					//Mafifold Absolute Pressure in KPa  MAp1 is external, MAP 2 isinternal and normally Barometric Pressure
float MAF[2];					//Mass Air Flow in grams/sec
float V_Batt;					//Battery voltage
float Injection_Time;			//Total injector open time in usec
float Spark_Advance;			//Igition angle before TDC in degrees
float Dwell;					//Coil charge time in msec
float Inj_End_Angle;			//Degrees after TDC normal injection pulse should end
float Lambda_Correction;		//Closed loop fuel correction applied as a % change
float Accel_Decel_Corr;		//Acceleration/Deceleration that is applied based on TPS_Dot as a % change
float Prime_Corr;				//Prime of warmup correction in usec that is added to pulse width 
float Fuel_Temp_Corr;			//Correction appied to injector pulse width as a % change based on coolant temp
uint32_t seconds;				//system time in seconds, used by TS for powercycle required warning 
uint32_t Post_Start_Time;		//Time in seconds since crank sync was achieved
uint16_t Post_Start_Cycles;		//engine cycles since crank sync was achieved
uint16_t Post_Start_Cylinders;	//engine cylinder count since crank sync was achieved
uint16_t Sync_Status;           //crank sync status,  see Freescale code for values 
uint16_t Cam_Errors;            // count of errors seen
uint16_t Crank_Errors;          // count of errors seen
uint16_t spare1;				//filler
float Pot_RPM;				//0-5V input used to set test rpm
float V_MAP[2];				//0-5V singal from MAP sensors
float V_CLT;					//0-5V signal from coolant sensor
float V_IAT;					//0-5V signal from the Inlet Air Temp sensor 
float V_TPS;					//0-5V signal from the Throttle Position Sensor
float V_MAF[2];				//0-5V signal from the Mass Air Flow Sensor 
float V_O2[2];				//0-1V  signal from the O2 Sensor or 0-5V a Wide Band O2 control 
float V_P[4];					//Optional 0-5V signal or 0/5V digital
uint32_t Last_Error; 
uint32_t Last_Error_Time;
};

// this must match the offsets in the .ini file AND must be a multiple of 4
#define OUTPUT_CHANNELS_SIZE  168        // don't use sizeof() here


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

#define Pot_RPM Output_Channels.Pot_RPM
#define V_MAP Output_Channels.V_MAP
#define V_CLT Output_Channels.V_CLT 
#define V_IAT Output_Channels.V_IAT 
#define V_TPS  Output_Channels.V_TPS
#define V_MAF Output_Channels.V_MAF 
#define V_O2 Output_Channels.V_O2 
#define V_P Output_Channels.V_P

#define Last_Error Output_Channels.Last_Error 
#define Last_Error_Time Output_Channels.Last_Error_Time


//*******************************************************
//stuff stored in ram for general use but removed from the output block



 

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

#define Config_Ign_Array ((CONST U08 * )(&Page_Ptr[0][224]))

#define Config_Inj_Array ((CONST U08 * )(&Page_Ptr[0][236]))

#define Config_LS_Array ((CONST U08 * )(&Page_Ptr[0][260]))

#define Config_Input_Array ((CONST U08 * )(&Page_Ptr[0][280]))

#define sqrt_Table ((CONST struct table * )(&Page_Ptr[0][308]))

#define Enable_Prime (*(CONST U08 * )(&Page_Ptr[0][696]) & ((2<<0)-1))
#define Prime_Corr_Table ((CONST struct table * )(&Page_Ptr[0][700]))
#define Prime_Decay_Table ((CONST struct table * )(&Page_Ptr[0][1024]))

#define Man_Crank_Corr_Table ((CONST struct table * )(&Page_Ptr[0][1316]))

#define Staged_Inj (*(CONST U08 * )(&Page_Ptr[0][1596]) & ((2<<0)-1))

 
#define Test_Enable (*(CONST U08 * )(&Page_Ptr[0][1616]) & ((2<<0)-1))
#define Test_Value (*(CONST U08 * )(&Page_Ptr[0][1617]) & ((2<<0)-1))
#define Test_RPM (*(CONST F32 * )(&Page_Ptr[0][1620]))
#define Test_TPS (*(CONST F32 * )(&Page_Ptr[0][1624]))

#define Test_Lambda_Array ((CONST F32 * )(&Page_Ptr[0][1628]))

#define Test_MAP_Array ((CONST F32 * )(&Page_Ptr[0][1636]))

#define Test_MAF_Array ((CONST F32 * )(&Page_Ptr[0][1644]))

#define Test_CLT (*(CONST F32 * )(&Page_Ptr[0][1652]))
#define Test_IAT (*(CONST F32 * )(&Page_Ptr[0][1656]))

#define Test_V_MAP_Array ((CONST F32 * )(&Page_Ptr[0][1660]))

#define Test_V_Batt (*(CONST F32 * )(&Page_Ptr[0][1668]))
#define Test_V_CLT (*(CONST F32 * )(&Page_Ptr[0][1672]))
#define Test_V_IAT (*(CONST F32 * )(&Page_Ptr[0][1676]))
#define Test_V_TPS (*(CONST F32 * )(&Page_Ptr[0][1680]))

#define Test_V_MAF_Array ((CONST F32 * )(&Page_Ptr[0][1684]))

#define Test_V_O2_Array ((CONST F32 * )(&Page_Ptr[0][1692]))

#define Jitter (*(CONST S16 * )(&Page_Ptr[0][1700]))
#define RPM_Pot (*(CONST S16 * )(&Page_Ptr[0][1702]))


#define RPM_Change_Rate_Array ((CONST U32 * )(&Page_Ptr[0][1724]))

#define Test_RPM_Array  ((CONST F32 * )(&Page_Ptr[0][1740]))

#define Test_RPM_Dwell_Array  ((CONST S16 * )(&Page_Ptr[0][1756]))

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
#define Coil_Trim_1_Table ((CONST struct table * )(&Page_Ptr[11][0]))
#define Coil_Trim_2_Table ((CONST struct table * )(&Page_Ptr[11][452]))
#define Coil_Trim_3_Table ((CONST struct table * )(&Page_Ptr[11][904]))
#define Coil_Trim_4_Table ((CONST struct table * )(&Page_Ptr[11][1356]))


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

