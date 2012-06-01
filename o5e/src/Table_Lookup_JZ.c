/**************************************************************************************************

	@file   	table_lookup_jz.c
	@author 	Jon Zeeff
	@date   	September, 2011
	@brief  	table lookup - fast, generic, 1D, 3D, variable axis, fixed axis, int16_t or uint8_t, binary search 
	@copyright 	MIT license
	@version 	1.5
	
	@note Generic, portable 1D or 2D table lookup
	Can use a fixed interval on the axis (tends to need more points to get accuracy but is faster)
	Table entries are int16_t (which will usually work with unsigned values too) or 8 bit unsigned
	Works with any bin point
	Uses a binary search when doing variable axis increments (faster)
	Linear search is available if you have small tables
	Logs errors if desired
	Does range and error checks
	ISO C90 compatible (yuck, check with "gcc -Wall -pedantic -Wextra")
	
	To use this, you need to fill out a table_jz structure and then pass the lookup 
	routine a pointer to the structure.

        Note: a 1D table means that rows = 1.  The origin is the upper left and axis values must increase or 
              stay the same as you go down (rows) or to the right (cols).
	
****************************************************************************************************/

/* 

Copyright (c) 2011 Jon Zeeff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdint.h>
#include "Table_Lookup_JZ.h"
#include "main.h"

/* Use this if you don't have error logging */
/* #define system_error(a,b,c,d) */   /* a routine to log a string */
/*  #define MAIN if you want to test this as a standalone program on a PC */
/* #define MAIN   */
/* #define TEST if you want some printout */
/* #define TEST   */

#ifndef FALSE
#   define FALSE 0
#   define TRUE 1
#endif

#ifdef MAIN
#   include <stdio.h>
#   include <memory.h>

/* example of how to define a table in rom or flash */

const struct table_3x3_16 table1 = {
    .rows = 3,
    .cols = 3,
    .min_x = 0,
    .max_x = 1000,
    .min_y = 0,
    .max_y = 100,
    .variable_axis = FALSE,     /* default is zero anyway */
    .byte_table = FALSE,        /* default is zero anyway */
    /* data, a full row on each line: */
    .data = {
             {1, 2, 3},
             {4, 5, 6},
             {7, 8, 9}}
};

int main()
{
    /* Example of use */
    int32_t i;
    int j;

/* This is the CLT table in binary form as it comes from the ecu.  Useful for testing table lookups. */

    uint8_t table[] = { 0x0, 0x1, 0x0, 0x10,
        0x0, 0x0, 0x5, 0x54, 0xa, 0xac, 0x10, 0x0, 0x15, 0x54, 0x1a, 0xac, 0x20, 0x0, 0x25, 0x54,
        0x2a, 0xac, 0x30, 0x0, 0x35, 0x54, 0x3a, 0xac, 0x40, 0x0, 0x45, 0x54, 0x4a, 0xac, 0x50, 0x0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 

        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

        0xff, 0xfe,   //  variable axis = yes, byte table = no
        0xff, 0xff,   //  filler 
        0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, // data
        0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0, 0x40, 0x0
    };

    /* swap from big endian */
    for (i = 0; i < 168; i += 2) {
        uint8_t j;
        if (i == 4 + 64 + 64)
            continue;           /* no need to swap these two byte elements */
        j = table[i];
        table[i] = table[i + 1];
        table[i + 1] = j;
    }

    //for (i = 0; i < 1000000; ++i)
    //    j += table_lookup_jz(11000, 0, (const struct table_jz *)table);

    printf("f(11000) = %d\n", table_lookup_jz(11000, 0, (const struct table_jz *)table));
    printf("j = %d\n", j);

    return 1;
}                               /* main() */
#endif

/* lsearch() is usually slower */
#if 0
/* given a value, n and a sorted array of n int16_t elements, return the index to the entry that is less than or equal to value */

inline unsigned lsearch_jz(register const int value, const int16_t * array, uint_fast16_t n)
{
    register int16_t *ptr = (int16_t *) array + --n;    /* move to last position in array */

#   ifdef TEST
    printf("lsearch %d %d %d for %d\n", array[0], array[1], array[2], value);
#   endif

    while (*ptr > value)
        --ptr;                  /* loop from large to small */

    return (unsigned)(ptr - array);
}
#endif

/* generic binary search for a int16_t array */
/* does not do bounds checking */

inline uint_fast16_t bsearch_jz(register const int16_t value, const int16_t * array, uint_fast16_t n)
{
    register uint_fast16_t lower;
    register uint_fast16_t middle;
    register uint_fast16_t upper;

#ifdef TEST
    printf("bsearch %d %d %d ... for %d\n", array[0], array[1], array[2], value);
#endif

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

#ifdef BSEARCH_TEST

#   include <stdio.h>

int main()
{
    int16_t array[6] = { 1, 5, 10, 15, 20, 29 };

    printf("array = 1, 5, 10, 15, 20, 29\n");
    printf("6 = %d\n", bsearch(6, array, 6));
    printf("0 = %d\n", bsearch(0, array, 6));
    printf("30 = %d\n", bsearch(30, array, 6));
    printf("1 = %d\n", bsearch(1, array, 6));
    printf("20 = %d\n", bsearch(20, array, 6));
    printf("17 = %d\n", bsearch(17, array, 6));

    return 0;
}

#endif
/* Given a fraction, find value between two values */

inline static int_fast16_t interpolate(const unsigned fraction, const int_fast16_t value1, const int_fast16_t value2)
/* Note: fraction is bin 8 and ranges 0 to 1 */
{
#ifdef TEST
    printf("interpolate %f between %d and %d\n", fraction / 256., (int)value1, (int)value2);
#endif
    if (value1 == value2 || fraction == 0)      /* for speed */
        return value1;

    if (value1 < value2)        /* postitive slope */
        return value1 + (((value2 - value1) * (int)fraction) / 256);    /* correct back to bin 0  */
    else
        return value1 - (((value1 - value2) * (int)fraction) / 256);    /* correct back to bin 0  */
}

/*  macro to extract value from table - could be u8 or s16 */
#define value(index)   (byte_table !=0 ? (int16_t)(*(uint8_t *)(index)) : (*(int16_t *)(index)))

/************************************************************************

@param x value
@param y value (optional)
@param pointer to table structure
@return lookup value from table

************************************************************************/
int table_lookup_jz(const int x, const int y, const struct table_jz *table)
{
    register uint_fast16_t xbin;     	/* bin 8, the x value converted to an index with fraction */
    register uint_fast16_t ybin;     	/* bin 8, the y value converted to an index with fraction */
    register uint_fast16_t x_index;     /* bin 0 index version of xbin */
    register uint_fast16_t y_index;     /* bin 0 index version of ybin */
    register uint8_t *ptr;      	/* caution: this pointer actually points to uint8s or int16s */
    register int value1;
    register uint_fast8_t entry_size;  	/* 0 or 1 for 8 bit or 16 bit lookup */
    int i;

    /* for readability */
#define rows (table->rows)
#define cols (table->cols)
#define variable_axis ((table->variable_axis & 1) != 0)
#define byte_table ((table->byte_table & 1) != 0)

    entry_size = byte_table ? 0U : 1U;  /* 0 = 8 bit, 1 = 16 bit, later used as 2**entry_table */

#ifdef TEST
    printf("\ntable lookup of %d, %d\n", x, y);
    printf("rows %d, cols %d, max rows %d, max cols %d\n", rows, cols, MAX_ROWS, MAX_COLS);
    printf("variable axis = %d, byte_table = %d\n", variable_axis, byte_table);
    printf("x-axis from %d to %d\n", table->x_axis[0], table->x_axis[cols - 1]);
    for (i = 0; i < cols; ++i)
        printf("col[%d] = %u\n", i, table->x_axis[i]);
    for (i = 0; i < cols; i += 1)
        printf("data[%d] = %u\n", i, *(int16_t *) & table->data[i*2]);
#endif

#define PARANOIA
#ifdef PARANOIA

    /* check for sane values */
    if (table == 0 || rows < 1 || cols < 2 || rows > MAX_ROWS || cols > MAX_COLS) {     /* error check */
        system_error(3879, __FILE__, __LINE__, "");
        return 255;
    }
    /* check for proper x axis sorting (must be ascending) */
    for (i = 1; i < cols; ++i) {
        if (table->x_axis[i] < table->x_axis[i - 1]) {
            system_error(3880, __FILE__, __LINE__, "");
            return 15;
        }
    }
    /* check for proper y axis sorting (must be ascending) */
    for (i = 1; i < rows; ++i) {
        if (table->y_axis[i] < table->y_axis[i - 1]) {
            system_error(3881, __FILE__, __LINE__, "");
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
            x_index = (uint_fast16_t)(cols - 1);
            xbin = 0;
        } else {                	/* in the middle, calc how far */
            xbin = ((cols - 1) * (uint32_t) (x - table->min_x) * 256) / (table->max_x - table->min_x);  /* result is bin 8 */
            x_index = (xbin >> 8);      /* convert factional index to integer index */
            xbin &= 0xff;
        }

        /* calc y index by offsetting and scaling */
        if (rows == 1 || y <= table->min_y) {
            y_index = ybin = 0;
            ptr = (uint8_t *) table->data + (x_index << entry_size);
        } else {                	/* find y position in table  */
            if (y >= table->max_y) {
                y_index = (uint_fast16_t)(rows - 1);
                ybin = 0;
            } else {            	/* in the middle */
                ybin = ((rows - 1) * (uint32_t) (y - table->min_y) * 256) / (table->max_y - table->min_y);      /* result is bin 8 */
                y_index = (ybin >> 8);
                y_index &= 0xff;
            }
            ptr = (uint8_t *) table->data + (((cols * y_index) + x_index) << entry_size);
        }
    } else {                    /* variable axis */

        // check for within table
        if (x < table->x_axis[0]) {              /* < first */
           x_index = xbin = 0;
        } else if (x > table->x_axis[cols-1]) {  /* > last */ 
           x_index = cols - 1;
           xbin = 0;
        } else {
           /* find x index and fractional index */
           x_index = bsearch_jz((int16_t) x, table->x_axis, (uint_fast16_t) cols);
           xbin = (unsigned)((x - table->x_axis[x_index]) * 256) / (table->x_axis[x_index + 1] - table->x_axis[x_index]);  /* bin 8 result */
        }

        /* do y axis if this is a 2D lookup */

        if (rows == 1 || y <= table->y_axis[0]) {   /* 1D or < first */
            y_index = ybin = 0;
            ptr = (uint8_t *) table->data + (x_index << entry_size);
        } else if (y > table->y_axis[rows-1]) {     /* > last */
            y_index = rows - 1;
            ybin = 0;
            ptr = (uint8_t *) table->data + (x_index << entry_size);
        } else {  /* find y index and fractional index */
            y_index = bsearch_jz((int16_t) y, table->y_axis, (uint_fast16_t) rows);
            ybin = (unsigned)((y - table->y_axis[y_index]) * 256) / (table->y_axis[y_index + 1] - table->y_axis[y_index]);
            ptr = (uint8_t *) table->data + (((cols * y_index) + x_index) << entry_size);
        }
    }

    /* now interpolate from current point to next point for accuracy */

#ifdef TEST
    printf("variable axis  = %d\n", variable_axis);
    printf("x index = %d\n", (int)x_index);
    printf("fraction = %f\n", xbin / 256.0);
    printf("y index  = %f\n", y_index + ybin / 256.0);
    printf("data[xindex][yindex] = %d\n", value(ptr));
    printf("data[xindex+1][yindex] = %d\n", value(ptr + (1 << entry_size)));
#endif

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

#ifdef TEST
        printf("after x interpolations, got %d and %d\n", value1, value2);
#endif

        /* interpolate between 2 values */
        return (int) interpolate(ybin, (int_fast16_t)value1, (int_fast16_t)value2);
    }
}                               /* table_lookup_jz() */
