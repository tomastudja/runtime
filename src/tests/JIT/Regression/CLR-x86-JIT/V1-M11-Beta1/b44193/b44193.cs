// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

namespace Test
{
    using System;

    public class App
    {
        public static int Main()
        {
            bool b;
            int i = 0;
            do
            {
                b = false;
                do
                {
                    b = false;
                    do
                    {
                        b = false;
                        do
                        {
                            b = false;
                            do
                            {
                                b = false;
                                do
                                {
                                    b = false;
                                    do
                                    {
                                        b = false;
                                    } while (i == 1);
                                } while (b);
                            } while (b);
                        } while (b);
                    } while (b);
                } while (b);
            } while (b);
            return 100;
        }
    }
}
