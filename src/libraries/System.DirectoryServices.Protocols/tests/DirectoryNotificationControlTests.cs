// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

namespace System.DirectoryServices.Protocols.Tests
{
    public class DirectoryNotificationRequestControlTests
    {
        [Fact]
        public void Ctor_Default()
        {
            var control = new DirectoryNotificationControl();
            Assert.True(control.IsCritical);
            Assert.True(control.ServerSide);
            Assert.Equal("1.2.840.113556.1.4.528", control.Type);

            Assert.Empty(control.GetValue());
        }
    }
}
