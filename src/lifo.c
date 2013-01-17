/**
 * @file       lifo.c
 * @headerfile lifo.h
 * @author     sstasiak
 * @brief      LIFO / singly linked list
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include "lifo.h"
#include "trap.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief push node onto front of list and return new root
 * @param[in] lifo list root
 * @param[in] node new node
 * @retval void* root
 * @example { void *lifo_root;
 *            lifo_t node; 
 *            lifo_root = lifo_push( lifo_root, &node ); }
 */
void *
  lifo_push( void *lifo,
             void *node )
{
  lifo_t *const n = (lifo_t*)node;
  if( n )
    n->next = lifo;
  return n;
}

/**
 * @public
 * @brief pop node from the front of the list
 * @param[in] lifo list root
 * @retval void* popped item
 * @example { void *lifo_root;
 *            node = lifo_pop( &lifo_root ); }
 */
void *
  lifo_pop( void *lifo )
{
  lifo_t **nn = lifo;
  lifo_t *n = *nn;

  if( n )
  {
    *nn = n->next;
    n->next = 0;      /**< not required, but safe             */
  }
  
  return n;
}

/**
 * @public
 * @brief map a function to every item in the list
 * @param[in] lifo list root
 * @param[in] func map function, return 0 on normal operation. returning
 *                 non-zero passes up the current item to the caller
 * @param[in] data data to pass into map function along with each item
 *                 in the list
 * @retval void* 0 or item depending on result of func calls
 */
void *
  lifo_map( void *lifo,
            int (*func)(void *item, void *data),
            void *data )
{
  lifo_t *p = lifo;
  while( p )
  {
    if( func(p, data) )
      return p;
    p = p->next;
  }
  return 0;
}