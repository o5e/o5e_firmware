/**********************************************************************************/
/* FILE NAME: FLASH_OPS.c                                                         */
/*                                                                                */
/* DESCRIPTION:                                                                   */
/* This file contains functions for Initializing and Operating 5554's FLASH Memory*/
/*                                                                                */
/*================================================================================*/
/* ORIGINAL AUTHOR:  Paul Schlein                                                 */
/*REV      AUTHOR          DATE          DESCRIPTION OF CHANGE                    */
/* ---     -----------     ----------    ---------------------                    */
/* 3.0     J. Zeeff	   2/Nov/11 	 System compatibilty routines, fixes      */
/* 2.0     P. Schlein      2/Nov/11      Include H0 Erase and Program             */
/* 1.0     P. Schlein      11/Oct/11     Initial version                          */
/**********************************************************************************/

#include "config.h"
#include "cpu.h"
#include "FLASH_OPS.h"
#include <stdint.h>

void init_FLASH(void)
{

/*  Initialize the FLASH for Minimum Wait States                                  */
/*  See RM Table 13-15, 13-22, pg 530; for 132 mhz-Best FLASH Peformance          */
/*  See RM Para 13.3.2.8, 13-19, pg 528 for FLASH BUS INTERFACE CONTROL UNIT      */

#ifdef MPC5554
    FLASH.BIUCR.B.M0PFE = 1;    //Configure Prefetch to e200z6 Core as Master
    FLASH.BIUCR.B.APC = 2;      //Address Pipelining Control-Freq Dependent
    FLASH.BIUCR.B.WWSC = 1;     //Write Wait State Control-Freq Dependent
    FLASH.BIUCR.B.RWSC = 3;     //Read Wait State Control-Freq Dependent
    FLASH.BIUCR.B.DPFEN = 3;    //Data Prefetch Enable-Any
    FLASH.BIUCR.B.IPFEN = 3;    //Instruction Prefetch Enable-Any
    FLASH.BIUCR.B.PFLIM = 6;    //Prefetch Limit-Unlimited
    FLASH.BIUCR.B.BFEN = 1;     //FBIU Line Read Buffers Enable
#endif

#ifdef MPC5634
// JZ - set flash timing for all 3 flash blocks
// WARNING - RM 10.3.3 says not to do this while running from flash

    CFLASH0.BIUCR.R = 0x00016B55;
    CFLASH1.BIUCR.R = 0x00016B55;
    CFLASH2.BIUCR.R = 0x00016B55;

// Set the CPU speed to 80 Mhz

//Program the FM Enhance PLL<<<no difference?>>>
//   MHz :  80 70 60 50 40 30 20 10
//ESYNCR1:  40 35 60 50 40 60 40 1,40
//ESYNCR2:  1  1  2  2  2  3  3  3
    asm {
//ESYNCR1
        lis r10, 0xC3F8
        lis r11, 0xF000         // EPREDIV -> 0-1 to 1110-15
        ori r11, r11, 40        // EMFD -> 32 to 96
//ESYNCR2
        li r12, 0x0001          // ERFD    -> 0-2,4,8 and 11-16
//save registers with the shortest possible time
        stw r11, 8(r10)         // ESYNCR1
        stw r12, 12(r10)        // ESYNCR2
wait_for_lock:
        lwz r9, 4(r10)          // load SYNSR
        andi.r9, r9, 0x8
        beq wait_for_lock
//flushes the BTB and Enable the BTB 
         li r10, 0x201
         mtspr 1013, r10
//enable SPE
         mfmsr r10
         oris r10, r10, 0x0200  //Enable SPE
         mtmsr r10
//initialize Crossbar
         lis r12, 0xFFF0
         ori r12, r12, 0x4000
         lis r11, 0x0001
         ori r11, r11, 0x0302
         stw r11, 0(r12)
    }
#endif
}                               // Init_FLASH()
#ifdef MPC5554
/**********************************************************************************//* FUNCTION     : Erase the First Block in the Mid Address Space, M0              *//* PURPOSE      :                                                                 *//* INPUT NOTES  : None                                                            *//* RETURN NOTES : None                                                            *//* WARNING      : No Abort or Suspend Logic Included                              *//**********************************************************************************/
    void Erase_M0(void)
{
    uint32_t *FLASH_Pntr = M0_BASE;

    FLASH.MCR.B.ERS = 0x01;     //Step 1 - Enter Erase Sequence 

/*  Erase First 128K Block in Mid Address Space, M0 - See RM Table-13-4           */
/*       Flash Partitioning, 13-6, pg 514.                                        */
/*  See RM 13.4.2.4 Flash Erase, 13-31, pg 539 for Erasure Process                */
/*  Because Block Select and Lock are Independent, the Low/Mid Address Space      */
/*       Locking Register, FLASH_LMLR, and its Secondary Counterpart, FLASH_SLMLR,*/
/*       and the Low/Mid Address Space Block Select Register, FLASH_LMSR are used.*/

/*  First, Perform the Block Unlocking by Writing Passwords to the LMLR and SLMLR */
/*  See RM 13.3.2.2 and 13.3.2.4, 13-12 and 13-15                                 */
    FLASH.LMLR.R = 0xA1A11111;  //Password to Set Low and Mid Lock Enable
    FLASH.SLMLR.R = 0xC3C33333; //Password to Set Secondary Lock Enable
    /*  Second, Select the M0 Block.  Note LMLR and SLMLR are Reset to 1s             */
    FLASH.LMLR.B.MLOCK = 0x0E;  //MLOCK = 0b1110
    FLASH.SLMLR.B.SMLOCK = 0x0E;        //SMLOCK = 0b1110

    /*  Now, Perform the Erase Process.                                               */
    FLASH.LMSR.B.MSEL = 0x01;   //Step 2 - Select M0 Block, Register Resets to 0s 

    /*Write to Anywhere in Flash *///Step 3 - Erase Interlock Write to Array
    /*Write to first M0 Address *///Erase Interlock Write
    *FLASH_Pntr = 0xffffffff;
    FLASH.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Erase Sequence
}

/**********************************************************************************/
/* FUNCTION     : Program the First Block in the Mid Address Space, M0            */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : address written                                                 */
/* WARNING      : No Abort or Suspend Logic Included                              */
//  NOTE:  destination is bytes relative to the begining of the flash block

/**********************************************************************************/
void *Program_M0(long long *source, uint32_t destination)
{
    long long *FLASH_Pntr = M0_BASE;
    void *address;

/*  Erase First 128K Block in Mid Address Space, M0 - See RM Table-13-4           */
/*       Flash Partitioning, 13-6, pg 514.                                        */
/*  See RM 13.4.2.3 FLASH Programming, 13-28, pg 536                              */

    /*  First, Perform the Block Unlocking by Writing Passwords to the LMLR and SLMLR */
    FLASH.LMLR.R = 0xA1A11111;  //Password to Set Low and Mid Lock Enable
    FLASH.SLMLR.R = 0xC3C33333; //Password to Set Secondary Lock Enable
    /*  Second, Select the M0 Block.  Note LMLR and SLMLR are Reset to 1s             */
    FLASH.LMLR.B.MLOCK = 0x0E;  //MLOCK = 0b1110
    FLASH.SLMLR.B.SMLOCK = 0x0E;        //SMLOCK = 0b1110

    /*  Now, Perform the Program Process.                                             */
    FLASH.MCR.B.PGM = 0x01;     //Step 1 - Enter Program Sequence 

    /*Write Further Data to M0 *///Step 3 - Program Data Write

    FLASH_Pntr += destination / 8;      //  NOTE:  destination is bytes offset relative to the begining of the flash block - not absolute
    address = (void *)FLASH_Pntr;

    *FLASH_Pntr = *source;      // 8 bytes

    FLASH.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Program Sequence

    return address;
}

/**********************************************************************************/
/* FUNCTION     : Erase the First Block in the High Address Space, H0             */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
/**********************************************************************************/
void Erase_H0(void)
{
    uint32_t *FLASH_Pntr = H0_BASE;

/*  Erase First 128K Block in High Address Space, H0 - See RM Table-13-4          */
/*       Flash Partitioning, 13-6, pg 514.                                        */
/*  See RM 13.4.2.4 Flash Erase, 13-31, pg 539 for Erasure Process                */
/*  Because Block Select and Lock are Independent, the High Address Space         */
/*       Locking Register, FLASH_HLR and the High Address Space Block Select      */
/*       Register, FLASH_HSR are used.                                            */

    FLASH.MCR.B.ERS = 0x01;     //Step 1 - Enter Erase Sequence 
/*  First, Perform the Block Unlocking by Writing Passwords to the HLR            */
/*  See RM 13.3.2.3 and 13.3.2.6, 13-14 and 13-17                                 */
    FLASH.HLR.R = 0xB2B22222;   //Password to Set High Lock Enable
/*  Second, Select the H0 Block.  Note HLR are Reset to 1s                        */
    FLASH.HLR.B.HBLOCK = 0xFFFFFFE;     //HLOCK = 0b1110

    /*  Now, Perform the Erase Process.                                               */
    FLASH.HSR.B.HBSEL = 0x01;   //Step 2 - Select H0 Block, Register Resets to 0s 

    /*Write to Anywhere in Flash *///Step 3 - Erase Interlock Write to Array
    /*Write to first M0 Address *///Erase Interlock Write
    *FLASH_Pntr = 0xffffffff;

    FLASH.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Erase Sequence

}

/**********************************************************************************/
/* FUNCTION     : Program the First Block in the High Address Space, H0             */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
//  NOTE:  destination is bytes relative to the begining of the flash block 
/**********************************************************************************/
void *Program_H0(long long *source, uint32_t destination)
{
    long long *FLASH_Pntr = H0_BASE;
    void *address;

/*  Program First 128K Block in High Address Space, H0 - See RM Table-13-4          */
/*       Flash Partitioning, 13-6, pg 514.                                        */
/*  See RM 13.4.2.3 FLASH Programming, 13-28, pg 536                              */
/*  Because Block Select and Lock are Independent, the High Address Space         */
/*       Locking Register, FLASH_HLR and the High Address Space Block Select      */
/*       Register, FLASH_HSR are used.                                            */

/*  First, Perform the Block Unlocking by Writing Passwords to the HLR            */
    /*  See RM 13.3.2.3 and 13.3.2.6, 13-14 and 13-17                                 */
    FLASH.HLR.R = 0xB2B22222;   //Password to Set High Lock Enable
    /*  Second, Select the H0 Block.  Note HLR are Reset to 1s                        */
    FLASH.HLR.B.HBLOCK = 0xFFFFFFE;     //HLOCK = 0b1110

    /*  Now, Perform the Program Process.                                             */
    FLASH.MCR.B.PGM = 0x01;     //Step 1 - Enter Program Sequence 

    FLASH_Pntr += destination / 8;      //  NOTE:  destination is relative to the begining of the flash block - not absolute
    address = (void *)FLASH_Pntr;

    *FLASH_Pntr = *source;      // 8 bytes

    FLASH.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Program Sequence
    return address;
}

#endif

//********************************************************************************************

#ifdef MPC5634

// Note: MPC5634 has 3 flash controllers instead of one.  We use the last 2 with the program code in the first
// We are using two high blocks, not a mid block

// allow code to look the same 
#   undef FLASH
#   define FLASH CFLASH1

/**********************************************************************************/
/* FUNCTION     : Erase the First Block in the Mid Address Space, M0              */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
/**********************************************************************************/
void Erase_M0(void)
{
    uint32_t *FLASH_Pntr = (uint32_t *)M0_BASE;

    FLASH.HLR.R = 0xB2B22222;   // Password to Set High Lock Enable
    FLASH.HLR.B.HBLOCK = 0x0C;  // unlock first two blocks

    FLASH.MCR.B.ERS = 0x01;     // Step 1 - Enter Erase Sequence 

    // Bank1, Array 1, block 8
    FLASH.HSR.B.HBSEL = 0x03;   // HBSEL portion of HSR

    /*Write to Anywhere in Flash */// Step 3 - Erase Interlock Write to Array
    /*Write to first M0 Address */// Erase Interlock Write
    *FLASH_Pntr = 0xffffffff;
    FLASH.MCR.B.EHV = 0x01;     // Step 4 - Start Internal Erase Sequence
}

/**********************************************************************************/
/* FUNCTION     : Program the First Block                                         */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : address written                                                 */
/* WARNING      : No Abort or Suspend Logic Included                              */
//  NOTE:  destination is bytes relative to the begining of the flash block

/**********************************************************************************/
void *Program_M0(long long *source, uint32_t destination)
{
    long long *FLASH_Pntr = (long long *)M0_BASE;
    void *address;

    // Bank1, Array 1, block 8

    FLASH.MCR.B.PGM = 0x01;     // Step 1 - Enter Program Sequence 

    FLASH.HLR.R = 0xB2B22222;   // Password to Set High Lock Enable
    FLASH.HLR.B.HBLOCK = 0x0C;  // unlock first two blocks

    FLASH_Pntr += destination / 8;      //  NOTE:  destination is bytes relative to the begining of the flash block - not absolute
    address = (void *)FLASH_Pntr;

    *FLASH_Pntr = *source;      // 8 bytes

    FLASH.MCR.B.EHV = 0x01;     // Step 4 - Start Internal Program Sequence

    return address;
}

#   undef FLASH
#   define FLASH CFLASH2

/**********************************************************************************/
/* FUNCTION     : Erase the First Block in the High Address Space, H0             */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
/**********************************************************************************/
void Erase_H0(void)
{
    uint32_t *FLASH_Pntr = (uint32_t *)H0_BASE;

    /*  First, Perform the Block Unlocking by Writing Passwords to the HLR            */
    FLASH.HLR.R = 0xB2B22222;   // Password to Set High Lock Enable
    FLASH.HLR.B.HBLOCK = 0x0C;  // unlock first two blocks

    FLASH.MCR.B.ERS = 0x01;     // Enter Erase Sequence 

    // Bank1, Array 2, block 12
    FLASH.HSR.B.HBSEL = 0x03;   // HBSEL portion of HSR

    // Write to first H0 Address - Erase Interlock Write
    *FLASH_Pntr = 0xffffffff;

    FLASH.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Erase Sequence
}

/**********************************************************************************/
/* FUNCTION     : Program the First Block in the High Address Space, H0             */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      : No Abort or Suspend Logic Included                              */
//  NOTE:  destination is bytes relative to the begining of the flash block 
/**********************************************************************************/
void *Program_H0(long long *source, uint32_t destination)
{
    long long *FLASH_Pntr = (long long *)H0_BASE;
    void *address;

    // Bank1, Array 2, block 12

    FLASH.MCR.B.PGM = 0x01;     // Step 1 - Enter Program Sequence 

    /*  First, Perform the Block Unlocking by Writing Passwords to the HLR            */
    FLASH.HLR.R = 0xB2B22222;   //Password to Set High Lock Enable
    FLASH.HLR.B.HBLOCK = 0x0C;  // unlock two low blocks

    FLASH_Pntr += destination / 8;      //  NOTE:  destination is relative to the begining of the flash block - not absolute
    address = (void *)FLASH_Pntr;       // save for return

    *FLASH_Pntr = *source;      // 8 bytes

    FLASH.MCR.B.EHV = 0x01;     //Step 4 - Start Internal Program Sequence
    return address;
}

#   undef FLASH

#endif

/**********************************************************************/

// Test if flash is done erasing or programming
// 1 = done, otherwise 0

int32_t Flash_Ready()
{

#ifdef MPC5554
    if (FLASH.MCR.B.DONE != 1)
        return 0;
#endif

#ifdef MPC5634
    if (CFLASH1.MCR.B.DONE != 1 /* || CFLASH1.MCR.B.PEG != 1 */ )
        return 0;
    if (CFLASH2.MCR.B.DONE != 1 /* || CFLASH2.MCR.B.PEG != 1 */ )
        return 0;
#endif
    return 1;

}

/**********************************************************************/

// Clear all programming - put back to read mode

void Flash_Finish(uint8_t block)
{

#ifdef MPC5554
    FLASH.MCR.B.EHV = 0;        //Step 7 - Program/Erase Sequence Complete    
    FLASH.MCR.B.PGM = 0;        //Step 9 - Terminate Program Sequence
    FLASH.MCR.B.STOP = 0;
    FLASH.BIUCR.B.BFEN = 0;     //FBIU Line Read Buffers Enable
    FLASH.BIUCR.B.BFEN = 1;     //FBIU Line Read Buffers Enable
#endif

#ifdef MPC5634

    if (block == 0) {
        CFLASH1.MCR.B.EHV = 0;  //Step 7 - Program/Erase Sequence Complete    
        CFLASH1.MCR.B.PGM = 0;  //Step 9 - Terminate Program Sequence
        CFLASH1.BIUCR.B.BFEN = 0;       //FBIU Line Read Buffers Enable
        CFLASH1.BIUCR.B.BFEN = 1;       //FBIU Line Read Buffers Enable
    }

    if (block == 1) {
        CFLASH2.MCR.B.EHV = 0;  //Step 7 - Program/Erase Sequence Complete    
        CFLASH2.MCR.B.PGM = 0;  //Step 9 - Terminate Program Sequence
        CFLASH2.BIUCR.B.BFEN = 0;       //FBIU Line Read Buffers Enable
        CFLASH2.BIUCR.B.BFEN = 1;       //FBIU Line Read Buffers Enable
    }
#endif

}                               // Flash_Finish()

void Erase_Finish(uint8_t block)
{

#ifdef MPC5554
    FLASH.MCR.B.EHV = 0;        //Step 7 - Program/Erase Sequence Complete    
    FLASH.MCR.B.ERS = 0;        //Step 9 - Terminate Erase
    FLASH.MCR.B.STOP = 0;
    FLASH.BIUCR.B.BFEN = 0;     //FBIU Line Read Buffers Enable
    FLASH.BIUCR.B.BFEN = 1;     //FBIU Line Read Buffers Enable
#endif

#ifdef MPC5634

    if (block == 0) {
        CFLASH1.MCR.B.EHV = 0;  //Step 7 - Program/Erase Sequence Complete    
        CFLASH1.MCR.B.ERS = 0;  //Step 9 - Terminate Erase
        CFLASH1.BIUCR.B.BFEN = 0;       //FBIU Line Read Buffers Enable
        CFLASH1.BIUCR.B.BFEN = 1;       //FBIU Line Read Buffers Enable
    }

    if (block == 1) {
        CFLASH2.MCR.B.EHV = 0;  //Step 7 - Program/Erase Sequence Complete    
        CFLASH2.MCR.B.ERS = 0;  //Step 9 - Terminate Erase
        CFLASH2.BIUCR.B.BFEN = 0;       //FBIU Line Read Buffers Enable
        CFLASH2.BIUCR.B.BFEN = 1;       //FBIU Line Read Buffers Enable
    }
#endif

}                               // Erase_Finish()

/***********************************************************************/

// program either flash block - 8 bytes will be written
// destination is a multiple of 8, and a
// !!! BYTE OFFSET within the block, not an ptr/address
// returns first address written

void *Flash_Program(uint_fast8_t block, long long *source, uint32_t destination)
{
    //if (*source == 0xffffffffffffffff)   // erased flash is already 0xff...
    //   return 0;  // TODO fix this

    if (block == BLOCK0)
        return Program_M0(source, destination);
    else
        return Program_H0(source, destination);
}

/***********************************************************************/

// erase either flash block

void Flash_Erase(uint_fast8_t block)
{
    if (block == BLOCK0)
        Erase_M0();
    else
        Erase_H0();
}
