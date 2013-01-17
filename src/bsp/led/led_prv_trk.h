/**
 * @file   led_prv_trk.h
 * @author sstasiak
 * @brief  private header for trk variant
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __led_prv_trk_h
#define   __led_prv_trk_h

#ifdef __cplusplus
extern "C"
{
#endif

#include "mpc563xm.h"

/**
 * @internal
 */
static inline void
  __led0_on( void )
{
  SIU.GPDO[188].R = 0;
}

/**
 * @internal
 */
static inline void
  __led1_on( void )
{
  SIU.GPDO[189].R = 0;
}

/**
 * @internal
 */
static inline void
  __led2_on( void )
{
  SIU.GPDO[190].R = 0;
}

/**
 * @internal
 */
static inline void
  __led3_on( void )
{
  SIU.GPDO[191].R = 0;
}

/**
 * @internal
 */
static inline void
  __led0_off( void )
{
  SIU.GPDO[188].R = 1;
}

/**
 * @internal
 */
static inline void
  __led1_off( void )
{
  SIU.GPDO[189].R = 1;
}

/**
 * @internal
 */
static inline void
  __led2_off( void )
{
  SIU.GPDO[190].R = 1;
}

/**
 * @internal
 */
static inline void
  __led3_off( void )
{
  SIU.GPDO[191].R = 1;
}

/**
 * @internal
 */
static inline void
  __led0_invert( void )
{
  SIU.GPDO[188].R ^= 1;
}

/**
 * @internal
 */
static inline void
  __led1_invert( void )
{
  SIU.GPDO[189].R ^= 1;
}

/**
 * @internal
 */
static inline void
  __led2_invert( void )
{
  SIU.GPDO[190].R ^= 1;
}

/**
 * @internal
 */
static inline void
  __led3_invert( void )
{
  SIU.GPDO[191].R ^= 1;
}

/**
 * @internal
 */
static inline void
  __led_init( void )
{
  union
  {
    uint16_t reg;
    struct 
    {
      uint16_t    :3;       /*  */
      uint16_t  pa:3;       /*  */
      uint16_t obe:1;       /*  */
      uint16_t ibe:1;       /*  */
      uint16_t dsc:2;       /*  */
      uint16_t ode:1;       /*  */
      uint16_t hys:1;       /*  */
      uint16_t src:2;       /*  */
      uint16_t wpe:1;       /*  */
      uint16_t wps:1;       /*  */
    };
  } const pcr = {
    .pa  = 0,
    .obe = 1,
    .ibe = 0,
    .dsc = 3,
    .ode = 1,
    .hys = 0,
    .src = 3,
    .wpe = 0,
    .wps = 0,
  };
  
  __led0_off();               /**< set initial state off to avoid glitch */
  __led1_off();
  __led2_off();
  __led3_off();
  
  SIU.PCR[188].R = pcr.reg;   /**< set dir                               */
  SIU.PCR[189].R = pcr.reg;
  SIU.PCR[190].R = pcr.reg;
  SIU.PCR[191].R = pcr.reg;
}

#ifdef __cplusplus
}
#endif

#endif // __led_prv_trk_h