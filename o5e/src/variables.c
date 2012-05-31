
// Routines related to .ini variables, variables.h and flash management

#define EXTERN
#include "variables.h"
#include "system.h"
#include <stdlib.h>
#include "main.h"

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

  // check for signature at beginning of page #1
  // TODO - check that all pages have data in them, not just the header
  if (*(Flash_Addr[Flash_Block]) != 'A') // flash is invalid (probably all 0xff)
      Flash_OK = 0;	   		 // flash appears to be empty
  else {
      Flash_OK = 1;
      Burn_Count = ((struct Flash_Header *)Flash_Addr[Flash_Block])->Burn_Count;
  }

  Error_String[0] = '\0';		// initially empty

  // halt if code and #define don't agree on this
  // note that the compiler will round up to multiple of 4
  if (sizeof(struct Outputs) != Output_Channels_Size) {
     system_error(2181, __FILE__, __LINE__, "");
     for (;;) {}
  }

  // TODO - perform sanity checks on all of flash

} // init_variables()


// set pointers to where every page is in flash

void
Set_Page_Locations(uint8_t block)
{
  uint_fast8_t i;
  uint8_t *ptr = Flash_Addr[block] + BLOCK_HEADER_SIZE;	        // start at base + room for header data

  // page positions in flash - set defaults of page 0 at first page, etc.
  for (i = 0; i < nPages; ++i) {        // 
      Page_Ptr[i] = ptr;
      ptr += Max_Page_Size;	        // we always use fixed page spacing
  }
 
  // scan directory looking for non-default values caused by additional flash burns
  uint64_t *directory = (uint64_t *)(Flash_Addr[block] + BLOCK_HEADER_DIRECTORY);    // start with first directory entry in the header

  for (i = nPages; i < 64; ++i) {       // 64 pages are tracked for a 128K block

      if (directory[i] < nPages)        // unused directory entries are all 0xff and fail this test
         Page_Ptr[(int)directory[i]] = ptr;     // this page is located at this greater address 

      ptr += Max_Page_Size;	        // we always use fixed page spacing
  }				        // for

  Flash_Block = block;		        // save which block we are using
  Ram_Page_Buffer_Page = -1;            // mark as unused
}
