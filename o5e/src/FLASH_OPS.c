/*********************************************************************************

        @file   FLASH_OPS.c
        @author P. Schlein, Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU - flash writing
        @note   functions for Initializing and Operating 5xxx's FLASH Memory
        @version .9
        @copyright 2011 P. Schlein, Jon Zeeff

*************************************************************************************/

// Portions Copyright 2011 P. Schlein - MIT License
// Portions Copyright 2011,2012  Jon Zeeff - All rights reserved
// Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>

#include <stdint.h>
#include "mpc563xm.h"
#include "config.h"
#include "FLASH_OPS.h"
#include "trap.h"

struct flash_attr flash_attr[] = 
{
	0,  16*1024, (uint8_t *)0x00000000,    // block 0, low address space, bank 0, array0
	1,  16*1024, (uint8_t *)0x00004000,    // block 1a
	2,  32*1024, (uint8_t *)0x00008000,    // block 1b, not bootable
	3,  32*1024, (uint8_t *)0x00010000,    // block 2a
	4,  16*1024, (uint8_t *)0x00018000,    // block 2b, not bootable
	5,  16*1024, (uint8_t *)0x0001c000,    // block 3
	6,  64*1024, (uint8_t *)0x00020000,    // block 4
	7,  64*1024, (uint8_t *)0x00030000,    // block 5
	8, 128*1024, (uint8_t *)0x00040000,	   // block 6, middle address space
	9, 128*1024, (uint8_t *)0x00060000,    // block 7,
    0, 128*1024, (uint8_t *)0x00080000,    // block 8, high address space, bank 1, array 1
    1, 128*1024, (uint8_t *)0x000A0000,    // block 9
    2, 128*1024, (uint8_t *)0x000C0000,    // block 10
    3, 128*1024, (uint8_t *)0x000E0000,    // block 11
};



void flash_erase( uint32_t block ) {                                    // we just handle bank 0 now...


    CFLASH0.LMLR.R = 0xA1A11111;                                        // magic number
    CFLASH0.LMLR.R &= ~( 1U << flash_attr[block].bitno_mlr );           // unlock
    CFLASH0.SLMLR.R = 0xC3C33333;
    CFLASH0.SLMLR.R &= ~( 1U << flash_attr[block].bitno_mlr );

    CFLASH0.MCR.B.ERS = 1;
    CFLASH0.LMSR.R = ( 1U << flash_attr[block].bitno_mlr );

    *(uint32_t *)(flash_attr[block].addr) = 0xffffffff;                 // erase interlock write
    CFLASH0.MCR.B.EHV = 1;                                              // start erase
    }



void flash_program( uint32_t block, long long *src, uint32_t block_offset ) { // just handle bank 0 now...


	CFLASH0.MCR.B.PGM = 1;
    CFLASH0.LMLR.R = 0xA1A11111;                                       // magic number
    CFLASH0.LMLR.R &= ~( 1U << flash_attr[block].bitno_mlr );          // unlock
    CFLASH0.SLMLR.R = 0xC3C33333;
    CFLASH0.SLMLR.R &= ~( 1U << flash_attr[block].bitno_mlr );

    *(long long *)(flash_attr[block].addr + block_offset) = *src;      // program 64 bits
    CFLASH0.MCR.B.EHV = 0x01;                                          // start programming
    }



int32_t Flash_Ready()                          // Test if flash is done erasing or programming
{                                              // 1 = done, otherwise 0

    if (CFLASH0.MCR.B.DONE != 1)
        return 0;

    trap( CFLASH0.MCR.B.PEG );                 /**< if we're done, it better have been succesful */
    return 1;
}



void Flash_Finish()                           // Clear programming, back to read mode
{
    CFLASH0.MCR.B.EHV = 0;                    // Step 7 - Program/Erase Sequence Complete
    CFLASH0.MCR.B.PGM = 0;                    // Step 9 - Terminate Program/Erase Sequence
    CFLASH0.MCR.B.ERS = 0;
}



