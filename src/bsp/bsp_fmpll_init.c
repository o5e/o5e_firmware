/**
 * @file       bsp_fmpll_init.c
 * @headerfile
 * @author     sstasiak
 * @brief      init 5634 fmpll on startup
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

void bsp_fmpll_init( void );

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public but only called on startup
 * @brief startup fmpll to 80MHz for OEM hardware on a 5634 - nothing more
 * @param none
 * @retval none
 * @attention NO STACK AVAILABLE for your use here
 */
__declspec(section ".init")
asm void
  bsp_fmpll_init( void )
{
    nofralloc
    
    /* MHz :    80 70 60 50 40 30 20 10   */
    /* ESYNCR1: 40 35 60 50 40 60 40 1,40 */
    /* ESYNCR2: 1  1  2  2  2  3  3  3    */

    lis   r3, 0xC3F8          /**< loadup PLL region base       */
    lis   r4, 0xF000          /**< EPREDIV -> 0-1 to 1110-15    */
    ori   r4, r4, 40          /**< EMFD -> 32 to 96             */
    stw   r4, 8(r3)           /**< write ESYNCR1                */
    li    r4, 1               /**< ERFD -> 0-2,4,8 and 11-16    */
    stw   r4, 12(r3)          /**< write ESYNCR2                */
@1: lwz   r4, 4(r3)           /**< check SYNSR, wait lock       */
    andi. r4, r4, (1<<3)      /**< is LOCK set? ...             */
    beq   @1                  /**< ... wait till set            */

    blr
}