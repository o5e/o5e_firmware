// See AN4147

#if 0
/*******************************************************************************
*
*	Freescale Semiconductor Inc.
*	(c) Copyright 2010 Freescale Semiconductor Inc.
*	ALL RIGHTS RESERVED.
*
*	@file spi_app_01.c
*	@version 0.0.3.0
*	@lastmodusr B16958
*	@lastmoddate Jul-1-2010
*	
********************************************************************************
*
*	@brief eDMA emulating SPI master
*
*	This example demonstates how the eDMA could be used to emulate SPI master,
*	sending and recieving data
*	Configuration:
*		- DMA channel 0 SCK generation activated periodically by PIT
*		- DMA channel 1 processes data in
*		- DMA channel 2 processes data out
*		- CS is solved by SW, but could be as an additional DMA channel
*		- Connection: Connect DSPI to GPIO used for eDMA spi emulation
*			- PC4: DSPI1_SIN  -> PB0 (DMA SOUT)
*			- PC5: DSPI1_SOUT -> PB1 (DMA SIN)
*			- PC2: DSPI1_SCK  -> PB2 (DMA SCK)
*			- PC3: DSPI1_CS0  -> PB3 (DMA CS0)
*	
*	@project AN4147
*	@author B16958
*	@ingroup spi_master
*
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/
//#include "spi_app_01.h"
//#include "MPC560xB.h"

/******************************************************************************
* Internal variable Definitions
******************************************************************************/
static  unsigned char *pInData=((unsigned char *)&SIU.PGPDO[3].R)+1; /*!< Pointer 
* to the input data accessing parallel GPIO registers. */
static  unsigned char *pOutData=((unsigned char *)&SIU.PGPDO[3].R)+2;/*!< Pointer 
* to the output data accessing parallel GPIO registers. */
static  unsigned long int toggleTimer[2]=
{
	1, /* enabling the timer at first channel iteration */
	0  /* disable the timer in next iteration */
};
/*!< This array is used to toggle (start/stop) periodic interrupt timer, meaning
* to enable/disable the transmission. Enabling is done by the application, whereas
* disabling is automatic, when a stop character is indicated. */

static  unsigned char generateClock[2]=
{
	1, /* depending on CPHA and CPOL */
	0 
};
/*!< This array is used to generate clock, so alternately outputting 1 and zero. Note, that
the order depends on the selected SPI properties CPHA and CPOL, which in our example 
equal to 1 and 0 respectively */

/******************************************************************************
* Internal Function Declarations
******************************************************************************/

static void SpiInit01(void);
static void SpiRun01(void);

/******************************************************************************
* External Function Definitions
******************************************************************************/
extern void SpiApp01(void)
{
	SpiInit01();
	SpiRun01();
}

/******************************************************************************
* Internal Function Definitions
******************************************************************************/

/***************************************************************************//*!
*
* @brief Initializes the SPI master using eDMA and also the DSPI peripheral
*			as a slave to check the data
*
*	Initialization of the following peripherals:
*		- SIU: PCR registers for DMA SPI master and DSPI SPI slave
*		- DSPI: spi slave, 8 bit data.
*		- PIT_0: To periodically trigger DMA at 9600Hz (SPI clock rate)
*		- DMAMUX: Routes PIT_0 flag to activate channel 0 DMA
*		- eDMA0: Setup to send alternately 0 and 1 to generate clock and link to
*			-channel_1 to recieve data (after iteration)
*			-channel_2 to transmit data (after completion)
*		- eDMA1: Setup one byte transfer from GPIO to PH0-PH7 (reading)
*		- eDMA2: Setup one byte transfer from PH8-PH15 to GPIO (writing)
*		- eDMA3: Setup to enable or disable the PIT timer to control the spi transmission.
*
*	@ingroup spi_master
*
******************************************************************************/
static void SpiInit01(void)
{
	/* Initialize PCR registers */
	/* DMA SPI */
	 SIU.PCR[16].B.PA = 0;          /* PB0: DMA SPI SOUT */
	 SIU.PCR[16].B.OBE = 1;         /* Output buffer enable */
	 SIU.PCR[17].B.PA = 0;          /* PB1: DMA SPI SIN */
	 SIU.PCR[17].B.IBE = 1;         /* Input buffer enable */
	 SIU.PCR[18].B.PA = 0;          /* PB2: DMA SPI SCK */
	 SIU.PCR[18].B.OBE = 1;         /* Output buffer enable */
	 SIU.PCR[19].B.PA = 0;          /* PB3: DMA SPI CS */
	 SIU.PCR[19].B.OBE = 1;         /* Output buffer enable */
	 
	 /*  DSPI */
	 SIU.PCR[34].B.PA = 1;          /* PC[2]: DSPI1 SCK */
	 SIU.PCR[34].B.IBE = 1;         /* Input buffer enable */
	 SIU.PCR[35].B.PA = 1;          /* PC[3]: DSPI1 CS0_1 */
	 SIU.PCR[35].B.IBE = 1;         /* Input buffer enable */
	 SIU.PCR[36].B.PA = 0;          /* PC[4]: DSPI1 SIN_1 */
	 SIU.PCR[36].B.IBE = 1;         /* Input buffer enable */
	 SIU.PCR[37].B.PA = 1;          /* PC[5]: DSPI1 SOUT_1 */
	 SIU.PCR[37].B.OBE = 1;         /* Output buffer enable */
	
	 SIU.GPDO[19].R=1; /* set CS to inactive before starting congifure */

	/* DSPI settings as slave device */
	DSPI_0.MCR.R = 0x01013C00;		/* SPI, Slave (CS0 incative high), disable FIFOs */
	DSPI_0.MCR.B.DIS_TXF = 1;		/* disable FIFOs */
	DSPI_0.MCR.B.DIS_RXF = 1;
	DSPI_0.CTAR[0].R=0x38000000;	/* 8 bit, CPHA,CPOL= 0,0 */

	DSPI_1.MCR.R = 0x01013C00;		/* SPI, Slave (CS0 incative high), disable FIFOs */
	DSPI_1.MCR.B.DIS_TXF = 1;		/* disable FIFOs */
	DSPI_1.MCR.B.DIS_RXF = 1;
	DSPI_1.CTAR[0].R=0x3A000000;	/* 8 bit, CPHA,CPOL= 1,0 */

	/* Initialize PIT 0 timer for periodic triggers of DMA channel 0 */
 	PIT.PITMCR.B.MDIS = 0;	/* enable the module */
  	PIT.CH[0].LDVAL.R=833;	/* setup 9600@8MHZ overflow rate */
  	PIT.CH[0].TFLG.B.TIF=1; /* clear the flag */

	/* Initialize the DMA channel 0 -- SPI clock */
	DMAMUX.CHCONFIG[0].R =0x00; /* disable the channel activation source */
	/* Setup the TCD for channel 0 --> single SCI Tx */
	EDMA.TCD[0].SADDR = (vuint32_t)( &(generateClock)); /* Generate Clock structure, 0&1 */
	EDMA.TCD[0].SSIZE = 0; /* Read 2**0 = 1 byte per transfer */
	EDMA.TCD[0].SOFF = 1; /* After transfer, decrement 1 */
	EDMA.TCD[0].SLAST = -2; /* After major loop, back to the beginning (2 iterations) */
	EDMA.TCD[0].SMOD = 0; /* Source modulo feature not used */
	EDMA.TCD[0].DADDR = (vuint32_t) &(SIU.GPDO[18].R); /* PB2 DMA SPI clock */
	EDMA.TCD[0].DSIZE =  0; /* Write 2**0 = 1 byte per transfer */
	EDMA.TCD[0].DOFF = 0; /* Do not increment destination addr */
	EDMA.TCD[0].DLAST_SGA = 0; /* After major loop no change to destination addr */
	EDMA.TCD[0].DMOD = 0; /* Destination modulo feature not used */
	EDMA.TCD[0].NBYTES = 1; /* Transfer 1 byte per minor loop */
	EDMA.TCD[0].BITER = 2; /* 2 iterations of major loop */
	EDMA.TCD[0].CITER = 2; /* Initialize current iteraction count */
	EDMA.TCD[0].D_REQ = 0; /* Disable channel when major loop is done (sending one byte only) */
	EDMA.TCD[0].INT_HALF = 0; /* no interrupt in half */
	EDMA.TCD[0].INT_MAJ = 0; /* no interrupt in major loop completion */
	EDMA.TCD[0].CITERE_LINK = 1; /* enable both linking */
	EDMA.TCD[0].BITERE_LINK = 1;
	EDMA.TCD[0].MAJORE_LINK = 1; 
	EDMA.TCD[0].BITERLINKCH = 2; /* after itteration: lint to channel 2 */	
	EDMA.TCD[0].CITERLINKCH = 2;	
	EDMA.TCD[0].MAJORLINKCH = 1; /* after completion: lint to channel 1 */	
	EDMA.TCD[0].E_SG = 0;
	EDMA.TCD[0].BWC = 0; /* Default bandwidth control- no stalls */
	EDMA.TCD[0].START = 0; /* Initialize status flags */
	EDMA.TCD[0].DONE = 0;
	EDMA.TCD[0].ACTIVE = 0;
	/* Setup the TCD for channel 1 -- SPI data IN */
	EDMA.TCD[1].SADDR = (vuint32_t)&(SIU.GPDI[17].R); /* Load data from SIN pin PB1 */
	EDMA.TCD[1].SSIZE = 0; /* Read 2**0 = 1 byte per transfer */
	EDMA.TCD[1].SOFF = 0; /* After transfer no change */
	EDMA.TCD[1].SLAST = 0; /* After major loop no change */
	EDMA.TCD[1].SMOD = 0; /* Source modulo feature not used */
	EDMA.TCD[1].DADDR = (vuint32_t)( &(SIU.GPDO[104].R)); /* writing to internal (GPDO bit-buffer) */
	EDMA.TCD[1].DSIZE =  0; /* Write 2**0 = 1 byte per transfer */
	EDMA.TCD[1].DOFF = 1; /* Do increment destination addr */
	EDMA.TCD[1].DLAST_SGA = -8; /* After major loop go back 8 bytes to destination addr */
	EDMA.TCD[1].DMOD = 0; /* Destination modulo feature not used */
	EDMA.TCD[1].NBYTES = 1; /* Transfer 1 byte per minor loop */
	EDMA.TCD[1].BITER = 8; /* 12 iterations of major loop */
	EDMA.TCD[1].CITER = 8; /* Initialize current iteraction count, (start with 2nd data!) */
	EDMA.TCD[1].D_REQ = 0; /* Disable channel when major loop is done (sending the buffer once) */
	EDMA.TCD[1].INT_HALF = 0; /* no interrupt in half */
	EDMA.TCD[1].INT_MAJ = 0; /* no interrupt in major loop completion */
	EDMA.TCD[1].CITERE_LINK = 0; /* Linking disabled */
	EDMA.TCD[1].BITERE_LINK = 0;
	EDMA.TCD[1].BITERLINKCH = 0;
	EDMA.TCD[1].CITERLINKCH = 0;
	EDMA.TCD[1].MAJORE_LINK = 1;/* Link after major loop to disable further transmission */
	EDMA.TCD[1].MAJORLINKCH = 3; /* after completion: link to channel 3 */	
	EDMA.TCD[1].E_SG = 0;
	EDMA.TCD[1].BWC = 0; /* Default bandwidth control- no stalls */
	EDMA.TCD[1].START = 0; /* Initialize status flags */
	EDMA.TCD[1].DONE = 0;
	EDMA.TCD[1].ACTIVE = 0;
	/* Setup the TCD for channel 2 -- SPI data OUT */
	EDMA.TCD[2].SADDR = (vuint32_t)( &(SIU.GPDO[112].R)); /* read from internal (GPDO) bit buffer */
	EDMA.TCD[2].SSIZE = 0; /* Read 2**0 = 1 byte per transfer */
	EDMA.TCD[2].SOFF = 1; /* After transfer no change */
	EDMA.TCD[2].SLAST = -8; /* After major loop no change */
	EDMA.TCD[2].SMOD = 0; /* Source modulo feature not used */
	EDMA.TCD[2].DADDR = (vuint32_t)&(SIU.GPDO[16].R); /* writing to PCR 17 (SOUT=PB0) */
	EDMA.TCD[2].DSIZE =  0; /* Write 2**0 = 1 byte per transfer */
	EDMA.TCD[2].DOFF = 0; /* Do increment destination addr */
	EDMA.TCD[2].DLAST_SGA = 0; /* After major loop go back 8 bytes to destination addr */
	EDMA.TCD[2].DMOD = 0; /* Destination modulo feature not used */
	EDMA.TCD[2].NBYTES = 1; /* Transfer 1 byte per minor loop */
	EDMA.TCD[2].BITER = 8; /* 12 iterations of major loop */
	EDMA.TCD[2].CITER = 8; /* Initialize current iteration count, (start with 2nd data!) */
	EDMA.TCD[2].D_REQ = 0; /* Disable channel when major loop is done (sending the buffer once) */
	EDMA.TCD[2].INT_HALF = 0; /* no interrupt in half */
	EDMA.TCD[2].INT_MAJ = 0; /* no interrupt in major loop completion */
	EDMA.TCD[2].CITERE_LINK = 0; /* Linking after minor loop disabled */
	EDMA.TCD[2].BITERE_LINK = 0;
	EDMA.TCD[2].BITERLINKCH = 0;
	EDMA.TCD[2].CITERLINKCH = 0;
	EDMA.TCD[2].MAJORE_LINK = 0; 
	EDMA.TCD[2].MAJORLINKCH = 0;
	EDMA.TCD[2].E_SG = 0;
	EDMA.TCD[2].BWC = 0; /* Default bandwidth control no stalls */
	EDMA.TCD[2].START = 0; /* Initialize status flags */
	EDMA.TCD[2].DONE = 0;
	EDMA.TCD[2].ACTIVE = 0;
	/* Setup the TCD for channel 3 -- to start/stop the transmission */
	EDMA.TCD[3].SADDR = (vuint32_t)(&toggleTimer); /* memory structure to enable/disable PIT0 */
	EDMA.TCD[3].SSIZE = 2; /* Read 2**2 = 4 byte per transfer */
	EDMA.TCD[3].SOFF = 4; /* After transfer word increment */
	EDMA.TCD[3].SLAST = -2*4; /* After major loop back two */
	EDMA.TCD[3].SMOD = 0; /* Source modulo feature not used */
	EDMA.TCD[3].DADDR = (vuint32_t)(&(PIT.CH[0].TCTRL.R)); /* PIT_0 - enable */
	EDMA.TCD[3].DSIZE =  2; /* Write 2**2 = 4 byte per transfer */
	EDMA.TCD[3].DOFF = 0; /* Do not increment destination addr */
	EDMA.TCD[3].DLAST_SGA = 0; /* After major no change to destination addr */
	EDMA.TCD[3].DMOD = 0; /* Destination modulo feature not used */
	EDMA.TCD[3].NBYTES = 4; /* Transfer 1 byte per minor loop */
	EDMA.TCD[3].BITER = 2; /* 12 iterations of major loop */
	EDMA.TCD[3].CITER = 2; /* Initialize current iteraction count, (start with 2nd data!) */
	EDMA.TCD[3].D_REQ = 0; /* Disable channel when major loop is done (sending the buffer once) */
	EDMA.TCD[3].INT_HALF = 0; /* no interrupt in half */
	EDMA.TCD[3].INT_MAJ = 0; /* no interrupt in major loop completion */
	EDMA.TCD[3].CITERE_LINK = 0; /* Linking disabled */
	EDMA.TCD[3].BITERE_LINK = 0;
	EDMA.TCD[3].BITERLINKCH = 0;
	EDMA.TCD[3].CITERLINKCH = 0;
	EDMA.TCD[3].MAJORE_LINK = 0; 
	EDMA.TCD[3].MAJORLINKCH = 0; 
	EDMA.TCD[3].E_SG = 0;
	EDMA.TCD[3].BWC = 0; /* Default bandwidth control- no stalls */
	EDMA.TCD[3].START = 0; /* Initialize status flags */
	EDMA.TCD[3].DONE = 0;
	EDMA.TCD[3].ACTIVE = 0;


	/* route DMA source to always enabled channel and enable periodic triggers */
	DMAMUX.CHCONFIG[0].R =0xC0 | 0x1F; /* only PIT0 -- always enabled channel */

}

/***************************************************************************//*!
*
* @brief	Transmits the message "Hello World!" and other message from emulated
*			SPI master to real SPI slave on DSPI_1.
*
*	Data transmission is started by channel 3, which enables PIT0.
*	Timer then triggers channel0, which generates clock and is linked to
*		- channel 1 after iteration of minor loop - SPI read
*		- channel 2 after completion of major loop - SPI write
*		- channel 3 is linked from channel 1 after transmission completes and
*			disables back PIT 0, which turns off the master
*
*	Note, that CS is solved by SW at this example.
*
*	@ingroup spi_master
*
******************************************************************************/
static void SpiRun01(void)
{
volatile unsigned char i;
unsigned char putData[12]="Hello World!";
unsigned char getData[12]="sir,yes sir!";

volatile unsigned char slaveGot[12];
volatile unsigned char masterGot[12];

	/* enable DMA channels 0,1,2,3 */
	EDMA.SERQR.R = 0;
	EDMA.SERQR.R = 1;
	EDMA.SERQR.R = 2;
	EDMA.SERQR.R = 3;
	SIU.GPDO[18].R=0; /* start with sck=0 */	


	/* prepare some data */
	for (i=0; i<12; i++)
	{
		/* preparing the data to be sent */
		DSPI_1.PUSHR.R = (unsigned long)getData[i];  /* on the slave side - DSPI */
		*pOutData=putData[i];							/* on the master side - DMA */
		
		/* now initiate transmission */
		SIU.GPDO[19].R=0; /* activate CS */
		EDMA.TCD[3].START = 1;
		/* waiting till transmission finishes */
		while (EDMA.TCD[3].DONE == 0) {};
		SIU.GPDO[19].R=1; /* inactivate CS */
		EDMA.TCD[3].DONE = 0; /* clear the flag */
		
		
		/* retrieving the data */
		slaveGot[i]=(unsigned char)DSPI_1.POPR.R; /* what was actually received on DSPI slave */
		masterGot[i]=*pInData;					  /* what the slave returned to the DMA master */
		
	}
	

  for (;;) 
  {
  	/* check what master and slave received */
  }
   

}
#if 0

// Non DMA version - not used

/* main.c:  performs a single transfer from DSPI_C to DSPI_B on MPC555x or MPC563x */
/* Rev 1.0 Jun 2 2008 SM - Ported from AN2865 example Rev 2.2 for DSPI C, DSPI B */
/*                         and used POPR[RXDATA] for RecDataMaster, RecDataSlave */
/* Rev 1.1 Aug 15 2008 SM - Modified SIU.DISR line for internal DSPI connections */
/* Rev 1.2 Aug 18 2008 D McKenna- Kept DSPI_MCR[HALT] set during  initialization*/
/* Copyright Freescale Semiconductor, Inc. 2007 All rights reserved. */
/* Notes:  */
/*  1. MMU not initialized; must be done by debug scripts or BAM */
/*  2. SRAM not initialized; must be done by debug scripts or in a crt0 type file */

#include "config.h"
#include "cpu.h"

vuint32_t i = 0;                /* Dummy idle counter */
vuint32_t RecDataMaster = 0;    /* Data recieved on master SPI */
vuint32_t RecDataSlave = 0;     /* Data received on slave SPI */

void initSysclk(void)
{
/* MPC563x: Use the next line        */
    FMPLL.ESYNCR1.B.CLKCFG = 0X7;       /* MPC563x: Change clk to PLL normal from crystal */
    FMPLL.SYNCR.R = 0x16080000; /* 8 MHz xtal: 0x16080000; 40MHz: 0x46100000 */
    while (FMPLL.SYNSR.B.LOCK != 1) {
    };                          /* Wait for FMPLL to LOCK  */
    FMPLL.SYNCR.R = 0x16000000; /* 8 MHz xtal: 0x16000000; 40MHz: 0x46080000 */
}

void initDSPI_C(void)
{
    DSPI_C.MCR.R = 0x80010001;  /* Configure DSPI_C as master */
    DSPI_C.CTAR[0].R = 0x780A7727;      /* Configure CTAR0  */
    DSPI_C.MCR.B.HALT = 0x0;    /* Exit HALT mode: go from STOPPED to RUNNING state */
    SIU.PCR[107].R = 0x0A00;    /* MPC555x: Config pad as DSPI_C SOUT output */
    SIU.PCR[108].R = 0x0900;    /* MPC555x: Config pad as DSPI_C SIN input */
    SIU.PCR[109].R = 0x0A00;    /* MPC555x: Config pad as DSPI_C SCK output */
    SIU.PCR[110].R = 0x0A00;    /* MPC555x: Config pad as DSPI_C PCS0 output */
}

void initDSPI_B(void)
{
    DSPI_B.MCR.R = 0x00010001;  /* Configure DSPI_B as slave */
    DSPI_B.CTAR[0].R = 0x780A7727;      /* Configure CTAR0  */
    DSPI_B.MCR.B.HALT = 0x0;    /* Exit HALT mode: go from STOPPED to RUNNING state */
    SIU.PCR[102].R = 0x0500;    /* MPC555x: Config pad as DSPI_B SCK input */
    SIU.PCR[103].R = 0x0500;    /* MPC555x: Config pad as DSPI_B SIN input */
    SIU.PCR[104].R = 0x0600;    /* MPC555x: Config pad as DSPI_B SOUT output */
    SIU.PCR[105].R = 0x0500;    /* MPC555x: Config pad as DSPI_B PCS0/SS input */
}

void ReadDataDSPI_B(void)
{
    while (DSPI_B.SR.B.RFDF != 1) {
    }                           /* Wait for Transmit Complete Flag = 1 */
    RecDataSlave = DSPI_B.POPR.B.RXDATA;        /* Read data received by slave SPI */
    DSPI_B.SR.R = 0x80020000;   /* Clear TCF, RDRF flags by writing 1 to them */
}

void ReadDataDSPI_C(void)
{
    while (DSPI_C.SR.B.RFDF != 1) {
    }                           /* Wait for Transmit Complete Flag = 1 */
    RecDataMaster = DSPI_C.POPR.B.RXDATA;       /* Read data received by master SPI */
    DSPI_C.SR.R = 0x90020000;   /* Clear TCF, RDRF, EOQ flags by writing 1 */
}

int mainxx(void)
{
/* Optional:  Use one of the next two lines for internal DSPI connections: */
    /* SIU.DISR.R = 0x0000C0FC; *//* MPC55xx except MPC563x: Connect DSPI_C, DSPI_D */
    SIU.DISR.R = 0x00A8A000;    /* MPC563x only: Connect DSPI_C, DSPI_B */
    initSysclk();               /* Set sysclk = 64MHz running from PLL */
    initDSPI_C();               /* Initialize DSPI_C as master SPI and init CTAR0 */
    initDSPI_B();               /* Initialize DSPI_B as Slave SPI and init CTAR0 */
    DSPI_B.PUSHR.R = 0x00001234;        /* Initialize slave DSPI_B's response to master */
    DSPI_C.PUSHR.R = 0x08015678;        /* Transmit data from master to slave SPI with EOQ */
    ReadDataDSPI_B();           /* Read data on slave DSPI */
    ReadDataDSPI_C();           /* Read data on master DSPI */
    while (1) {
        i++;
    }                           /* Wait forever */
}

#endif
