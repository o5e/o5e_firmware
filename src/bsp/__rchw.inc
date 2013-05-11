#
# @file       __rchw.inc
# @author     sstasiak
# @brief      gc reset configuration halfword
# @attention  { not for use in safety critical systems       }
# @attention  { not for use in pollution controlled vehicles }
#
# (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
#
#----------------------------------------------------------
#
#   rchw fields and their descriptions:
#

# Watchdog timer enable:
#   This bit determines if the software watchdog timer is enabled after passing
#   control to the user application code.
#
# SWT_OFF - Disable software watchdog timer
# SWT_ON  - Software watchdog timer maintains its default state out of reset,
#           i.e. enabled. The timeout period is programmed to be 261600 system
#           clocks.
SWT_OFF     .equ    (0 << 27)
SWT_ON      .equ    (1 << 27)

# Device core Watchdog timer enable:
#   This bit determines if the core software watchdog timer is enabled after
#   passing control to the user application code.
#
# WTE_OFF - Disable core software watchdog timer
# WTE_ON  - Software watchdog timer maintains its default state out of reset,
#           i.e. enabled. The timeout period is programmed to be 2.5*2^17 
#           system clocks.
WTE_OFF     .equ    (0 << 26)
WTE_ON      .equ    (1 << 26)

# Port size:
#   Defines the width of the data bus connected to the memory on CS0. After
#   system reset, CS0 is changed to a 16-bit port by the BAM, which fetches
#   the RCHW from either 16- or 32-bit external memories. Then the BAM
#   reconfigures the EBI as a 16-bit bus or a 32-bit bus, according to the
#   settings of this bit.
#
# PS0_32BITS - 32-bit CS0 port size.
# PS0_16BITS - 16-bit CS0 port size.
PS0_32BITS  .equ    (0 << 25)
PS0_16BITS  .equ    (1 << 25)

# VLE Code Indicator:
#   This bit is used to configure the MMU entries 1-3 coded as either Classic
#   Book E instructions or as Freescale VLE instructions.
#
# VLE_OFF - User code executes as classic Book E code
# VLE_ON  - User code executes as Freescale VLE code
VLE_OFF     .equ    (0 << 24)
VLE_ON      .equ    (1 << 24)

# Boot identifier:
#   This field serves two functions. First, it is used to indicate which
#   block in flash memory contains the boot program. Second, it identifies
#   whether the flash memory is programmed or invalid. The value of a
#   valid boot identifier is 0x5A.
BOOT_ID     .equ    (0x5A << 16)

#----------------------------------------------------------
#
#   internal boot rchw configuration options:
#
SWT_OPT     .equ    SWT_OFF
WTE_OPT     .equ    WTE_OFF
PS0_OPT     .equ    PS0_32BITS
VLE_OPT     .equ    VLE_OFF
