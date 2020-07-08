// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.ComponentModel.Tests;
using Xunit;

namespace System.ComponentModel.Design.Tests
{
    public class ComponentChangedEventArgsTests
    {
        public static IEnumerable<object[]> Ctor_TestData()
        {
            yield return new object[] { "component", new MockPropertyDescriptor(), "oldValue", "newValue" };
            yield return new object[] { null, null, null, null };
        }

        [Theory]
        [MemberData(nameof(Ctor_TestData))]
        public void Ctor(object component, MemberDescriptor member, object oldValue, object newValue)
        {
            var eventArgs = new ComponentChangedEventArgs(component, member, oldValue, newValue);
            Assert.Same(component, eventArgs.Component);
            Assert.Same(member, eventArgs.Member);
            Assert.Same(oldValue, eventArgs.OldValue);
            Assert.Same(newValue, eventArgs.NewValue);
        }
    }
}
