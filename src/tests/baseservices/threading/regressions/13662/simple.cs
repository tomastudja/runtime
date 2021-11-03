// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Threading;


public class Test_simple {

	public static int Main(){

		int rValue = 100;
		Timer[] tArray = new Timer[100];
		int val = 0;
		while(val < 10){
			Interlocked.Increment(ref val);
			Console.WriteLine("Loop {0}",val);
			for(int i = 0;i<tArray.Length;i++)
				tArray[i] = new Timer(new TimerCallback(TFunc),0,1000,1000000);		

			Thread.Sleep(1000);
			GC.Collect();
			GC.WaitForPendingFinalizers();
		}		
		return rValue;
	}

	public static void TFunc(Object o)
	{
		Thread.Sleep(1);
	}
}
