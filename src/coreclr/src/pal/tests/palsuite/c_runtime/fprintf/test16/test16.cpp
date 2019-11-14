// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================================
**
** Source:      test16.c (fprintf)
**
** Purpose:     Tests the decimal notation double specifier (%f).
**              This test is modeled after the fprintf series.
**
**
**==========================================================================*/

#include <palsuite.h>
#include "../fprintf.h"

/* 
 * Depends on memcmp, strlen, fopen, fseek and fgets.
 */

int __cdecl main(int argc, char *argv[])
{
    double val = 2560.001;
    double neg = -2560.001;
    
    if (PAL_Initialize(argc, argv) != 0)
        return(FAIL);

    DoDoubleTest("foo %f", val,  "foo 2560.001000", "foo 2560.001000");
    DoDoubleTest("foo %lf", val,  "foo 2560.001000", "foo 2560.001000");
    DoDoubleTest("foo %hf", val,  "foo 2560.001000", "foo 2560.001000");
    DoDoubleTest("foo %Lf", val,  "foo 2560.001000", "foo 2560.001000");
    DoDoubleTest("foo %I64f", val,  "foo 2560.001000", "foo 2560.001000");
    DoDoubleTest("foo %12f", val,  "foo  2560.001000", "foo  2560.001000");
    DoDoubleTest("foo %-12f", val,  "foo 2560.001000 ", "foo 2560.001000 ");
    DoDoubleTest("foo %.1f", val,  "foo 2560.0", "foo 2560.0");
    DoDoubleTest("foo %.8f", val,  "foo 2560.00100000", "foo 2560.00100000");
    DoDoubleTest("foo %012f", val,  "foo 02560.001000", "foo 02560.001000");
    DoDoubleTest("foo %#f", val,  "foo 2560.001000", "foo 2560.001000");
    DoDoubleTest("foo %+f", val,  "foo +2560.001000", "foo +2560.001000");
    DoDoubleTest("foo % f", val,  "foo  2560.001000", "foo  2560.001000");
    DoDoubleTest("foo %+f", neg,  "foo -2560.001000", "foo -2560.001000");
    DoDoubleTest("foo % f", neg,  "foo -2560.001000", "foo -2560.001000");

    PAL_Terminate();
    return PASS;
}
