#
# @file       __start.s
# @headerfile
# @author     sstasiak
# @brief      gc reset entry point
# @attention  { not for use in safety critical systems       }
# @attention  { not for use in pollution controlled vehicles }
#
# (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
#

    .public __start

#----------------------------------------------------------------------------
#
#   hard reset entry point
#
    .section    .init, text
    .function   "__start", __start_entry, __end_entry-__start_entry
__start_entry:
__start:

    .extern bsp_fmpll_init                  # fire up fmpll
    bl      bsp_fmpll_init

    .extern bsp_flash_init                  # adjust flash waitstates
    bl      bsp_flash_init

    mfmsr   r1                              # turn on spe apu
    oris    r1, r1, (1<<9)
    mtmsr   r1

    li      r1, (1<<9) | (1<<0)             # enable and flush btb
    mtspr   1013, r1

    # internal ram ecc values MUST be 'sanitized' before use, mostly
    # for the benefit of proper crt init. Word stores need to be performed
    # ro properly set ecc. crt does byte writes and those will fail.
    li      r1, 752                         # 94k/4 bytes/32 GPRs = 752
    mtctr   r1

    .extern _internal_ram_start
    lis     r1, _internal_ram_start@ha      # linker script supplied
    addi    r1, r1, _internal_ram_start@l   #
@loop:
    stmw    r0, 0(r1)                       # all 32 gprs
    addi    r1, r1, 32*4                    # 32 GPRs * 4 bytes = 128
    bdnz    @loop

    # stack usable now that ecc values are reset
    .extern _stack_addr                     # defined by linker
    lis     rsp, _stack_addr@ha
    addi    rsp, rsp, _stack_addr@l

    # setup a junk terminating record on the very
    # bottom of the stack - if it gets this bad
    # hopefully it can be caught (if using an eabi
    # compliant language )
    .extern __stack_abort
    stwu    rsp, -8(rsp)
    lis     r2, __stack_abort@ha            # worth a try
    addi    r2, r2, __stack_abort@l
    stw     r2, 4(rsp)                      # lr save word

    # setup SDA/SDA2 eabi mandated sections (even if unused)
    # mwerks is not exactly compliant, because the eabi spec
    # says that these values should be set to 0 if sdata/sbss
    # sections are unused (mwerks is !0)
    .extern _SDA_BASE_, _SDA2_BASE_
    lis     r2, _SDA2_BASE_@ha              # defined by linker
    addi    r2, r2, _SDA2_BASE_@l

    lis     r13, _SDA_BASE_@ha              # defined by linker
    addi    r13, r13, _SDA_BASE_@l

    # c/c++ runtime init
    .extern __init
    bl      __init

    .extern led_init
    bl      led_init

    # final board init (INTC, decrementer, etc)
    .extern bsp_init
    bl      bsp_init

    .extern os_init
    bl      os_init

    # start timebase
    mfhid0  r3
    ori     r3, r3, 0x4000
    mthid0  r3

    # unmask external interrupts before dropping into main
    wrteei  1

    # ensure argc/argv null'd out
    xor     r3, r3, r3
    xor     r4, r4, r4

    .extern main
    bl      main

    # drop into crt handled exit()
    .extern exit
    b       exit                            # noreturn
__end_entry: