/**
 * @file       <filename.c>
 * @headerfile <filename.h>
 * @author     <author>
 * @brief      <one liner description>
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

void exit( int );
void __exit( void );

/**
 * @internal
 * @brief perform hard reset via SIU
 */
static void
  __siu_reset( void );

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */

/**
 * @internal
 * @brief app/process termination point
 */
__declspec(section ".init")
void
  __exit(void) __attribute((noreturn))
{
  while( 1 )
    __siu_reset();
}

/**
 * @internal
 * @brief perform hard reset via SIU
 */
__declspec(section ".init")
static void
  __siu_reset( void )
{
  /* TODO */
}

/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief manditory stdlib exit() implementation
 * @param[in] int return value, unused, but required for proper runtime
 *       compatibility
 */
__declspec(section ".init")
void
  exit( int return_value ) __attribute((noreturn))
{
  (void)return_value;         /*< unused */

/**
 * msl docs say destructors and functions
 * registered w/atexit() are all stored
 * in the same place .. so the following code
 * chunk should handle c and c++ teardown
 */

#ifdef __cplusplus

  __declspec(section ".init")
  extern vfptr_t _dtors[];    /*< linker defined */

  vfptr_t *dtor = _dtors;
  while(*dtor) {
    (*dtor)();
    dtor++;
  }

#endif

  __exit();
}

// TEST SUPPORT funcs ---------------------------------------------------------
// PRIVATE funcs --------------------------------------------------------------

