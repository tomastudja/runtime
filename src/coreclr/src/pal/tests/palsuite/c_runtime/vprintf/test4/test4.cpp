// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*============================================================================
**
** Source:  test4.c
**
** Purpose: Test #4 for the vprintf function. Tests the pointer
**          specifier (%p).
**
**
**==========================================================================*/



#include <palsuite.h>
#include "../vprintf.h"




int __cdecl main(int argc, char *argv[])
{
    void *ptr = (void*) 0x123456;
    INT64 lptr = I64(0x1234567887654321);

    if (PAL_Initialize(argc, argv))
    {
        return FAIL;
    }
/*
**  Run only on 64 bit platforms
*/
#if defined(HOST_64BIT)
	Trace("Testing for 64 Bit Platforms \n");
    DoPointerTest("%p", NULL, "NULL", "0000000000000000");
    DoPointerTest("%p", ptr, "pointer to 0x123456", "0000000000123456");
    DoPointerTest("%17p", ptr, "pointer to 0x123456", " 0000000000123456");
    DoPointerTest("%17p", ptr, "pointer to 0x123456", " 0000000000123456");
    DoPointerTest("%-17p", ptr, "pointer to 0x123456", "0000000000123456 ");
    DoPointerTest("%+p", ptr, "pointer to 0x123456", "0000000000123456");
    DoPointerTest("%#p", ptr, "pointer to 0x123456", "0X0000000000123456");
    DoPointerTest("%lp", ptr, "pointer to 0x123456", "00123456");
    DoPointerTest("%hp", ptr, "pointer to 0x123456", "00003456");
    DoPointerTest("%Lp", ptr, "pointer to 0x123456", "00123456");
    DoI64Test("%I64p", lptr, "pointer to 0x1234567887654321", 
        "1234567887654321");

#else
	Trace("Testing for Non 64 Bit Platforms \n");
    DoPointerTest("%p", NULL, "NULL", "00000000");
    DoPointerTest("%p", ptr, "pointer to 0x123456", "00123456");
    DoPointerTest("%9p", ptr, "pointer to 0x123456", " 00123456");
    DoPointerTest("%09p", ptr, "pointer to 0x123456", " 00123456");
    DoPointerTest("%-9p", ptr, "pointer to 0x123456", "00123456 ");
    DoPointerTest("%+p", ptr, "pointer to 0x123456", "00123456");
    DoPointerTest("%#p", ptr, "pointer to 0x123456", "0X00123456");
    DoPointerTest("%lp", ptr, "pointer to 0x123456", "00123456");
    DoPointerTest("%hp", ptr, "pointer to 0x123456", "00003456");
    DoPointerTest("%Lp", ptr, "pointer to 0x123456", "00123456");
    DoI64Test("%I64p", lptr, "pointer to 0x1234567887654321", 
        "1234567887654321");

#endif
	
    PAL_Terminate();
    return PASS;
}

