
/*********************************************************************************

        @file   SPI.c                                                              
        @author Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU - detection
        @note   Not completed pending clarifications....
        @version .9
        @copyright MIT License

*************************************************************************************/

/* 
Copyright(c) 2011 Jon Zeeff
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include "config.h"
#include "system.h"
#include <stdint.h>
#include "cpu.h"
#include "OS.h"
#include "mpc5500_ccdcfg.h"
#include "eDMA_OPS.h"
#include "main.h"

void SPI_Task(void)
{
    static uint_fast8_t i;

    task_open();                // standard OS entry - required on all tasks

    for (;;) {

        // check if incoming packet is available

        task_wait(1000);
    }                           // for ever

    task_close();
}                               // SPI_Task()


/*
Notes about ENC28J60 ethernet:

SPI mode 0,0 only. 
Most commands are 8 bits followed by data
Slave only
SCK at Idle in a low state;
data being clocked in on the rising edge of SCK. 
CS pin must be held low while any operation is performed and returned high when finished. 
detect available packet via polling
Packets read have to be stripped of header, crc, etc.

*/

static void Init_SPI(void)
{

// Initialize PCR registers 

#define B0000  (0 << 10)        // always GPIO?
#define B0001  (1 << 10)
#define B0010  (2 << 10)        // some 3 or 4 function pins
#define B0011  (3 << 10)        // some 3 or 4 function pins
#define B0100  (4 << 10)        // only 4 function pins
#define B1000  (8 << 10)

// in/out
#define UNUSED  (0 << 8)
#define OUTPUT  (1 << 9)
#define INPUT   (1 << 8)

#ifdef MPC5634
    // Pad setup for output pins
    SIU.PCR[16].R = B0000 | OUTPUT;  // PB0: DMA SPI SOUT 
    SIU.PCR[17].R = B0000 | OUTPUT;  // PB1: DMA SPI SIN 
    SIU.PCR[18].R = B0000 | OUTPUT;  // PB2: DMA SPI SCK 
    SIU.PCR[19].R = B0000 | OUTPUT;  // PB3: DMA SPI CS 
#endif
#ifdef MPC5554
    // Pad setup for output pins
    SIU.PCR[102].R = B0000 | OUTPUT;  // PB0: DMA SPI SOUT 
    SIU.PCR[103].R = B0000 | OUTPUT;  // PB1: DMA SPI SIN 
    SIU.PCR[104].R = B0000 | OUTPUT;  // PB2: DMA SPI SCK 
    SIU.PCR[105].R = B0000 | OUTPUT;  // PB3: DMA SPI CS 
#endif

    uint16_t SPI_Receive_Buffer[100];
    uint16_t SPI_Send_Buffer[100];

    // set up two SPI controllers, one for master/send and one for slave/receive

    // DMA setup chan 12 and 13 for SPI_B
    EDMA.CERQR.R = 12;          /* stop DMA on this channel */
    EDMA.TCD[12].SADDR = (uint32_t) &DSPI_B.POPR.R;    /* Load address of source data */
    EDMA.TCD[12].SSIZE = 0;     /* Read 2**0 = 1 byte per transfer */
    EDMA.TCD[12].SOFF = 0;      /* After transfer, add 0 to src addr */
    EDMA.TCD[12].SLAST = 0;     /* After major loop, don't reset src addr */
    EDMA.TCD[12].DADDR = (uint32_t) &SPI_Receive_Buffer; /* Load address of destination */
    EDMA.TCD[12].DSIZE = 0;     /* Write 2**0 = 1 byte per transfer */
    EDMA.TCD[12].DOFF = 1;      /* Do increment destination addr */
    EDMA.TCD[12].DLAST_SGA = 0; /* After major loop, no dest addr change */
    EDMA.TCD[12].NBYTES = 1;    /* Transfer 1 byte per minor loop */
    EDMA.TCD[12].BITER = sizeof(SPI_Receive_Buffer);    /* COUNT minor loop iterations */
    EDMA.TCD[12].CITER = sizeof(SPI_Receive_Buffer);    /* COUNT Initialize current iteraction count */
    EDMA.TCD[12].D_REQ = 1;     /* Disable channel when major loop is done */
    Zero_DMA_Channel(12);
    EDMA.SERQR.R = 12;          /* start DMA on this channel */

    /* Module Control Register */
    // MCR = 0x817F0C00 = master, rooe, all pcsis =1, clear TXF, RXF
    DSPI_B.MCR.B.MSTR = 1;      // master
    DSPI_B.MCR.B.CONT_SCKE = 0; // continuous clock
    DSPI_B.MCR.B.DCONF = 0;     // SPI mode
    DSPI_B.MCR.B.FRZ = 0;
    DSPI_B.MCR.B.MTFE = 0;
    DSPI_B.MCR.B.PCSSE = 0;
    DSPI_B.MCR.B.ROOE = 1;
    DSPI_B.MCR.B.PCSIS0 = 1;    // Peripheral chip select inactive state
    DSPI_B.MCR.B.PCSIS1 = 1;
    DSPI_B.MCR.B.PCSIS2 = 1;
    DSPI_B.MCR.B.PCSIS3 = 1;
    DSPI_B.MCR.B.PCSIS5 = 1;
    DSPI_B.MCR.B.MDIS = 0;
    DSPI_B.MCR.B.DIS_TXF = 0;
    DSPI_B.MCR.B.DIS_RXF = 0;
    DSPI_B.MCR.B.CLR_TXF = 1;
    DSPI_B.MCR.B.CLR_RXF = 1;
    DSPI_B.MCR.B.SMPL_PT = 0;
    DSPI_B.MCR.B.HALT = 0;

    /* Clock and Transfer Attributes Registers */
    //CTAR = 0x7AA82204  dbr = 0, FMSZ = 16 bits, cpol = 1, pbr = 0, br=16
    DSPI_B.CTAR[0].B.DBR = 0;
    DSPI_B.CTAR[0].B.FMSZ = 0xf;        // frame size
    DSPI_B.CTAR[0].B.CPOL = 1;          // polarity
    DSPI_B.CTAR[0].B.CPHA = 0;
    DSPI_B.CTAR[0].B.LSBFE = 0;
    DSPI_B.CTAR[0].B.PCSSCK = 0;
    DSPI_B.CTAR[0].B.PASC = 0;
    DSPI_B.CTAR[0].B.PDT = 0;
    DSPI_B.CTAR[0].B.PBR = 0;
    DSPI_B.CTAR[0].B.CSSCK = 0;
    DSPI_B.CTAR[0].B.ASC = 0;

    /* DMA/Interrupt Request Select and Enable Register */
    DSPI_B.RSER.B.TFFFRE = 1;
    DSPI_B.RSER.B.TFFFDIRS = 1;
    DSPI_B.RSER.B.RFDFRE = 1;
    DSPI_B.RSER.B.RFDFDIRS = 1;

#ifdef MPC5554
    SIU.DISR.R = 0x0000C0FC;    // MPC55xx except MPC563x: Connect DSPI_C, DSPI_D 
#endif
#ifdef MPC5634
    SIU.DISR.R = 0x00A8A000;    // MPC563x only: Connect DSPI_C, DSPI_B 
#endif
    // DSPI_C.PUSHR.R = (unsigned long)xxx;     /* write */
    // yyy = (unsigned char)DSPI_C.POPR.R;     /* read */
}

uint_fast16_t
Write_SPI(const uint8_t * const bytes, const uint_fast16_t count)
{
    if (count > SERIAL_BUFFER_SIZE) {   // too big - not allowed 
        system_error(25829, __FILE__, __LINE__, "");
        return 0;
    }

    if (count == 0) return 0;

#if 0
    EDMA.CERQR.R = 18;          /* stop DMA on this channel */

    memcpy((void *)&SCI_TxQ0, (void *)bytes, count);    // Copy bytes to output buffer - free up callers buffer

    EDMA.TCD[18].CITER = EDMA.TCD[18].BITER = count;    // Outer loop count
    EDMA.TCD[18].SADDR = (uint32_t) & SCI_TxQ0;         // reset Start Address to beginning of buffer
    //EDMA.TCD[18].SLAST = -count;                      // After major loop, reset src addr

    ESCI_A.SR.R = 0x80000000;   /* Clear TDRE flag */

    EDMA.SERQR.R = 18;          /* start DMA on this channel */

    return count;               // how many we wrote    
#endif

}                               // write_serial()

//  copy any bytes received from the serial port to into the callers buffer
//  return number of bytes
//  Note: the DMA transfer is "in progress"

uint_fast16_t 
Read_SPI(uint8_t * bytes, const uint_fast16_t max_bytes)
{
    uint32_t n;

}


void ReadDataDSPI_B(void)
{
    while (DSPI_B.SR.B.RFDF != 1) {
    }                           /* Wait for Transmit Complete Flag = 1 */
    //RecDataSlave = DSPI_B.POPR.B.RXDATA;        /* Read data received by slave SPI */
    DSPI_B.SR.R = 0x80020000;   /* Clear TCF, RDRF flags by writing 1 to them */
}
