// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

using Xunit;
namespace Test
{
    using System;

    public class App
    {
        static void Method1(float param2) { }

        [Fact]
        public static void TestEntryPoint()
        {
            ulong local3 = 168u;
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
            try { Method1((float)local3 + App.m_afForward5[0]); }
            catch (Exception) { }
        }

        public static float[] m_afForward5 = null;
    }
}
/*
---------------------------
Assert Failure (PID 856, Thread 1076/434)        
---------------------------
conv >= 0

d:\com99\src\vm\wks\..\jitinterface.cpp, Line: 5970

Abort - Kill program
Retry - Debug
Ignore - Keep running


Image:
D:\bugs\bug.exe

---------------------------
Abort   Retry   Ignore   
---------------------------
*/
