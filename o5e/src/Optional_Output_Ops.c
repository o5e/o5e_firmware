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
#include "Table_Lookup_JZ.h"


int16_t Generic_Output_Scale;
int16_t Generic_Output_Translate;
uint8_t Generic_Output_1_Condition_1 = 0;
uint8_t Generic_Output_1_Condition_2 = 0;
uint8_t Generic_Output_1_Condition_3 = 0;
uint8_t Generic_Output_1_Condition_4 = 0;
uint8_t Generic_Output_1;
int16_t Generic_Output_1_Link_1_on_ecu;
int16_t Generic_Output_1_Link_1_off_ecu;
int16_t Generic_Output_1_Link_2_on_ecu;
int16_t Generic_Output_1_Link_2_off_ecu;
int16_t Generic_Output_1_Link_3_on_ecu;
int16_t Generic_Output_1_Link_3_off_ecu;
int16_t Generic_Output_1_Link_4_on_ecu;
int16_t Generic_Output_1_Link_4_off_ecu;

int16_t Generic_Output_1_Link_1_value;
int16_t Generic_Output_1_Link_2_value;
int16_t Generic_Output_1_Link_3_value;
int16_t Generic_Output_1_Link_4_value;



void Generic_Output_1_Task(void)
    {
        task_open();                // standard OS entry
        task_wait(1);


//convert setpoints to correct bin point/translate point
/*Generic_Output_Scale =  (int16_t) table_lookup_jz(Generic_Output_1_Link_1, 0, Generic_Output_Link_Shift_Table);
Generic_Output_Translate = (int16_t) table_lookup_jz(Generic_Output_1_Link_1, 1, Generic_Output_Link_Shift_Table);      
Generic_Output_1_Link_1_on_ecu = (Generic_Output_1_Link_1_on_set *Generic_Output_Scale) - Generic_Output_Translate;
Generic_Output_1_Link_1_off_ecu=  (Generic_Output_1_Link_1_off_set *Generic_Output_Scale) - Generic_Output_Translate; 

Generic_Output_Scale =  (int16_t) table_lookup_jz(Generic_Output_1_Link_2, 0, Generic_Output_Link_Shift_Table);
Generic_Output_Translate = (int16_t) table_lookup_jz(Generic_Output_1_Link_2, 1, Generic_Output_Link_Shift_Table);      
Generic_Output_1_Link_2_on_ecu = (Generic_Output_1_Link_1_on_set *Generic_Output_Scale) - Generic_Output_Translate;
Generic_Output_1_Link_2_off_ecu=  (Generic_Output_1_Link_1_off_set *Generic_Output_Scale) - Generic_Output_Translate; 

Generic_Output_Scale =  (int16_t) table_lookup_jz(Generic_Output_1_Link_3, 0, Generic_Output_Link_Shift_Table);
Generic_Output_Translate = (int16_t) table_lookup_jz(Generic_Output_1_Link_3, 1, Generic_Output_Link_Shift_Table);      
Generic_Output_1_Link_3_on_ecu = (Generic_Output_1_Link_1_on_set *Generic_Output_Scale) - Generic_Output_Translate;
Generic_Output_1_Link_3_off_ecu=  (Generic_Output_1_Link_1_off_set *Generic_Output_Scale) - Generic_Output_Translate; 

Generic_Output_Scale =  (int16_t) table_lookup_jz(Generic_Output_1_Link_4, 0, Generic_Output_Link_Shift_Table);
Generic_Output_Translate = (int16_t) table_lookup_jz(Generic_Output_1_Link_4, 1, Generic_Output_Link_Shift_Table);      
Generic_Output_1_Link_4_on_ecu = (Generic_Output_1_Link_4_on_set *Generic_Output_Scale) - Generic_Output_Translate;
Generic_Output_1_Link_4_off_ecu=  (Generic_Output_1_Link_4_off_set *Generic_Output_Scale) - Generic_Output_Translate; 


//set link 1
if (Generic_Output_1_Link_1 == 0)
    Generic_Output_1_Link_1_value = RPM;
else if(Generic_Output_1_Link_1 == 1)
    Generic_Output_1_Link_1_value = Reference_VE;
else if (Generic_Output_1_Link_1 == 2)
    Generic_Output_1_Link_1_value = IAT;
else if (Generic_Output_1_Link_1 == 3)
    Generic_Output_1_Link_1_value = CLT;
else if (Generic_Output_1_Link_1 == 4)
    Generic_Output_1_Link_1_value = TPS;
else if (Generic_Output_1_Link_1 == 6)
    Generic_Output_1_Link_1_value = MAP[0];
else if (Generic_Output_1_Link_1 == 6)
    Generic_Output_1_Link_1_value = MAP[1];
else
    Generic_Output_1_Link_1_value = 0;
//set link 2
if (Generic_Output_1_Link_2 == 0)
    Generic_Output_1_Link_2_value = RPM;
else if(Generic_Output_1_Link_1 == 1)
    Generic_Output_1_Link_2_value = Reference_VE;
else if (Generic_Output_1_Link_2 == 2)
    Generic_Output_1_Link_2_value = IAT;
else if (Generic_Output_1_Link_2 == 3)
    Generic_Output_1_Link_2_value = CLT;
else if (Generic_Output_1_Link_2 == 4)
    Generic_Output_1_Link_2_value = TPS;
else if (Generic_Output_1_Link_2 == 6)
    Generic_Output_1_Link_2_value = MAP[0];
else if (Generic_Output_1_Link_2 == 6)
    Generic_Output_1_Link_2_value = MAP[1];
else
    Generic_Output_1_Link_2_value = 0;
//set link 3
if (Generic_Output_1_Link_3 == 0)
    Generic_Output_1_Link_3_value = RPM;
else if(Generic_Output_1_Link_3 == 1)
    Generic_Output_1_Link_3_value = Reference_VE;
else if (Generic_Output_1_Link_3 == 2)
    Generic_Output_1_Link_3_value = IAT;
else if (Generic_Output_1_Link_3 == 3)
    Generic_Output_1_Link_3_value = CLT;
else if (Generic_Output_1_Link_3 == 4)
    Generic_Output_1_Link_3_value = TPS;
else if (Generic_Output_1_Link_3 == 6)
    Generic_Output_1_Link_3_value = MAP[0];
else if (Generic_Output_1_Link_3 == 6)
    Generic_Output_1_Link_3_value = MAP[1];
else
    Generic_Output_1_Link_3_value = 0;
//set link 4
if (Generic_Output_1_Link_4 == 0)
    Generic_Output_1_Link_4_value = RPM;
else if(Generic_Output_1_Link_4 == 1)
    Generic_Output_1_Link_4_value = Reference_VE;
else if (Generic_Output_1_Link_4 == 2)
    Generic_Output_1_Link_4_value = IAT;
else if (Generic_Output_1_Link_4 == 3)
    Generic_Output_1_Link_4_value = CLT;
else if (Generic_Output_1_Link_4 == 4)
    Generic_Output_1_Link_4_value = TPS;
else if (Generic_Output_1_Link_4 == 6)
    Generic_Output_1_Link_4_value = MAP[0];
else if (Generic_Output_1_Link_4 == 6)
    Generic_Output_1_Link_4_value = MAP[1];
else
    Generic_Output_1_Link_4_value = 0;

*/

       
        for (;;) {
        /*
          if(Generic_Output_1_type > 0){ //if output enablebed

	            //link 1 switch condition
			if (Generic_Output_1_type <= 4){ //Switch with 1 link
			   //link 1
			   //on condition
			   if (Generic_Output_1_Link_1_on == 0){
			       // on below
			       if (Generic_Output_1_Link_1_value <= Generic_Output_1_Link_1_on_set ) 
			           Generic_Output_1_Condition_1 = 1;
			       //off above
			       else if(Generic_Output_1_Link_1_value >= Generic_Output_1_Link_1_on_set ) 
			           Generic_Output_1_Condition_1 = 0; 
			       
			   }
			            //todo - the bin points need to be sorted here!!!
				//on above
			   else { 
			      if (Generic_Output_1_Link_1_value <= Generic_Output_1_Link_1_on_set )
			          Generic_Output_1_Condition_1 = 1;
			      // off below
			      else if (Generic_Output_1_Link_1_value <= Generic_Output_1_Link_1_on_set ) 
			           Generic_Output_1_Condition_1 = 0;			   	
			   }
			          //todo - the bin points need to be sorted here!!! 
               
               Generic_Output_1 = Generic_Output_1_Condition_1;

	            //link 2 switch condition
			   if (Generic_Output_1_type >= 2){ //Switch with 2 links
			      //link 1
			      //on condition
			      if (Generic_Output_1_Link_2_on == 0){
			          // on below
			          if (Generic_Output_1_Link_2_value <= Generic_Output_1_Link_2_on_set ) 
			              Generic_Output_1_Condition_2 = 1;
			          //off above
			          else if(Generic_Output_1_Link_2_value >= Generic_Output_1_Link_2_on_set ) 
			              Generic_Output_1_Condition_2 = 0; 
			       
			   	  }
			            //todo - the bin points need to be sorted here!!!
				  //on above
			  	  else { 
			      	if (Generic_Output_1_Link_2_value <= Generic_Output_1_Link_2_on_set )
			      	    Generic_Output_1_Condition_2 = 1;
			     	 // off below
			     	 else if (Generic_Output_1_Link_2_value <= Generic_Output_1_Link_2_on_set ) 
			           Generic_Output_1_Condition_2 = 0;			   	
			  	  }
			          //todo - the bin points need to be sorted here!!!
			      if (Generic_Output_1_Link_2_logic == 0)    
			         Generic_Output_1 = Generic_Output_1 + Generic_Output_1_Condition_2;
			      else
			         Generic_Output_1 = Generic_Output_1 * Generic_Output_1_Condition_2;


	              //link 3 switch condition
				  if (Generic_Output_1_type == 3){ //Switch with 3 links
			  	     //link 1
			  	     //on condition
			  	     if (Generic_Output_1_Link_3_on == 0){
			     	      // on below
			    	    if (Generic_Output_1_Link_3_value <= Generic_Output_1_Link_3_on_set ) 
			     	      Generic_Output_1_Condition_3 = 1;
			     	    //off above
			      	    else if(Generic_Output_1_Link_3_value >= Generic_Output_1_Link_3_on_set ) 
			      	       Generic_Output_1_Condition_3 = 0; 
			       
			  	     }
			            //todo - the bin points need to be sorted here!!!
					//on above
			      	 else { 
			   	        if (Generic_Output_1_Link_3_value <= Generic_Output_1_Link_3_on_set )
			               Generic_Output_1_Condition_3 = 1;
			   	        // off below
			    	    else if (Generic_Output_1_Link_3_value <= Generic_Output_1_Link_3_on_set ) 
			               Generic_Output_1_Condition_3 = 0;			   	
			   	     }
			          //todo - the bin points need to be sorted here!!! 
			         if (Generic_Output_1_Link_2_logic == 0)    
			            Generic_Output_1 = Generic_Output_1 + Generic_Output_1_Condition_3;
			         else
			            Generic_Output_1 = Generic_Output_1 * Generic_Output_1_Condition_3;

	                  //link 4 switch condition
				     if (Generic_Output_1_type == 4){ //Switch with 4 links
			  	        //link 1
			  	        //on condition
			  	        if (Generic_Output_1_Link_4_on == 0){
			   	           // on below
			   	           if (Generic_Output_1_Link_4_value <= Generic_Output_1_Link_4_on_set ) 
			                  Generic_Output_1_Condition_4 = 1;
			   	           //off above
			    	       else if(Generic_Output_1_Link_4_value >= Generic_Output_1_Link_4_on_set ) 
			                  Generic_Output_1_Condition_4 = 0; 
			       
			  	        }
			            //todo - the bin points need to be sorted here!!!
				        //on above
			   	        else { 
			    	       if (Generic_Output_1_Link_4_value <= Generic_Output_1_Link_4_on_set )
			    	          Generic_Output_1_Condition_4 = 1;
			    	       // off below
			    	       else if (Generic_Output_1_Link_4_value <= Generic_Output_1_Link_4_on_set ) 
			                  Generic_Output_1_Condition_4 = 0;			   	
			  	        }
			          //todo - the bin points need to be sorted here!!! 
			            if (Generic_Output_1_Link_2_logic == 0)    
			               Generic_Output_1 = Generic_Output_1 + Generic_Output_1_Condition_4;
			            else
			               Generic_Output_1 = Generic_Output_1 * Generic_Output_1_Condition_4;
	
			     	 }//link 4 switch condition
		          }//link 3 switch condition
		       }//link 2 switch condition
		       if (Generic_Output_1 > 1)
		           Generic_Output_1 = 1;
		       Set_Pin(GENERIC_OUPUT_1, Generic_Output_1);

		       
		    			
			}//link 1 switch condition
			*/
			
			//Fan switch
			if( CLT >= Generic_Output_1_Link_1_on_set)
		       Set_Pin(GENERIC_OUPUT_1, 1);
			else if (CLT <= Generic_Output_1_Link_1_off_set)
		 	   Set_Pin(GENERIC_OUPUT_1, 0);
			
			
			//Water injectiojn switch
			if( RPM >= Generic_Output_2_Link_1_on_set || Reference_VE >= Generic_Output_2_Link_2_on_set || IAT >= Generic_Output_2_Link_3_on_set)
		       Set_Pin(GENERIC_OUPUT_2, 1);
			else if ( RPM <= Generic_Output_2_Link_1_off_set && Reference_VE <= Generic_Output_2_Link_2_off_set && IAT <= Generic_Output_2_Link_3_off_set)
		 	   Set_Pin(GENERIC_OUPUT_2, 0);			
			
			
						
//			}else if(Generic_Output_1_type == 5){ //PWM Fixed
//			}else if(Generic_Output_1_type == 6){ //PWM Fixed Frequency w/dutycycle Table
//			}else if(Generic_Output_1_type == 7){ //PWM Fixed DutyCycle w/frequency Table
//			}else if(Generic_Output_1_type == 8){ //PWM Fixed Frequency w/dutycycle contol
//			}else if(Generic_Output_1_type == 9){ //PWM Fixed DutyCycle w/frequeny contol
			
//			}else{ //off	Generic_Output_1_type == 0		
//			
//			}

         // }//if output enabled
   
           task_wait(97);    
        } // for

    task_close();     

} // Cam_Pulse_Task()

void Generic_Output_2_Task(void)
    {
        task_open();                // standard OS entry
        task_wait(1);

        //static uint_fast8_t tooth;

       
        for (;;) {
			if (Generic_Output_2_type == 0)	{ //off
				
			}else if(Generic_Output_2_type == 1){ //Switch w/1 link
				
			}else if(Generic_Output_2_type == 2){ //Switch w/2 links
				
			}else if(Generic_Output_2_type == 3){ //Switch w/3 links
			}else if(Generic_Output_2_type == 4){ //Switch w/4 links
			}else if(Generic_Output_2_type == 5){ //PWM FIxed
			}else if(Generic_Output_2_type == 6){ //PWM Fixed Frequency w/dutycycle Table
			}else if(Generic_Output_2_type == 7){ //PWM Fixed DutyCycle w/frequency Table
			}else if(Generic_Output_2_type == 8){ //PWM Fixed Frequency w/dutycycle contol
			}else if(Generic_Output_2_type == 9){ //PWM Fixed DutyCycle w/frequeny contol
			
			}


   
            
        } // for

    task_close();     

} // Cam_Pulse_Task()
/*
void Generic_Output_3_Task(void)
    {
        task_open();                // standard OS entry
        task_wait(1);

        //static uint_fast8_t tooth;

       
        for (;;) {
			if (Generic_Output_3_type == 0)	{ //off
				
			}else if(Generic_Output_3_type == 1){ //Switch w/1 link
				
			}else if(Generic_Output_3_type == 2){ //Switch w/2 links
				
			}else if(Generic_Output_3_type == 3){ //Switch w/3 links
			}else if(Generic_Output_3_type == 4){ //Switch w/4 links
			}else if(Generic_Output_3_type == 5){ //PWM FIxed
			}else if(Generic_Output_3_type == 6){ //PWM Fixed Frequency w/dutycycle Table
			}else if(Generic_Output_3_type == 7){ //PWM Fixed DutyCycle w/frequency Table
			}else if(Generic_Output_3_type == 8){ //PWM Fixed Frequency w/dutycycle contol
			}else if(Generic_Output_3_type == 9){ //PWM Fixed DutyCycle w/frequeny contol
			
			}


   
            
        } // for

    task_close();     

} // Cam_Pulse_Task()

void Generic_Output_4_Task(void)
    {
        task_open();                // standard OS entry
        task_wait(1);

        //static uint_fast8_t tooth;

       
        for (;;) {
			if (Generic_Output_4_type == 0)	{ //off
				
			}else if(Generic_Output_4_type == 1){ //Switch w/1 link
				
			}else if(Generic_Output_4_type == 2){ //Switch w/2 links
				
			}else if(Generic_Output_4_type == 3){ //Switch w/3 links
			}else if(Generic_Output_4_type == 4){ //Switch w/4 links
			}else if(Generic_Output_4_type == 5){ //PWM FIxed
			}else if(Generic_Output_4_type == 6){ //PWM Fixed Frequency w/dutycycle Table
			}else if(Generic_Output_4_type == 7){ //PWM Fixed DutyCycle w/frequency Table
			}else if(Generic_Output_4_type == 8){ //PWM Fixed Frequency w/dutycycle contol
			}else if(Generic_Output_4_type == 9){ //PWM Fixed DutyCycle w/frequeny contol
			
			}


   
            
        } // for

    task_close();     

} // Cam_Pulse_Task()*/

