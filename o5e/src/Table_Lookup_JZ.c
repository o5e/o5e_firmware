/**************************************************************************************************

	@file   	table_lookup_jz.c
	@author 	Jon Zeeff
	@date   	September, 2011
	@brief  	table lookup - fast, generic, 1D, 2D, variable axis, int32_t, binary search 
	@copyright 	MIT license
	@version 	1.7
	
	@note Generic, portable 1D or 2D table lookup
	Can use a fixed interval on the axis (tends to need more points to get accuracy but is slightly faster)
	Table entries are int16_t (which will usually work with unsigned values too) or 8 bit unsigned
	Works with any bin point
	Uses a binary search when doing variable axis increments (faster)
	Linear search is available if you have small tables
	Logs errors if desired
	Does range and error checks
	ISO C90 compatible (yuck, check with "gcc -Wall -pedantic -Wextra")
	
	To use this, you need to fill out a table_jz structure and then pass the lookup 
	routine a pointer to the structure.

        Notes: a 1D table means that rows = 1.  The origin is the upper left and axis values must increase or 
              stay the same as you go down (rows) or to the right (cols).
	
****************************************************************************************************/

/* Copyright (c) 2011, 2012 Jon Zeeff */
/* Copyright (c) 2013 - altered for s32 search */

#include <stdint.h>
#include "Table_Lookup_JZ.h"
#include "err.h"

#ifndef FALSE
#   define FALSE 0
#   define TRUE 1
#endif


static inline uint32_t bsearch_jz (register const int32_t value, const int32_t * const array, uint32_t n);

/* generic binary search for a int32_t array */
/* does not do bounds checking */

static inline uint32_t bsearch_jz(register const int32_t value, const int32_t * const array, uint32_t n)
{
    register uint32_t lower;
    register uint32_t middle;
    register uint32_t upper;

    lower = 0;
    upper = --n;                /* note, n is now max index */

    for (;;) {
        middle = (upper + lower) / 2;   /* 1/2 way between them */

        if (middle == lower)
            break;       	/* done */

        if (array[middle] < value)
            lower = middle;
        else if (array[middle] > value)
            upper = middle;
        else                    /* equal */
            break;
    }
    return middle;     

} // bsearch()

/* Given a fraction, find value between two values */

inline static int32_t interpolate(const unsigned fraction, const int32_t value1, const int32_t value2)
/* Note: fraction is bin 8 and ranges 0 to 1 */
{
    if (value1 == value2 || fraction == 0)      /* for speed */
        return value1;

    if (value1 < value2)        /* postitive slope */
        return value1 + (int32_t)(((value2 - value1) * (int)fraction) / 256);    /* correct back to bin 0  */
    else
        return value1 - (int32_t)(((value1 - value2) * (int)fraction) / 256);    /* correct back to bin 0  */
}

/*  macro to extract value from table - could be s16 or s32 */
#define value(index)   (byte_table !=0 ? (int32_t)(*(int32_t *)(index)) : (*(int32_t *)(index)))

/************************************************************************

@param x value
@param y value (optional)
@param pointer to table structure
@return lookup value from table

************************************************************************/
int table_lookup_jz(const int x, const int y, const struct table_jz * const table)
{
    register uint32_t xbin;     	/* bin 8, the x value converted to an index with fraction */
    register uint32_t ybin;     	/* bin 8, the y value converted to an index with fraction */
    register uint32_t x_index;     /* bin 0 index version of xbin */
    register uint32_t y_index;     /* bin 0 index version of ybin */
    register uint8_t *ptr;      	/* caution: this pointer actually points to int16s or int32s */
    register int value1;
    register uint8_t entry_size;  	/* 0 or 1 for 16 bit or 32 bit lookup */
    int i;

    /* for readability */
#define rows (table->rows)
#define cols (table->cols)
#define variable_axis ((table->variable_axis & 1) != 0)
#define byte_table ((table->byte_table & 1) != 0)

    entry_size = byte_table ? 0U : 1U;  /* 0 = 8 bit, 1 = 16 bit, later used as 2**entry_table */

#define PARANOIA
#ifdef PARANOIA

    /* check for sane values */
    if (table == 0 || rows < 1 || cols < 2 || rows > MAX_ROWS || cols > MAX_COLS) {     /* error check */
        err_push( CODE_OLDJUNK_FC );
        return 255;
    }
    /* check for proper x axis sorting (must be ascending) */
    for (i = 1; i < cols; ++i) {
        if (table->x_axis[i] < table->x_axis[i - 1]) {
            err_push( CODE_OLDJUNK_FB );
            return 15;
        }
    }
    /* check for proper y axis sorting (must be ascending) */
    for (i = 1; i < rows; ++i) {
        if (table->y_axis[i] < table->y_axis[i - 1]) {
            err_push( CODE_OLDJUNK_FA );
            return 15;
        }
    }
#endif

    /* find fractional indexes that point into the table */

    if (!variable_axis) {       	/* use fixed axis increment method */
        /* calc x index by offsetting and scaling */
        if (x <= table->min_x)  	/* less than min  */
            x_index = xbin = 0;
        else if (x >= table->max_x) {   /* greater than max */
            x_index = (uint32_t)(cols - 1);
            xbin = 0;
        } else {                	/* in the middle, calc how far */
            xbin = (uint32_t)(((cols - 1) * (uint64_t) (x - table->min_x) * 256) / (table->max_x - table->min_x));  /* result is bin 8 */
            x_index = (xbin >> 8);      /* convert factional index to integer index */
            xbin &= 0xff;
        }

        /* calc y index by offsetting and scaling */
        if (rows == 1 || y <= table->min_y) {
            y_index = ybin = 0;
            ptr = (uint8_t *) table->data + (x_index << entry_size);
        } else {                	/* find y position in table  */
            if (y >= table->max_y) {
                y_index = (uint32_t)(rows - 1);
                ybin = 0;
            } else {            	/* in the middle */
                ybin = (uint32_t)(((rows - 1) * (uint64_t) (y - table->min_y) * 256) / (table->max_y - table->min_y));      /* result is bin 8 */
                y_index = (ybin >> 8);
                y_index &= 0xff;
            }
            ptr = (uint8_t *) table->data + (((cols * y_index) + x_index) << entry_size);
        }
    } else {                    /* variable axis */

        // check for within table
        if (x <= table->x_axis[0]) {              /* < first */
           x_index = xbin = 0;
        } else if (x >= table->x_axis[cols-1]) {  /* > last */ 
           x_index = (uint32_t)cols - 1;
           xbin = 0;
        } else {
           /* find x index and fractional index */
           x_index = bsearch_jz((int32_t) x, table->x_axis, (uint32_t) cols);
           xbin = (uint32_t)(((x - table->x_axis[x_index]) * 256) / (table->x_axis[x_index + 1] - table->x_axis[x_index]));  /* bin 8 result */
        }

        /* do y axis if this is a 2D lookup */

        if (rows == 1 || y <= table->y_axis[0]) {   /* 1D or < first */
            y_index = ybin = 0;
            ptr = (uint8_t *) table->data + (x_index << entry_size);
        } else if (y >= table->y_axis[rows-1]) {     /* > last */
            y_index = (uint32_t)rows - 1;
            ybin = 0;
            ptr = (uint8_t *) table->data + (x_index << entry_size);
        } else {  /* find y index and fractional index */
            y_index = bsearch_jz((int32_t) y, table->y_axis, (uint32_t) rows);
            ybin = (uint32_t)(((y - table->y_axis[y_index]) * 256) / (table->y_axis[y_index + 1] - table->y_axis[y_index]));
            ptr = (uint8_t *) table->data + (((cols * y_index) + x_index) << entry_size);
        }
    }

    /* now interpolate from current point to next point for accuracy */

    /* interpolate using the factional distance between points  */
    if (x_index + 1 >= cols || xbin == 0)       /* out of bounds on x-axis */
        value1 = value(ptr);                    /* use last point in table */
    else
        value1 = interpolate(xbin, value(ptr), value(ptr + (1 << entry_size)));

    if (rows == 1)              /* 1D lookup, we are done */
        return value1;

    if (y_index + 1 >= rows || ybin == 0)       /* out of bounds (unusual case) */
        return value1;

    {
        /* interpolate using x values from the next row */
        register int value2;

        if (x_index + 1 >= cols)        /* out of bounds on x-axis */
            value2 = value(ptr + (cols << entry_size));
        else
            value2 = interpolate(xbin, value(ptr + (cols << entry_size)), value(ptr + ((cols + 1) << entry_size)));

        /* interpolate between 2 values */
        return (int) interpolate(ybin, (int32_t)value1, (int32_t)value2);
    }
}                               /* table_lookup_jz() */
