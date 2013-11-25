/*********************************************************************************

    @file      Optional_Output_Ops.c                                                              
    @date      May 2013
    @brief     Open5xxxECU - this file contains functions for controlling user setup outputs
    @note      www.Open5xxxECU.org
    @version   1.0
    @copyright 2013 - M. Eberhardt

**********************************************************************************/
// Portions Copyright 2011, 2013 M. Eberhardt - BSD 3 clause  License


#include <stdint.h>
#include "mpc563xm.h"
#include "config.h"
#include "variables.h"
#include "typedefs.h"
#include "cocoos.h"
#include "etpu_util.h"
#include "etpu_app_eng_pos.h"
#include "eTPU_OPS.h"
#include "bsp.h"   /**< pickup systime      */
#include "main.h"  /**< pickup Degree_Clock */
#include "Optional_Output_Ops.h"
#include "Table_Lookup.h"

#define Number_Outputs 15


U08 Turn_It_On;
U08 i;
U08 LINK1[16];
U08 LINK2[16];
U08 LINK3[16];
U08 LINK4[16];
float Ref;
float Link_Variables[16];


void Output_Task(void)
{
	
   task_open();                // standard OS entry
   task_wait(1);

       //set all outputs to off
       for (i = 0; i < Number_Outputs; ++i)       
          
          LINK1[i]=0;
          LINK2[i]=0;
          LINK3[i]=0;
          LINK4[i]=0;

       
    for (;;) {
    
       //Update the values of the link option variables
       Link_Variables[1] = RPM;
       Link_Variables[2] = Reference_VE;
       Link_Variables[3] = IAT;
       Link_Variables[4] = CLT;
       Link_Variables[5] = TPS;
       Link_Variables[6] = MAP[0];
       Link_Variables[7] = MAP[1];
       Link_Variables[8] = MAF[0];
       Link_Variables[9] = Lambda[0];
       Link_Variables[10] = V_Batt;
       Link_Variables[11] = V_P[0];
       Link_Variables[12] = V_P[1];
       Link_Variables[13] = V_P[2];
       Link_Variables[14] = V_P[3]; 
       

              
       //evalluate all outputs
       for (i = 0; i < Number_Outputs; ++i){
       




          ///////////////////////////////////////////////////////////////////////////////////////////////////
          //Function CONFIG_SWITCHED_OUTPUTS-switch Case
          // In the First Section below, we establish what each link wants the OUTPUT pin to do.  In the
          // Second Section, we check the logic of each link for each OUTPUT to account for OR or AND
          // selections and set up the OUTPUT pin.
          ////////////////////////////////////////////////////////////////////////////////////////////////
          if (Config_Output_Array[i] == 3){
          

             //Initialize all of the interim variables, LINKx, to zero

            
             
			 //First Section-check each link against the user selected thresholds and save the result as the
			 //interim variable "LINKx" 

			 //Check if the first of four "Link_x" is selected
			 //If the Link is selected (Output_Link_1_Array(i)==1) evaluate the Link.  If not,
			 //proceed to the next Link_x
			 
             if (Output_Link_1_Array[i] > 0) { ////this is probobly not needed since 1 link is always used

                //Get the value against whichthe threshold is evaluated
                Ref = Link_Variables[Output_Link_1_Array[i]];
  
                //Check to see if the Output is ON above or below the selected "on_set" threshold value
                //  Note: LINKx = 1  is ON, =0 is OFF 
                //Output is ON above 'on_set' threshold,
                if (Output_Link_1_on_Array[i] == 1) {

                   if (Ref > Output_Link_1_on_set_Array[i]) {
                      LINK1[i]= 1;

                   //Check if Output (i) is OFF below "off_set" threshold
                   } else if (Ref < Output_Link_1_off_set_Array[i]){
                      LINK1[i]= 0;
                   } //endif

                   // Output is ON  below  "on_set" threshold value,
     			} else {

				   if (Ref < Output_Link_1_on_set_Array[i]) {
                      LINK1[i]= 1;

                   //Check if Output (i) is off below threshold
                   } else if (Ref > Output_Link_1_off_set_Array[i]){
                      LINK1[i]= 0;
                   } //endif

                }  //endif      

             }  //endif
                           
             //Check if the second of four "Link_x" is selected
			 //If the Link is selected (Output_Link_1_Array(i)==1) evaluate the Link.  If not,
			 //proceed to the next Link_x
             if (Output_Link_2_Array[i] > 0) {

                //Get the value against whichthe threshold is evaluated
                Ref = Link_Variables[Output_Link_2_Array[i]];

                //Check to see if the Output is ON above or below the selected "on_set" threshold value
                //  Note: LINKx = 1  is ON, =0 is OFF 
                //Output is ON above 'on_set' threshold,
                if (Output_Link_2_on_Array[i] == 1) {

                   if (Ref > Output_Link_2_on_set_Array[i]) {
                      LINK2[i]= 1;

                   //Check if Output (i) is OFF below "off_set" threshold
                   } else if (Ref< Output_Link_2_off_set_Array[i]){ 
                       LINK2[i]= 0;
                   } //endif

                      // Output is ON  below  "on_set" threshold value,
     			} else {

				   if (Ref < Output_Link_2_on_set_Array[i]) {
                      LINK2[i]= 1;

                   //Check if Output (i) is off below threshold
                   } else if (Ref > Output_Link_2_off_set_Array[i]){
                      LINK2[i]= 0;
                   } //endif

                }  //endif

             }  //endif
                    
             //Check if the 3rd of four "Link_x" is selected
			 //If the Link is selected (Output_Link_1_Array(i)==1) evaluate the Link.  If not,
			 //proceed to the next Link_x
             if (Output_Link_3_Array[i] > 0) {

                //Get the value against whichthe threshold is evaluated
                Ref = Link_Variables[Output_Link_3_Array[i]];

                //Check to see if the Output is ON above or below the selected "on_set" threshold value
                //  Note: LINKx = 1  is ON, =0 is OFF 
                //Output is ON above 'on_set' threshold,
                if (Output_Link_3_on_Array[i] == 1) {

                   if(Ref > Output_Link_3_on_set_Array[i]) {
                      LINK3[i]= 1;

                   //Check if Output (i) is OFF below "off_set" threshold
                   } else if (Ref< Output_Link_3_off_set_Array[i]){
                      LINK3[i]= 0;
                   } //endif

                // Output is ON  below  "on_set" threshold value,
     			} else {

				   if(Ref < Output_Link_3_on_set_Array[i]) {
                      LINK3[i]= 1;

                   //Check if Output (j) is off below threshold
                   } else if (Ref > Output_Link_3_off_set_Array[i]){
                      LINK3[i]= 0;
                   } //endif

                }  //endif

             }  //endif
                    
             //Check if the 4th of four "Link_x" is selected					
             //If the Link is selected (Output_Link_1_Array(i)==1) evaluate the Link.  If not,
			 //proceed to the next Link_x
             if (Output_Link_4_Array[i] > 0) {

                //Get the value against whichthe threshold is evaluated
                Ref = Link_Variables[Output_Link_4_Array[i]];

                //Check to see if the Output is ON above or below the selected "on_set" threshold value
                //  Note: LINKx = 1  is ON, =0 is OFF 
                //Output is ON above 'on_set' threshold,
                if (Output_Link_4_on_Array[i] > 0) {

                   if (Ref > Output_Link_4_on_set_Array[i]) {
                      LINK4[i]= 1;

                   //Check if Output (j) is OFF below "off_set" threshold
                   } else if (Ref< Output_Link_4_off_set_Array[i]){
                      LINK4[i]= 0;
                   } //endif

                // Output is ON  below  "on_set" threshold value,
     			} else {

				   if (Ref < Output_Link_4_on_set_Array[i]) {
                      LINK4[i]= 1;

                   //Check if Output (j) is off below threshold
                   } else if (Ref > Output_Link_4_off_set_Array[i]){
                      LINK4[i]= 0;
                   } //endif

                }  //endif

             }  //endif


              //////////////////////////////////////////////////////////////////////////////////////////////////

              //Second Section-check the logic selected and setup the OUTPUT pin.

              //The OR selections are checked first and the result is returned as variable TURN_IT_ON
              //If any Link_xselected (Output_Link_x_Array(i) == 1) and
              // is OR'd (Output_Link_x_logic_Array (i)==0) and
              //is ON (LINKx=1), turn the selected channel ON
              
              //First default to off
              Turn_It_On = 0;              

              if ((LINK1[i]==1) 
                 || ((Output_Link_2_Array[i] > 0) &&(Output_Link_2_logic_Array[i]==0) && (LINK2[i]==1)) 
                 || ((Output_Link_3_Array[i] > 0) && (Output_Link_3_logic_Array[i]==0) && (LINK3[i]==1)) 
                 || ((Output_Link_4_Array[i] > 0) &&(Output_Link_4_logic_Array[i]==0) && (LINK4[i]==1))) {

                 Turn_It_On = 1;  //OR logic says TURNITON--NOW

              } //endif

              //Now evaluate the AND logic selections
              //Each Link_x is evaluated and andany one of the if AND'd Linksis OFF,
              //then  the OUTPUT should be OFF.  
              //If the Link_x is not selected (Output_Link_x_Array(i)==0) or
              // is a logical OR (Output_Link_x_logic_Array (i)==0), that Link_x cannot effect the outcome
              //of the AND process.

              if ((LINK1[i]==1)
                 && ((LINK2[i]==1) || (Output_Link_2_Array[i]==0) || (Output_Link_2_logic_Array[i]==0))
                 && ((LINK3[i]==1) || (Output_Link_3_Array[i]==0) || (Output_Link_3_logic_Array[i]==0)) 
                 && ((LINK4[i]==1) || (Output_Link_4_Array[i]==0) || (Output_Link_4_logic_Array[i]==0))) {

                 Turn_It_On = 1;  //AND logic says TURN IT ON--NOW

              } //endif

              //Finally, after all of this logic, we can set the OUTUT Pin
              //If both the OR and AND processes has not turned on the output, ensure that it is OFF

              //TURN_IT_ON was set to zero on entering this sub function and before the OR and AND logic.

              if (Turn_It_On ==0) {
                 SIU.GPDO[116 + i].R = 0;
              } else {
                 SIU.GPDO[116 + i].R = 1;
              }  //endif

          }//if (Config_Output_Array == 3)

       } //for (i = 0; i < Number_Outputs; ++i)  
   
             task_wait(97);    
   } // for

   task_close();     

} // Output_Task()
