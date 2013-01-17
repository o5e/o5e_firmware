/**********************************************************************************/
/* FILE NAME: Enrichment_OPS.c                                                    */
/*                                                                                */
/* DESCRIPTION:                                                                   */
/* This file contains functions for "Enriching" or "Deriching the base            */
/* Pulse_Width during acceleration or deceleration                                */
/*================================================================================*/
/* ORIGINAL AUTHOR:  Paul Schlein                                                 */
/* REV      AUTHOR          DATE          DESCRIPTION OF CHANGE                   */
/* ---     -----------     ----------    ---------------------                    */
/* 2.0     M. Eberhardt    23/Dec/11     Rewrote to remove placeholders           */
/* 1.0     P. Schlein      12/Sep/11     Initial version with Placeholders        */
/*================================================================================*/

//#include <stdint.h>
//#include "config.h"
//#include "Engine_OPS.h"
#include "Enrichment_OPS.h"
//#include "variables.h"
//#include "Table_Lookup_JZ.h"
//#include "etpu_util.h"
//#include "cocoos.h"

//static int16_t Accel_Corr;
//static uint8_t CLT_Corr;
//static uint8_t Post_Start_Corr;

/**********************************************************************************/
/* FUNCTION     : Acceleration/Deceleration Correction                            */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : None                                                            */
/*                                                                                */
/*                    Descriprtion                                                */
/* This funciton works on the theroy that enrichment or derichment during         */
/* acceleration and deceleration respectively is require because fuel is being    */
/* "stored" in the intake track.  This routine tracks the amount of fuel that is  */
/* stored at any time and provides an appropiate correction for the injection     */
/* pulse time to maintian a constant air/fuel ratio.  To allow this to work the   */
/* the engine requirements must be mapped and the infomation stored in lookup     */
/* tables.                                                                        */
/**********************************************************************************/

void Enrichment_Task()
{
#if 0
    //int16_t Max;                // bin 7
    //int16_t Step;               // bin 7
    //int16_t Accel_Temp;         // bin 12
    //static int16_t Current_Wetting = 0; // bin 12, set to zero on start-up
    //int16_t Wetting_Temp_Corr;
    //int16_t Manifold_Wetting;
    //static int16_t Current_Wetting = 0; // bin 12, set to zero on start-up
    int16_t CLT_Fuel_Corr; //bin 12
    int16_t Prime_Corr; //bin 12
    

    task_open();
    task_wait(1);

    for (;;) {
       //Coolant temp correction
//        CLT_Fuel_Corr = (int16_t) table_lookup_jz(CLT, 0, Fuel_Temp_Corr_Table);

       //Prime pulse require
//       Prime_Corr = (int16_t) table_lookup_jz(CLT, 0, Wetting_Temp_Corr_Table);
       




        Wetting_Temp_Corr = (int16_t) table_lookup_jz(CLT, Post_Start_Time, Wetting_Temp_Corr_Table);
        Manifold_Wetting = (int16_t) table_lookup_jz(RPM, Load, Manifold_Wetting_Table); 

        // Determine is enrichment or derichment is required. The assumption is that 1 or the other can always 
        // be applied and that the "Current_Wetting" tracking will reduce to the correction to effectively    
        // zero when no correction is actually required                                                       
        //                                                                                                    
        // Test - The first thing to happen is the required Manifold_Wetting for the current load and rpm     
        // is multiplied be Wetting_Temp_Corr to corrected for temperature and Post_Stat_Time wich accouts    
        // for changes in required wetting due to air velocity, air pressure, air temp, manifold surface temp 
        // (which is assumed follow but lag coolant temp). The corrected wetting value is compared to         
        // Cureent_Wetting (which is a running total of the stored fuel) to determing if fuel must be added   
        // or subtracted to account for what is going into or coming of of storage in the manifold.           
        //                                                                                                    
        // Note - On system start-up Current_Wetting is set to zero.  This is done so the ECU will know the    
        // system is dry and the entire required wetting must be added.  This is effectively a system "Prime" 
        // but is handled without any additonal code being require.                                           

        if (((Manifold_Wetting * Wetting_Temp_Corr) >> 12) - Current_Wetting <= 0) {    // Is Accel correction required?
            Max = (int16_t) table_lookup_jz(RPM, Load, Max_Enrich_Table);
            Step = (int16_t) table_lookup_jz(RPM, Load, Accel_Step_Table);
        } else {                // Decel correction is required
            Max = (int16_t) table_lookup_jz(RPM, Load, Max_Derich_Table);
            Step = (int16_t) table_lookup_jz(RPM, Load, Decel_Step_Table);
        }                       // if

        // Once the correct values have been looked up the correction is calculated.                           
        // Step = the distance to go from current wetting to the required wetting in 1 cylce.  The entire     
        // amount required can not be added/subtracted in 1 shot because most of the fuel spays into   
        // the engine like it's supposed and over-doing the speed on the storage terms will bugger the 
        // the mixure you're trying to stabalize.  The closer you are to the wetting target the        
        // smaller the correction that can be made and this is accounted for automatically because     
        // "Step" is the % to move from where you are to where to are going  so when you are close you 
        // get a small correction and when you are far you get a big correction.                       
        //                                                                                                    
        // Step is a "per cycle" value but to apply the enrichment/derichment factor in a timely       
        // manner the enrichment routine may be called multiple times per cylce.  To account for this  
        // Step is multiplied by "Inv_Update_Count" which is 1/(the number of updates per cycle).      

        // Step = (Step * Inv_Update_Count) >> 12;
        Accel_Corr = ((((Manifold_Wetting * Wetting_Temp_Corr) >> 12) - Current_Wetting) * Step) >> 7;
        Accel_Temp = (Pulse_Width * Max) >> 7;

        // Test if the calculation correction is larger than the max permitted correciton and if so clamp the  
        // clamp the correction to the max permited value.                                                    

        if (Accel_Corr > Accel_Temp)
            Accel_Corr = Accel_Temp;

        // Accel_Corr is global and used by the fuel calculations

        Current_Wetting += Accel_Corr;  // 

        task_wait(2);  // TODO
    }                           // for

    task_close();
#endif

}                               // Enrichment_Task() 


