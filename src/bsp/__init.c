/**
 * @file       __init.cpp
 * @headerfile __init.h
 * @author     sean
 * @brief      startup runtime init hook
 *
 * @note I placed every bit of this code in the .init section because the
 *       memcpy and memset is guaranteed to be 'available' for copying other
 *       generic .text sections up to ram for their use later.
 *
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| TYPES    |--------------------------------------------------------- */

void __init( void );

/**
 * @internal
 * @brief memcpy implementation to avoid bringing in the entire
 *        mwerks runtime library for a simple initialization
 */
static inline void
  _memcpy_( void * dst,
            const void * src,
            unsigned int n );

/**
 * @internal
 * @brief mwerks memset to avoid bringing in their entire runtime library
 *        for a simple initialization
 * @note ugly manual loop unrolling compliments of mwerks
 */
static inline void
  _fill_mem_( void * dst,
              int val,
              unsigned long n );

/**
 * @internal
 * @brief mwerks MSL simple runtime initialization to avoid bringing in
 *        their entire runtime library
 */
static void
  __rom_copy( void );

/**
 * @internal
 * @brief mwerks MSL simple runtime initialization to avoid bringing in
 *        their entire runtime library
 */
static void
  __bss_zero( void );

/**
 * @internal
 * @brief call all c++ static ctor initializers in the order as laid down by
 *        the linker
 */
static void
  __init_cpp( void );

/**
 * Special symbol for array of static ctor calls
 */
typedef void (*vfptr_t) (void);

/**
 * Special symbol for copying sections from rom to ram. If size field is
 * zero, there are no more valid sections
 */
typedef struct __rom_copy_info
{
  char *       rom;    /* address in rom */
  char *       addr;   /* address in ram (executing address) */
  unsigned int size;   /* size of section */
} __rom_copy_info;

/**
 * Special symbol for initializing bss type sections. If size field is zero,
 * there are no more valid sections
 */
typedef struct __bss_init_info
{
  char *       addr;   /* address in ram (executing address) */
  unsigned int size;   /* size of section */
} __bss_init_info;

/* --| INTERNAL |--------------------------------------------------------- */

__declspec(section ".init")
extern __rom_copy_info  _rom_copy_info[];     /*< linker defined */

__declspec(section ".init")
extern __bss_init_info  _bss_init_info[];     /*< linker defined */

/**
 * @internal
 * @brief mwerks memcpy implementation to avoid bringing in their entire
 *        runtime library for a simple initialization
 */
static inline void
  _memcpy_( void * dst,
            const void * src,
            unsigned int n )
{
  const char * p;
        char * q;

  if ((unsigned long) src < (unsigned long) dst)
  {
    for (p = (const char *) src + n, q = (char *) dst + n, n++; --n;)
      *--q = *--p;
  }
  else
  {
    for (p = (const char *) src - 1, q = (char *) dst - 1, n++; --n;)
      *++q = *++p;
  }
}

/**
 * @internal
 * @brief mwerks memset to avoid bringing in their entire runtime library
 *        for a simple initialization
 * @note ugly manual loop unrolling compliments of mwerks
 */
static inline void
  _fill_mem_( void * dst,
              int val,
              unsigned long n )
{

#define cps ((unsigned char *) src)
#define cpd ((unsigned char *) dst)
#define lps ((unsigned long *) src)
#define lpd ((unsigned long *) dst)
#define deref_auto_inc(p) *++(p)

  unsigned long     v = (unsigned char) val;
  unsigned long     i;

  // cpd = ((unsigned char *) dst) - 1;
  dst = ((unsigned char *) dst) - 1;

  if (n >= 32)
  {
    i = (~ (unsigned long) dst) & 3;

    if (i)
    {
      n -= i;

      do
        deref_auto_inc(cpd) = (unsigned char)v;
      while (--i);
    }

    if (v)
      v |= v << 24 | v << 16 | v <<  8;

    // lpd = ((unsigned long *) (cpd + 1)) - 1;
    dst = ((unsigned long *) (cpd + 1)) - 1;

    i = n >> 5;

    if (i)
      do
      {
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
        deref_auto_inc(lpd) = v;
      }
      while (--i);

    i = (n & 31) >> 2;

    if (i)
      do
        deref_auto_inc(lpd) = v;
      while (--i);

    // cpd = ((unsigned char *) (lpd + 1)) - 1;
    dst = ((unsigned char *) (lpd + 1)) - 1;

    n &= 3;
  }

  if (n)
    do
      deref_auto_inc(cpd) = (unsigned char)v;
    while (--n);

}

/**
 * @internal
 * @brief mwerks MSL simple runtime initialization to avoid bringing in
 *        their entire runtime library
 */
__declspec(section ".init")
static void
  __rom_copy( void )
{
  __rom_copy_info *dci = _rom_copy_info;

  while( !(dci->rom == 0 && dci->addr == 0 && dci->size == 0) )
  {
    if( dci->size && (dci->addr != dci->rom) )
        _memcpy_(dci->addr, dci->rom, dci->size);
    dci++;
  }
}

/**
 * @internal
 * @brief mwerks MSL simple runtime initialization to avoid bringing in
 *        their entire runtime library
 */
__declspec(section ".init")
static void
  __bss_zero( void )
{
  __bss_init_info *bii = _bss_init_info;

  while( !(bii->addr == 0 && bii->size == 0) )
  {
    if( bii->size )
        _fill_mem_(bii->addr, 0, bii->size);
    bii++;
  }
}

#ifdef __cplusplus
/**
 * @internal
 * @brief call all c++ static ctor initializers in the order as laid down by
 *        the linker
 */
__declspec(section ".init")
static void
  __init_cpp( void )
{
  __declspec(section ".init")
  extern vfptr_t _ctors[];                      /*< linker defined */

  vfptr_t *ctor = _ctors;

  while(*ctor) {
    (*ctor)();
    ctor++;
  }
}
#endif

/* --| PUBLIC   |--------------------------------------------------------- */

/**
 * @public
 * @brief startup runtime hook to init .bss/.data and ctor/dtors for c++
 *        apps
 */
__declspec(section ".init")
void
  __init( void )
{
  __rom_copy();   /**< init .data/.sdata sections */
  __bss_zero();   /**< init .bss sections */
#ifdef __cplusplus
  __init_cpp();   /**< call static ctors */
#endif
}