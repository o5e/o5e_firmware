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
   Copyright 2012, Sean Stasiak <sstasiak at gmail dot com> */

#include <stdint.h>
#include "variables.h"
#include "err.h"
#include "FLASH_OPS.h"    /**< pickup xx_BASE #define's */

struct Outputs Output_Channels;

uint16_t const pageSize[NPAGES] = { 168, 1810, 1544, 1990, 1672, 1672, 1672, 1672, 1952, 1952, 1952, 1952, 1952, 1952 };
// Current flash or ram location of each page
volatile uint8_t *Page_Ptr[NPAGES];
// Ram buffer to store a single page before writing to flash
uint8_t Ram_Page_Buffer[MAX_PAGE_SIZE];
int8_t Ram_Page_Buffer_Page;	// which page # is in the buffer (-1 means none)
int Flash_OK;		// is flash empty or has values
uint8_t Burn_Count;		// how many flash burns 

uint8_t Flash_Block;		// flash block currently being used - 0=BLK1B_BASE or 1=BLK2A_BASE
uint8_t *Flash_Addr[2] = { (uint8_t *)BLK1B_BASE, (uint8_t *)BLK2A_BASE };

// Called on cpu startup

void
init_variables(void)
{
  // find the newest (last written) block
  if (*(Flash_Addr[1]) == 'A') 
     if ((*(Flash_Addr[0]) != 'A') || ((struct Flash_Header *)Flash_Addr[1])->Burn_Count > ((struct Flash_Header *)Flash_Addr[0])->Burn_Count)
         Set_Page_Locations(1);		// record where each page is in block 1
     else
         Set_Page_Locations(0);		// best to use first block
  else
     Set_Page_Locations(0);		// record where each page is in block 0
     
  /* if both blocks are wiped out, Page_Ptr[]'s always default to block 0 (M0_BASE) */

  // check for signature at beginning of page #1
  // TODO - check that all pages have data in them, not just the header
  if (*(Flash_Addr[Flash_Block]) != 'A') // flash is invalid (probably all 0xff)
      Flash_OK = 0;	   		 // flash appears to be empty
  else {
      Flash_OK = 1;
      // take initial Burn_Count from existing flash (otherwise 0)
      Burn_Count = ((struct Flash_Header *)Flash_Addr[Flash_Block])->Burn_Count;
  }

  // halt if code and #define don't agree on this
  // note that the compiler will round up to multiple of 4
  if (sizeof(struct Outputs) != OUTPUT_CHANNELS_SIZE) {
     err_push( CODE_OLDJUNK_E4 );
     for (;;) {}
  }

  // TODO - perform sanity checks on all of flash

} // init_variables()


// set pointers to where every page is in flash

void
Set_Page_Locations(uint8_t block)
{
  uint8_t i;
  uint8_t *ptr = Flash_Addr[block] + BLOCK_HEADER_SIZE;	        // start at base + room for header data

  // page positions in flash - set defaults of page 0 at first page, etc.
  for (i = 0; i < NPAGES; ++i) {        // 
      Page_Ptr[i] = ptr;
      ptr += MAX_PAGE_SIZE;	        // we always use fixed page spacing
  }

  Flash_Block = block;		        // save which block we are using
  Ram_Page_Buffer_Page = -1;            // mark as unused
}
