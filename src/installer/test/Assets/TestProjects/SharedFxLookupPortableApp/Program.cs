// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;

namespace SharedFxLookupPortableApp
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");
            Console.WriteLine(string.Join(Environment.NewLine, args));
            Console.WriteLine($"Framework Version:{GetFrameworkVersionFromAppDomain()}");

            // A small operation involving NewtonSoft.Json to ensure the assembly is loaded properly
            var t = typeof(Newtonsoft.Json.JsonReader);
        }

        private static string GetFrameworkVersionFromAppDomain()
        {
            return System.AppDomain.CurrentDomain.GetData("FX_PRODUCT_VERSION") as string;
        }
    }
}
