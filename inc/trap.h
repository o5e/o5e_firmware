/**
 * @file   trap.h
 * @author sstasiak
 * @brief  simple assert-like macro that is ppc specific
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __trap_h
#define   __trap_h

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NDEBUG
  #define trap(x) \
    do { if (!(x)) { asm { trap } } } while(0)
#else
  #define trap(x) \
    do { (void)sizeof(x); } while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif // __trap_h