#
# @file       __stack_abort.s
# @headerfile
# @author     sstasiak
# @brief      construct a fake return frame for EABI languages
#             to call into the runtime abort()
# @attention  { not for use in safety critical systems       }
# @attention  { not for use in pollution controlled vehicles }
#
# (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
#

    .public __stack_abort

#----------------------------------------------------------
#
#   __stack_abort
#     noreturn
#
    .section    .init, text
    .function   "__stack_abort", __start_stack_abort, __end_stack_abort-__start_stack_abort
__start_stack_abort:
__stack_abort:

    .extern __exit
    lis     r3, __exit@ha
    addi    r3, r3, __exit@l
    mtlr    r3

    li      r3, -2
    blr

__end_stack_abort:
