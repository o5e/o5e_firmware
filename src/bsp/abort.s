#
# @file       abort.s
# @headerfile
# @author     sstasiak
# @attention  { not for use in safety critical systems       }
# @attention  { not for use in pollution controlled vehicles }
#
# (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
#

    .public abort

#----------------------------------------------------------
#
#   abort() __attribute((noreturn)) crt hook
#   
#   standard crt behavior on abort() is to bypass any
#   atexit() or destructor calls and die asap which is
#   handled by the __exit() call
#
    .section    .init, text
    .function   "abort", __start_abort, __end_abort-__start_abort
__start_abort:
abort:

    .extern __exit
    lis     r3, __exit@ha
    addi    r3, r3, __exit@l
    mtlr    r3
    blr

__end_abort: