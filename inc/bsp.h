/**
 * @file   bsp.h
 * @author sstasiak
 * @brief  some generic platform interactions to ease the urge
 *         of directy touching hardware
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __bsp_h
#define   __bsp_h

#ifdef __cplusplus
extern "C"
{
#endif

#define bsp_declare_state()         register int __msr_state
#define bsp_enable_interrupts()     asm { wrtee  __msr_state; }
#define bsp_disable_interrupts()    asm { mfmsr  __msr_state; \
                                          wrteei 0; }

/**
 * @brief system time in milliseconds since power on.
 * @warning { READ-ONLY }
 */
extern uint32_t systime;

/**
 * @public
 * @brief Final initialization before dropping into mainline code
 * @details Safe for c/c++ since the runtime is already up and any static
 *          classes are already initialized. Typically, this is the final
 *          step before dropping into main()
 */
void
  bsp_init( void );

/**
 * @public
 * @brief atomic fetch of entire timebase
 * @note runs at core clock frequency
 * @retval uint64_t current time/**
 * @public
 * @brief decrementer init hook, override as desired
 */
void
  bsp_decr_init( void );

/**
 * @public
 * @brief atomic fetch of entire timebase
 * @note runs at core clock frequency
 * @retval uint64_t current time
 */
uint64_t
  bsp_get_timebase( void );

/**
 * @public
 * @brief fetch only the lower timebase
 * @note runs at core clock frequency
 * @retval uint32_t current time
 */
uint32_t
  bsp_get_timebase_lower( void );

/**
 * @brief default ivor handler, override as desired
 */
void
  default_ivor_handler( void );

/**
 * @brief default ivor0 handler, override as desired
 * @note Critical input (SPR 400)
 */
void
  ivor0_handler( void );

/**
 * @brief default ivor1 handler, override as desired
 * @note Machine check interrupt (SPR 401)
 */
void
  ivor1_handler( void );

/**
 * @brief default ivor2 handler, override as desired
 * @note Data storage interrupt (SPR 402)
 */
void
  ivor2_handler( void );

/**
 * @brief default ivor3 handler, override as desired
 * @note Instruction storage interrupt (SPR 403)
 */
void
  ivor3_handler( void );

/**
 * @brief default ivor4 handler, override as desired
 * @note  External input interrupt (SPR 404)
 */
void
  ivor4_handler( void );

/**
 * @brief default ivor5 handler, override as desired
 * @note Alignment interrupt (SPR 405)
 */
void
  ivor5_handler( void );

/**
 * @brief default ivor6 handler, override as desired
 * @note Program interrupt (SPR 406)
 */
void
  ivor6_handler( void );

/**
 * @brief default ivor7 handler, override as desired
 * @note Floating-point unavailable interrupt (SPR 407)
 */
void
  ivor7_handler( void );

/**
 * @brief default ivor8 handler, override as desired
 * @note System call interrupt (SPR 408)
 */
void
  ivor8_handler( void );

/**
 * @brief default ivor9 handler, override as desired
 * @note Auxiliary processor (SPR 409)
 */
void
  ivor9_handler( void );

/**
 * @brief default ivor10 handler, override as desired
 * @note Decrementer interrupt (SPR 410)
 */
void
  ivor10_handler( void );

/**
 * @brief default ivor11 handler, override as desired
 * @note Fixed-interval timer interrupt (SPR 411)
 */
void
  ivor11_handler( void );

/**
 * @brief default ivor12 handler, override as desired
 * @note Watchdog timer interrupt (SPR 412)
 */
void
  ivor12_handler( void );

/**
 * @brief default ivor13 handler, override as desired
 * @note Data TLB error interrupt (SPR 413)
 */
void
  ivor13_handler( void );

/**
 * @brief default ivor14 handler, override as desired
 * @note Instruction TLB error interrupt (SPR 414)
 */
void
  ivor14_handler( void );

/**
 * @brief default ivor15 handler, override as desired
 * @note Debug interrupt (SPR 415)
 */
void
  ivor15_handler( void );

/**
 * @brief default ivor32 handler, override as desired
 * @note SPE APU unavailable interrupt (SPR 528)
 */
void
  ivor32_handler( void );

/**
 * @brief default ivor33 handler, override as desired
 * @note SPE floating-point data exception interrupt (SPR 529)
 */
void
  ivor33_handler( void );

/**
 * @brief default ivor34 handler, override as desired
 * @note SPE floating-point round exception interrupt (SPR 530)
 */
void
  ivor34_handler( void );
  
/**
 * @public
 * @typedef vector_fptr_t
 * @brief default interrupt handler function type
 */
typedef void(*vector_fptr_t)(void);

/**
 * @public
 * @brief set a user defined vector handler
 * @param[in] vector vector number as specified by RM
 * @param[in] vector_fptr word aligned handler
 * @retval vector_fptr_t reference to previous handler
 * @note priority is set in @see { bsp_vector_set_pri() }
 */
vector_fptr_t
  bsp_vector_install( int vector,
                      vector_fptr_t vector_fptr );

/**
 * @public
 * @brief set priority of designated vector
 * @param[in] vector vector number as specified by RM
 * @param[in] pri priority of 0 to 15
 * @retval none
 */
void
  bsp_vector_set_pri( int vector,
                      unsigned pri );

#ifdef __cplusplus
}
#endif

#endif // __bsp_h