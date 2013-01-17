/**
 * @file   fifo.h
 * @author sstasiak
 * @brief  FIFO / doubly linked list
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __fifo_h
#define   __fifo_h

#ifdef __cplusplus
extern "C"
{
#endif

typedef union fifo_t fifo_t;   /**< fwd decl convienience    */
union fifo_t                   /**< base, for callers to use */
{
  struct
  {
    fifo_t *next;
    fifo_t *prev;
  };
  struct
  {
    fifo_t *head;
    fifo_t *tail;
  };
};

/**
 * @public
 * @brief initialize fifo root
 * @param[in] fifo list root
 * @retval none
 * @note { not threadsafe }
 */
static inline void
  fifo_init( void *fifo )
{
  fifo_t *const f = (fifo_t*)fifo;
  f->head = 0;
  f->tail = 0;
}

/**
 * @public
 * @brief push node onto front of list
 * @param[in] fifo list root
 * @param[in] node new node
 * @retval none
 * @note { not threadsafe }
 */
void
  fifo_push( void *fifo,
             void *node );

/**
 * @public
 * @brief pop node from the back of the list
 * @param[in] fifo list root
 * @retval void* popped node
 * @note { not threadsafe }
 */
void *
  fifo_pop( void *fifo );

/**
 * @public
 * @brief map a function to every item in the list starting from
 *        the front
 * @param[in] fifo list root
 * @param[in] func map function, return 0 on normal operation. returning
 *                 non-zero passes up the current item to the caller
 * @param[in] data data to pass into map function along with each item
 *                 in the list
 * @retval void* 0 or item depending on result of func calls
 * @note { not threadsafe }
 */
void *
  fifo_map( void *fifo,
            int (*func)(void *item, void *data),
            void *data );

#ifdef __cplusplus
}
#endif

#endif // __fifo_h