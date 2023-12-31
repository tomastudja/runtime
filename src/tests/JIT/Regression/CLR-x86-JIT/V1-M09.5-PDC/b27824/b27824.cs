// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

using Xunit;
namespace Test
{
    using System;

    public class AA
    {
        static AA[] m_axForward3;
        static void GoToEnd() { throw new Exception(); }

        [Fact]
        public static void TestEntryPoint()
        {
            bool param1 = false;
            bool[] local2 = new bool[7];
            float local3 = -40.0f;
            try
            {
                while (500.20f <= local3 + local3)
                {
                    GC.Collect();
                    AA.m_axForward3 = new AA[7];
                }
                do
                {
                    AA aa;
                    for (aa = new AA(); true; local2 = local2)
                    {
                        GC.Collect();
                        GoToEnd();
                    }
                } while (local2[2]);

                do
                {

                } while (true);
                GC.Collect();
            }
            catch (Exception)
            {
            }
        }
    }
}
