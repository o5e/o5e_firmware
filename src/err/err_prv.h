/**
 * @file   err_prv.h
 * @author sean
 * @brief  locals/private/inlines for err_ usage
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __err_prv_h
#define   __err_prv_h

#ifdef __cplusplus
extern "C"
{
#endif

#define ERR_LED   LED3        /**< default LED to indicate code is present */

struct err_t                  /**< err item                                */
{
  fifo_t;                     /**< required super to leverage fifo api     */
  uint32_t code;
  uint64_t ts;
};

extern err_t err_pool[ERR_DEPTH];
extern err_t *err_free_root;              /**< pool of free err_t blocks   */
extern fifo_t err_fifo_root;              /**< code fifo stack             */

#ifdef __cplusplus
}
#endif

#endif // __err_prv_h