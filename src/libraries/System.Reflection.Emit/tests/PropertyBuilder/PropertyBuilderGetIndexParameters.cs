// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

namespace System.Reflection.Emit.Tests
{
    public class PropertyBuilderTest7
    {
        [Fact]
        public void GetIndexParameters_ThrowsNotSupportedException()
        {
            TypeBuilder type = Helpers.DynamicType(TypeAttributes.Class | TypeAttributes.Public);
            PropertyBuilder property = type.DefineProperty("TestProperty", PropertyAttributes.None, typeof(int), null);
            MethodBuilder method = type.DefineMethod("TestProperty", MethodAttributes.Public, CallingConventions.HasThis, typeof(int), null);

            ILGenerator methodILGenerator = method.GetILGenerator();
            methodILGenerator.Emit(OpCodes.Ldarg_0);
            methodILGenerator.Emit(OpCodes.Ret);
            property.AddOtherMethod(method);

            Type myType = type.CreateType();
            Assert.Throws<NotSupportedException>(() => property.GetIndexParameters());
        }
    }
}
