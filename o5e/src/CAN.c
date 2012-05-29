
// NOTE: this is close to correct but not tested

/********************************************************************

  @file   CAN.c
  @author Jon Zeeff (jon@zeeff.com)
  @date   Apr, 2012
  @brief  Program to read/write streams to CAN bus
  @copyright MIT License
 
  Written as part of the Open5xxxECU project
  Planned additions: NMEA 2000, SimNet, Seatalk NG, OBD-II, CANopen, DeviceNet and J1939

********************************************************************** */

/*
Copyright (c) 2011 Jon Zeeff
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "config.h"
#include "system.h"
#include <stdint.h>
#include "OS.h"
#include "main.h"
#include "FLASH_OPS.h"
#include "variables.h"
#include "crc.h"
#include <string.h>
#include <stdio.h>

// used to unload small FlexCAN buffers into something big enough to send/receive any tuner packet
static vuint8_t Tx_Q[SERIAL_BUFFER_SIZE];
static vuint8_t Rx_Q[SERIAL_BUFFER_SIZE];
static uint_fast16_t Tx_Q_Count;
static uint_fast16_t Rx_Q_Count;
static uint8_t *Tx_Q_Ptr;
static uint8_t *Rx_Q_Ptr;

void init_CAN (void);
uint_fast16_t write_CAN (const uint8_t * const bytes,
			 const uint_fast16_t count);
uint_fast16_t read_CAN (uint8_t * bytes, const uint_fast16_t max_bytes);
void CAN_task (void);
inline static void receiveMsg ();
inline static void TransmitMsg ();

// put bytes in the Tx queue

uint_fast16_t
write_CAN (const uint8_t * const bytes, const uint_fast16_t count)
{
  if (Tx_Q_Count == 0)            // if empty, reset to beginning
     Tx_Q_Ptr = Tx_Q;

  if (count + TX_Q_Count > sizeof (Tx_Q))	// don't over run Q
    return 0;
  memcpy (Tx_Q, bytes, count);
  Tx_Q_Count = count;
  Tx_Q_Ptr = Tx_Q;
  return count;
}

// return bytes we have receieved so far and reset Q
uint_fast16_t
read_CAN (uint8_t * bytes, const uint_fast16_t max_bytes)
{
  if (Rx_Q_Count > 0)
    {
      uint_fast16_t count = Rx_Q_Count;
      if (count > max_bytes)	// don't over run callers buffer
	count = max_bytes;

      memcpy (bytes, Rx_Q, count);
      Rx_Q_Count = 0;
      Rx_Q_Ptr = Rx_Q;		// reset to beginning
      return count;
    }
  else
    return 0;
}

// called every n msec to service buffers
// 1) check for incoming CAN tuner packets and copy to large buffer
// 2) load outgoing buffers as needed

void
CAN_task (void)
{
  task_open ();
  task_wait(1);

  initCAN_A ();

  for (;;)
    {
      receiveMsg ();
      if (Tx_Q_Count > 0)
	TransmitMsg ();
      task_wait (4);		// do other things for ~4 msec
    }				// for

}				// CAN_task()


inline static void
initCAN_A (void)
{
  uint_fast8_t i;

  CAN_A.MCR.B.SOFTRST = 1;	/* Reset */
  while( CAN_A.MCR.B.SOFTRST ==1){}

  CAN_A.MCR.R = 0x5000003F;	/* Put in Freeze Mode & enable all 64 msg buffers */
  CAN_A.CR.R = 0x04DB0006;	/* Configure for 8MHz OSC, 100kHz bit time */

  for (i = 0; i < 64; i++)
      CAN_A.BUF[i].CS.B.CODE = 0;	/* Inactivate all message buffers */

  for (i = 0; i < 32; i++)
      CAN_A.BUF[i].CS.B.CODE = 4;	/* Message Buffer x set to RX INACTIVE */
  for (i = 32; i < 64; i++)
      CAN_A.BUF[i].CS.B.CODE = 8;	/* Message Buffer x set to TX INACTIVE */

  SIU.PCR[83].R = 0x062C;	/* Configure pad as CNTXA, open drain */
  SIU.PCR[84].R = 0x0500;	/* Configure pad as CNRXA */

  CAN_A.IMRH.R  = 0x00000000;	/* no interrupts */
  CAN_A.IMRL.R  = 0x00000000;

  CAN_A.MCR.R = 0x0000003F;	/* Negate FlexCAN A halt state for 64 MB */

}


inline static void
TransmitMsg (void)
{
  uint_fast16_t len;
  uint_fast8_t mbuf;

// !! For now, assume all 4 buffers are empty (ie, have been transmitted)
// if last mbuf not emptied yet, return

  for (mbuf = 60; mbuf < 64; ++mbuf)
    {				// use MBs 60-63 for tx
      if (Tx_Q_Count == 0)	// done
	return;

      len = Tx_Q_Count;
      if (len > 8)		// can send max of 8 bytes at a time
	len = 8;

      CAN_A.BUF[mbuf].CS.B.CODE = 0x8;	/* inactivate msg. buf. */
      CAN_A.BUF[mbuf].CS.B.IDE = 0;	/* Use standard ID length */
      CAN_A.BUF[mbuf].ID.B.STD_ID = 555;	/* Transmit ID is 555 */
      memcpy (CAN_A.BUF[mbuf].DATA.B, Tx_Q_Ptr, len);
      CAN_A.BUF[mbuf].CS.B.RTR = 0;	/* Data frame, not remote Tx request frame */
      CAN_A.BUF[mbuf].CS.B.LENGTH = len;
      CAN_A.BUF[mbuf].CS.B.SRR = 1;	/* Tx frame (not req'd for standard frame) */
      CAN_A.BUF[mbuf].CS.B.CODE = 0xC;	/* Activate msg. buf. to transmit data frame */

      Tx_Q_Count -= len;
      Tx_Q_Ptr += len;
    }				// for

}				// TransmitMsg

inline static void
receiveMsg (void)
{
  uint32_t RxLENGTH;
#define  RxCODE   CAN_A.BUF[mbuf].CS.B.CODE
#define  RxID     CAN_A.BUF[mbuf].ID.B.STD_ID

  for (mbuf = 0; mbuf < 32; ++mbuf)
    {
      if ((CAN_A.IFFRL.R >> mbuf) & 0x1 == 0x0)
	break;			/* nothing more available */

      RxLENGTH = CAN_A.BUF[mbuf].CS.B.LENGTH;

      // add incoming tuner packet to large Rx buffer
      if (RxID == TUNER_CAN_CODE)
	{
	  if (Rx_Q_Count + RxLENGTH < sizeof (Rx_Q))
	    {			// don't over run Q
	      memcpy (Rx_Q_Ptr, CAN_A.BUF[mbuf].DATA.B, RxLENGTH);
	      Rx_Q_Ptr += RxLENGTH;
	      Rx_Q_Count += RxLENGTH;
	    }
	}

      // process other CAN packets/ids here

    }				// for

  // cleanup
  volatile uint32_t dummy = (volatile) CAN_A.TIMER.R;	/* Read TIMER to unlock message buffers */
  CAN_A.IFRL.R = 0x000000;	/*Clear CAN A MB int flags */

}