// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Threading;

class OpenMutexPos
{
    Mutex mut;
    ManualResetEvent mre;

    public OpenMutexPos()
    {
        mre = new ManualResetEvent(false);
    }

    public static int Main()
    {
        OpenMutexPos omp = new OpenMutexPos();
        return omp.Run();
    }

    private int Run()
    {
        int iRet = -1;
        string sName = Common.GetUniqueName();
        // Basic test, owned
        using (mut = new Mutex(true, sName))
        {
            try
            {
                Mutex mut1 = Mutex.OpenExisting(sName);
                iRet = 100;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Unexpected exception thrown: " +
                    ex.ToString());
            }
        }

        Console.WriteLine(100 == iRet ? "Test Passed" : "Test Failed");
        return iRet;
    }
}
