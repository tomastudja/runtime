// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;
using System.Threading;


interface IGen<T>
{
	void Target<U>();
	T Dummy(T t);
}

class Gen<T> : IGen<T>
{
	public T Dummy(T t) {return t;}

	public void Target<U>()
	{		
		//dummy line to avoid warnings
		Test_thread19.Eval(typeof(U)!=null);
		Interlocked.Increment(ref Test_thread19.Xcounter);
	}
	public static void DelegateTest<U>()
	{
		IGen<T> obj = new Gen<T>();
		ThreadStart d = new ThreadStart(obj.Target<U>);
		
		
		d();
		Test_thread19.Eval(Test_thread19.Xcounter==1);
		Test_thread19.Xcounter = 0;
	}
}

public class Test_thread19
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
		Gen<int>.DelegateTest<object>();
		Gen<double>.DelegateTest<string>();
		Gen<string>.DelegateTest<Guid>();
		Gen<object>.DelegateTest<int>(); 
		Gen<Guid>.DelegateTest<double>(); 

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


