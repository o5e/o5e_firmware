/**
 * @file       fifo.c
 * @headerfile fifo.h
 * @author     sstasiak
 * @brief      FIFO / doubly linked list
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include "fifo.h"
#include "trap.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
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
             void *node )
{
  trap( fifo );
  trap( node );
  fifo_t *const f = fifo;
  fifo_t *const n = node;

  n->prev = 0;                      /**< inserting into front             */
  if( f->head || f->tail )          /**< 1+ nodes in list                 */
  {
    trap( f->head->prev == 0 );     /**< current head node shouldn't be   */
                                    /*   pointing to anything             */
    f->head->prev = n;              /**< point head node to new node      */
    n->next = f->head;              /**< point new node back to head node */
    f->head = n;                    /**< push to head                     */
  }
  else                              /**< first node                       */
  {
    f->head = f->tail = n;
    n->next = 0;                    /**< must terminate                   */
  }
}

/**
 * @public
 * @brief pop node from the back of the list
 * @param[in] fifo list root
 * @retval void* popped node
 * @note { not threadsafe }
 */
void *
  fifo_pop( void *fifo )
{
  trap( fifo );
  fifo_t *const f = fifo;
  fifo_t *n = 0;

  if( f->head || f->tail )          /**< 1+ nodes in list                 */
  {
    n = f->tail;                    /**< pop from tail                    */
    trap( n->next == 0 );           /**< last node shouldn't have been    */
                                    /*   pointing to anything next        */
    if( f->head != f->tail )        /**< 2+ nodes in list ?               */
    {
      f->tail = n->prev;            /**< safe to update tail              */
      f->tail->next = 0;            /**< terminate list properly          */
      n->prev = 0;                  /**< not required, but safe           */
    }
    else                            /**< that was last node in the list   */
    {
      trap( n->prev == 0 );         /**< which means it shouldn't have    */
                                    /*   been pointing to anything prev   */
      f->tail = f->head = 0;        /**< now empty                        */
    }
  }

  return n;
}

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
            void *data )
{
  trap( fifo );
  fifo_t *const f = fifo;
  fifo_t *n = f->head;
  while( n )
  {
    if( func(n, data) )
      return n;
    n = n->next;
  }
  return 0;
}