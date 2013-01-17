/**
 * @file       bsp_vector_install.c
 * @headerfile bsp.h
 * @author     sstasiak
 * @brief      install caller suppplied vector handler dynamically
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */
 
#include <stdint.h>
#include "trap.h"
#include "bsp.h"
#include "bsp_prv.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
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
                      vector_fptr_t vector_fptr )
{
  extern vector_fptr_t vectors[210];   /**< ref to table in vector_table.c */

  trap( vector_fptr != 0 );
  trap( vector < sizeof(vectors)/sizeof(vector_fptr_t) ||
        vector != 147 || vector != 148 || vector != 150 ||
        vector != 151 || vector != 154 || vector != 175 ||
        vector != 194 || vector != 195 || vector != 196 ||
        vector != 197 || vector != 198 || vector != 199 );

  bsp_declare_state();
  bsp_disable_interrupts();           /**< atomic swap                     */
  vector_fptr_t const prev_fptr = vectors[vector];
  vectors[vector] = vector_fptr;
  bsp_enable_interrupts();

  return prev_fptr;
}