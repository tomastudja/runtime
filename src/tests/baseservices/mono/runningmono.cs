// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;

namespace TestRunningMono
{
    class Program
    {
        public static int Main()
        {
             const int Pass = 100, Fail = 1;
             bool isMono = typeof(object).Assembly.GetType("Mono.RuntimeStructs") != null;

             if(isMono)
             {
                 return Pass;
             }
             else
             {
                 return Fail;
             }
        }
   }
}

