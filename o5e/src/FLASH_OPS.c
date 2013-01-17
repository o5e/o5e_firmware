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

#define BLOCK0 0  // BLK1B
#define BLOCK1 1  // BLK2A

static void Erase_BLK1B(void);
static void Erase_BLK2A(void);
static void *Program_BLK1B(long long *source, uint32_t destination);
static void *Program_BLK2A(long long *source, uint32_t destination);


//********************************************************************************************

// Note: MPC5634 has 3 flash controllers instead of one.  We use the last 2 with the program code in the first
// We are using two high blocks, not a mid block

/* WRONG - Rev2 devices are 2 banks ONLY - this scheme is broken TODO: fix */

/**********************************************************************************/
/* FUNCTION     : Erase the First Block in the Mid Address Space, M0              */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
/**********************************************************************************/
static void Erase_BLK1B(void)
{
    uint32_t *FLASH_Pntr = (uint32_t *)BLK1B_BASE;

    /*  First, Perform the Block Unlocking by Writing Passwords to the LMLR            */
    CFLASH0.LMLR.R = 0xA1A11111;
    trap( CFLASH0.LMLR.B.LME );           /**< assert LMLR is modifiable  */
    CFLASH0.LMLR.R &= ~(1<<2);            /**< unlock BLK1B               */
    CFLASH0.SLMLR.R = 0xC3C33333;
    trap( CFLASH0.SLMLR.B.SLE );          /**< assert SLMLR is modifiable */
    CFLASH0.SLMLR.R &= ~(1<<2);           /**< unlock BLK1B               */

    CFLASH0.MCR.B.ERS = 0x01;     // Step 1 - Enter Erase Sequence 

    // Bank0, Array 0, block 1b
    CFLASH0.LMSR.R = (1<<2);

    /*Write to Anywhere in Flash */// Step 3 - Erase Interlock Write to Array
    /*Write to first M0 Address */// Erase Interlock Write
    *FLASH_Pntr = 0xffffffff;
    CFLASH0.MCR.B.EHV = 0x01;     // Step 4 - Start Internal Erase Sequence
}

/**********************************************************************************/
/* FUNCTION     : Program the First Block                                         */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : address written                                                 */
/* WARNING      : No Abort or Suspend Logic Included                              */
//  NOTE:  destination is bytes relative to the begining of the flash block

/**********************************************************************************/
static void *Program_BLK1B(long long *source, uint32_t destination)
{
    long long *FLASH_Pntr = (long long *)BLK1B_BASE;
    void *address;

    // Bank0, Array 0, block 1b

    CFLASH0.MCR.B.PGM = 0x01;     // Step 1 - Enter Program Sequence 

    /*  First, Perform the Block Unlocking by Writing Passwords to the LMLR            */
    CFLASH0.LMLR.R = 0xA1A11111;
    trap( CFLASH0.LMLR.B.LME );           /**< assert LMLR is modifiable  */
    CFLASH0.LMLR.R &= ~(1<<2);            /**< unlock BLK1B               */
    CFLASH0.SLMLR.R = 0xC3C33333;
    trap( CFLASH0.SLMLR.B.SLE );          /**< assert SLMLR is modifiable */
    CFLASH0.SLMLR.R &= ~(1<<2);           /**< unlock BLK1B               */
    
    FLASH_Pntr += destination / 8;      //  NOTE:  destination is bytes relative to the begining of the flash block - not absolute
    address = (void *)FLASH_Pntr;

    *FLASH_Pntr = *source;      // 8 bytes

    CFLASH0.MCR.B.EHV = 0x01;     // Step 4 - Start Internal Program Sequence

    return address;
}

/**********************************************************************************/
/* FUNCTION     : Erase the First Block in the High Address Space, H0             */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
/**********************************************************************************/
static void Erase_BLK2A(void)
{
    uint32_t *FLASH_Pntr = (uint32_t *)BLK2A_BASE;

    /*  First, Perform the Block Unlocking by Writing Passwords to the LMLR            */
    CFLASH0.LMLR.R = 0xA1A11111;
    trap( CFLASH0.LMLR.B.LME );           /**< assert LMLR is modifiable  */
    CFLASH0.LMLR.R &= ~(1<<3);            /**< unlock BLK2A               */
    CFLASH0.SLMLR.R = 0xC3C33333;
    trap( CFLASH0.SLMLR.B.SLE );          /**< assert SLMLR is modifiable */
    CFLASH0.SLMLR.R &= ~(1<<3);           /**< unlock BLK2A               */
    
    CFLASH0.MCR.B.ERS = 0x01;     // Enter Erase Sequence 

    // Bank0, Array 0, block 2a
    CFLASH0.LMSR.R = (1<<3);

    // Write to first H0 Address - Erase Interlock Write
    *FLASH_Pntr = 0xffffffff;

    CFLASH0.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Erase Sequence
}

/**********************************************************************************/
/* FUNCTION     : Program the First Block in the High Address Space, H0             */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
//  NOTE:  destination is bytes relative to the begining of the flash block 
/**********************************************************************************/
static void *Program_BLK2A(long long *source, uint32_t destination)
{
    long long *FLASH_Pntr = (long long *)BLK2A_BASE;
    void *address;

    // Bank0, Array 0, block 2a

    CFLASH0.MCR.B.PGM = 0x01;     // Step 1 - Enter Program Sequence 

    /*  First, Perform the Block Unlocking by Writing Passwords to the LMLR            */
    CFLASH0.LMLR.R = 0xA1A11111;
    trap( CFLASH0.LMLR.B.LME );           /**< assert LMLR is modifiable  */
    CFLASH0.LMLR.R &= ~(1<<3);            /**< unlock BLK2A               */
    CFLASH0.SLMLR.R = 0xC3C33333;
    trap( CFLASH0.SLMLR.B.SLE );          /**< assert SLMLR is modifiable */
    CFLASH0.SLMLR.R &= ~(1<<3);           /**< unlock BLK2A               */

    FLASH_Pntr += destination / 8;      //  NOTE:  destination is relative to the begining of the flash block - not absolute
    address = (void *)FLASH_Pntr;       // save for return

    *FLASH_Pntr = *source;      // 8 bytes

    CFLASH0.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Program Sequence
    return address;
}

/**********************************************************************/

// Test if flash is done erasing or programming
// 1 = done, otherwise 0

int32_t Flash_Ready()
{

    if (CFLASH0.MCR.B.DONE != 1)
        return 0;

    trap( CFLASH0.MCR.B.PEG );   /**< if we're done, it better have been succesful */
    return 1;

}

/**********************************************************************/

// Clear all programming - put back to read mode

void Flash_Finish( uint8_t block )
{
  (void)block;              /**< everything in bank0 now */
    CFLASH0.MCR.B.EHV = 0;  //Step 7 - Program/Erase Sequence Complete
    CFLASH0.MCR.B.PGM = 0;  //Step 9 - Terminate Program/Erase Sequence
    CFLASH0.MCR.B.ERS = 0;
    /* CAN'T DO THE FOLLOWING FROM FLASH! */
//    CFLASH0.BIUCR.B.BFEN = 0;       //FBIU Line Read Buffers Enable
//    CFLASH0.BIUCR.B.BFEN = 1;       //FBIU Line Read Buffers Enable

}                               // Flash_Finish()

/***********************************************************************/

// program either flash block - 8 bytes will be written
// destination is a multiple of 8, and a
// !!! BYTE OFFSET within the block, not an ptr/address
// returns first address written

void *Flash_Program(uint8_t block, long long *source, uint32_t destination)
{
    //if (*source == 0xffffffffffffffff)   // erased flash is already 0xff...
    //   return 0;  // TODO fix this

    if (block == BLOCK0)
        return Program_BLK1B(source, destination);
    else
        return Program_BLK2A(source, destination);
}

/***********************************************************************/

// erase either flash block

void Flash_Erase(uint8_t block)
{
    if (block == BLOCK0)
        Erase_BLK1B();
    else
        Erase_BLK2A();
}
