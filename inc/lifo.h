/**
 * @file   lifo.h
 * @author sstasiak
 * @brief  LIFO / singly linked list
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __lifo_h
#define   __lifo_h

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct lifo_t lifo_t;   /**< fwd decl convienience    */
struct lifo_t                   /**< base, for callers to use */
{
  lifo_t *next;
};

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
             void *node );

/**
 * @public
 * @brief pop node from the front of the list
 * @param[in] lifo list root
 * @retval void* popped item
 * @example { void *lifo_root;
 *            node = lifo_pop( &lifo_root ); }
 */
void *
  lifo_pop( void *lifo );

/**
 * @public
 * @brief map a function to every item in the list
 * @param[in] lifo list root
 * @param[in] func map function, return 0 on normal operation. returning
 *                 non-zero passes up the current item to the caller
 * @param[in] data data to pass into map function along with each item
 *                 in the list
 * @retval void* 0 or item depending on result of func calls
 * @note { not threadsafe }
 */
void *
  lifo_map( void *lifo,
            int (*func)(void *item, void *data),
            void *data );

#ifdef __cplusplus
}
#endif

#endif // __lifo_h