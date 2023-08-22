// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

// Generated by Fuzzlyn v1.5 on 2023-04-09 16:37:03
// Run on X64 Windows
// Seed: 6230188300048624105
// Reduced from 155.1 KiB to 0.2 KiB in 00:01:48
// Hits JIT assert in Release:
// Assertion failed '((tree->gtFlags & GTF_VAR_DEF) == 0) && (tree->GetLclNum() == lclNum) && tree->gtVNPair.BothDefined()' in 'Program:Main(Fuzzlyn.ExecutionServer.IRuntime)' during 'VN based copy prop' (IL size 25; hash 0xade6b36b; FullOpts)
// 
//     File: D:\a\_work\1\s\src\coreclr\jit\copyprop.cpp Line: 161
// 
public class Runtime_84619
{
    public static sbyte s_27;
    [Fact]
    public static void TestEntryPoint()
    {
        long vr1 = 0;
        for (int vr3 = 0; vr3 < -1; vr3++)
        {
            s_27 = (sbyte)(vr1 | vr1);
        }
    }
}
