// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;
using System.Threading;


interface IGen<T>
{
	void Target(object p);
	T Dummy(T t);
}

class Gen<T> : IGen<T>
{
	public T Dummy(T t) { return t; }

	public void Target(object p)
	{		
			ManualResetEvent evt = (ManualResetEvent) p;
			Interlocked.Increment(ref Test_thread10.Xcounter);
			evt.Set();
	}
	
	public static void ThreadPoolTest()
	{
		ManualResetEvent[] evts = new ManualResetEvent[Test_thread10.nThreads];
		WaitHandle[] hdls = new WaitHandle[Test_thread10.nThreads];

		for (int i=0; i<Test_thread10.nThreads; i++)
		{
			evts[i] = new ManualResetEvent(false);
			hdls[i] = (WaitHandle) evts[i];
		}

		IGen<T> obj = new Gen<T>();

		for (int i = 0; i <Test_thread10.nThreads; i++)
		{	
			WaitCallback cb = new WaitCallback(obj.Target);
			ThreadPool.QueueUserWorkItem(cb,evts[i]);
		}

		WaitHandle.WaitAll(hdls);
		Test_thread10.Eval(Test_thread10.Xcounter==Test_thread10.nThreads);
		Test_thread10.Xcounter = 0;
	}
}

public class Test_thread10
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
		Gen<int>.ThreadPoolTest();
		Gen<double>.ThreadPoolTest();
		Gen<string>.ThreadPoolTest();
		Gen<object>.ThreadPoolTest(); 
		Gen<Guid>.ThreadPoolTest(); 

		Gen<int[]>.ThreadPoolTest(); 
		Gen<double[,]>.ThreadPoolTest();
		Gen<string[][][]>.ThreadPoolTest(); 
		Gen<object[,,,]>.ThreadPoolTest();
		Gen<Guid[][,,,][]>.ThreadPoolTest();

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


