// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

namespace Test
{
    using System;

    public struct AA
    {
        void Method1() { }

        static void Main1()
        {
            (new AA[137])[101].Method1();
            throw new DivideByZeroException();
        }
        public static int Main()
        {
            try
            {
                Main1();
                return 1;
            }
            catch (DivideByZeroException)
            {
                return 100;
            }
        }
    }
}
