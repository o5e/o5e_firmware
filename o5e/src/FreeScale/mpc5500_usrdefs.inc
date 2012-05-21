#************************************************************************
#* FILE NAME: mpc5500_usrdefs.inc            COPYRIGHT (c) Freescale 2004 
#*                                                All Rights Reserved     
#* DESCRIPTION:                                                           
#* This file contains user definitions for the MPC5500 assembly functions.
#* The user will only need to make changes to this file for the assembly
#*  portion of this code.
#* 
#*========================================================================
#* ORIGINAL AUTHOR: G. Jackson           
#* REV      AUTHOR        DATE       DESCRIPTION OF CHANGE                
#* ---   -----------   -----------   ---------------------                  
#* 0.1   G. Jackson     12/Apr/04    Initial version        
#* 0.2   G. Jackson     15/Apr/04    Added compiler designations
#* 0.3   G. Jackson     13/May/04    Added runtime variables
#* 0.4   G. Jackson     06/Jun/04    Added EXT_BOOT config option
#* 0.5   G. Jackson     30/Jun/04    Added RCHW variables
#* 1.0   G. Jackson     07/Oct/04    Internal and external RAM set to 
#*                                     CACHE_INHIBIT (TLBs 3 & 11)
#* 1.1   Chris Baker    19/Jul/06    Added support for MPC5533, MPC5534,
#*                                     MPC5553, MPC5554, MPC5565, MPC5567
#*                                   Removed Codewarrior simulator support
#*                                   Moved FLSH_RUN equate to make it
#*                                     easier to find
#*                                   Changed loss of clock and loss of
#*                                     lock equates
#*                                   Changed FLASH_SETTINGS equate
#*                                   Added FMPLL_SYNCR equates for maximum
#*                                     supported frequencies
#*                                   Added SPR equates for P&E Gnu
#* 1.2   Chris Baker    25/Oct/06    Added MPC5561 and MPC5566 support
#*                                   Changed CACHE_SETTINGS equate to
#*                                     select 4 ways cache organisation
#*                                   Removed I_SRAM_SIZE equate
#*                                   Changed stack in cache TLB entry TLB10
#*                                     to use CACHE_STACK_BASE_ADDR
#*                                   Added device specific TLB entry size
#*                                     to SRAM TLB, TLB3
#*                                   Removed l1csr0 SPR capitalisation
#************************************************************************

# Processor and toolset selection
# Only set one of the following to 1
    .equ _MPC5533_, 0
    .equ _MPC5534_, 0
    .equ _MPC5553_, 0
    .equ _MPC5554_, 1
    .equ _MPC5561_, 0
    .equ _MPC5565_, 0
    .equ _MPC5566_, 0
    .equ _MPC5567_, 0

    .include "mpc5500_defs.inc"

# Select the compiler toolset
# Only set one of the following to 1
    .equ __GRNHS__,  0  # Green Hills
    .equ __PEGNU__,  1  # P&E Micro
    .equ __DIABCC__, 0  # Wind River
    .equ __CWWRKS__, 0  # Metrowerks

# Set to 1 for code in Flash
# Set to 0 for code in SRAM
    .equ FLSH_RUN, 0


#*************** Runtime Variables *****************
#  These runtime variables are used in __start.s
# main(), assembly cfg_* calls, and cfg_mpc5500ccd() need to   **
# have far absolute addressing if flashing to ROM with         **
# distant addresses on the MPC5500.                            **
# The .equ statements below should be set to "1" to be valid   **
#  and set to "0" to be inactive.                              **

    .equ FAR_ADDRESS,      0  # Used for a FAR_ADDRESS call
    .equ FAR_ADDRESS_MAIN, 1  # Used for a FAR_ADDRESS call to main
    .equ EXT_BOOT,         0  # Set to (1) for External boot. 
#                             #  BAM sets up external bus and CS[0]
# Reset Configuration Half Word Variables **
    .equ RCHW_WTE, WDOG_DISABLE # Watchdog control at reset
    .equ RCHW_PS0, CS0_32BIT    # CS0 data port size at reset
    .equ BOOT_ID,  MPC5500_ID   # Valid boot ID for MPC5500 devices

#*************************************************************
#******** Special Initialization Option Constants  ***********
# The "I_" prefixed variables are initialization defines

# To match MMU entry size:
    .equ I_XSRAM_SIZE,  SIZE_512K  #  512 KB External RAM Size
    .equ I_XSRAM_SPACE, SIZE_4M    #    4 MB External RAM Space

#*************************************************************
#      User Defined Options
#  These values should be modified based on user requirements
#*************************************************************
# Cache definitions used by cfg_CACHE and cfg_STACK:
#  Copy back mode (CWM=1) and Push buffer disabled (DPB=1) is
#   required by errata #32 and #34 to allow MMU control of cache.
#   These errata may go away in the future (see current errata)
#  4 way cache is recommended to reduce power consumption on the
#   MPC5561 and MPC5566.
    .equ CACHE_CLEAR,(CLFC_NO_OP | CINV_INV_OP | CE_DISABLE)
    .equ CACHE_SETTINGS, (CWM_COPYBCK | DPB_NOPUSH | CORG_4_WAY | CE_ENABLE)

#*******************************************************************
# Flash definitions used by cfg_FLASH:
#     Internal Flash: FLASH_BIUCR (0xC3F8_801C)
	.if _MPC5533_ | _MPC5534_
    .equ FLASH_SETTINGS, (CPU_PREFTCH_ON | APC_2 | WWSC_1 | RWSC_2 | DPFEN_1 | IPFEN_1 | PFLIM_1 | BFEN_EN)
	.else
    .equ FLASH_SETTINGS, (CPU_PREFTCH_ON | APC_2 | WWSC_1 | RWSC_3 | BFEN_EN)
    .endif

#     External Flash: CS0 OR settings used by cfg_FLASH:
#     The next line is commented out as an example of optimizing
#      external Flash boot times.
#    .equ CS0_OR_OPTIONS, (AMASK_8M | OR0SCY_2 | OR0BSCY_0)

#*******************************************************************
# FMPLL definitions used by cfg_FMPLL80
#  Set the internal clock to 32 MHz with PREDIV=1, MFD=6, and RFD=1.
    .equ FMPLL80_SYNCR_SETTING1, (MFD_6 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 128 MHz with PREDIV=1, MFD=10, and RFD=1.
    .equ FMPLL80_SYNCR_SETTING2, (MFD_10 | RFD_1 | LOCEN_EN)

#*******************************************************************
# FMPLL definitions used by cfg_FMPLL128)40 for 40MHz crystal
#  Set the internal clock to 12 MHz with PREDIV=5, MFD=6, and RFD=1.
    .equ FMPLL128_40_SYNCR_SETTING1, (PREDIV_5 | MFD_6 | RFD_4 | LOCEN_EN)
#  Set the internal clock to 48 MHz with PREDIV=5, MFD=6, and RFD=1.
    .equ FMPLL128_40_SYNCR_SETTING2, (PREDIV_5 | MFD_6 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 80 MHz with PREDIV=5, MFD=18, and RFD=1.
    .equ FMPLL128_40_SYNCR_SETTING3, (PREDIV_5 | MFD_10 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 128 MHz with PREDIV=10, MFD=33, and RFD=1.
    .equ FMPLL128_40_SYNCR_SETTING4, (PREDIV_5 | MFD_16 | RFD_1 | LOCEN_EN)

#*******************************************************************
# FMPLL definitions used by cfg_FMPLL132
#  Set the internal clock to 48 MHz with PREDIV=1, MFD=6, and RFD=1.
    .equ FMPLL132_SYNCR_SETTING1, (MFD_6 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 64 MHz with PREDIV=2, MFD=6, and RFD=1.
    .equ FMPLL132_SYNCR_SETTING2, (MFD_8 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 32 MHz with PREDIV=2, MFD=18, and RFD=1.
    .equ FMPLL132_SYNCR_SETTING3, (PREDIV_2 | MFD_8 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 72 MHz with PREDIV=2, MFD=18, and RFD=1.
    .equ FMPLL132_SYNCR_SETTING4, (PREDIV_2 | MFD_18 | RFD_1 | LOCEN_EN)
#  Set the internal clock to 132 MHz with PREDIV=2, MFD=33, and RFD=1.
    .equ FMPLL132_SYNCR_SETTING5, (PREDIV_2 | MFD_33 | RFD_1 | LOCEN_EN)

#*******************************************************************
# FMPLL definitions used by cfg_FMPLL
# Set LOCIRQ and LOLIRQ and clear LOLRE and LOCRE bits in the FMPLL_SYNCR.
	.equ FMPLL_SYNCR_BITSET, (LOCIRQ_REQUESTED | LOLIRQ_REQUESTED)
	.equ FMPLL_SYNCR_BITCLEAR, (~LOCRE_ASSERT & ~LOLRE_ASSERT)
    
#*******************************************************************
# SIU definitions used by cfg_FMPLL
# The SIU definition below will generate a reset of the device when used.
#  A system reset or an external reset will result depending on settings.
    .equ SIU_SRCR_SYSRST, (SSR_SYSRST | SER_NORST | CRE_NO)    

#*******************************************************************
# ERRLOGREG (Error Log Register) address definition
    .equ ERRLOGREG, ERRLOGAD_ETPUPRAM_HI # Assembler token address

#*********************************************************************

##*************************************************************************/
# FUNCTION     : MMU DATA Tables                                          */
# PURPOSE      : This defines the MMU data tables for the TLB entries     */
#                which are set in the file mpc5500_asmcfg.s               */
# INPUT NOTES  : Requires that the TLB settings be in MPC5500_defs.inc    */
# RETURN NOTES : mmu_tlb0 [TLB0_MAS[0:3] through mmu_tlb11 [TLB0_MAS[0:3] */
# WARNING      : Registers used: none. Section is: .rodata                */
##*************************************************************************/


# Section declaration:
    
    .if __PEGNU__
    .section ".rodata"
    .else                # __CWWRKS__ | __DIABCC__ | __GRNHS__
    .section .rodata
    .endif


#*************************************************************************/
#* DESCRIPTION:                                                          */  
#* This table contains definitions for the MPC5554 MMU TLB entries.      */
#* The bit definitions used in the TLB defines are located below.        */  
#* The second half of the file is the TLB setup code in mpc5500_asmcfg.s */
#*************************************************************************/ 

#*** TLB DEFINES ***/

#** TLB entry 15 - BAM in Flash set to 1M **
mmu_tlb15:
# TLB0_MAS0
    .long (TLB_SELECT | TLB_ENTRY15 )
# TLB0_MAS1
    .long (TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_1M )
# TLB0_MAS2
    .long (BAM_BASE_ADDR | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB0_MAS3
    .long ( BAM_BASE_ADDR | READWRITEEXECUTE )


#** TLB entry 0 - PBRIDGE_B set to 1M **
mmu_tlb0:
# TLB1_MAS0
    .long ( TLB_SELECT | TLB_ENTRY0 )
# TLB1_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_1M )
# TLB1_MAS2
    .long ( PBRIDGEB_BASE_ADDR | CACHE_WRITE_BACK | CACHE_INHIBIT | MEM_NO_COHERENCE | PAGE_GUARDED | PAGE_BIG_ENDIAN )
# TLB1_MAS3
    .long ( PBRIDGEB_BASE_ADDR | READWRITEEXECUTE )

#** TLB entry 1 - Split Internal FLASH (1) set to 1MB **
mmu_tlb1:
# TLB5_MAS0
    .long ( TLB_SELECT | TLB_ENTRY1 )
# TLB5_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_1M )
# TLB5_MAS2
    .long ( FLASH_BASE_ADDR | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB5_MAS3
    .long ( FLASH_BASE_ADDR | READWRITEEXECUTE )

#** TLB entry 2 - External Memory 1 (Flash) 4M **
mmu_tlb2:
# TLB2_MAS0
    .long ( TLB_SELECT | TLB_ENTRY2 )
# TLB2_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_4M )
# TLB2_MAS2
    .long ( EXTMEM1_BASE_ADDR | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB2_MAS3
    .long ( EXTMEM1_BASE_ADDR | READWRITEEXECUTE )

#** TLB entry 3 - Internal SRAM reduced from 1M space to 64K or 256K only **
	.if _MPC5561_ | _MPC5566_
	.equ RAM_TLB_SIZE, TSIZ_256K
	.else # _MPC5533_, _MPC5534_, _MPC5553_, _MPC5554_, _MPC5565_, _MPC5567_
	.equ RAM_TLB_SIZE, TSIZ_64K
	.endif
mmu_tlb3:
# TLB3_MAS0
    .long ( TLB_SELECT | TLB_ENTRY3 )
# TLB3_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | RAM_TLB_SIZE )
# TLB3_MAS2
    .long ( SRAM_BASE_ADDR | CACHE_WRITE_BACK | CACHE_INHIBIT | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB3_MAS3
    .long ( SRAM_BASE_ADDR | READWRITEEXECUTE )

#** TLB entry 4 - PBRIDGE_A set to 1MB **
mmu_tlb4:
# TLB4_MAS0
    .long ( TLB_SELECT | TLB_ENTRY4 )
# TLB4_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_1M )
# TLB4_MAS2
    .long ( PBRIDGEA_BASE_ADDR | CACHE_WRITE_BACK | CACHE_INHIBIT | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB4_MAS3
    .long ( PBRIDGEA_BASE_ADDR | READWRITEEXECUTE )

#** TLB entry 5 - Turn OFF TLB5 to eliminate duplication of Internal FLASH (1)
mmu_tlb5:
# TLB4_MAS0
    .long ( TLB_SELECT | TLB_ENTRY5 )
# TLB0_MAS1
    .long (TLB_ENTRY_INVALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_1M )

#** TLB entry 6 - Internal FLASH (2) set to 256KB **
mmu_tlb6:
# TLB6_MAS0
    .long ( TLB_SELECT | TLB_ENTRY6 )
# TLB6_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_256K )
# TLB6_MAS2
    .long ( (FLASH_BASE_ADDR + OFFSET_1M) | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB6_MAS3
    .long ( (FLASH_BASE_ADDR + OFFSET_1M) | READWRITEEXECUTE )

#** TLB entry 7 - Internal FLASH (3) set to 256KB **
mmu_tlb7:
# TLB7_MAS0
    .long ( TLB_SELECT | TLB_ENTRY7 )
# TLB7_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_256K )
# TLB7_MAS2
    .long ( (FLASH_BASE_ADDR + OFFSET_1M + OFFSET_256K) | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB7_MAS3
    .long ( (FLASH_BASE_ADDR + OFFSET_1M + OFFSET_256K) | READWRITEEXECUTE )

#** TLB entry 8 - Internal FLASH (4) set to 256KB **
mmu_tlb8:
# TLB8_MAS0
    .long ( TLB_SELECT | TLB_ENTRY8 )
# TLB8_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_256K )
# TLB8_MAS2
    .long ( (FLASH_BASE_ADDR + OFFSET_1M + OFFSET_512K) | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB8_MAS3
    .long ( (FLASH_BASE_ADDR + OFFSET_1M + OFFSET_512K) | READWRITEEXECUTE )

#** TLB entry 9 - Internal FLASH (5) set to 256KB **
mmu_tlb9:
# TLB9_MAS0
    .long ( TLB_SELECT | TLB_ENTRY9 )
# TLB9_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_256K )
# TLB9_MAS2
    .long ( (FLASH_BASE_ADDR + OFFSET_1M + OFFSET_768K) | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB9_MAS3
    .long ( (FLASH_BASE_ADDR + OFFSET_1M + OFFSET_768K) | READWRITEEXECUTE )

#** TLB entry 10 - Cache used for Stack SRAM **
mmu_tlb10:
# TLB10_MAS0
    .long ( TLB_SELECT | TLB_ENTRY10 )
# TLB10_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_4K )
# TLB10_MAS2
    .long ( CACHE_STACK_BASE_ADDR | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB10_MAS3
    .long ( CACHE_STACK_BASE_ADDR | READWRITEEXECUTE )

#** TLB entry 11 - External SRAM set to overall size of 4MB **
mmu_tlb11:
# TLB11_MAS0
    .long ( TLB_SELECT | TLB_ENTRY11 )
# TLB11_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_4M )
# TLB11_MAS2
    .long ( (SRAM_BASE_ADDR - I_XSRAM_SPACE) | CACHE_WRITE_BACK | CACHE_INHIBIT | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB11_MAS3
    .long ( (SRAM_BASE_ADDR - I_XSRAM_SPACE) | READWRITEEXECUTE )

#** TLB entry 14 - Shadow Block set to overall size of 4K (Actual Shadow Block is 1K) **
mmu_tlb14:
# TLB14_MAS0
    .long ( TLB_SELECT | TLB_ENTRY14 )
# TLB14_MAS1
    .long ( TLB_ENTRY_VALID | ENTRY_PROTECTED | GLOBAL_MATCH | TS_IS_COMPARE | TSIZ_4K )
# TLB14_MAS2
    .long ( SHDW_BLK_ADDR | CACHE_WRITE_BACK | CACHE_ACTIVE | MEM_NO_COHERENCE | PAGE_NOT_GUARDED | PAGE_BIG_ENDIAN )
# TLB14_MAS3
    .long ( SHDW_BLK_ADDR | READWRITE )

#*********************************************************************/

#*************************************************************
# Equates for P&E allowing syntax that uses SPR name
# abreviations rather than register numbers
#*************************************************************

  .if __PEGNU__      
  /*.EQU bucsr,1013
    .EQU csrr0,58
    .EQU csrr1,59
    .EQU ctr,9
    .EQU dac1,316  
    .EQU dac2,317
    .EQU dbcnt,562
    .EQU dbcr0,308
    .EQU dbcr1,309
    .EQU dbcr2,310
    .EQU dbcr3,561
    .EQU dbsr,304
    .EQU dear,61  
    .EQU dec,22 
    .EQU decar,54
    .EQU dsrr0,574
    .EQU dsrr1,575
    .EQU esr,62
    .EQU hid0,1008
    .EQU hid1,1009
    .EQU iac1,312
    .EQU iac2,313
    .EQU iac3,314
    .EQU iac4,315
    .EQU ivor0,400
    .EQU ivor1,401
    .EQU ivor2,402  
    .EQU ivor3,403
    .EQU ivor4,404
    .EQU ivor5,405
    .EQU ivor6,406
    .EQU ivor7,407
    .EQU ivor8,408
    .EQU ivor9,409
    .EQU ivor10,410
    .EQU ivor11,411
    .EQU ivor12,412
    .EQU ivor13,413
    .EQU ivor14,414
    .EQU ivor15,415
    .EQU ivor32,528
    .EQU ivor33,529
    .EQU ivor34,530
    .EQU ivpr,63
    .EQU lr,8 */
    .EQU l1cfg0,515
    .EQU l1csr0,1010
  /*.EQU l1finv0,1016 */
    .EQU mas0,624
    .EQU mas1,625
    .EQU mas2,626
    .EQU mas3,627
  /*.EQU mas4,628
    .EQU mas6,630
    .EQU mcsr,572
    .EQU mmucfg,1015
    .EQU mmucsr0,1012 */
    .EQU pid0,48
  /*.EQU pir,286
    .EQU pvr,287
    .EQU spefscr,512
    .EQU sprg0,272
    .EQU sprg1,273
    .EQU sprg2,274
    .EQU sprg3,275
    .EQU sprg4,276
    .EQU sprg5,277
    .EQU sprg6,278
    .EQU sprg7,279
    .EQU srr0,26
    .EQU srr1,27
    .EQU svr,1023
    .EQU sprg4.r,260
    .EQU sprg5.r,261
    .EQU sprg6.r,262
    .EQU sprg7.r,263
    .EQU usprg0,256
    .EQU tbl,268 
    .EQU tbu,269
    .EQU tcr,340
    .EQU tlb0cfg,688
    .EQU tlb1cfg,689
    .EQU tsr,336
    .EQU xer,1
    .EQU tbl.s,284
    .EQU tbu.s,285 */    
  .endif
