// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

namespace System.Reflection.Emit.Tests
{
    public class ILOffsetTests
    {
        [Fact]
        public void ILOffset_NothingEmitted_ReturnsZero()
        {
            ModuleBuilder module = Helpers.DynamicModule();
            MethodBuilder method = module.DefineGlobalMethod("method1", MethodAttributes.Public | MethodAttributes.Static, typeof(Type), new Type[0]);
            ILGenerator ilGenerator = method.GetILGenerator();

            // Method has not been emitted.
            Assert.Equal(0, ilGenerator.ILOffset);
        }

        [Fact]
        public void ILOffset_SomethingEmitted_ReturnsNonZero()
        {
            ModuleBuilder module = Helpers.DynamicModule();
            MethodBuilder method = module.DefineGlobalMethod("method1", MethodAttributes.Public | MethodAttributes.Static, typeof(Type), new Type[0]);
            ILGenerator ilGenerator = method.GetILGenerator();
            ilGenerator.Emit(OpCodes.Ret);

            Assert.Equal(1, ilGenerator.ILOffset);
        }
    }
}
