/*********************************************************************************

        @file   Knock.c                                                              
        @author Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU - knock detection
        @note   Not completed pending clarifications....
        @version .9
        @copyright MIT License

*************************************************************************************/

/* 
Copyright(c) 2011 Jon Zeeff
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include "config.h"
#include "cpu.h"
#include "system.h"
#include "variables.h"
#include "main.h"

void Knock_Task(void)
{
    static uint_fast8_t i;

    task_open();                // standard OS entry - required on all tasks

    for (;;) {

        // read A/D values captured by A/D converter during the angle window set by the eTPU knock window code
        // look for specified freq +/-

    }                           // for ever

    task_close();
}                               // Knock_Task()

