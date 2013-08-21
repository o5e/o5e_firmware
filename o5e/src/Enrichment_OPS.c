/*********************************************************************************

    @file      Enrichment_OPS.c                                                              
    @date      December, 2011
    @brief     Open5xxxECU - this file contains functions for fuel enrichnemt (or derichment) 
    @note      www.Open5xxxECU.org
    @version   2.1
    @copyright 2011, 2012 - P. Schlein, M. Eberhardt, J. Zeeff

**********************************************************************************/

#include <stdint.h>
#include "config.h"
#include "Enrichment_OPS.h"
#include "Engine_OPS.h"
#include "variables.h"
#include "Table_Lookup.h"
#include "main.h"  /**< pickup Degree_Clock */


   //Accel/decel variables
   uint8_t Accel_Decel_Flag = 0;
   float TPS_Last = 0;
   float TPS_Dot_Decay_Last = 1.0;
   float TPS_Dot_Degree = 0;
   float TPS_Dot_Decay_Rate = 1.0;
   float Load_Ref_AFR;
   float TPS_Dot_Last = 0.0;
   float TPS_Dot_Limit;
   float TPS_Dot_Decay;
   float TPS_Dot_Corr;
   int8_t TPS_Dot_Sign = 0;
   float TPS_Dot_Temp;
   uint32_t Degree_Clock_Last = 0;
   
   
   #define TPS_Dot_Dead 0.01f
   
   //prime variables
   uint32_t Prime_Post_Start_Last = 1;
   float Prime_Decay = 1.0f;
   float Prime_Decay_Last = 1.0f;
   
   
   #define Enrich_Threshold 6000
   #define Prime_Cycles_Threshold 100

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
     //is Accel/decel enabled?  If no the correction is 0
    if (Enable_Accel_Decel == 0)
       Accel_Decel_Corr = 0;
        // set the accel/deccel variables to current conditions
	
    else{
    	

          if ((RPM == 0) ||(Accel_Decel_Flag == 0)) {	
              // set the accel/deccel variables to current conditions
            TPS_Last = TPS;
            TPS_Dot_Limit = 1.0f;
            TPS_Dot_Corr = 0.0f;
            TPS_Dot = 0;
            TPS_Dot_Last = 0.0f;
            TPS_Dot_Sign = 0.0f;
            Accel_Decel_Flag = 1;
            Degree_Clock_Last = Degree_Clock;
		  }//if
            //get a TPS change         
          TPS_Dot_Temp = (TPS_Last - TPS);
          //smooth by 4
          TPS_Last = (3 * TPS_Last + TPS) / 4.0f;
            //get           
          TPS_Dot = TPS_Dot_Temp / 8.0f;
          TPS_Dot_Degree = (Degree_Clock - Degree_Clock_Last);
            // check if acceleration enrich required
          if (TPS_Dot >= TPS_Dot_Dead && TPS_Dot > TPS_Dot_Last) {
              TPS_Dot_Limit = table_lookup(RPM, 1, Accel_Limit_Table);
              TPS_Dot_Corr = table_lookup(RPM, 1, Accel_Sensativity_Table);
              TPS_Dot_Decay_Rate = table_lookup(RPM, 1, Accel_Decay_Table);
              TPS_Dot_Decay_Rate = 1 + (TPS_Dot_Decay_Rate *  Inverse100);
              TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot - TPS_Dot_Dead));

              // update the last clock
              Degree_Clock_Last = Degree_Clock;
              TPS_Dot_Degree = 0;
              TPS_Dot_Decay_Last = 1.0f;
              TPS_Dot_Sign = 1;
                // decel required 
          } else if (TPS_Dot <= (-TPS_Dot_Dead) && TPS_Dot < TPS_Dot_Last) {
              TPS_Dot_Limit = table_lookup(RPM, 1, Decel_Limit_Table);
              TPS_Dot_Corr = table_lookup(RPM, 1, Decel_Sensativity_Table);
              TPS_Dot_Decay_Rate = table_lookup(RPM, 1, Decel_Decay_Table);
              TPS_Dot_Decay_Rate = 1 + (TPS_Dot_Decay_Rate *  Inverse100);
              TPS_Dot_Corr = (TPS_Dot_Corr * (TPS_Dot_Dead - TPS_Dot));
              // update the last clock
              Degree_Clock_Last = Degree_Clock;
              TPS_Dot_Degree = 0;
              TPS_Dot_Decay_Last = 1.0f;
              TPS_Dot_Sign = -1;
          }
          TPS_Dot_Last = TPS_Dot;
          // calculate the required decay
          if (TPS_Dot_Degree >= 720) {
              Degree_Clock_Last = Degree_Clock_Last + 720;
              TPS_Dot_Decay = (TPS_Dot_Decay_Last * TPS_Dot_Decay_Rate);
              TPS_Dot_Decay_Last = TPS_Dot_Decay;
              TPS_Dot_Corr = (TPS_Dot_Corr * TPS_Dot_Decay);
          }
          if (TPS_Dot_Corr > TPS_Dot_Limit)
              TPS_Dot_Corr = TPS_Dot_Limit;
          if (TPS_Dot_Sign < 0) 
              Accel_Decel_Corr = -TPS_Dot_Corr ;
          else
              Accel_Decel_Corr = TPS_Dot_Corr;
          Accel_Decel_Corr = 1.0f + (Accel_Decel_Corr * Inverse100);
    }            

}// Get_Accel_Decel_Corr

static float Get_Prime_Decay() {
	// Update Prime decay each cycle - this is a log decay of the prime pulse
	if (Post_Start_Cycles > Prime_Post_Start_Last) {
		// reset cycle number
		Prime_Post_Start_Last = Post_Start_Cycles;
		// Get the decay rate for current conditions
		Prime_Decay = table_lookup(RPM, 1, Prime_Decay_Table );

		Prime_Decay = 1.0f + (Prime_Decay * Inverse100);
		// decrease decay by the new value
		Prime_Decay = Prime_Decay_Last * Prime_Decay;
		// reset last
		Prime_Decay_Last = Prime_Decay;
	}
	return Prime_Decay;
}

void Get_Prime_Corr(void)    
{
	
                // check if enrichment cals shold be done - this might want to be a % of redline
        // maintain some timers for use by enrichment
        // did we just start?
        if ((RPM < Enrich_Threshold) && (Enable_Prime == 1)) {
            // Prime pulse - extra fuel to wet the manifold on start-up   
            // check if in prime needed conditions   
            if (Post_Start_Cycles < Prime_Cycles_Threshold) {

                Prime_Corr = table_lookup(CLT, 1, Prime_Corr_Table) * Inverse100;

                // apply the decay
                Prime_Corr = 1+ (Prime_Corr * Get_Prime_Decay());
               
                // Reduce the Prime correction by the decay rate and add to pulse_width            
            }
         
        }
        else
           Prime_Corr = 1;
}//Get_Prime_Corr

#if __CWCC__
#pragma pop
#endif

