/**
 * @file   err.h
 * @author sstasiak
 * @brief  internal error reporting api
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __err_h
#define   __err_h

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * [     16bits        ][   8bits   ][    8bits     ]
 * [                ...][ module_id ][   code_id    ]
 *                  |||
 *                  ||+- fatal
 *                  |+-- recoverable
 *                  +--- information
 */

#define _CODE( mod_, code_ )    ( ((mod_&0xfful) << 8) | (code_&0xfful) )

#define ALL_MODULES             (  1 )
#define xxxxx_MODULE            (  2 )
#define yyyyy_MODULE            (  3 )
#define TUNER_MODULE            (  4 )

#define GLOBAL_CODE( code_ )        ( _CODE( ALL_MODULES, code_ ) )
#define xxxxx_CODE( code_ )         ( _CODE( xxxxx_MODULE, code_ ) )
#define yyyyy_CODE( code_ )         ( _CODE( yyyyy_MODULE, code_ ) )
#define TUNER_CODE( code_ )         ( _CODE( TUNER_MODULE, code_ ) )

#define MODULE( code_ )             ( (code_&0x0000ff00) >> 8 )

#define FATAL_BIT                   ( 1<<16 )
#define RECOVERABLE_BIT             ( 1<<17 )
#define INFO_BIT                    ( 1<<18 )

#define FATAL( code_ )              ( code_ | FATAL_BIT )
#define RECOVERABLE( code_ )        ( code_ | RECOVERABLE_BIT )
#define INFO( code_ )               ( code_ | INFO_BIT )

#define IS_FATAL( code_ )           ( code_ & FATAL_BIT )
#define IS_RECOVERABLE( code_ )     ( code_ & RECOVERABLE_BIT )
#define IS_INFO( code_ )            ( code_ & INFO_BIT )

/**
 * @public Unique error codes, don't invent your own willy nilly
 *         without putting them here first. If someone needs to
 *         lookup an error code, this is the first place they will
 *         stop
 */
enum
{
  CODE_NONE = 0,

  CODE_NO_MAP                 = INFO(GLOBAL_CODE( 0x00 )),
  CODE_OLDJUNK_FF             = FATAL(GLOBAL_CODE( 0xFF )),   /**< replace while 'gut' */
  CODE_OLDJUNK_FE             = FATAL(GLOBAL_CODE( 0xFE )),   /**< replace while 'gut' */
  CODE_OLDJUNK_FD             = FATAL(GLOBAL_CODE( 0xFD )),   /**< replace while 'gut' */
  CODE_OLDJUNK_FC             = FATAL(GLOBAL_CODE( 0xFC )),   /**< replace while 'gut' */
  CODE_OLDJUNK_FB             = FATAL(GLOBAL_CODE( 0xFB )),   /**< replace while 'gut' */
  CODE_OLDJUNK_FA             = FATAL(GLOBAL_CODE( 0xFA )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F9             = FATAL(GLOBAL_CODE( 0xF9 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F8             = FATAL(GLOBAL_CODE( 0xF8 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F7             = FATAL(GLOBAL_CODE( 0xF7 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F6             = FATAL(GLOBAL_CODE( 0xF6 )),   /**< replace while 'gut' */
//CODE_OLDJUNK_F5             = FATAL(GLOBAL_CODE( 0xF5 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F4             = FATAL(GLOBAL_CODE( 0xF4 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F3             = FATAL(GLOBAL_CODE( 0xF3 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F2             = FATAL(GLOBAL_CODE( 0xF2 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F1             = FATAL(GLOBAL_CODE( 0xF1 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_F0             = FATAL(GLOBAL_CODE( 0xF0 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_EF             = FATAL(GLOBAL_CODE( 0xEF )),   /**< replace while 'gut' */
  CODE_OLDJUNK_EE             = FATAL(GLOBAL_CODE( 0xEE )),   /**< replace while 'gut' */
  CODE_OLDJUNK_ED             = FATAL(GLOBAL_CODE( 0xED )),   /**< replace while 'gut' */
  CODE_OLDJUNK_EC             = FATAL(GLOBAL_CODE( 0xEC )),   /**< replace while 'gut' */
  CODE_OLDJUNK_EB             = FATAL(GLOBAL_CODE( 0xEB )),   /**< replace while 'gut' */
  CODE_OLDJUNK_EA             = FATAL(GLOBAL_CODE( 0xEA )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E9             = FATAL(GLOBAL_CODE( 0xE9 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E8             = FATAL(GLOBAL_CODE( 0xE8 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E7             = FATAL(GLOBAL_CODE( 0xE7 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E6             = FATAL(GLOBAL_CODE( 0xE6 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E5             = FATAL(GLOBAL_CODE( 0xE5 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E4             = FATAL(GLOBAL_CODE( 0xE4 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E3             = FATAL(GLOBAL_CODE( 0xE3 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E2             = FATAL(GLOBAL_CODE( 0xE2 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E1             = FATAL(GLOBAL_CODE( 0xE1 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_E0             = FATAL(GLOBAL_CODE( 0xE0 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_DF             = FATAL(GLOBAL_CODE( 0xDF )),   /**< replace while 'gut' */
  CODE_OLDJUNK_DE             = FATAL(GLOBAL_CODE( 0xDE )),   /**< replace while 'gut' */
  CODE_OLDJUNK_DD             = FATAL(GLOBAL_CODE( 0xDD )),   /**< replace while 'gut' */
  CODE_OLDJUNK_DC             = FATAL(GLOBAL_CODE( 0xDC )),   /**< replace while 'gut' */
  CODE_OLDJUNK_DB             = FATAL(GLOBAL_CODE( 0xDB )),   /**< replace while 'gut' */
  CODE_OLDJUNK_DA             = FATAL(GLOBAL_CODE( 0xDA )),   /**< replace while 'gut' */
  CODE_OLDJUNK_D9             = FATAL(GLOBAL_CODE( 0xD9 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_D8             = FATAL(GLOBAL_CODE( 0xD8 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_D7             = FATAL(GLOBAL_CODE( 0xD7 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_D6             = FATAL(GLOBAL_CODE( 0xD6 )),   /**< replace while 'gut' */
  CODE_OLDJUNK_D5             = FATAL(GLOBAL_CODE( 0xD5 )),   /**< replace while 'gut' */

  CODE_xxxxx_FATAL            = FATAL(xxxxx_CODE( 0x00 )),
  CODE_xxxxx_RECOVERABLE      = RECOVERABLE(xxxxx_CODE( 0x01 )),
  CODE_xxxxx_INFO             = INFO(xxxxx_CODE( 0x02 )),

  CODE_yyyyy_FATAL            = FATAL(yyyyy_CODE( 0x00 )),
  CODE_yyyyy_RECOVERABLE      = RECOVERABLE(yyyyy_CODE( 0x01 )),
  CODE_yyyyy_INFO             = INFO(yyyyy_CODE( 0x02 )),

  CODE_TUNER_                 = INFO(TUNER_CODE( 0x00 )),

};

#define ERR_DEPTH     32                     /**< depth of code FIFO stack */
typedef struct err_t err_t;                                  /**< fwd decl */

/**
 * @public
 * @brief init code logger
 * @retval none
 * @note { not threadsafe }
 */
void
  err_init( void );

/**
 * @public
 * @brief push an error code into the stack
 * @param[in] code error code
 * @retval none
 * @note { threadsafe }
 */
void
  err_push( uint32_t code );

/**
 * @public
 * @brief pop oldest error from the stack
 * @param none
 * @retval err_t* err or 0 if none
 * @note { threadsafe }
 */
err_t const *
  err_pop( void );

/**
 * @public
 * @brief extract timestamp from err
 * @param[in] e err
 * @retval uint64_t timestamp
 */
uint64_t
  err_get_ts( err_t const *e );
  
/**
 * @public
 * @brief extract err code from err
 * @param[in] e err
 * @retval uint32_t code
 */
uint32_t
  err_get_code( err_t const *e );

/**
 * @public
 * @brief return err_t back to the free pool
 * @param[in] e previously pop'd err
 * @retval none
 * @note { threadsafe }
 */
void
  err_destroy( err_t const *e );

#ifdef __cplusplus
}
#endif

#endif // __err_h