// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;
using System.Threading;
using Xunit;

class Gen<T> 
{
	public static void Target()
	{			
		Interlocked.Increment(ref Test_thread25.Xcounter);
	}
	public static void ThreadPoolTest()
	{
		Thread[] threads = new Thread[Test_thread25.nThreads];

		for (int i = 0; i < Test_thread25.nThreads; i++)
		{	
			threads[i]  = new Thread(new ThreadStart(Gen<T>.Target));
			threads[i].Start();
		}

		for (int i = 0; i < Test_thread25.nThreads; i++)
		{	
			threads[i].Join();
		}
		
		Test_thread25.Eval(Test_thread25.Xcounter==Test_thread25.nThreads);
		Test_thread25.Xcounter = 0;
	}
}

public class Test_thread25
{
	public static int nThreads = 50;
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
	
	[Fact]
	public static int TestEntryPoint()
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


