/**************************************************************************/
/* FILE NAME: mpc5500_ccdcfg.h               COPYRIGHT (c) Freescale 2004 */
/*                                                All Rights Reserved     */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for the MPC5500          */
/*     C functions.                                                       */
/*     Additions to this file should be made only if additional defines   */
/*     are needed.                                                        */
/*     New definitions will need to be unique for this file and for the   */
/*     assembly equates in mpc5500_defs.inc.                              */
/*========================================================================*/
/*                                                                        */
/* REV      AUTHOR        DATE       DESCRIPTION OF CHANGE                */
/* ---   -----------   -----------   ---------------------                */
/* 0.1   J. Yokubaitis   8/Oct/03    Initial version.                     */
/* 0.2   G. Jackson     13/Nov/03    Converted for reconfiguration file.  */
/* 0.3   G. Jackson     02/Dec/03    Set up global Pbridge addresses in   */
/*                                     MMU to cover all addresses.        */
/* 0.4   G. Jackson     15/Apr/04    Removed references to application    */
/*                                     code.                              */
/* 0.5   G. Jackson     29/Apr/04    Removed user definitions to          */
/*                                     mpc5500_usrccdcfg.h                */
/* 0.6   G. Jackson     13/May/04    Added EBI ORn address mask entries   */
/* 0.7   G. Jackson     05/Jun/04    Added fields and prototypes for the  */
/*                                     PCRs                               */
/* 1.0   G. Jackson     30/Jun/04    Added fields for external bus enable */
/* 1.1   C. Baker       19/Jul/06    Removed init_LED2 and invert_LED2    */
/*                                     function declarations              */
/* 1.2   C. Baker       25/Oct/06    Added support for MPC5533, MPC5534,  */
/*                                     MPC5561, MPC5565, MPC5566, MPC5567 */
/*                                     to XBAR defines.                   */
/*                                   Split PBRG_XBAR_CHNG defines into    */
/*                                   separate PBRG and XBAR defines.      */
/**************************************************************************/


#include "typedefs.h"

/*************************************************************/
/*************************************************************/
/* The user should not modify any of the definitions below   */


/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/

/**************** External Bus Configuration Control ************/
/* A "YES" will set up the external bus  */
#define EXTERNAL_BUS_EN_NO  0
#define EXTERNAL_BUS_EN_YES 1

/*  Define Data Bus Port Size */
#define DATA_PORT_16 16
#define DATA_PORT_32 32

/**************** XCLKS initialization constants *****************/
/* Fields for changing the XCLKS */
#define XCLK_CHNG_NO  0x0
#define XCLK_CHNG_YES 0x1

/******* PBRIDGE initialization constants ***********/
/* Fields for changing the PBRIDGE */
#define PBRG_CHNG_NO  0x0
#define PBRG_CHNG_YES 0x1

/******* XBAR initialization constants ***********/
/* Fields for changing the XBAR */
#define XBAR_CHNG_NO  0x0
#define XBAR_CHNG_YES 0x1

/* Fields for Engineering clock division (ENGDIV[0:5]) factor */
#define ENGDIV_2   0x00000100
#define ENGDIV_4   0x00000200
#define ENGDIV_8   0x00000400
#define ENGDIV_10  0x00000500
#define ENGDIV_16  0x00000800
#define ENGDIV_32  0x00001000
#define ENGDIV_64  0x00002000
#define ENGDIV_128 0x00003F00

/* Fields for External Bust Tap Select (EBTS) signals hold time */
#define EBTS_HOLD_NO 0x00000000  /* 0x0; Zero hold     */
#define EBTS_HOLD_YS 0x00000004  /* 0x1; non-zero hold */


/* Fields for External bus division (EBDF[0:1]) factor */
#define EBDF_DIV_2 0x00000001
#define EBDF_DIV_4 0x00000003

/**************** SIU initialization constants *****************/

/* Definitions for SIU_PCR (Pad Configuration Register) */
/* The PCRs are 16-bit registers */
/* Constants for the Data Byte Field Designation */
#define DATA_BYT_MSB  0        /* Data[0:15] active  */
#define DATA_BYT_LSB 16        /* Data[16:31] active */
/* Fields for Pin Assignment (PA) */
#define PA_GPIO  0x0000    /* 0b000 */
#define PA_PRIM  0x0400    /* 0b001 */
#define PA_ALT1  0x0800    /* 0b010 Use only for 3 or 4 pin function */
#define PA_PRIM2 0x0C00    /* 0b011 Use only for 3 or 4 pin function */
#define PA_ALT2  0x1000    /* 0b100 Use only for 4 pin function      */
/* Fields for Output Buffer Enable (OBE) */
#define OBE_NO   0x0000    /* 0     */
#define OBE_YS   0x0200    /* 1  Output buffer function */
/* Fields for Input Buffer Enable (IBE) */
#define IBE_NO   0x0000    /* 0     */
#define IBE_YS   0x0100    /* 1  Input buffer function */
/* Fields for Drive Strength Control (DSC) */
#define DSC_10PF 0x0000   /* 0b00 10 pF drive strength */
#define DSC_20PF 0x0040   /* 0b01 20 pF drive strength */
#define DSC_30PF 0x0080   /* 0b10 30 pF drive strength */
#define DSC_50PF 0x00C0   /* 0b11 50 pF drive strength */
/* Fields for Open Drain Enable (ODE) */
#define ODE_DIS  0x0000   /* 0 Open drain output disabled */
#define ODE_EN   0x0020   /* 1 Open drain output enabled  */
/* Fields for Input Hysteresis (HYS) */
#define HYS_DIS  0x0000   /* 0 Input hysteresis disabled  */
#define HYS_EN   0x0010   /* 1 Input hysteresis enabled   */
/* Fields for Slew Rate Control (SRC) */
#define SRC_MIN  0x0000   /* 0b00 Slew rate control minimum */
#define SRC_MED  0x0004   /* 0b01 Slew rate control medium  */
#define SRC_MAX  0x000C   /* 0b11 Slew rate control maximum */
/* Fields for Weak Pull Enable (WPE) */
#define WPE_DIS  0x0000   /* 0 Weak pull disabled */
#define WPE_EN   0x0002   /* 1 Weak pull enabled  */
/* Fields for Weak Pull Select (WPS) */
#define WPS_DN   0x0000   /* 0 Weak pull select down */
#define WPS_UP   0x0001   /* 1 Weak pull select up   */



/**************** EBI initialization constants *****************/

/* Definitions for EBI_MCR (Module Configuration Register) */
/* Fields for SIZEN (Size Enable) */
#define SIZEN_TSIZ_01  0x00000000
#define SIZEN_SIZE     0x04000000

/* Fields for SIZE */
#define SIZE_XFR_32    0x00000000
#define SIZE_XFR_8     0x01000000
#define SIZE_XFR_16    0x02000000

/* Fields for Automatic CLKOUT Gating Enable (ACGE) */
#define ACGE_DISABLED 0x00000000
#define ACGE_ENABLED  0x00008000

/* Fields for External Master Mode (EXTM) */
#define EXTM_INACTIVE 0x00000000 /*single master mode */
#define EXTM_ACTIVE   0x00004000

/* Fields for External Arbitration (EARB) */
#define EARB_INTERNAL 0x00000000
#define EARB_EXTERNAL 0x00002000

/* Fields for External Arbitration Request Prority (EARP) */
#define EARP_MCU      0x00000000
#define EARP_EQUAL    0x00000800
#define EARP_EXTERNAL 0x00001800

/* Fields for Module disable mode (MDIS) */
#define MDIS_INACTIVE 0x00000000 /* Module disable mode is inactive */
#define MDIS_ACTIVE   0x00000040 /* Module disable mode is active   */

/* Fields for Data Bus Mode (DBM) */
#define DBUS_32       0x00000000
#define DBUS_16       0x00000001

/* Definitions for EBI_BRn (Base Registers where n = 0,1,2,3) */

/* Fields for the Base Address (BA) */
#define BA_20000  0x20000000   /* 0x20000 (Base address)                  */
#define BA_20800  0x20800000   /* 0x20800 (Base +   8 Meg of space)       */
#define BA_30000  0x30000000   /* 0x30000 (Base + 268 M address location) */
#define BA_3FF80  0x3FF80000   /* 0x3FF80 (Top of external space - 512K)  */

/* Fields for the Port Size (PS) */
#define PS_32BIT  0x000000000
#define PS_16BIT  0x000000800

/* Fields for the Burst Length (BL) */
#define BL_8WRD  0x00000000
#define BL_4WRD  0x00000040

/* Fields for the Write Enable/Byte Select (WEBS) */
#define WEBS_WE   0x00000000
#define WEBS_BE   0x00000020

/* Fields for the Toggle Burst Data In Progress (TBDIP)*/
#define TBDIP_BURST   0x00000000
#define TBDIP_BEFORE  0x00000010

/* Fields for the Burst Inhibit (BI) */
#define BI_ENABLE   0x00000000
#define BI_DISABLE  0x00000002

/* Fields for the Valid bit (V) */
#define V_INVALID 0x00000000
#define V_VALID   0x00000001

/* Definitions for EBI_ORn (Option Registers where n = 0,1,2,3) */

/* Fields for the Address Mask (AM) */
#define AM_512M  0xE0000000 /* 0xE0000; 512M space */
#define AM_256M  0xF0000000 /* 0xF0000; 256M space */
#define AM_128M  0xF8000000 /* 0xF8000; 128M space */
#define AM_64M   0xFC000000 /* 0xFC000;  64M space */
#define AM_32M   0xFE000000 /* 0xFE000;  32M space */
#define AM_16M   0xFF000000 /* 0xFF000;  16M space */
#define AM_8M    0xFF800000 /* 0xFF800;   8M space */
#define AM_4M    0xFFC00000 /* 0xFFC00;   4M space */
#define AM_2M    0xFFE00000 /* 0xFFE00;   2M space */
#define AM_1M    0xFFF00000 /* 0xFFF00;   1M space */
#define AM_512K  0xFFF80000 /* 0xFFF80; 512K space */
#define AM_256K  0xFFFC0000 /* 0xFFFC0; 256K space */
#define AM_128K  0xFFFE0000 /* 0xFFFE0; 128K space */
#define AM_64K   0xFFFF0000 /* 0xFFFF0;  64K space */
#define AM_32K   0xFFFF8000 /* 0xFFFF8;  32K space */

/* Fields for the Cycle length (SCY) */
#define OR_SCY_0   0x00000000
#define OR_SCY_1   0x00000010
#define OR_SCY_2   0x00000020
#define OR_SCY_3   0x00000030
#define OR_SCY_4   0x00000040
#define OR_SCY_5   0x00000050
#define OR_SCY_6   0x00000060
#define OR_SCY_7   0x00000070
#define OR_SCY_8   0x00000080
#define OR_SCY_9   0x00000090
#define OR_SCY_10  0x000000A0
#define OR_SCY_11  0x000000B0
#define OR_SCY_12  0x000000C0
#define OR_SCY_13  0x000000D0
#define OR_SCY_14  0x000000E0
#define OR_SCY_15  0x000000F0

/* Fields for the Burst Cycle length (BSCY) */
#define OR_BSCY_0   0x00000000
#define OR_BSCY_1   0x00000002
#define OR_BSCY_2   0x00000004
#define OR_BSCY_3   0x00000006

/* Fields for Chip Selects */

#define CS_0  0
#define CS_1  1
#define CS_2  2
#define CS_3  3

/**************** PBRIDGE initialization constants *****************/

/* Definitions for PBRIDGE_A and PBRIDGE_B */

/* Fields for the Master Privilege Control Register (MPCR) */
#define MPCR_DEFAULT  0x77770000

/* Fields for the PACRx and OPACRx registers */
#define PACR_DEFAULT  0x44444444
#define OPACR_DEFAULT 0x44444444

/**************** XBAR initialization constants *****************/

/* Definitions for the XBAR */

/* Fields for the Master Priority registers (XBAR_MPRn)     */
/*        where n = 0,1,3,6,7                               */
/* NOTE: MPC5565 and MPC5567 have 3 bits per field. The     */
/*         other devices only have 2 bits per field.        */
/* WARNING: Masters must be assigned unique priority levels */

/* Only on devices with FlexRay */
#define MST6_PRI_0 0x00000000
#define MST6_PRI_1 0x01000000
#define MST6_PRI_2 0x02000000
#define MST6_PRI_3 0x03000000
#define MST6_PRI_4 0x04000000

/* Only on the MPC5533 and MPC5534 (CPU Load/Store) */
#define MST4_PRI_0 0x00000000
#define MST4_PRI_1 0x00010000
#define MST4_PRI_2 0x00020000
#define MST4_PRI_3 0x00030000
#define MST4_PRI_4 0x00040000

/* Only on devices with FEC */
#define MST3_PRI_0 0x00000000
#define MST3_PRI_1 0x00001000
#define MST3_PRI_2 0x00002000
#define MST3_PRI_3 0x00003000
#define MST3_PRI_4 0x00004000

/* On all MPC5500 devices */
#define MST2_PRI_0 0x00000000
#define MST2_PRI_1 0x00000100
#define MST2_PRI_2 0x00000200
#define MST2_PRI_3 0x00000300
#define MST2_PRI_4 0x00000400
#define MST1_PRI_0 0x00000000
#define MST1_PRI_1 0x00000010
#define MST1_PRI_2 0x00000020
#define MST1_PRI_3 0x00000030
#define MST1_PRI_4 0x00000040
#define MST0_PRI_0 0x00000000
#define MST0_PRI_1 0x00000001
#define MST0_PRI_2 0x00000002
#define MST0_PRI_3 0x00000003
#define MST0_PRI_4 0x00000004

/* Fields for the Slave General Purpose Control register (XBAR_SGPCRn) */
/*        where n = 0,1,3,6,7                                          */
#define SGPCR_DEFAULT 0x00000000
#define RO_RW         0x00000000
#define RO_RO         0x80000000
#define ARB_FX        0x00000000
#define ARB_RROBIN    0x00000100
#define PCTL_PARKMST  0x00000000
#define PCTL_LASTMST  0x00000010
#define PCTL_NOMST    0x00000020
#define PARK_MSTPRT0  0x00000000
#define PARK_MSTPRT1  0x00000001
#define PARK_MSTPRT2  0x00000002
#define PARK_MSTPRT3  0x00000003
#define PARK_MSTPRT4  0x00000004
#define PARK_MSTPRT6  0x00000006

/* Fields used for all GPIO ports */
/* PA field - port assignment*/
#define GPIO_FUNCTION      0x0000
#define PRIMARY_FUNCTION   0x0C00
#define ALTERNATE_FUNCTION 0x0800

#define OUTPUT_BUFFER_ENABLE 0x0200
#define INPUT_BUFFER_ENABLE  0x0100
#define OUTPUT_MODE          0x0200
#define INPUT_MODE           0x0100
#define READBACK_ENABLE      0x0100

/* Fields used for Input */
#define ENABLE_HYSTERESIS  0x0010
#define DISABLE_HYSTERESIS 0x0000

/* treat WPE and WPS as 1 field for weak pull configuration */
#define WEAK_PULL_UP      0x0003
#define WEAK_PULL_DOWN    0x0002
#define WEAK_PULL_DISABLE 0x0000

/* Fields used for Output */
/* DSC field - drive strangth control */
#define DRIVE_STRENGTH_10PF 0x0000
#define DRIVE_STRENGTH_20PF 0x0040
#define DRIVE_STRENGTH_30PF 0x0080
#define DRIVE_STRENGTH_50PF 0x00C0

#define OUTPUT_DRAIN_ENABLE 0x0020

/* SRC field - slew rate control */
#define MINIMUM_SLEW_RATE 0x0000
#define MEDIUM_SLEW_RATE  0x0004
#define MAXIMUM_SLEW_RATE 0x000C

/* Pin Name assignments */
#define PIN_EMIOS10 189
#define PIN_EMIOS11 190
#define PIN_EMIOS12 191
#define PIN_EMIOS13 192

#define PIN_HI 1
#define PIN_LO 0

/**************************************************************************/
/*                                                                        */
/* Definition for needed Special Purpose registers                        */
/*                                                                        */
/*                                                                        */
/*  CPU Special Purpose Registers (SPR's) are defined in mpc5554_spr.h    */
/*                                                                        */
/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* Overall initialization function prototype */
void cfg_mpc5500ccd();

/**************************************************************************/
/*                       C Code Prototypes                                */
/**************************************************************************/

/* This function calls for configuration of the external clocks */
void cfg_XCLKS();

/* This function configures PBRIDGE_A and PBRIDGE_B    */
void cfg_PBRIDGE();

/* This function configures Slave ports and Master priorities */
/*     in the XBAR                                            */
void cfg_XBAR();


/**************************************************************************/
/*                       SIU Prototypes                                   */
/**************************************************************************/

/* This function initializes the EBI. */
void cfg_PCR();

/* This function configures the Pad Configuration Registers (PCR's) for ADDR[8:31] */
void cfg_PCR_ADDR(uint16_t PA_A, uint16_t OBE_A, uint16_t IBE_A, \
                 uint16_t DSC_A, uint16_t ODE_A, uint16_t HYS_A, \
                 uint16_t SRC_A, uint16_t WPE_A, uint16_t WPS_A);

/* This function configures the Pad Configuration Registers (PCR's) for DATA[0:15] */
/*                                                                 and DATA[16:31] */
void cfg_PCR_DATA(uint16_t PA_D, uint16_t OBE_D, uint16_t IBE_D, \
                 uint16_t DSC_D, uint16_t ODE_D, uint16_t HYS_D, \
                 uint16_t SRC_D, uint16_t WPE_D, uint16_t WPS_D, \
                 uint8_t DATA_BITS);

/* This function configures the Pad Configuration Registers (PCR's) for Control */
void cfg_PCR_CTRL(uint16_t PA_C, uint16_t OBE_C, uint16_t IBE_C, \
                 uint16_t DSC_C, uint16_t ODE_C, uint16_t HYS_C, \
                 uint16_t SRC_C, uint16_t WPE_C, uint16_t WPS_C, \
                 uint8_t FRST_PCR, uint8_t PCR_CNT);


/**************************************************************************/
/*                       EBI Prototypes                                   */
/**************************************************************************/

/* This function initializes the EBI. */
void cfg_EBI();

/* This function configures the EBI module before any chip select configuration */
void cfg_EBI_mod(uint32_t SIZEN_val, uint32_t SIZE_val, uint32_t ACGE_val, \
                 uint32_t EXTM_val, uint32_t EARB_val, uint32_t EARP_val, \
                 uint32_t MDIS_val, uint32_t DBM_val);

/* This function configures the base register for a certain chip select */
void cfg_CSn_BR(uint8_t CS_val, uint32_t BM_val, uint32_t PS_val, \
                         uint32_t BL_val, uint32_t WEBS_val, uint32_t TBDIP_val, \
                         uint32_t BI_val, uint32_t V_val);

/* This function configure the option register for a certain chip select */
void cfg_CSn_OR(uint8_t CS_val, uint32_t AM_val, uint32_t SCY_val, \
                         uint32_t BSCY_val);


/**************************************************************************/
/*                  GPIO Function Prototypes                              */
/**************************************************************************/
/* Initialize one LED pin */
void init_LED(uint8_t init_pin, uint8_t pinout_val);

/* Invert LED pin */
void invert_LED(uint8_t inv_pin);

/* Port configuration function */
void mc_gpio_config( uint16_t port, uint16_t config);

/* General Purpose output macro */
void mc_gpio_output( uint16_t port, uint8_t value);


/****************************************************************/


#ifdef  __cplusplus
}
#endif

