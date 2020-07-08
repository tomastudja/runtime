// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;

public interface IGenX<T> 
{
	string m(T t);
}

public interface IGenY<T> 
{
	string m(T tArr);
}

class Gen<T,U> : IGenX<T>, IGenY<U> 
{
	string IGenX<T>.m(T t)
	{
   		return "IGenX.m";
  	}
  	string IGenY<U>.m(U tArr) 
  	{
    		return "IGenY.m";
  	}
}

public class Test
{
	public static int counter = 0;
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

		Gen<int,int> GenIntInt = new Gen<int,int>();
		Eval(((IGenX<int>)GenIntInt).m(5).Equals("IGenX.m"));
		Eval(((IGenY<int>)GenIntInt).m(5).Equals("IGenY.m"));

		Gen<int,string> GenIntString = new Gen<int,string>();
		Eval(((IGenX<int>)GenIntString).m(5).Equals("IGenX.m"));
		Eval(((IGenY<string>)GenIntString).m("S").Equals("IGenY.m"));

		Gen<string,int> GenStringInt = new Gen<string,int>();
		Eval(((IGenX<string>)GenStringInt).m("S").Equals("IGenX.m"));
		Eval(((IGenY<int>)GenStringInt).m(5).Equals("IGenY.m"));
		
		Gen<string,string> GenStringString = new Gen<string,string>();
		Eval(((IGenX<string>)GenStringString).m("S").Equals("IGenX.m"));
		Eval(((IGenY<string>)GenStringString).m("S").Equals("IGenY.m"));
		
		
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

