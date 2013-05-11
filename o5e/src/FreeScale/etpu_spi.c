/**************************************************************************
 * FILE NAME: $RCSfile: etpu_spi.c,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the ETPU Serial Peripheral Interface (SPI) API.     *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_spi.c,v $
 * Revision 2.0  2004/12/03 12:04:30  r47354
 * Updated rev to 2.0 to indictae ETPU. to eTPU-> change.
 *
 * Revision 1.2  2004/12/03 11:27:41  r47354
 * Changes to remove etpu_config.h from builds.
 *
 * Revision 1.1  2004/12/02 15:32:57  r47354
 * Initial version.
 *
 *........................................................................*
 * 0.1   J. Scott     10/May/04     Initial version.                      * 
 * 0.2   J. Scott     17/May/04     Updated				                        * 
 * 0.3	 J. Scott     27/May/04	    Updated				                        * 
 * 0.4   G.Emerson    06/July/04	Added Disable channel to run & recieve  * 
 *                                  APIs                                  * 
 *                                  GCT compliance                        * 
 *                                  Update for new build structure        * 
 *                                  Post review changes                   * 
 *                                  Add disclaimer                        * 
 *                                  Remove debug comments                 * 
 * 0.5   G.Emerson    05/Aug/04	    change timer to timebase              * 
 * 0.6   G.Emerson    09/Sep/04	    Change baud_rate comments             * 
 *                                  Fix compile warnings                  * 
 * 0.7   G.Emerson    13/Sep/04     Fix LSB_first warning                 * 
 * 0.8   G.Emerson    15/Oct/04     Fix clock_channel desc.               * 
 * 0.9   G. Emerson    1/Nov/04     Remove paths in #includes             *
 * 0.10  G. Emerson   22/Nov/04     Make hsr occur before channel is      *
 *                                  enabled.                              *
 **************************************************************************/
#include "etpu_util.h"          /* Utility routines for working eTPU */
#include "etpu_spi.h"           /* eTPU SPI API header */
extern uint32_t fs_etpu_data_ram_start;

/******************************************************************************
FUNCTION     : fs_etpu_spi_init
PURPOSE      : To initialize three eTPU channels for the SPI function.
INPUTS NOTES : This function has 6 parameters:
          clock_channel - This is the number of the clock channel.
                          1-30 for ETPU_A and 65-94 for ETPU_B.
                          The recieve channel is clock_channel number - 1.
                          The transmit channel is clock_channel number + 1.	  
               priority	- This is the priority to assign to the channel.
               			      This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
              baud_rate - This is the required rate of data transmission.
                          Maximum baud rate is dependant upon system frequency,
                          timebase frequency and eTPU loading; see eTPU SPI
                          App Note section 4.1.
                          The eTPU microcode schedules an event every ‘half period’ 
                          of the programmed eTPU SPI baud_rate. The half period is 
                          calculated by the initialisation API according to the 
                          following formula
                          
                          half_period = timebase_freq/(2* baud_rate).
                          If this formula resolves to a non-integer value then 
                          the API will effectively round down half_period. This 
                          will result in an error in the baud_rate. The accuracy of
                          data transfers is unaffected by this error.
              shift_dir - Selection of MSB or LSB first shift direction.
               			      This parameter should be assigned a value of
               			      FS_ETPU_SPI_SHIFT_DIR_MSB or FS_ETPU_SPI_SHIFT_DIR_LSB
               polarity	-	This is the polarity of the clock. This parameter
                          should be assigned a value of:
                          FS_ETPU_SPI_CLK_POL_POS or FS_ETPU_SPI_CLK_POL_NEG
	        transfer_size - This is the size of transfer for Transmit and Receive
	       			            This parameter should be assigned a value of between
	       			            0x1 and 0x18 (1 and 24 decimal, 1 and 11000 binary)
               timebase -	This is the timebase to use as a reference for the SPI
                         	clock. This parameter should be assigned to a value
                         	of: FS_ETPU_TCR1 or FS_ETPU_TCR2.
          timebase_freq - This is the prescaled frequency of the timebase supplied
                          to the function
******************************************************************************/
int32_t fs_etpu_spi_init (uint8_t clock_channel, uint8_t priority, 
                          uint32_t baud_rate,uint8_t shift_dir, 
                          uint8_t polarity, uint32_t transfer_size,
                          uint8_t timebase, uint32_t timebase_freq)
{
    uint32_t *pba;              /*parameter base address for channel */
    uint8_t *pba8;              /*parameter base address for channel */
    uint8_t mode;
    uint32_t half_period;

/*first disable channels*/
    fs_etpu_disable ((uint8_t) (clock_channel - 1));
    fs_etpu_disable (clock_channel);
    fs_etpu_disable ((uint8_t) (clock_channel + 1));

    if (eTPU->CHAN[clock_channel].CR.B.CPBA == 0)
      {
          /* get parameter RAM
             number of parameters passed from eTPU C code */
          pba = fs_etpu_malloc (FS_ETPU_SPI_NUM_PARMS);

          if (pba == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else                        /*set pba to what is in the CR register */
      {
          pba = fs_etpu_data_ram (clock_channel);
      }

    pba8 = (uint8_t *) pba;

    half_period = timebase_freq / (baud_rate * 2);

/* write parameters */
    *(pba + ((FS_ETPU_SPI_HALF_PERIOD - 1) >> 2)) = (uint32_t) half_period;

    *(pba8 + ((FS_ETPU_SPI_BIT_COUNT))) = (uint8_t) transfer_size;

/* write function modes: Function Mode 0 = Polarity, 
                         Function Mode 1 = Shift Dir */

    mode = 0x00;

    if (polarity == FS_ETPU_SPI_CLK_POL_NEG)
      {
          mode = 1;
      }
    if (shift_dir == FS_ETPU_SPI_SHIFT_DIR_LSB)
      {
          mode = (uint8_t) (mode + 2);
      }

    eTPU->CHAN[clock_channel].SCR.R = (mode);

/* write hsr */

    if (timebase == FS_ETPU_TCR1)
      {
          eTPU->CHAN[clock_channel].HSRR.R = FS_ETPU_SPI_INIT_TCR1;
      }
    else
      {
          eTPU->CHAN[clock_channel].HSRR.R = FS_ETPU_SPI_INIT_TCR2;
      }


/* RX and TX channels have same base address as CLK */
    eTPU->CHAN[clock_channel - 1].CR.R =
        (uint32_t) (((uint32_t)pba - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[clock_channel + 1].CR.R =
        (uint32_t) (((uint32_t)pba - fs_etpu_data_ram_start) >> 3);

    eTPU->CHAN[clock_channel].CR.R =
        (priority << 28) + (FS_ETPU_SPI_TABLE_SELECT << 24) +
        (FS_ETPU_SPI_FUNCTION_NUMBER << 16) + (((uint32_t)pba -
        fs_etpu_data_ram_start) >> 3);

    return (0);

}

/******************************************************************************
FUNCTION     : fs_etpu_spi_transmit_data
PURPOSE      : For the eTPU to both Transmit and Receive Data.
INPUTS NOTES : This function has 2 parameters:
               clock_channel - This is the number of the clock channel.
                           	  0-31 for ETPU_A and 64-95 for ETPU_B.
               tx_data - This is the data to be transmitted.
               			     It will be placed in data_reg to be read by the
               			     SPI function
RETURNS NOTES: none
******************************************************************************/
void fs_etpu_spi_transmit_data (uint8_t clock_channel, uint32_t tx_data)
{
    uint32_t bit_count;
    uint8_t LSB_first;
    uint32_t *pba;              /*parameter base address for channel */
    uint32_t *pba_ext;          /*parameter base address for channel 
                                  (sign extended area) */
    uint8_t *pba8;              /*parameter base address for channel */

    pba = fs_etpu_data_ram (clock_channel);
    pba_ext = pba + 0x1000;
    pba8 = (uint8_t *) pba;

/* shift data to the left if going MSB first */
    LSB_first = (uint8_t) (eTPU->CHAN[clock_channel].SCR.B.FM1);
    if (LSB_first != FS_ETPU_SPI_SHIFT_DIR_LSB)
      {
          pba8 = (uint8_t *) pba;
          bit_count = *(pba8 + FS_ETPU_SPI_BIT_COUNT);
          tx_data <<= (24 - bit_count);
      }
    /* preserve bit_count */

    *(pba_ext + ((FS_ETPU_SPI_DATA_REG - 1) >> 2)) = tx_data;

/* write hsr */
    eTPU->CHAN[clock_channel].HSRR.R = FS_ETPU_SPI_RUN;

}

/******************************************************************************
FUNCTION     : fs_etpu_spi_get_data
PURPOSE      : To retrieve the data_reg data from the eTPU SPI channel.
INPUTS NOTES : This function has 1 parameter:
               clock_channel - This is the number of the clock channel.
                           	  0-31 for ETPU_A and 64-95 for ETPU_B.

RETURNS NOTES: none
******************************************************************************/
uint32_t fs_etpu_spi_get_data (uint8_t clock_channel)
{
    uint32_t SPI_data;
    uint32_t bit_count;
    uint8_t LSB_first;
    uint32_t *pba;              /*parameter base address for channel */
    uint8_t *pba8;              /*parameter base address for channel */

/* this array is used to filter out the relevant data from the data_reg */
    const uint32_t fs_spi_data_mask[] =
        { 0x00000000, 0x00000001, 0x00000003, 0x00000007,
        0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
        0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
        0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
        0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
        0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
        0x00FFFFFF
    };

    pba = fs_etpu_data_ram (clock_channel);
    pba8 = (uint8_t *) pba;
    bit_count = *(pba8 + FS_ETPU_SPI_BIT_COUNT);
    SPI_data = (*(pba + ((FS_ETPU_SPI_DATA_REG - 1) >> 2))) & 0x00FFFFFF;

    LSB_first = (uint8_t) (eTPU->CHAN[clock_channel].SCR.B.FM1);
    if (LSB_first != FS_ETPU_SPI_SHIFT_DIR_LSB)
      {
          /* MSB first */
          SPI_data = SPI_data & fs_spi_data_mask[bit_count];
      }

    else                        /*mask off meaningless bits */
      {
          /* LSB first */
          SPI_data >>= (24 - bit_count);
      }

    return (SPI_data);
}

/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of Freescale
 *  Semiconductor, Inc. This software is provided on an "AS IS"
 *  basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, Freescale
 *  Semiconductor DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
 *  REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)
 *  AND ANY ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL Freescale Semiconductor BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  Freescale Semiconductor assumes no responsibility for the
 *  maintenance and support of this software
 ********************************************************************/
