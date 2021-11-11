﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Text;
using System.Runtime.InteropServices;

namespace Char;

public partial class PInvoke_Default
{
    [DllImport("Char_BestFitMappingNative")]
    public static extern bool Char_In([In]char c);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool Char_InByRef([In]ref char c);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool Char_InOutByRef([In, Out]ref char c);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool CharBuffer_In_String([In]String s);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool CharBuffer_InByRef_String([In]ref String s);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool CharBuffer_InOutByRef_String([In, Out]ref String s);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool CharBuffer_In_StringBuilder([In]StringBuilder s);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool CharBuffer_InByRef_StringBuilder([In]ref StringBuilder s);

    [DllImport("Char_BestFitMappingNative")]
    public static extern bool CharBuffer_InOutByRef_StringBuilder([In, Out]ref StringBuilder s);

    private static unsafe void RunTest(bool bestFitMapping, bool throwOnUnmappableChar)
    {
        Console.WriteLine(" -- Validate P/Invokes: BestFitMapping not set, ThrowOnUnmappableChar not set");

        Test.ValidateChar(
            bestFitMapping,
            throwOnUnmappableChar,
            new Test.Functions<char>(
                &Char_In,
                &Char_InByRef,
                &Char_InOutByRef));

        Test.ValidateString(
            bestFitMapping,
            throwOnUnmappableChar,
            new Test.Functions<string>(
                &CharBuffer_In_String,
                &CharBuffer_InByRef_String,
                &CharBuffer_InOutByRef_String));

        Test.ValidateStringBuilder(
            bestFitMapping,
            throwOnUnmappableChar,
            new Test.Functions<StringBuilder>(
                &CharBuffer_In_StringBuilder,
                &CharBuffer_InByRef_StringBuilder,
                &CharBuffer_InOutByRef_StringBuilder));
    }
}
