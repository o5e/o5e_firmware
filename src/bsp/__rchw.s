#
# @file       __rchw.s
# @headerfile __rchw.inc
# @author     sstasiak
# @brief      gc reset configuration halfword
# @attention  { not for use in safety critical systems       }
# @attention  { not for use in pollution controlled vehicles }
#
# (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
#

    .include    __rchw.inc
    .extern     __start       # crt entry point
    .public     __rchw        # export symbol to avoid deadstrip of section

#----------------------------------------------------------
#
#   BAM reset configuration halfword definition:
#
    .section    .rchw, rodata
__rchw:
    .long       BOOT_ID | SWT_OPT | WTE_OPT | PS0_OPT | VLE_OPT
    .long       __start