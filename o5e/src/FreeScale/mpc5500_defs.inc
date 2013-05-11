#*************************************************************************
#* FILE NAME: mpc5500_defs.inc              COPYRIGHT (c) Freescale 2004 
#*                                                All Rights Reserved     
#* DESCRIPTION:                                                           
#* This file contains prototypes and definitions for the MPC5500 
#*  assembly functions.
#*
#*  Users should make no changes in this file. 
#*  User defines are in mpc5500_usrdefs.inc
#*========================================================================
#* ORIGINAL AUTHOR: G. Jackson           
#* REV      AUTHOR        DATE       DESCRIPTION OF CHANGE                
#* ---   -----------   -----------   ---------------------                  
#* 0.1   G. Jackson     26/Mar/04    Set up assembly definitions     
#* 0.2   G. Jackson     15/Apr/04    Add SRAM address definitions    
#* 0.3   G. Jackson     13/May/04    Removed 20MSB address definitions
#* 1.0   G. Jackson     25/May/04    Changed OPCNT_OFFSET to IP_ADVANCE
#* 1.1   R. Dees        25/May/04    Added more comments
#* 1.2   G. Jackson     30/Jun/04    Added RCHW constants
#* 1.3   C. Baker       19/Jul/06    Added equate for an MFD of 33
#*                                   Changed DPFEN equates from _NO, _BRST
#*                                     and _ANY to _0, _1 and _3
#*                                   Added .if around equate for DPFEN_3
#*                                   Changed IPFEN equates from _NO, _BRST
#*                                     and _ANY to _0, _1 and _3
#*                                   Added .if around equate for IPFEN_3
#*                                   Changed PFLIM equate from _U to _6
#*                                   Added .if around equate for PFLIM_6
#* 1.4   C. Baker       25/Oct/06    Added L1CSR0[CORG] equates
#*                                   Added device specific INT_SRAM_SIZE equates
#*                                   Added CACHE_STACK_BASE_ADDR equate
#*                                   Fixed comments
#*************************************************************************

#*************************************************************
#*************************************************************
# User should not modify any of the definitions below

#************************************************************************
#                            Definitions                                 
#************************************************************************

# Base addresses
    .equ FLASH_BASE_ADDR,       0x00000000
    .equ SRAM_BASE_ADDR,        0x40000000				# INT_SRAM_BASE

	.if _MPC5533_
	.equ INT_SRAM_SIZE,         0x0000C000				# MPC5533 = 48K
	.else
	.if _MPC5561_
	.equ INT_SRAM_SIZE,         0x00030000				# MPC5561 = 192K
	.else
	.if _MPC5566_
	.equ INT_SRAM_SIZE,         0x00020000				# MPC5566 = 128K
	.else
	.equ INT_SRAM_SIZE,         0x00010000				# MPC5534, MPC5553, MPC5554, MPC5565, MPC5567 = 64K
	.endif
	.endif
	.endif

    .equ CACHE_STACK_BASE_ADDR, 0x40040000				# 4k cache used as stack if available
    .equ INT_SRAM_128BYTSEGS,   (INT_SRAM_SIZE >> 7)	# Number of 128 byte segments
    .equ SHDW_BLK_ADDR,         0x00FFF000				# Shadow Block starts at 0xFF_FC00 for 1K size
    .equ BAM_BASE_ADDR,         0x11300000
    .equ EXTMEM1_BASE_ADDR,     0x20000000
    .equ EXTMEM2_BASE_ADDR,     0x3FF80000

    .equ PBRIDGEA_BASE_ADDR,    0xC3F00000
    .equ EXTBUSINT_BASE_ADDR,   0xC3F84000
    .equ SIU_BASE_ADDR,         0xC3F90000
    .equ ETPU_BASE_ADDR,        0xC3FC0000

    .equ PBRIDGEB_BASE_ADDR,    0xFFF00000
    .equ XBAR_BASE_ADDR,        0xFFF04000
    .equ ECSM_BASE_ADDR,        0xFFF40000
    .equ EDMA_BASE_ADDR,        0xFFF44000
    .equ INTC_BASE_ADDR,        0xFFF48000
    .equ EQADC_BASE_ADDR,       0xFFF80000

# Size for defined address spaces
    .equ SIZE_4M,    0x00400000  #   4 MB
    .equ SIZE_1M,    0x00100000  #   1 MB
    .equ SIZE_768K,  0x000C0000  # 768 KB 
    .equ SIZE_512K,  0x00080000  # 512 KB 
    .equ SIZE_256K,  0x00040000  # 256 KB 
    .equ SIZE_64K,   0x00010000  #  64 KB 
    .equ SIZE_4K,    0x00001000  #   4 KB

#*************** Reset Configuration Half Word constants *********
    .equ WDOG_DISABLE, 0x00000000  # Watchdog is default disabled
    .equ WDOG_ENABLE,  0x04000000  # Watchdog is write once enabled
    .equ CS0_32BIT,    0x00000000  # CS0 external data bus is 32-bits
    .equ CS0_16BIT,    0x02000000  # CS0 external data bus is 16-bits
    .equ MPC5500_ID,   0x005A0000  # RCHW boot ID for MPC5500 devices

#*************** Cache initialization constants **************
# Definitions for the L1CSR0 (SPR1010) (L1 Cache control and Status Register 0)
# bit reference: L1CSR0((CWM<<20)|(DPB<<19)|(CLFC<<8)|(CINV<<1)|(CE)) 

# Fields used for L1CSR0 (L1 Cache Control and Status Register 0)
# WAM = Write Access mode = bit [10]
    .equ WAM_ACC_CHECK,    0x0000000  # 0x0
    .equ WAM_ACC_NO_CHECK, 0x0020000  # 0x1
# CWM = Cache Write Mode = bit [11]
    .equ CWM_WRITE,        0x00000000  # 0x0
    .equ CWM_COPYBCK,      0x00100000  # 0x1
# DPB = Disable Push Buffer = bit [12]
    .equ DPB_PUSH,         0x00000000  # 0x0
    .equ DPB_NOPUSH,       0x00080000  # 0x1
# CLFC = Cache Lock Bits Flash Clear = bit [23]
    .equ CLFC_NO_OP,       0x00000000  # 0x0
    .equ CLFC_CLR_OP,      0x00000100  # 0x1
# CORG = Cache Organisation = bit [27]
    .equ CORG_8_WAY,       0x00000000  # 0x0
    .equ CORG_4_WAY,       0x00000010  # 0x1
# CINV = Cache Invalidate = bit [30]
    .equ CINV_NO_OP,       0x00000000  # 0x0
    .equ CINV_INV_OP,      0x00000002  # 0x1
# CE = Cache Enable = bit [31]
    .equ CE_DISABLE,       0x00000000  # 0x0
    .equ CE_ENABLE,        0x00000001  # 0x1


#*************** FMPLL initialization constants **************
# Define the address for the FMPLL registers 
    .equ FMPLL_SYNCRREG, 0xC3F80000
    .equ FMPLL_SYNSRREG, 0xC3F80004
    .equ SIU_ECCRREG,    0xC3F90984
    .equ SIU_SRCRREG,    0xC3F90010

#*************************************************************
# Definitions for FMPLL_SYNCR (FMPLL Synthesizer Control Register) 
# bit reference: SYNCR((MFD<<23)|(RFD<<19)|(locen<<18)|
#                 (lolre<<17)|(locre<<16)|(disclk<<15)|(lolirq<<14)|
#                 (locirq<<13)|(rate<<12)|(depth<<10)|(exp))

# Fields used for PREDIV (Pre-Divider, bits [1:3])
    .equ PREDIV_1,  0x00000000  # 0x0
    .equ PREDIV_2,  0x10000000  # 0x1
    .equ PREDIV_3,  0x20000000  # 0x2
    .equ PREDIV_4,  0x30000000  # 0x3
    .equ PREDIV_5,  0x40000000  # 0x4

# Fields used for MFD (Muliplication Factor Divider, bits [4:8]) 
    .equ MFD_4,  0x00000000  # 0x00
    .equ MFD_6,  0x01000000  # 0x02
    .equ MFD_8,  0x02000000  # 0x04
    .equ MFD_10, 0x03000000  # 0x06
    .equ MFD_12, 0x04000000  # 0x08
    .equ MFD_14, 0x05000000  # 0x0A
    .equ MFD_16, 0x06000000  # 0x0C
    .equ MFD_18, 0x07000000  # 0x0E
    .equ MFD_33, 0x0E800000  # 0x1D

# Fields used for RFD (Reduced Frequency Divider, bits [10:12]) 
    .equ RFD_1,   0x00000000  # 0x0
    .equ RFD_2,   0x00080000  # 0x1
    .equ RFD_4,   0x00100000  # 0x2
    .equ RFD_8,   0x00180000  # 0x3
    .equ RFD_16,  0x00200000  # 0x4
    .equ RFD_32,  0x00280000  # 0x5
    .equ RFD_64,  0x00300000  # 0x6
    .equ RFD_128, 0x00380000  # 0x7

# Fields for LOCEN (Loss-of-clock enable, bit [13]) 
    .equ LOCEN_DIS, 0x00000000  # 0x0
    .equ LOCEN_EN,  0x00040000  # 0x1

# Fields for LOLRE (Loss-of-lock reset enable, bit [14]) 
    .equ LOLRE_IGNORE,   0x00000000  # 0x0
    .equ LOLRE_ASSERT,   0x00020000  # 0x1

# Fields for LOCRE (Loss-of-clock reset enable, bit [15]) 
    .equ LOCRE_IGNORE,   0x00000000  # 0x0
    .equ LOCRE_ASSERT,   0x00010000  # 0x1

# Fields for DISCLK (Disable CLKOUT, bit [16]) 
    .equ DISCLK_NORMAL, 0x00000000  # 0x0
    .equ DISCLK_LOW,    0x00008000  # 0x1

# Fields for LOLIRQ (Loss-of-lock interrupt request, bit [17]) 
    .equ LOLIRQ_NOT_REQUESTED, 0x00000000  # 0x0
    .equ LOLIRQ_REQUESTED,     0x00004000  # 0x1

# Fields for LOCIRQ (Loss-of-clock interrupt request, bit [18]) 
    .equ LOCIRQ_NOT_REQUESTED, 0x00000000  # 0x0
    .equ LOCIRQ_REQUESTED,     0x00002000  # 0x1

# Fields for RATE (Modulation rate, bit [19]) 
    .equ RATE_FREF_80, 0x00000000  # 0x0
    .equ RATE_FREF_40, 0x00001000  # 0x1

# Fields for DEPTH (Modulation depth percentage, bits [20:21])
    .equ DEPTH_0, 0x00000000  # 0x0
    .equ DEPTH_1, 0x00000400  # 0x1
    .equ DEPTH_2, 0x00000800  # 0x2

# Fields for EXP (Expected difference, bits [22:31])
    .equ EXP_0, 0x00000000  # 0x0

#*************************************************************
# Definitions for the FMPLL_SYNSR (Synthesizer Status Register) 
# bit reference: SYNSR ((lolf<<9)|(locf<<2))

# Fields for LOLF (Loss-of-lock flag, bit [22]) 
    .equ LOLF_NO_CHANGE, 0x00000000  # 0x0
    .equ LOLF_CLEAR,     0x00000200  # 0x1

# Fields for LOCF (Loss-of-clock flag, bit [29]) 
    .equ LOCF_NO_CHANGE, 0x00000000  # 0x0
    .equ LOCF_CLEAR,     0x00000004  # 0x1

#*************************************************************
# Definitions for the SIU_ECCR (External Clock Control Register) 
# bit reference: ECCR((ENGDIV<<8)|(EBTS<<3)|(EBDF)) 
    .equ ECCRREG, 0xC3F90984  # ECCR register address

# Fields for ENGDIV (Engineering clock values, bits [18:23]) 
    .equ ENGDIV_BY128, 0x00003F00  # 0x3F

# Fields for EBTS (external signals hold time, bit [28]) 
    .equ EBTS_NO_HOLD, 0x00000000  # 0x0
    .equ EBTS_HOLD,    0x00000004  # 0x1

# Fields for EBDF (CLKOUT divides, bits [30:31]) 
    .equ EBDF_DIVBY2, 0x00000001  # 0x1
    .equ EBDF_DIVBY4, 0x00000002  # 0x2

#*************************************************************
# Definitions for the SIU_SRCR (System Reset Control Register) 
# bit reference: SIU_SRCR ((SSR<<31)|(SER<<30)|(CRE<<15))

# Fields for SSR (software system reset, bit [0]) 
    .equ SSR_NORST,  0x00000000  # 0x0
    .equ SSR_SYSRST, 0x80000000  # 0x1

# Fields for SER (external system reset, bit [1]) 
    .equ SER_NORST, 0x00000000  # 0x0
    .equ SER_XRST,  0x40000000  # 0x1

# Fields for CRE (checkstop reset enable, bit [16]) 
    .equ CRE_NO, 0x00000000  # 0x0
    .equ CRE_EN, 0x00008000  # 0x1


#*************** Flash initialization constants ****************

# Definitions for FLASH_BIUCR (Flash BIU Control Register) 
# bit reference: FLSHBIUCR ((M3PFE<<19)|(M2PFE<<18)|
#               (M1PFE<<17)|(M0PFE<<16)|(APC<<13)|(WWSC<<11)|
#               (RWSC<<8)|(DPFEN<<6)|(IPFEN<<4)|(PFLIM<<1)|(BFEN))

# Fields for Flash Bus Interface Control 
# Fields for Prefetch Control (MnPFE Master n Prefetch Enable)
# Fields for M3PFE (Master 3 (EBI) prefetch enable, bit [12])
    .equ EBI_PREFTCH_OFF,  0x00000000  # 0x0
    .equ EBI_PREFTCH_ON,   0x00080000  # 0x1
# Fields for M2PFE (Master 2 (eDMA) prefetch enable, bit [13])
    .equ EDMA_PREFTCH_OFF, 0x00000000  # 0x0
    .equ EDMA_PREFTCH_ON,  0x00040000  # 0x1
# Fields for M1PFE (Master 1 (Nexus) prefetch enable, bit [14])
    .equ NEX_PREFTCH_OFF,  0x00000000  # 0x0
    .equ NEX_PREFTCH_ON,   0x00020000  # 0x1
# Fields for M0PFE (Master 0 (e200z core) prefetch enable, bit [15])
    .equ CPU_PREFTCH_OFF,  0x00000000  # 0x0
    .equ CPU_PREFTCH_ON,   0x00010000  # 0x1

# Fields for APC (access pipelining control, bits [16:18]) 
    .equ APC_1,  0x00002000  # 0x1
    .equ APC_2,  0x00004000  # 0x2
    .equ APC_3,  0x00006000  # 0x3
    .equ APC_4,  0x00008000  # 0x4
    .equ APC_5,  0x0000A000  # 0x5
    .equ APC_6,  0x0000C000  # 0x6
    .equ APC_NO, 0x0000E000  # 0x7

# Fields for WWSC (write wait state control, bits [19:20]) 
    .equ WWSC_1, 0x00000800  # 0x1
    .equ WWSC_2, 0x00001000  # 0x2
    .equ WWSC_3, 0x00001800  # 0x3

# Fields for RWSC (read wait state control, bits [21:23]) 
    .equ RWSC_0, 0x00000000  # 0x0
    .equ RWSC_1, 0x00000100  # 0x1
    .equ RWSC_2, 0x00000200  # 0x2
    .equ RWSC_3, 0x00000300  # 0x3
    .equ RWSC_4, 0x00000400  # 0x4
    .equ RWSC_5, 0x00000500  # 0x5
    .equ RWSC_6, 0x00000600  # 0x6
    .equ RWSC_7, 0x00000700  # 0x7

# Fields for DPFEN (data prefetch enable, bits [24:25]) 
    .equ DPFEN_0,   0x00000000  # 0x0
    .equ DPFEN_1, 0x00000040  # 0x1

    .if _MPC5533_ | _MPC5553_ | _MPC5554_
    .equ DPFEN_3,  0x000000C0  # 0x3
    .endif

# Fields for IPFEN (instruction prefetch enable, bits [26:27]) 
    .equ IPFEN_0,   0x00000000  # 0x0
    .equ IPFEN_1, 0x00000010  # 0x1

    .if _MPC5533_ | _MPC5553_ | _MPC5554_
    .equ IPFEN_3,  0x00000030  # 0x3
    .endif

# Fields for PFLIM (additional line prefetch (limit), bits [28:30])
    .equ PFLIM_0,  0x00000000  # 0x0 
    .equ PFLIM_1,  0x00000002  # 0x1
    .equ PFLIM_2,  0x00000004  # 0x2
    .equ PFLIM_3,  0x00000006  # 0x3

    .if _MPC5533_ | _MPC5553_ | _MPC5554_
    .equ PFLIM_4,  0x00000008  # 0x4
    .equ PFLIM_5,  0x0000000A  # 0x5
    .equ PFLIM_6,  0x0000000C  # 0x6
    .endif

# Fields for BFEN (enable line read buffer hits, bit [31]) 
    .equ BFEN_DIS, 0x00000000  # 0x0
    .equ BFEN_EN,  0x00000001  # 0x1

# Define the address for the Flash BIUCR register
    .equ FLASH_BIUCRREG, 0xC3F8801C 

# set up for errata 105 flash access by all masters.
    .equ FLASHACCESS,  0xFFFF
    .equ FLASH_BIUAPR, 0xC3F88020

#*************************************************************
# Definitions for CS0OR0 (Chip Select 0 option register) 
# bit reference: CS0OR0AVAL ((AM<<12)|(SCY<<4)|(BSCY<<1))

# Fields for CS0OR0
# Fields for AM (Address Mask, bits [0:16] ([0:2]=b111))
    .equ AMASK_8M,  0xFF800000 # 0xFF800; 8M space  
# Fields for SCY (Primary wait states, bits [24:27])
    .equ OR0SCY_1,  0x00000010 # 0x1    
    .equ OR0SCY_2,  0x00000020 # 0x2    
    .equ OR0SCY_3,  0x00000030 # 0x3    
    .equ OR0SCY_4,  0x00000040 # 0x4  
    .equ OR0SCY_5,  0x00000050 # 0x5
# Fields for BSCY (Secondary wait states, bits [29:30])
    .equ OR0BSCY_0, 0x00000000 # 0x0
    .equ OR0BSCY_1, 0x00000002 # 0x1
    .equ OR0BSCY_2, 0x00000004 # 0x2
    .equ OR0BSCY_3, 0x00000006 # 0x3

# Define the addresses for the Chip Select 0 registers
    .equ CS0BRREG,   0xC3F84010
    .equ CS0ORREG,   0xC3F84014

#*************************************************************
# Define parameter variables for SRAM download control
# Starting SRAM address defined in the linker file 
#     __SRAM_LOAD set to ADDR(.heap) defined in the linker file 
# Number of opcodes to be loaded defined in the linker file
#     __SRAM_LOAD_SIZE set to SIZEOF(.flash_data) defined in the linker file
# Instruction pointer advance to next loaded opcode loaded into SRAM
    .equ IP_ADVANCE,  4   # Offset to next opcode loaded into SRAM

# Copy of ROM data into ".data" and ".sdata" sections of SRAM
#   __DATA_ROM set to ROM address for loading into ".data" of SRAM
#   __SDATA_ROM set to ROM address for loading into ".sdata" of SRAM
#   __ROM_COPY_SIZE number of bytes to be loaded into SRAM from ROM
    .equ CPY_OFFSET,  1   # Byte offset to next address and decrement 
#                         #  the ROM_COPY_SIZE

#*************** MMU initialization constants ****************

#********* TLB address offsets *********

# Offset addresses for use in defining beginning of flash blocks
    .equ OFFSET_1M,    0x00100000  #   1 MB 
    .equ OFFSET_768K,  0x000C0000  # 768 KB 
    .equ OFFSET_512K,  0x00080000  # 512 KB 
    .equ OFFSET_256K,  0x00040000  # 256 KB 
    .equ OFFSET_64K,   0x00010000  #  64 KB 
    .equ OFFSET_4K,    0x00001000  #   4 KB 

# Definitions for MMU (Memory Management Unit Registers) 

#*************************************************************
# Definitions for MAS0 (MMU Assist Register 0) 
# bit reference: MAS0AVAL ((tlbsel<<28)|(eselcam<<16))

# Fields for MAS0 (MMU Assist Register 0 ) 
# Field for TLBSEL  (TLB Select, bits [2:3]) 
    .equ TLB_SELECT,  0x10000000   # Always 0x01 for MPC5554
# Fields for ESELCAM (Entry Select for TLB CAM, bits [11:15])
    .equ TLB_ENTRY0,  0x00000000   # Select TLB Entry slot 0
    .equ TLB_ENTRY1,  0x00010000   # Select TLB Entry slot 1
    .equ TLB_ENTRY2,  0x00020000   # Select TLB Entry slot 2
    .equ TLB_ENTRY3,  0x00030000   # Select TLB Entry slot 3  
    .equ TLB_ENTRY4,  0x00040000   # Select TLB Entry slot 4  
    .equ TLB_ENTRY5,  0x00050000   # Select TLB Entry slot 5  
    .equ TLB_ENTRY6,  0x00060000   # Select TLB Entry slot 6  
    .equ TLB_ENTRY7,  0x00070000   # Select TLB Entry slot 7  
    .equ TLB_ENTRY8,  0x00080000   # Select TLB Entry slot 8  
    .equ TLB_ENTRY9,  0x00090000   # Select TLB Entry slot 9  
    .equ TLB_ENTRY10, 0x000A0000   # Select TLB Entry slot 10 
    .equ TLB_ENTRY11, 0x000B0000   # Select TLB Entry slot 11 
    .equ TLB_ENTRY12, 0x000C0000   # Select TLB Entry slot 12 
    .equ TLB_ENTRY13, 0x000D0000   # Select TLB Entry slot 13 
    .equ TLB_ENTRY14, 0x000E0000   # Select TLB Entry slot 14 
    .equ TLB_ENTRY15, 0x000F0000   # Select TLB Entry slot 15 
    .equ TLB_ENTRY16, 0x00100000   # Select TLB Entry slot 16 
    .equ TLB_ENTRY17, 0x00110000   # Select TLB Entry slot 17 
    .equ TLB_ENTRY18, 0x00120000   # Select TLB Entry slot 18 
    .equ TLB_ENTRY19, 0x00130000   # Select TLB Entry slot 19 
    .equ TLB_ENTRY20, 0x00140000   # Select TLB Entry slot 20 
    .equ TLB_ENTRY21, 0x00150000   # Select TLB Entry slot 21 
    .equ TLB_ENTRY22, 0x00160000   # Select TLB Entry slot 22 
    .equ TLB_ENTRY23, 0x00170000   # Select TLB Entry slot 23 

#*************************************************************
# Definitions for MAS1 (MMU Assist Register 1) 
# bit reference: MAS1AVAL ((valid<<31)| (iprot<<30)|
#                      (tid<<16)|(ts<<12)|(tsize<<8))

# Fields for MAS1 (MMU Assist Register 1 ) 
# Field for V (Valid, bit [0])
    .equ TLB_ENTRY_INVALID,   0x00000000  # 0x0   
    .equ TLB_ENTRY_VALID,     0x80000000  # 0x1   
# Field for IPROT (Invalidation Protection, bit [1]) 
    .equ ENTRY_NOT_PROTECTED, 0x00000000  # 0x0 # From Invalidation  
    .equ ENTRY_PROTECTED,     0x40000000  # 0x1 # From Invalidation  
# Field for TID (Process ID, bits [8:15])
    .equ GLOBAL_MATCH,        0x00000000  # 0x0 # Match all process IDs  
# Field for TS (Translation address space, bit [19])
    .equ TS_IS_COMPARE,       0x00000000  # 0x0 # Match entry to instruction space  
    .equ TS_DS_COMPARE,       0x00001000  # 0x1 # Match entry to data space 
# Field for TSIZE (TLB entry page size, bits [20:23])
    .equ TSIZ_4K,    0x00000100  # 0x1 # TLB page size =  4K bytes 
    .equ TSIZ_16K,   0x00000200  # 0x2 # TLB page size = 16K bytes 
    .equ TSIZ_64K,   0x00000300  # 0x3 # TLB page size = 64K bytes 
    .equ TSIZ_256K,  0x00000400  # 0x4 # TLB page size =256K bytes 
    .equ TSIZ_1M,    0x00000500  # 0x5 # TLB page size =  1M bytes 
    .equ TSIZ_4M,    0x00000600  # 0x6 # TLB page size =  4M bytes 
    .equ TSIZ_16M,   0x00000700  # 0x7 # TLB page size = 16M bytes 
    .equ TSIZ_64M,   0x00000800  # 0x8 # TLB page size = 64M bytes 
    .equ TSIZ_256M,  0x00000900  # 0x9 # TLB page size =256M bytes 

#*************************************************************
# Definitions for MAS2 (MMU Assist Register 2) 
# bit reference: MAS2AVAL((epn<<12)|(w<<4)|(i << 3)|
#                         (m << 2)|(g << 1)|(e)) 

# Fields for MAS2 (MMU Assist Register 2 )
# The EPN field is defined above with global address space
# EPN (effective page number, bits [0:19])
# Field for W (Write-through required, bit [27])
    .equ CACHE_WRITE_BACK,  0x00000000  # 0x0 # Stores write-back to cache
    .equ CACHE_WRITE_THRU,  0x00000010  # 0x1 # Stores write through to memory
    .equ CACHE_ACTIVE,      0x00000000  # 0x0 # Cache is active for TLB entry
# Field for I (Cache Inhibited, bit [28])
    .equ CACHE_INHIBIT,     0x00000008  # 0x1 # Cache is inhibited for TLB entry
# Field for M (Memory Coherence required, bit [29])
    .equ MEM_NO_COHERENCE,  0x00000000  # 0x0 # Only valid setting for MPC5554
    .equ MEM_COHERENCE,     0x00000004  # 0x1 # Not valid--ignored on MPC5554
# Field for G (Page Guarded, bit [30])
    .equ PAGE_NOT_GUARDED,  0x00000000  # 0x0 # Cache page not guarded
    .equ PAGE_GUARDED,      0x00000002  # 0x1 # Cache page guarded
# Field for E (Endianess, bit [31])
    .equ PAGE_BIG_ENDIAN,   0x00000000  # 0x0 # Big endian byte order
    .equ PAGE_LTL_ENDIAN,   0x00000001  # 0x1 # True little endian byte order

#*************************************************************
# Definitions for MAS3 (MMU Assist Register 3) 
# bit reference: MAS3AVAL ((rpn<<12)|(permissions))

# Fields for MAS3 (MMU Assist Register 3 ) 
# The RPN field is defined above with global address space 
# RPN == real page number 

# Field for U0-U3 (USR bits, bits [22:25])
    .equ MAS3_USR0,      0x00000000  # 0x0  # User bit value =0000 
# Field for UX (User Execute Access, bit [26])   
    .equ USR_NO_EXECUTE, 0x00000000  # 0x0  # User cannot execute code 
    .equ USR_EXECUTE,    0x00000020  # 0x1  # User executable permission 
# Field for SX (Supervisor Execute Access, bit [27])
    .equ SUP_NO_EXECUTE, 0x00000000  # 0x0  # Supervisor cannot execute code 
    .equ SUP_EXECUTE,    0x00000010  # 0x1  # Supervisor executable permission
# Field for UW (User Write Access, bit [28]) 
    .equ USR_NO_WRITE,   0x00000000  # 0x0  # User cannot write code 
    .equ USR_WRITE,      0x00000008  # 0x1  # User write permission 
# Field for SW (Supervisor Write Access, bit [29])
    .equ SUP_NO_WRITE,   0x00000000  # 0x0  # Supervisor cannot write code 
    .equ SUP_WRITE,      0x00000004  # 0x1  # Supervisor write permission 
# Field for UR (User Read Access, bit [30])
    .equ USR_NO_READ,    0x00000000  # 0x0  # User cannot read code 
    .equ USR_READ,       0x00000002  # 0x1  # User read permission 
# Field for SR (Supervisor Read Access, bit [31])
    .equ SUP_NO_READ,    0x00000000  # 0x0  # Supervisor cannot read code 
    .equ SUP_READ,       0x00000001  # 0x1  # Supervisor read permission 

# Field to define special cases - all access and read+execute, bits [26:31]
    .equ READWRITE,        0x0000000F  # 0x0f read, write permission only
    .equ READWRITEEXECUTE, 0x0000003F  # 0x3f read, write and execute permission       
    .equ READEXECUTE,      0x00000033  # 0x33 read and execute permission only        
# bits [0:19] = RPN: Real page number. Same format as EPN. Use RPN        
#                   identical to EPN if identical mapping of     
#                   effective to physical address.                                         
# bits [26:31] = (UX,SX,UW,SW,UR,SR):                                      
#             Permission bits. User and supervisor read,                 
#             write, and execute permission bits. See                    
#             Table 10-3 [1] for more information on                     
#             the page permission bits as they are                       
#              defined by Book E.                                        
#                                                                        

#**************************************************************************
#           Set up  ERRLOG save error log register                         
#**************************************************************************

# Definitions for ERRLOG (Error Log Register) 

# Fields for Error Log Register 
    .equ MFDERR_N,     0x00000000  # 0x0
    .equ MFDERR_Y,     0x80000000  # 0x1
    .equ LOCINTERR_N,  0x00000000  # 0x0
    .equ LOCINTERR_Y,  0x40000000  # 0x1
    .equ LOLINTERR_N,  0x00000000  # 0x0
    .equ LOLINTERR_Y,  0x20000000  # 0x1
    .equ PLLNORMERR_N, 0x00000000  # 0x0
    .equ PLLNORMERR_Y, 0x10000000  # 0x1

# Machine check errors    
    .equ ERRLOGCPPER,  0x02000000  # Error log CP_PERR error input
    .equ ERRLOGCPER,   0x01000000  # Error log CPERR error input
    .equ ERRLOGXCPT,   0x00800000  # Error log exception error input
    .equ ERRLOGWRT,    0x00400000  # Error log write error input

# Error Log definitions used by cfg_FMPLL and cfg_MCHK_hndlr
    .equ ERRLOG_MFDERR, (MFDERR_Y)         # Error on change of FMPLL[MFD]
    .equ ERRLOG_LOCINTERR, (LOCINTERR_Y)   # FMPLL fails to lock after LOC interrupt enable
    .equ ERRLOG_LOLINTERR, (LOLINTERR_Y)   # FMPLL fails to lock after LOL interrupt enable
    .equ ERRLOG_PLLNORMERR, (PLLNORMERR_Y) # FMPLL not in normal mode (loss of clock)
    
    .equ ERRLOG_PUSHERR, (ERRLOGCPPER)     # Machine check cache push parity error
    .equ ERRLOG_PARERR, (ERRLOGCPER)       # Machine check cache parity error
    .equ ERRLOG_XERR, (ERRLOGXCPT)         # Machine check from exception error
    .equ ERRLOG_WRTERR, (ERRLOGWRT)        # Machine check from write error

# Error Log address locations
    .equ ERRLOGAD_DMATCD_00,   0xFFF45000  # DMA TCD00
    .equ ERRLOGAD_DMATCD_63,   0xFFF457E0  # DMA TCD63
    .equ ERRLOGAD_ETPUPRAM_LO, 0xC3FC8000  # Start of eTPU Parameter RAM
    .equ ERRLOGAD_ETPUPRAM_HI, 0xC3FC8B00  # Locate at end of eTPU Parameter RAM

#**************************************************************

