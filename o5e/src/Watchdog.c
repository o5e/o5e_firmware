/*********************************************************************************

    @file Watchdog.c                                                              
    @date   October, 2011
    @brief  Open5xxxECU - sets up watchdog to reset cpu if failure
    @note www.Open5xxxECU.org
    @version 1.3
    @copyright MIT License

**********************************************************************************/

/*
Copyright (c) 2011 Jon Zeeff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "config.h"
#include "system.h"
#include "cpu.h"
#include "variables.h"
#include "Functions.h"

// registers from the reference manual (note: 4 byte words, not bytes)
#define SWT_BASE  (uint32_t *)0xFFF38000
#define SWT_CR 0x0000  // SWT Control Register (SWT_CR) 
#define SWT_IR 0x0001  // SWT Interrupt Register (SWT_IR)  
#define SWT_TO 0x0002  // SWT Time-Out Register (SWT_TO) 
#define SWT_WN 0x0003  // SWT Window Register (SWT_WN) 
#define SWT_SR 0x0004  // SWT Service Register (SWT_SR)  
#define SWT_CO 0x0005  // SWT Counter Output Register (SWT_CO)  
#define SWT_SK 0x0006  // SWT Service Key Register (SWT_SK)

// Eventually we will have an external hardware watchdog

void Init_Watchdog(void)
{

#if 0

#ifdef MPC5634
   *(SWT_BASE + SWT_SR) = 0xC520;       // clear soft lock bit
   *(SWT_BASE + SWT_SR) = 0xD928;       // clear soft lock bit
   *(SWT_BASE + SWT_TO) = (uint32_t)(CPU_CLOCK * 3);   // 3 second timeout
   *(SWT_BASE + SWT_CR) = 0xff000103;   // default except for CSL 
#endif
}

// TODO watchdog feeding doesn't work for unknown reasons
void Feed_Watchdog(void)
{
#ifdef MPC5634
    asm { wrteei 0 }
    *(SWT_BASE + SWT_SR) = 0xA602; 
    *(SWT_BASE + SWT_SR) = 0xB480; 
    asm { wrteei 1 }
#endif


#endif

}                              

#if 0

// Code from FreeScale
// This function clears the TSR[ENW] bit by writing 1 to the bit location.
// prevent an initial watchdog timeout by writing TSR[ENW] with 1.
// method based on application note AN2817.pdf

// Feed it
// ECSM.SWTSR.R = 0x40; 
// or
// ECSM.SWTSR.R = 0x80;     

asm void wd_srvrc(void)
{
  lis r12, 0xC000
  mtspr 336, r12
}

static uint32_t clr_wd_status(void)
{
  vuint32_t tmp1;
  asm("lis r6, 0x4000");			/*# load r6 with TSR[WIS] bit (bit 1)*/
  asm("mtspr TSR, r6");				/*# move the val back to TSR*/
  return tmp1;
}

static uint32_t clr_wd_next(void)
{
  vuint32_t tmp1;
  asm("lis r6, 0x8000");			/*# load r6 with TSR[WIS] bit (bit 1)*/
  asm("mtspr TSR, r6");				/*# move the val back to TSR*/
  return tmp1;
}

#endif
