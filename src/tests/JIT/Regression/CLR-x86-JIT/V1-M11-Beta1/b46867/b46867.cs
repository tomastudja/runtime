// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

namespace Test
{
    using System;

    public class AA
    {
        static void Method2(double param3, long param4, __arglist)
        {
            param3 = (double)param4;
        }
        public static int Main()
        {
            Method2(1.0d, 1, __arglist());
            return 100;
        }
    }
}
