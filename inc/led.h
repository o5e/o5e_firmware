/**
 * @file   led.h
 * @author sstasiak
 * @brief  led control
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __led_h
#define   __led_h

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @public
 * @enum led_t
 */
typedef enum led_t
{
  LED0,
  LED1,
  LED2,
  LED3
} led_t;

/**
 * @public
 * @brief init all leds to default state
 * @param none
 * @retval none
 */
void
  led_init( void );

/**
 * @public
 * @brief turn on led
 * @param[in] l desired led
 * @retval none
 */
void
  led_on( led_t l );

/**
 * @public
 * @brief turn off led
 * @param[in] l desired led
 * @retval none
 */
void
  led_off( led_t l );

/**
 * @public
 * @brief invert led state
 * @param[in] l desired led
 * @retval none
 */
void
  led_invert( led_t l );

/**
 * @public
 * @brief lower 4 bits are driven to leds
 * @param[in] l integer in the range of 0 to 15
 * @retval none
 */
void
  led_set( unsigned l );

#ifdef __cplusplus
}
#endif

#endif // __led_h