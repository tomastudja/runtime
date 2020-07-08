// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

namespace System.Composition.AttributeModel.Tests
{
    public class ExportMetadataAttributeTests
    {
        [Theory]
        [InlineData(null, null)]
        [InlineData("Name", "Value")]
        public void Ctor_Name_Value(string name, string value)
        {
            var attribute = new ExportMetadataAttribute(name, value);
            Assert.Equal(name ?? string.Empty, attribute.Name);
            Assert.Equal(value, attribute.Value);
        }
    }
}
