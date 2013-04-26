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

#include <stdint.h>
#include "config.h"
#include "Enrichment_OPS.h"
#include "variables.h"
#include "Table_Lookup_JZ.h"
#include "main.h"  /**< pickup Degree_Clock */



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
   uint32_t TPS_Last = 0;       // bin 14
   uint32_t TPS_Dot_Decay_Last = (1 << 14);     // bin 14
   uint32_t TPS_Dot_Degree = 0;
   uint32_t TPS_Dot_Decay_Rate = (1 << 14);
   uint32_t Load_Ref_AFR;
   int32_t TPS_Dot_Last = 0;    // bin 14
   int32_t TPS_Dot_Limit;       // bin 14
   int32_t TPS_Dot_Decay;       // bin 14
   int32_t TPS_Dot_Corr;
   int32_t TPS_Dot_Sign = 0;
   int32_t TPS_Dot_Temp;
   uint32_t Degree_Clock_Last = 0;
   
   #define TPS_Dot_Dead 2000

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif


void Get_Accel_Decel_Corr(void)
{


//Accel_Decel_Ops()            
            /**********************************************************************************/
            /*                           accel/decel enrichment                               */
            /* This working by looking at the rate the throttle is moving  and  calculating   */
            /* an enrichment or a derichment to compensate for fuel that  condenses on the    */
            /* manifold wall due to the pressure increase when the throttle opens. This is    */
            /* done by watching the throttle change rate since throttle is the first variable */
            /* to change.                                                                     */
            /*                                                                                */
            /* The accel/decel variables are set to base values above at the start of the     */
            /* fuel routine so they get current sensor values to work with.                   */
            /*                                                                                */
            /* Get the TPS change. This simply compares the throttle position each pass       */
            /* through.  It probbly should be an actual rate by dividing by  the change in    */
            /* crank position but that added too much noise to the  calculation when I tried  */
            /*                                                                                */
            /* The throttle change rate is compared to a dead band.  The deadband helps       */
            /* clean up noise but more importantly no throttle enrichment is required for     */
            /* slow throttle change rates.                                                    */
            /*                                                                                */
            /* When TPS_Dot is above the deadband, the sensativity value is used to calculate */
            /* howw much enrichment is required.  The faster the throttle is moving the more  */
            /* enrifchment should be added.                                                   */
            /*                                                                                */
            /* When the TPS_Dot stops increasing a decay is applied which deceases the        */
            /* enrichment by the specified % each engine cycle.  It's done by cycle because   */
            /* each cylinder has it's own manifold runner and port so each cylinder require   */
            /* the enrichment.                                                                */
            /*                                                                                */
            /* If TPS_Dot goes negative, ie the throttle is closing, accel enrichment ends    */
            /* imediately and a calculation is done to determine if decel derichment is       */
            /* required.  Decel derichment works exactly the same a acel enrichment, only     */
            /* using negative TPS_Dot rates an dpusle width reductions to compensate for fuel */
            /* being remover from the port and manifold walls due to pressure drop            */
            /*                                                                                */
            /**********************************************************************************/

          if (RPM == 0) {	
              // set the accel/deccel variables to current conditions
            TPS_Last = TPS;
            TPS_Dot_Limit = 1 << 14;
            TPS_Dot_Corr = 0;
            TPS_Dot = 0;
            TPS_Dot_Last = 0;
            TPS_Dot_Sign = 0;

            Degree_Clock_Last = Degree_Clock;
		  }//if
            //get a TPS change         
            TPS_Dot_Temp = (TPS_Last - TPS);
            TPS_Last = (3 * TPS_Last + TPS) >> 2;
            //get           
            TPS_Dot = TPS_Dot_Temp << 3;
            TPS_Dot_Degree = (Degree_Clock - Degree_Clock_Last);
            // check if acceleration enrich required
            if (TPS_Dot >= TPS_Dot_Dead && TPS_Dot > TPS_Dot_Last) {
                TPS_Dot_Limit = table_lookup_jz(RPM, 0, Accel_Limit_Table);
                TPS_Dot_Corr = table_lookup_jz(RPM, 0, Accel_Sensativity_Table);
                TPS_Dot_Decay_Rate = table_lookup_jz(RPM, 0, Accel_Decay_Table);
                TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot - TPS_Dot_Dead)) >> 14;

                // update the last clock
                Degree_Clock_Last = Degree_Clock;
                TPS_Dot_Degree = 0;
                TPS_Dot_Decay_Last = 1 << 14;
                TPS_Dot_Sign = 1;
                // decel required 
            } else if (TPS_Dot <= (-TPS_Dot_Dead) && TPS_Dot < TPS_Dot_Last) {
                TPS_Dot_Limit = table_lookup_jz(RPM, 0, Decel_Limit_Table);
                TPS_Dot_Corr = table_lookup_jz(RPM, 0, Decel_Sensativity_Table);
                TPS_Dot_Decay_Rate = table_lookup_jz(RPM, 0, Decel_Decay_Table);
                TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot_Dead - TPS_Dot)) >> 14;
                // update the last clock
                Degree_Clock_Last = Degree_Clock;
                TPS_Dot_Degree = 0;
                TPS_Dot_Decay_Last = 1 << 14;
                TPS_Dot_Sign = -1;
            }
            TPS_Dot_Last = TPS_Dot;
            // calculate the required decay
            if (TPS_Dot_Degree >= 720) {
                Degree_Clock_Last = Degree_Clock_Last + 720;
                TPS_Dot_Decay = (TPS_Dot_Decay_Last * TPS_Dot_Decay_Rate) >> 14;
                TPS_Dot_Decay_Last = TPS_Dot_Decay;
                TPS_Dot_Corr = (TPS_Dot_Corr * TPS_Dot_Decay) >> 14;
            }
            if (TPS_Dot_Corr > TPS_Dot_Limit)
                TPS_Dot_Corr = TPS_Dot_Limit;
            if (TPS_Dot_Sign < 0) 
                TPS_Dot_Corr = TPS_Dot_Corr * (-1);
            

        }

#if __CWCC__
#pragma pop
#endif

