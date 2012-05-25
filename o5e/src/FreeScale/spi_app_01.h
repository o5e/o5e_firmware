/***************************************************************************//*!
*
*	Freescale Semiconductor Inc.
*	(c) Copyright 2010 Freescale Semiconductor Inc.
*	ALL RIGHTS RESERVED.
*
********************************************************************************
*
*	@file spi_app_01.h
*	
*	@version 0.0.1.0
*	
*	@lastmodusr B16958
*	
*	@lastmoddate May-18-2010
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
*	@project ANN-xxxx
*	
*	@author B16958
*	
*	@ingroup spi_master
*
******************************************************************************/
#ifndef SPI_APP_01_H
#define SPI_APP_01_H

/******************************************************************************
* External Function Declarations
******************************************************************************/
extern void SpiApp01(void);

#endif /* SPI_APP_01_H */