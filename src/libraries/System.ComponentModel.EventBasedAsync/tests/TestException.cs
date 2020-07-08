// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.ComponentModel.EventBasedAsync.Tests
{
    public class TestException : Exception
    {
        public TestException(string message)
            : base(message)
        {
        }
    }
}
