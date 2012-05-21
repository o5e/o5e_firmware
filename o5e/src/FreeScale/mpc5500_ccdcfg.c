/**************************************************************************/
/* FILE NAME: mpc5500_ccdcfg.c               COPYRIGHT (c) Freescale 2004 */
/*                                                All Rights Reserved     */
/* DESCRIPTION:                                                           */
/* This file contains functions for the MPC5500 assembly configuration.   */
/* The user does not need to make any changes in this file.               */
/* The user will change function field values in mpc5500_ccdcfg.h.        */
/* Choices for field values are found in mpc5500_ccdcfg.h.                */
/*========================================================================*/
/* REV      AUTHOR       DATE       DESCRIPTION OF CHANGE                 */
/* ---   -----------   ----------   ---------------------                 */
/* 0.1   J. Yokubaitis 24/Nov/03     Initial version                      */
/* 0.2   G. Jackson    15/Mar/04     Split into C code configuration      */
/* 0.3   G. Jackson    15/Apr/04     C application functions removed      */
/* 0.4   G. Jackson    13/May/04     Added CS2 and CS3 initializations    */
/* 0.5   G. Jackson    08/Jun/04     Added PCR initialization for the     */
/*                                    external bus and chip select 0 (CS0)*/
/*                                    with external boot config option    */
/*                                    control.                            */
/* 0.6   G. Jackson    10/Jun/04     Changed EXT_BOOT to INT_BOOT as a    */
/*                                   config option in mpc5500_usrccdcfg.h.*/
/* 1.0   G. Jackson    30/Jun/04     Changed INT_BOOT to I_EXT_BUS_EN     */
/*                                    Added I_DATA_PORT_SIZE              */
/*                                    Added I_CFG_PBRG_XBAR               */
/* 1.1   C. Baker      19/Jul/06     Removed device specific #include,    */
/*                                     renamed cfg_mpc5500ccd to cfg_5500 */
/* 1.2   Chris Baker   25/Oct/06     Split I_CFG_PBRG_XBAR into I_CFG_PBRG*/
/*                                     and I_CFG_XBAR                     */
/*                                   Added XBAR configurations for all    */
/*                                     devices to cfg_XBAR                */
/**************************************************************************/

#include "config.h"
#include "mpc5500_usrccdcfg.h"


/***************************************************************************/
/* FUNCTION     : cfg_mpc5500ccd()                                         */
/* PURPOSE      : This function provides "C" code configuration for the    */
/*                 MPC5500 family device. Previous initialization has set  */
/*                 up the FMPLL clock speed, internal SRAM and the stack.  */
/*                 The cfg_mpc5500ccd function completes initialization by */
/*                 making desired changes to the XCLKS, EBI chip selects,  */
/*                 with default settings for PBRIDGE_A, PBRIDGE_B, and the */
/*                 XBAR.                                                   */
/*                                                                         */
/*                 In cases where (numerous) alternative initializations   */
/*                 are desired, a sample configuration is included here.   */  
/*                 Actual initialization required for a specific system    */ 
/*                 remains the responsibility of the developer.            */
/* INPUT NOTES  : (from mpc5500_usrcdefs.h) I_CFG_XCLKS, I_EXT_BUS_EN,     */
/*                  I_CFG_PBRG, I_CFG_XBAR                                 */
/* RETURN NOTES : None                                                     */
/* WARNING      : An external boot will configure CS0, the address bus,    */
/*                 and the data bus; I_EXT_BUS_EN = YES will reconfigure   */
/*                 the external bus with settings in mpc5500_usrcdefs.h.   */                        
/***************************************************************************/

/* MPC5500 initialization functions */
void cfg_mpc5500ccd() {
	
/*void cfg_5500(uint8_t External_Boot) { */
   if(I_CFG_XCLKS) {   /* Option to change external clock speeds */
     cfg_XCLKS();      /* Set up the External Clock speeds */
    }                  /* end I_CFG_XCLKS */

   if(I_EXT_BUS_EN) {  /* Set up external bus if enabled */

     cfg_PCR();        /* Set up Pad Configuration Registers for external bus */ 

     cfg_EBI();        /* Configure the External Bus Interface Chip Selects */

    }                  /* end external bus enable */ 

   if(I_CFG_PBRG) {  /* Skip if not configuring PBRIDGEs */
                          /* NOTE: These steps may not be necessary.    */
                          /*       The user must know the exact effect. */

     cfg_PBRIDGE();    /* Configure PBRIDGE_A and PBRIDGE_B. */
    }

   if(I_CFG_XBAR) {  /* Skip if not configuring XBAR */
                          /* NOTE: These steps may not be necessary.    */
                          /*       The user must know the exact effect. */
     cfg_XBAR();       /* Configure the XBAR master/slave channel priorities. */
    }


} /* End of cfg_mpc5500ccd()  */


/**************************************************************************/
/*                       C Code Functions                                 */
/**************************************************************************/


/*************************************************************************/
/* FUNCTION     : cfg_XCLKS                                              */
/* PURPOSE      : This function configures the external clocks (CLKOUT   */
/*                 and ENGCLK). The PCRs are also set for these 2 pins.  */
/* INPUT NOTES  : I_CFG_XCLKS -- used to permit external clock changes.  */
/*                               Defined in mpc5500_usrcdefs.h           */
/* RETURN NOTES : None                                                   */
/* WARNING      : None                                                   */
/*************************************************************************/

void cfg_XCLKS() 
{

/* This function configures the external clocks */
/*   Engineering clock is set to divide by 128    */
/*   CLKOUT is set to divide-by-2                 */

/* Set the value for Pad Configuration Register (PCR214) for ENGCLK */
    SIU.PCR[214].R = (OBE_ENGCLK_VAL | DSC_ENGCLK_VAL);


/* Set the value for Pad Configuration Register (PCR229) for CLKOUT */
    SIU.PCR[229].R = (OBE_CLKOUT_VAL | DSC_CLKOUT_VAL);

/* Set the value for the external clocks */

    SIU.ECCR.R = (ECCR_ENGDIV | ECCR_EBTS | ECCR_EBDF);

} /* End of cfg_XCLKS  */

/******************************************************************/
/* FUNCTION     : cfg_PCR                                         */
/* PURPOSE      : This function configures the SIU PCR's for      */
/*                  external bus operation.                       */
/* INPUT NOTES  : None                                            */
/* RETURN NOTES : None                                            */
/* WARNING      : This function will be skipped if there is       */
/*                 an external boot.                              */
/*                Entries for *_C1 and *_C2 are control variables */
/*                Entries for *_CG1 through *_CG4 are Control     */
/*                 Group numbers. A Control Group is independent  */
/*                 of the control variables (*_C1,etc.)           */
/* ****************************************************************/
void cfg_PCR()
{

/* This function configures the Pad Configuration Registers (PCR's) for ADDR[8:31] */
    cfg_PCR_ADDR(PA_A_VAL, OBE_A_VAL, IBE_A_VAL, DSC_A_VAL, ODE_A_VAL, HYS_A_VAL, SRC_A_VAL, WPE_A_VAL, WPS_A_VAL);

/* This function configures the Pad Configuration Registers (PCR's) for DATA[0:15] */
    cfg_PCR_DATA(PA_DL, OBE_DL, IBE_DL, DSC_DL, ODE_DL, HYS_DL, SRC_DL, WPE_DL, WPS_DL, DATA_BYT_MSB);

/* This function configures the Pad Configuration Registers (PCR's) for DATA[16:31] */
/*  This function call is not needed for a 16-bit external data bus */
  if(I_DATA_PORT_SIZE == 32) {
    cfg_PCR_DATA(PA_DH, OBE_DH, IBE_DH, DSC_DH, ODE_DH, HYS_DH, SRC_DH, WPE_DH, WPS_DH, DATA_BYT_LSB);
   }

/* This function configures the Pad Configuration Registers (PCR's) for Control */
    cfg_PCR_CTRL(PA_C1, OBE_C1, IBE_C1, DSC_C1, ODE_C1, HYS_C1, SRC_C1, WPE_C1, WPS_C1, FRST_PCR_CG1, PCR_CNT_CG1);

/* This function configures the Pad Configuration Registers (PCR's) for Control */
    cfg_PCR_CTRL(PA_C2, OBE_C2, IBE_C2, DSC_C2, ODE_C2, HYS_C2, SRC_C2, WPE_C2, WPS_C2,FRST_PCR_CG2, PCR_CNT_CG2);
    cfg_PCR_CTRL(PA_C2, OBE_C2, IBE_C2, DSC_C2, ODE_C2, HYS_C2, SRC_C2, WPE_C2, WPS_C2,FRST_PCR_CG3, PCR_CNT_CG3);
    cfg_PCR_CTRL(PA_C2, OBE_C2, IBE_C2, DSC_C2, ODE_C2, HYS_C2, SRC_C2, WPE_C2, WPS_C2,FRST_PCR_CG4, PCR_CNT_CG4);

}

/*****************************************************************/
/* FUNCTION     : cfg_EBI                                        */
/* PURPOSE      : This function configures the EBI Chip Selects. */
/* INPUT NOTES  : None                                           */
/* RETURN NOTES : None                                           */
/* WARNING      : None                                           */
/* ***************************************************************/
void cfg_EBI()
{
/* This function configures the module before any chip select configuration */
    cfg_EBI_mod(SIZEN_VAL, SIZE_VAL, ACGE_VAL, EXTM_VAL, EARB_VAL, EARP_VAL, MDIS_VAL, DBM_VAL);

/* This function configures the base register for a CS_0 */
    cfg_CSn_BR(CS_0, CS0_BA, CS0_PS, CS0_BL, CS0_WEBS, CS0_TBDIP, CS0_BI, CS0_V);

/* This function configures the option register for CS_0 */
/*   Chip Select 0 is targeted for external Flash with wait states */
    cfg_CSn_OR(CS_0, CS0_AM, CS0_SCY, CS0_BSCY);

/* This function configures the base register for a CS_1 */
    cfg_CSn_BR(CS_1, CS1_BA, CS1_PS, CS1_BL, CS1_WEBS, CS1_TBDIP, CS1_BI, CS1_V);

/* This function configures the option register for CS_1 */
/*   Chip Select 1 is targeted for external RAM with 4 wait states */
    cfg_CSn_OR(CS_1, CS1_AM, CS1_SCY, CS1_BSCY);

/* This function configures the base register for a CS_2 */
    cfg_CSn_BR(CS_2, CS2_BA, CS2_PS, CS2_BL, CS2_WEBS, CS2_TBDIP, CS2_BI, CS2_V);

/* This function configures the option register for CS_2 */
/*   Chip Select 1 is targeted for external RAM with 0 wait states */
    cfg_CSn_OR(CS_2, CS2_AM, CS2_SCY, CS2_BSCY);

/* This function configures the base register for a CS_3 */
    cfg_CSn_BR(CS_3, CS3_BA, CS3_PS, CS3_BL, CS3_WEBS, CS3_TBDIP, CS3_BI, CS3_V);

/* This function configures the option register for CS_3 */
/*   Chip Select 1 is targeted for external RAM with 0 wait states */
    cfg_CSn_OR(CS_3, CS3_AM, CS3_SCY, CS3_BSCY);

} /* End of cfg_EBI */

/**************************************************************************/
/*                    Beginning of SIU functions                          */
/**************************************************************************/

/******************************************************************/
/* FUNCTION     : cfg_PCR_ADDR                                    */
/* PURPOSE      : This function configures the module before any  */ 
/*                chip select configuration                       */
/* INPUT NOTES  : PA_A, OBE_A, IBE_A, DSC_A, ODE_A, HYS_A,        */
/*                SRC_A, WPE_A, WPS_A                             */
/* RETURN NOTES : None                                            */
/* WARNING      : Address PCRs 4:27 are automatically written in  */
/*                   this function.                               */
/*                1 PCR is set per 16-bit register write.         */
/******************************************************************/
void cfg_PCR_ADDR(uint16_t PA_A, uint16_t OBE_A, uint16_t IBE_A,
                  uint16_t DSC_A, uint16_t ODE_A, uint16_t HYS_A,
                  uint16_t SRC_A, uint16_t WPE_A, uint16_t WPS_A)
{
  uint8_t PCRNUM;

   for (PCRNUM=4; PCRNUM <=27; PCRNUM++) 
    {
    SIU.PCR[PCRNUM].R = (PA_A | OBE_A | IBE_A | DSC_A | ODE_A | \
                         HYS_A | SRC_A | WPE_A | WPS_A);
    }
}

/******************************************************************/
/* FUNCTION     : cfg_PCR_DATA                                    */
/* PURPOSE      : This function configures PCRs for 16 Data pins. */ 
/*                                                                */
/* INPUT NOTES  : PA_D, OBE_D, IBE_D, DSC_D, ODE_D, HYS_D,        */
/*                SRC_D, WPE_D, WPS_D, DATA_BITS                  */
/* RETURN NOTES : None                                            */
/* WARNING      : For DATA[0:15] DATA_BITS must = 0; PCR[28:43]   */
/*                For DATA[16:31] DATA_BITS must = 16; PCR[44:59] */
/*                1 PCR is set per 16-bit register write.         */
/******************************************************************/
void cfg_PCR_DATA(uint16_t PA_D, uint16_t OBE_D, uint16_t IBE_D,
                 uint16_t DSC_D, uint16_t ODE_D, uint16_t HYS_D,
                 uint16_t SRC_D, uint16_t WPE_D, uint16_t WPS_D,
                 uint8_t DATA_BITS)
{
  uint8_t PCRNUM;       /* PCRNUM starts at either 28 or 44 */

   /* 1 PCR is set per 16-bit write */
   for (PCRNUM=28+DATA_BITS; PCRNUM<(44+DATA_BITS); PCRNUM++)
    {
     SIU.PCR[PCRNUM].R = (PA_D | OBE_D | IBE_D | DSC_D | ODE_D | \
                          HYS_D | SRC_D | WPE_D | WPS_D);
    }
}


/**********************************************************************/
/* FUNCTION     : cfg_PCR_CTRL                                        */
/* PURPOSE      : This function configures the module before any      */ 
/*                chip select configuration                           */
/* INPUT NOTES  : PA_C, OBE_C, IBE_C, DSC_C, ODE_C, HYS_C,            */
/*                SRC_C, WPE_C, WPS_C, FRST_PCR, PCR_CNT              */
/* RETURN NOTES : None                                                */
/* WARNING      : FRST_PCR is the first PCR written.                  */
/*                PCR_CNT is the number of 16-bit PCR register writes.*/
/*                1 PCR is set per 16-bit register write.             */
/**********************************************************************/
void cfg_PCR_CTRL(uint16_t PA_C, uint16_t OBE_C, uint16_t IBE_C, \
                  uint16_t DSC_C, uint16_t ODE_C, uint16_t HYS_C, \
                  uint16_t SRC_C, uint16_t WPE_C, uint16_t WPS_C, \
                  uint8_t FRST_PCR, uint8_t PCR_CNT)
{
  uint8_t PCRNUM;

   for (PCRNUM=FRST_PCR; PCRNUM<(FRST_PCR+PCR_CNT); PCRNUM++)  /* PCRNUM = FRST_PCR to FRST_PCR+PCR_CNT */
    {
    SIU.PCR[PCRNUM].R = (PA_C | OBE_C | IBE_C | DSC_C | ODE_C | \
                         HYS_C | SRC_C | WPE_C | WPS_C);
                            /* 1 PCR is set per 16-bit line */
    }
}


/**************************************************************************/
/*                    Beginning of EBI functions                          */
/**************************************************************************/

/******************************************************************/
/* FUNCTION     : cfg_EBI_mod                                     */
/* PURPOSE      : This function configures the module before any  */ 
/*                chip select configuration                       */
/* INPUT NOTES  : SIZEN_val,SIZE_val,ACGE_val,EXTM_val,           */
/*                 EARB_val,EARP_val,MDIS_val,DBM_val             */
/* RETURN NOTES : None                                            */
/* WARNING      : None                                            */
/******************************************************************/
void cfg_EBI_mod(uint32_t SIZEN_val, uint32_t SIZE_val, uint32_t ACGE_val, 
                 uint32_t EXTM_val, uint32_t EARB_val, uint32_t EARP_val, 
                 uint32_t MDIS_val, uint32_t DBM_val)
{
    EBI.MCR.R = (SIZEN_val | SIZE_val | ACGE_val | EXTM_val | \
                  EARB_val | EARP_val | MDIS_val | DBM_val);
}

/******************************************************************/
/* FUNCTION     : cfg_EBI_BRn                                     */
/* PURPOSE      : This function configures the EBI chip select    */
/*                 base register.                                 */
/* INPUT NOTES  : CS_val,BM_val,PS_val,BL_val,WEBS_val,TBDIP_val, */
/*                BI_val, V_val                                   */
/* RETURN NOTES : None                                            */
/* WARNING      : None                                            */
/******************************************************************/
void cfg_CSn_BR(uint8_t CS_val, uint32_t BM_val, uint32_t PS_val,
                uint32_t BL_val, uint32_t WEBS_val, uint32_t TBDIP_val,
                uint32_t BI_val, uint32_t V_val)
{
    EBI.CS[CS_val].BR.R = (BM_val | PS_val | BL_val | WEBS_val | \
                           TBDIP_val | BI_val | V_val);
}

/******************************************************************/
/* FUNCTION     : cfg_EBI_ORn                                     */
/* PURPOSE      : This function configures the EBI chip select    */
/*                  option register.                              */
/* INPUT NOTES  : CS_val, AM_val, SCY_val, BSCY_val               */
/* RETURN NOTES : None                                            */
/* WARNING      : None                                            */
/******************************************************************/
void cfg_CSn_OR(uint8_t CS_val, uint32_t AM_val, uint32_t SCY_val,
                  uint32_t BSCY_val)
{
    EBI.CS[CS_val].OR.R = ( AM_val | SCY_val | BSCY_val );
}


/**************************************************************************/
/*                    End of EBI functions                                */
/**************************************************************************/


/**************************************************************************/
/* FUNCTION     : cfg_PBRIDGE                                             */
/* PURPOSE      : This function configures PBRIDGE A and B master         */
/*                 privilege and peripheral access control.               */
/*                 Default settings are used.                             */
/* INPUT NOTES  : PBA_MPCR,PBA_PACR0,PBA_OPACRn (where n=0,1,2)           */
/*                PBB_MPCR,PBB_PACRn,PBB_OPACRm (where n=0,2 m=0,1,2,3)   */
/*                Inputs are user defined mpc5500_ccdcfg.h                */
/* RETURN NOTES : None                                                    */
/* WARNING      : Reference: MPC5554 RM chapter 5.3                       */
/*                MPCR default value = 0x77770000;                        */
/*                PACRn & OPACRn default value = 0x44444444               */
/**************************************************************************/

#ifdef MPC5554

void cfg_PBRIDGE()
{
 /* Settings for the PBRIDGE_A   */     
    PBRIDGE_A.MPCR.R   = PBA_MPCR;
    PBRIDGE_A.PACR0.R  = PBA_PACR0;
    PBRIDGE_A.OPACR0.R = PBA_OPACR0;
    PBRIDGE_A.OPACR1.R = PBA_OPACR1;
    PBRIDGE_A.OPACR2.R = PBA_OPACR2;

 /* Settings for the PBRIDGE_B   */     
    PBRIDGE_B.MPCR.R   = PBB_MPCR;
    PBRIDGE_B.PACR0.R  = PBB_PACR0;
    PBRIDGE_B.PACR2.R  = PBB_PACR2;
    PBRIDGE_B.OPACR0.R = PBB_OPACR0;
    PBRIDGE_B.OPACR1.R = PBB_OPACR1;
    PBRIDGE_B.OPACR2.R = PBB_OPACR2;
    PBRIDGE_B.OPACR3.R = PBB_OPACR3;
} /* End of cfg_PBRIDGE */

#endif

/*************************************************************************/
/* FUNCTION     : cfg_XBAR                                               */
/* PURPOSE      : This function configures the Cross Bar (XBAR) master   */
/*                 channel priorities and slave port configurations.     */
/*                 Default settings are used.                            */
/* INPUT NOTES  : MPRn_MST2,MPRn_MST1,MPRn_MST0 (where n=0,1,3,6,7)      */
/*                SGPCRn_RO, SGPCRn_ARB, SGPCRn_PCTL, SGPCRn_PARK        */
/*                                              (where n=0,1,3,6,7)      */
/*                Inputs are user defined mpc5500_ccdcfg.h               */
/* RETURN NOTES : None                                                   */
/* WARNING      : MPRn default value = 0x00000210                        */
/*                SGPCRn default value = 0x00000000                      */
/*************************************************************************/

void cfg_XBAR()
{
	/* Priority settings for the masters on each XBAR channel */
	/* highest priority = 0                                   */

	#if defined(_MPC5533_) || defined(_MPC5534_)
		/* In some cases the core could dominate the use of the   */
		/*   flash slave port because there is no cache.          */
		/* Setting the core instruction port to a lower priority  */
		/*   ensures other masters are able to access the flash.  */

		/* Master0 (core instruction) priority 2                  */
		/* Master1 (eDMA)             priority 0                  */
		/* Master2 (EBI)              priority 3                  */
		/* Master4 (core load/store)  priority 1                  */
		XBAR.MPR0.R = ( MPR0_MST4 | MPR0_MST2 | MPR0_MST1 | MPR0_MST0 );
		XBAR.MPR1.R = ( MPR1_MST4 | MPR1_MST2 | MPR1_MST1 | MPR1_MST0 );
		XBAR.MPR3.R = ( MPR3_MST4 | MPR3_MST2 | MPR3_MST1 | MPR3_MST0 );
		XBAR.MPR6.R = ( MPR6_MST4 | MPR6_MST2 | MPR6_MST1 | MPR6_MST0 );
		XBAR.MPR7.R = ( MPR7_MST4 | MPR7_MST2 | MPR7_MST1 | MPR7_MST0 );
	#endif /* _MPC5533_ | _MPC5534_ */

	#if defined(_MPC5553_) || defined(_MPC5566_)
		/* Master0 (core) priority 0                              */
		/* Master1 (eDMA) priority 1                              */
		/* Master2 (EBI)  priority 2                              */
		/* Master3 (FEC)  priority 3                              */
		XBAR.MPR0.R = ( MPR0_MST3 | MPR0_MST2 | MPR0_MST1 | MPR0_MST0 );
		XBAR.MPR1.R = ( MPR1_MST3 | MPR1_MST2 | MPR1_MST1 | MPR1_MST0 );
		XBAR.MPR3.R = ( MPR3_MST3 | MPR3_MST2 | MPR3_MST1 | MPR3_MST0 );
		XBAR.MPR6.R = ( MPR6_MST3 | MPR6_MST2 | MPR6_MST1 | MPR6_MST0 );
		XBAR.MPR7.R = ( MPR7_MST3 | MPR7_MST2 | MPR7_MST1 | MPR7_MST0 );
	#endif /* _MPC5553_ | _MPC5566_ */

	#ifdef _MPC5554_
		/* Master0 (core) priority 0                              */
		/* Master1 (eDMA) priority 1                              */
		/* Master2 (EBI)  priority 2                              */
		XBAR.MPR0.R = ( MPR0_MST2 | MPR0_MST1 | MPR0_MST0 );
		XBAR.MPR1.R = ( MPR1_MST2 | MPR1_MST1 | MPR1_MST0 );
		XBAR.MPR3.R = ( MPR3_MST2 | MPR3_MST1 | MPR3_MST0 );
		XBAR.MPR6.R = ( MPR6_MST2 | MPR6_MST1 | MPR6_MST0 );
		XBAR.MPR7.R = ( MPR7_MST2 | MPR7_MST1 | MPR7_MST0 );
	#endif /* _MPC5554_ */

	#ifdef _MPC5561_
		/* Master0 (core)     priority 0                              */
		/* Master1 (eDMA)     priority 1                              */
		/* Master2 (EBI)      priority 2                              */
		/* Master6 (FlexRay)  priority 3                              */
		XBAR.MPR0.R = ( MPR0_MST6 | MPR0_MST2 | MPR0_MST1 | MPR0_MST0 );
		XBAR.MPR1.R = ( MPR1_MST6 | MPR1_MST2 | MPR1_MST1 | MPR1_MST0 );
		XBAR.MPR3.R = ( MPR3_MST6 | MPR3_MST2 | MPR3_MST1 | MPR3_MST0 );
		XBAR.MPR5.R = ( MPR5_MST6 | MPR5_MST2 | MPR5_MST1 | MPR5_MST0 );
		XBAR.MPR6.R = ( MPR6_MST6 | MPR6_MST2 | MPR6_MST1 | MPR6_MST0 );
		XBAR.MPR7.R = ( MPR7_MST6 | MPR7_MST2 | MPR7_MST1 | MPR7_MST0 );
	#endif /* _MPC5561_ */

	#if defined(_MPC5565_) || defined(_MPC5567_)
		/* While the MPC5565 does not have the FEC or FlexRay     */
		/*   module, the XBAR has master ports 3 and 6, and these */
		/*   must be given a unique value in the Master Priority  */
		/*   Registers to prevent conflicts.                      */
		/* Master0 (core)    priority 0                           */
		/* Master1 (eDMA)    priority 1                           */
		/* Master2 (EBI)     priority 2                           */
		/* Master3 (FEC)     priority 3                           */
		/* Master6 (FlexRay) priority 4                           */
		XBAR.MPR0.R = ( MPR0_MST6 | MPR0_MST3 | MPR0_MST2 | MPR0_MST1 | MPR0_MST0 );
		XBAR.MPR1.R = ( MPR1_MST6 | MPR1_MST3 | MPR1_MST2 | MPR1_MST1 | MPR1_MST0 );
		XBAR.MPR3.R = ( MPR3_MST6 | MPR3_MST3 | MPR3_MST2 | MPR3_MST1 | MPR3_MST0 );
		XBAR.MPR6.R = ( MPR6_MST6 | MPR6_MST3 | MPR6_MST2 | MPR6_MST1 | MPR6_MST0 );
		XBAR.MPR7.R = ( MPR7_MST6 | MPR7_MST3 | MPR7_MST2 | MPR7_MST1 | MPR7_MST0 );
	#endif /* _MPC5565_ | _MPC5567_ */

	/* Configuration settings for Slave channels              */
	/* XBAR_SGPCRs are set to  Read/Write, arbitration fixed  */
	/*                                                        */
	/* MPC5533 and MPC5534 have 2 core Master XBAR ports.     */
	/* Parking should use both master 0 and master 4          */
	/*                                                        */
	/* Slave0 (flash)    park on Master0 (core instruction)   */
	/* Slave1 (EBI)      park on Master0 (core instruction)   */
	/* Slave4 (SRAM)     park on Master4 (core load/store)    */
	/* Slave6 (PbridgeA) park on Master4 (core load/store)    */
	/* Slave7 (PbridgeB) park on Master4 (core load/store)    */
	/*                                                        */
	/* For all other devices all slave ports can be parked on */
	/*   master 0 (default settings).                         */
	XBAR.SGPCR0.R = ( SGPCR0_RO | SGPCR0_ARB | SGPCR0_PCTL | SGPCR0_PARK );
	XBAR.SGPCR1.R = ( SGPCR1_RO | SGPCR1_ARB | SGPCR1_PCTL | SGPCR1_PARK );
	XBAR.SGPCR3.R = ( SGPCR3_RO | SGPCR3_ARB | SGPCR3_PCTL | SGPCR3_PARK );
#ifdef MPC5554
	XBAR.SGPCR6.R = ( SGPCR6_RO | SGPCR6_ARB | SGPCR6_PCTL | SGPCR6_PARK );
#endif
	XBAR.SGPCR7.R = ( SGPCR7_RO | SGPCR7_ARB | SGPCR7_PCTL | SGPCR7_PARK );

	#ifdef _MPC5561_
		XBAR.SGPCR5.R = ( SGPCR5_RO | SGPCR5_ARB | SGPCR5_PCTL | SGPCR5_PARK );
	#endif /* _MPC5561_ */
} /* End of cfg_XBAR */

