// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Generated by Fuzzlyn v1.4 on 2021-08-24 18:42:34
// Run on .NET 7.0.0-dev on Arm Linux
// Seed: 4314857431407232792
// Reduced from 582.3 KiB to 3.3 KiB in 02:00:04
// Crashes the runtime
public interface I0
{
}

public interface I3
{
}

public struct S0 : I0
{
    public sbyte F0;
    public short F1;
    public sbyte F2;
    public int F4;
    public bool F5;
    public S0(bool f5): this()
    {
    }

    public S1 M34(S1[] arg0)
    {
        return default(S1);
    }
}

public struct S1 : I0, I3
{
    public short F0;
    public S0 F1;
    public S0 F2;
    public S0 F3;
    public S1(S0 f1, S0 f2, S0 f3): this()
    {
    }
}

public class Runtime_58293
{
    private static I s_rt;
    public static I3 s_21;
    public static S1 s_29;
    public static S1[][] s_32 = new S1[][]{new S1[]{new S1(new S0(false), new S0(false), new S0(true))}};
    public static I3[][] s_42;
    public static int Main()
    {
        s_rt = new C();
        var vr3 = s_32[0][0].F3.F2;
        M33(vr3);
		return 100;
    }

    public static I0[] M33(sbyte arg0)
    {
        S1 var3;
        I3 var8;
        S1 var9;
        uint var11;
        S0 var12;
        var vr24 = s_32[0];
        new S0(false).M34(vr24);
        I0 var0 = new S0(true);
        var vr0 = new S1[]{new S1(new S0(true), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(true))};
        new S0(true).M34(vr0);
        try
        {
            bool var1 = s_29.F2.F5;
            s_rt.WriteLine(var1);
        }
        finally
        {
            s_42 = new I3[][]{new I3[]{new S1(new S0(true), new S0(true), new S0(true)), new S1(new S0(true), new S0(true), new S0(true))}, new I3[]{new S1(new S0(false), new S0(true), new S0(true)), new S1(new S0(false), new S0(true), new S0(true)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(true), new S0(false), new S0(false))}, new I3[]{new S1(new S0(true), new S0(true), new S0(false)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(false), new S0(false), new S0(false)), new S1(new S0(false), new S0(false), new S0(true)), new S1(new S0(true), new S0(true), new S0(false))}, new I3[]{new S1(new S0(false), new S0(true), new S0(false)), new S1(new S0(true), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(true), new S0(false), new S0(true))}, new I3[]{new S1(new S0(false), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(false), new S0(true), new S0(true)), new S1(new S0(false), new S0(true), new S0(true))}, new I3[]{new S1(new S0(false), new S0(true), new S0(true)), new S1(new S0(false), new S0(false), new S0(false))}};
        }

        var vr30 = new S1[]{new S1(new S0(true), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(false)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(true), new S0(false), new S0(true)), new S1(new S0(true), new S0(false), new S0(false))};
        s_21 = s_29.F2.M34(vr30);
        var0 = new S1(new S0(true), new S0(false), new S0(true));
        var vr2 = new S0(true);
        I3 vr23;
        I3 var2 = new S1(new S0(true), new S0(true), new S0(false));
        bool vr25;
        bool vr26;
        bool vr28;
        bool vr29;
        bool vr31;
        return new I0[]{new S0(false), new S0(true), new S1(new S0(true), new S0(true), new S0(false))};
    }
}

public interface I { void WriteLine<T>(T val); }
public class C : I
{
    public void WriteLine<T>(T val)
    {
        System.Console.WriteLine(val);
    }
}