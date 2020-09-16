// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*============================================================================
**
** Source:  test7.c
**
** Purpose: Test #7 for the vprintf function. Tests the wide char
**          specifier (%C).
**
**
**==========================================================================*/



#include <palsuite.h>
#include "../vprintf.h"



PALTEST(c_runtime_vprintf_test7_paltest_vprintf_test7, "c_runtime/vprintf/test7/paltest_vprintf_test7")
{
    WCHAR wb = (WCHAR) 'b';
    
    if (PAL_Initialize(argc, argv))
    {
        return FAIL;
    }


    DoWCharTest("foo %C", wb, "foo b");
    DoWCharTest("foo %hC", wb, "foo b");
    DoCharTest("foo %lC", 'c', "foo c");
    DoWCharTest("foo %LC", wb, "foo b");
    DoWCharTest("foo %I64C", wb, "foo b");
    DoWCharTest("foo %5C", wb, "foo     b");
    DoWCharTest("foo %.0C", wb, "foo b");
    DoWCharTest("foo %-5C", wb, "foo b    ");
    DoWCharTest("foo %05C", wb, "foo 0000b");
    DoWCharTest("foo % C", wb, "foo b");
    DoWCharTest("foo %#C", wb, "foo b");

    PAL_Terminate();
    return PASS;
}

