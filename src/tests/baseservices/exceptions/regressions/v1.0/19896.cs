// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;

public class b19896
{
	public static int Main()
	{
		int retVal = 200;

		try
		{
			try
			{
				throw new Exception();
			}
			catch
			{
				Type.GetType("System.Foo", true);
			}
		}

		catch(System.TypeLoadException)
		{
			Console.WriteLine("TEST PASSED");
			retVal = 100;
		}

		return retVal;
	}
}

//EOF
