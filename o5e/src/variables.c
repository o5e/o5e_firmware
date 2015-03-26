/*********************************************************************************

        @file   variables.c                                                              
        @author Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU - variable handling
        @note   Routines related to .ini variables, variables.h and flash management
        @version .9
        @copyright 2011 Jon Zeeff

*************************************************************************************/

/* Copyright(c) 2011 Jon Zeeff
   Copyright 2012, Sean Stasiak <sstasiak at gmail dot com> 
   Copyright 2011, 2012, 2013, Mark Eberhardt */

#include <stdint.h>
#include "variables.h"
#include "err.h"
#include "FLASH_OPS.h"    /**< pickup xx_BASE #define's */

#include "led.h"

struct Outputs Output_Channels;

uint16_t const pageSize[NPAGES] = {1768,1664,1756,1760,1620,1668,1668,1668,1668,1808,1808,1808 };

volatile uint8_t *Page_Ptr[NPAGES];                // Current flash or ram location of each page
uint8_t Ram_Page_Buffer[MAX_PAGE_SIZE];            // single page ram buffer for writing to flash
int8_t Ram_Page_Buffer_Page;                       // which page # is in the buffer (-1 means none)
int Flash_OK;		                               // is flash empty or has values
uint8_t Burn_Count; 
uint8_t Flash_Block;		                       // flash block currently being used



/* Called on cpu startup.
   Find most recent valid flash block and set the page pointers.
   Instead of a special signature we could just use a larger burn count. The whole
   page should also have a CRC...  */
  

void
init_variables(void)
{

   uint8_t *blk1, *blk2;
   

   Flash_Block = BLOCK4;                                         // default flash block
   blk1 = flash_attr[BLOCK4].addr;
   blk2 = flash_attr[BLOCK5].addr;

   if ( *blk1 == 'A'  &&  *blk2 == 'A' ) {                       // 2 valid, check burn count
      if ( ((struct Flash_Header *)blk2)->Burn_Count > ((struct Flash_Header *)blk1)->Burn_Count )
         Flash_Block = BLOCK5;
      }
   else if ( *blk2 == 'A' )
      Flash_Block = BLOCK5;
   
   Set_Page_Locations( Flash_Block );

   if ( *(flash_attr[Flash_Block].addr) == 'A' ) {
      Flash_OK = 1;
      Burn_Count =  ((struct Flash_Header *)flash_attr[Flash_Block].addr)->Burn_Count;
   }

   if (sizeof(struct Outputs) != OUTPUT_CHANNELS_SIZE) {         // halt if code and #define don't agree
      err_push( CODE_OLDJUNK_E4 );                               // compiler rounds up to multiple of 4
      for (;;) {}
   }
}



void
Set_Page_Locations(uint32_t block)                                  // set pointers to every page in flash
{
   uint32_t i;
   uint8_t *ptr = flash_attr[block].addr + BLOCK_HEADER_SIZE;	    // start at base + room for header data


   for( i = 0; i < NPAGES; ++i) {                                   // page 0 at first page, etc.
      Page_Ptr[i] = ptr;
      ptr += MAX_PAGE_SIZE;	                                        // we always use fixed page spacing
   }

   Ram_Page_Buffer_Page = -1;                                       // mark as unused
}
