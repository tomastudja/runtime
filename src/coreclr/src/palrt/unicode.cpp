// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

#include "common.h"

// This is a simplified implementation of IsTextUnicode.
// https://github.com/dotnet/runtime/issues/4778
BOOL IsTextUnicode(CONST VOID* lpv, int iSize, LPINT lpiResult)
{
    *lpiResult = 0;

    if (iSize < 2) return FALSE;

    BYTE * p = (BYTE *)lpv;

    // Check for Unicode BOM
    if ((*p == 0xFF) && (*(p+1) == 0xFE))
    {
        *lpiResult |= IS_TEXT_UNICODE_SIGNATURE;
        return TRUE;
    }

    return FALSE;
}

