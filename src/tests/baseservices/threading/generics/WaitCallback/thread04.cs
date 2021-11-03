// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;
using System.Threading;

class Gen 
{
	public virtual void Target<U>(object p)
	{		
			//dummy line to avoid warnings
			Test_thread04.Eval(typeof(U)!=p.GetType());
			ManualResetEvent evt = (ManualResetEvent) p;
			Interlocked.Increment(ref Test_thread04.Xcounter);
			evt.Set();
	}
	public static void ThreadPoolTest<U>()
	{
		ManualResetEvent[] evts = new ManualResetEvent[Test_thread04.nThreads];
		WaitHandle[] hdls = new WaitHandle[Test_thread04.nThreads];

		for (int i=0; i<Test_thread04.nThreads; i++)
		{
			evts[i] = new ManualResetEvent(false);
			hdls[i] = (WaitHandle) evts[i];
		}

		Gen obj = new Gen();

		for (int i = 0; i < Test_thread04.nThreads; i++)
		{	
			WaitCallback cb = new WaitCallback(obj.Target<U>);
			ThreadPool.QueueUserWorkItem(cb,evts[i]);
		}

		WaitHandle.WaitAll(hdls);
		Test_thread04.Eval(Test_thread04.Xcounter==Test_thread04.nThreads);
		Test_thread04.Xcounter = 0;
	}
}

public class Test_thread04
{
	public static int nThreads =50;
	public static int counter = 0;
	public static int Xcounter = 0;
	public static bool result = true;
	public static void Eval(bool exp)
	{
		counter++;
		if (!exp)
		{
			result = exp;
			Console.WriteLine("Test Failed at location: " + counter);
		}
	
	}
	
	public static int Main()
	{
		Gen.ThreadPoolTest<object>();
		Gen.ThreadPoolTest<string>();
		Gen.ThreadPoolTest<Guid>();
		Gen.ThreadPoolTest<int>(); 
		Gen.ThreadPoolTest<double>(); 

		if (result)
		{
			Console.WriteLine("Test Passed");
			return 100;
		}
		else
		{
			Console.WriteLine("Test Failed");
			return 1;
		}
	}
}		


