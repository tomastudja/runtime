// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices.Tests.Common;
using Xunit;

namespace System.Runtime.InteropServices.Tests
{
    public partial class GetEndComSlotTests
    {
        [Fact]
        [PlatformSpecific(TestPlatforms.AnyUnix)]
        public void GetEndComSlot_Unix_ThrowsPlatformNotSupportedException()
        {
            Assert.Throws<PlatformNotSupportedException>(() => Marshal.GetEndComSlot(null));
        }

        [Fact]
        [PlatformSpecific(TestPlatforms.Windows)]
        public void GetEndComSlot_NullType_ThrowsArgumentNullException()
        {
            AssertExtensions.Throws<ArgumentNullException>(null, () => Marshal.GetEndComSlot(null));
        }

        [Fact]
        [PlatformSpecific(TestPlatforms.Windows)]
        public void GetEndComSlot_NotRuntimeType_ThrowsArgumentException()
        {
            AssemblyBuilder assemblyBuilder = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Assembly"), AssemblyBuilderAccess.Run);
            ModuleBuilder moduleBuilder = assemblyBuilder.DefineDynamicModule("Module");
            TypeBuilder typeBuilder = moduleBuilder.DefineType("Type");
            AssertExtensions.Throws<ArgumentException>("t", () => Marshal.GetEndComSlot(typeBuilder));
        }

        public static IEnumerable<object[]> GetStartComSlot_InvalidGenericType_TestData()
        {
            yield return new object[] { typeof(int).MakeByRefType() };
            yield return new object[] { typeof(GenericClass<>).GetTypeInfo().GenericTypeParameters[0] };
        }

        [Theory]
        [MemberData(nameof(GetStartComSlot_InvalidGenericType_TestData))]
        [PlatformSpecific(TestPlatforms.Windows)]
        public void GetEndComSlot_InvalidGenericType_ThrowsArgumentNullException(Type type)
        {
            AssertExtensions.Throws<ArgumentNullException>(null, () => Marshal.GetEndComSlot(type));
        }
        public static IEnumerable<object[]> GetStartComSlot_NotComVisibleType_TestData()
        {
            yield return new object[] { typeof(GenericClass<>) };
            yield return new object[] { typeof(GenericClass<string>) };
            yield return new object[] { typeof(GenericStruct<>) };
            yield return new object[] { typeof(GenericStruct<string>) };
            yield return new object[] { typeof(IGenericInterface<>) };
            yield return new object[] { typeof(IGenericInterface<string>) };
            yield return new object[] { typeof(NonComVisibleClass) };
            yield return new object[] { typeof(NonComVisibleStruct) };
            yield return new object[] { typeof(INonComVisibleInterface) };
            yield return new object[] { typeof(int[]) };
            yield return new object[] { typeof(int[][]) };
            yield return new object[] { typeof(int[,]) };

            AssemblyBuilder assemblyBuilder = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Assembly"), AssemblyBuilderAccess.RunAndCollect);
            ModuleBuilder moduleBuilder = assemblyBuilder.DefineDynamicModule("Module");
            TypeBuilder typeBuilder = moduleBuilder.DefineType("Type");
            Type collectibleType = typeBuilder.CreateType();
            yield return new object[] { collectibleType };
        }

        [Theory]
        [MemberData(nameof(GetStartComSlot_NotComVisibleType_TestData))]
        [PlatformSpecific(TestPlatforms.Windows)]
        public void GetEndComSlot_NotComVisibleType_ThrowsArgumentException(Type type)
        {
            AssertExtensions.Throws<ArgumentException>("t", () => Marshal.GetEndComSlot(type));
        }
    }
}
