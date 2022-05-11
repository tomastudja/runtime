﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices;

namespace LibraryImportGenerator.UnitTests
{
    internal static class CodeSnippets
    {
        /// <summary>
        /// Partially define attribute for pre-.NET 7.0
        /// </summary>
        public static readonly string LibraryImportAttributeDeclaration = @"
namespace System.Runtime.InteropServices
{
    internal enum StringMarshalling
    {
        Custom = 0,
        Utf8,
        Utf16,
    }

    sealed class LibraryImportAttribute : System.Attribute
    {
        public LibraryImportAttribute(string a) { }
        public StringMarshalling StringMarshalling { get; set; }
        public Type StringMarshallingCustomType { get; set; }
    }
}
";

        /// <summary>
        /// Trivial declaration of LibraryImport usage
        /// </summary>
        public static readonly string TrivialClassDeclarations = @"
using System.Runtime.InteropServices;
partial class Basic
{
    [LibraryImportAttribute(""DoesNotExist"")]
    public static partial void Method1();

    [LibraryImport(""DoesNotExist"")]
    public static partial void Method2();

    [System.Runtime.InteropServices.LibraryImportAttribute(""DoesNotExist"")]
    public static partial void Method3();

    [System.Runtime.InteropServices.LibraryImport(""DoesNotExist"")]
    public static partial void Method4();
}
";
        /// <summary>
        /// Trivial declaration of LibraryImport usage
        /// </summary>
        public static readonly string TrivialStructDeclarations = @"
using System.Runtime.InteropServices;
partial struct Basic
{
    [LibraryImportAttribute(""DoesNotExist"")]
    public static partial void Method1();

    [LibraryImport(""DoesNotExist"")]
    public static partial void Method2();

    [System.Runtime.InteropServices.LibraryImportAttribute(""DoesNotExist"")]
    public static partial void Method3();

    [System.Runtime.InteropServices.LibraryImport(""DoesNotExist"")]
    public static partial void Method4();
}
";

        /// <summary>
        /// Declaration with multiple attributes
        /// </summary>
        public static readonly string MultipleAttributes = @"
using System;
using System.Runtime.InteropServices;

sealed class DummyAttribute : Attribute
{
    public DummyAttribute() { }
}

sealed class Dummy2Attribute : Attribute
{
    public Dummy2Attribute(string input) { }
}

partial class Test
{
    [DummyAttribute]
    [LibraryImport(""DoesNotExist""), Dummy2Attribute(""string value"")]
    public static partial void Method();
}
";

        /// <summary>
        /// Validate nested namespaces are handled
        /// </summary>
        public static readonly string NestedNamespace = @"
using System.Runtime.InteropServices;
namespace NS
{
    namespace InnerNS
    {
        partial class Test
        {
            [LibraryImport(""DoesNotExist"")]
            public static partial void Method1();
        }
    }
}
namespace NS.InnerNS
{
    partial class Test
    {
        [LibraryImport(""DoesNotExist"")]
        public static partial void Method2();
    }
}
";

        /// <summary>
        /// Validate nested types are handled.
        /// </summary>
        public static readonly string NestedTypes = @"
using System.Runtime.InteropServices;
namespace NS
{
    partial class OuterClass
    {
        partial class InnerClass
        {
            [LibraryImport(""DoesNotExist"")]
            public static partial void Method();
        }
    }
    partial struct OuterStruct
    {
        partial struct InnerStruct
        {
            [LibraryImport(""DoesNotExist"")]
            public static partial void Method();
        }
    }
    partial class OuterClass
    {
        partial struct InnerStruct
        {
            [LibraryImport(""DoesNotExist"")]
            public static partial void Method();
        }
    }
    partial struct OuterStruct
    {
        partial class InnerClass
        {
            [LibraryImport(""DoesNotExist"")]
            public static partial void Method();
        }
    }
}
";

        /// <summary>
        /// Containing type with and without unsafe
        /// </summary>
        public static readonly string UnsafeContext = @"
using System.Runtime.InteropServices;
partial class Test
{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method1();
}
unsafe partial class Test
{
    [LibraryImport(""DoesNotExist"")]
    public static partial int* Method2();
}
";
        /// <summary>
        /// Declaration with user defined EntryPoint.
        /// </summary>
        public static readonly string UserDefinedEntryPoint = @"
using System.Runtime.InteropServices;
partial class Test
{
    [LibraryImport(""DoesNotExist"", EntryPoint=""UserDefinedEntryPoint"")]
    public static partial void NotAnExport();
}
";

        /// <summary>
        /// Declaration with all LibraryImport named arguments.
        /// </summary>
        public static readonly string AllLibraryImportNamedArguments = @"
using System.Runtime.InteropServices;
partial class Test
{
    [LibraryImport(""DoesNotExist"",
        StringMarshalling = StringMarshalling.Utf16,
        EntryPoint = ""UserDefinedEntryPoint"",
        SetLastError = true)]
    public static partial void Method();
}
";

        /// <summary>
        /// Declaration using various methods to compute constants in C#.
        /// </summary>
        public static readonly string UseCSharpFeaturesForConstants = @"
using System.Runtime.InteropServices;
partial class Test
{
    private const bool IsTrue = true;
    private const bool IsFalse = false;
    private const string EntryPointName = nameof(Test) + nameof(IsFalse);
    private const int One = 1;
    private const int Two = 2;

    [LibraryImport(nameof(Test),
        StringMarshalling = (StringMarshalling)2,
        EntryPoint = EntryPointName,
        SetLastError = IsFalse)]
    public static partial void Method1();

    [LibraryImport(nameof(Test),
        StringMarshalling = (StringMarshalling)Two,
        EntryPoint = EntryPointName,
        SetLastError = !IsTrue)]
    public static partial void Method2();

    [LibraryImport(nameof(Test),
        StringMarshalling = (StringMarshalling)2,
        EntryPoint = EntryPointName,
        SetLastError = 0 != 1)]
    public static partial void Method3();
}
";

        /// <summary>
        /// Declaration with default parameters.
        /// </summary>
        public static readonly string DefaultParameters = @"
using System.Runtime.InteropServices;
partial class Test
{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(int t = 0);
}
";

        /// <summary>
        /// Declaration with LCIDConversionAttribute.
        /// </summary>
        public static readonly string LCIDConversionAttribute = @"
using System.Runtime.InteropServices;
partial class Test
{
    [LCIDConversion(0)]
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method();
}
";

        /// <summary>
        /// Define a MarshalAsAttribute with a customer marshaller to parameters and return types.
        /// </summary>
        public static readonly string MarshalAsCustomMarshalerOnTypes = @"
using System;
using System.Runtime.InteropServices;
namespace NS
{
    class MyCustomMarshaler : ICustomMarshaler
    {
        static ICustomMarshaler GetInstance(string pstrCookie)
            => new MyCustomMarshaler();

        public void CleanUpManagedData(object ManagedObj)
            => throw new NotImplementedException();

        public void CleanUpNativeData(IntPtr pNativeData)
            => throw new NotImplementedException();

        public int GetNativeDataSize()
            => throw new NotImplementedException();

        public IntPtr MarshalManagedToNative(object ManagedObj)
            => throw new NotImplementedException();

        public object MarshalNativeToManaged(IntPtr pNativeData)
            => throw new NotImplementedException();
    }
}

partial class Test
{
    [LibraryImport(""DoesNotExist"")]
    [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(NS.MyCustomMarshaler), MarshalCookie=""COOKIE1"")]
    public static partial bool Method1([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(NS.MyCustomMarshaler), MarshalCookie=""COOKIE2"")]bool t);

    [LibraryImport(""DoesNotExist"")]
    [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalType = ""NS.MyCustomMarshaler"", MarshalCookie=""COOKIE3"")]
    public static partial bool Method2([MarshalAs(UnmanagedType.CustomMarshaler, MarshalType = ""NS.MyCustomMarshaler"", MarshalCookie=""COOKIE4"")]bool t);
}
";

        /// <summary>
        /// Declaration with user defined attributes with prefixed name.
        /// </summary>
        public static readonly string UserDefinedPrefixedAttributes = @"
using System;
using System.Runtime.InteropServices;

namespace System.Runtime.InteropServices
{
    // Prefix with ATTRIBUTE so the lengths will match during check.
    sealed class ATTRIBUTELibraryImportAttribute : Attribute
    {
        public ATTRIBUTELibraryImportAttribute(string a) { }
    }
}

partial class Test
{
    [ATTRIBUTELibraryImportAttribute(""DoesNotExist"")]
    public static partial void Method1();

    [ATTRIBUTELibraryImport(""DoesNotExist"")]
    public static partial void Method2();

    [System.Runtime.InteropServices.ATTRIBUTELibraryImport(""DoesNotExist"")]
    public static partial void Method3();
}
";

        public static readonly string DisableRuntimeMarshalling = "[assembly:System.Runtime.CompilerServices.DisableRuntimeMarshalling]";

        /// <summary>
        /// Declaration with parameters with <see cref="StringMarshalling"/> set.
        /// </summary>
        public static string BasicParametersAndModifiersWithStringMarshalling(string typename, StringMarshalling value, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"", StringMarshalling = StringMarshalling.{value})]
    public static partial {typename} Method(
        {typename} p,
        in {typename} pIn,
        ref {typename} pRef,
        out {typename} pOut);
}}
";

        public static string BasicParametersAndModifiersWithStringMarshalling<T>(StringMarshalling value, string preDeclaration = "") =>
            BasicParametersAndModifiersWithStringMarshalling(typeof(T).ToString(), value, preDeclaration);

        /// <summary>
        /// Declaration with parameters with <see cref="StringMarshallingCustomType"/> set.
        /// </summary>
        public static string BasicParametersAndModifiersWithStringMarshallingCustomType(string typeName, string stringMarshallingCustomTypeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"", StringMarshallingCustomType = typeof({stringMarshallingCustomTypeName}))]
    public static partial {typeName} Method(
        {typeName} p,
        in {typeName} pIn,
        ref {typeName} pRef,
        out {typeName} pOut);
}}
";

        public static string BasicParametersAndModifiersWithStringMarshallingCustomType<T>(string stringMarshallingCustomTypeName, string preDeclaration = "") =>
            BasicParametersAndModifiersWithStringMarshallingCustomType(typeof(T).ToString(), stringMarshallingCustomTypeName, preDeclaration);

        public static string CustomStringMarshallingParametersAndModifiers<T>()
        {
            string typeName = typeof(T).ToString();
            return BasicParametersAndModifiersWithStringMarshallingCustomType(typeName, "Native", DisableRuntimeMarshalling) + $@"
[CustomTypeMarshaller(typeof({typeName}))]
struct Native
{{
    public Native({typeName} s) {{ }}

    public {typeName} ToManaged() => default;
}}";
        }

        /// <summary>
        /// Declaration with parameters.
        /// </summary>
        public static string BasicParametersAndModifiers(string typeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial {typeName} Method(
        {typeName} p,
        in {typeName} pIn,
        ref {typeName} pRef,
        out {typeName} pOut);
}}";

        /// <summary>
        /// Declaration with parameters.
        /// </summary>
        public static string BasicParametersAndModifiersNoRef(string typeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial {typeName} Method(
        {typeName} p,
        in {typeName} pIn,
        out {typeName} pOut);
}}";

        /// <summary>
        /// Declaration with parameters and unsafe.
        /// </summary>
        public static string BasicParametersAndModifiersUnsafe(string typeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static unsafe partial {typeName} Method(
        {typeName} p,
        in {typeName} pIn,
        ref {typeName} pRef,
        out {typeName} pOut);
}}";

        public static string BasicParametersAndModifiers<T>(string preDeclaration = "") => BasicParametersAndModifiers(typeof(T).ToString(), preDeclaration);

        /// <summary>
        /// Declaration with [In, Out] style attributes on a by-value parameter.
        /// </summary>
        public static string ByValueParameterWithModifier(string typeName, string attributeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [{attributeName}] {typeName} p);
}}";

        public static string ByValueParameterWithModifier<T>(string attributeName, string preDeclaration = "") => ByValueParameterWithModifier(typeof(T).ToString(), attributeName, preDeclaration);

        /// <summary>
        /// Declaration with by-value parameter with custom name.
        /// </summary>
        public static string ByValueParameterWithName(string methodName, string paramName) => $@"
using System.Runtime.InteropServices;
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void {methodName}(
        int {paramName});
}}";

        /// <summary>
        /// Declaration with parameters with MarshalAs.
        /// </summary>
        public static string MarshalAsParametersAndModifiers(string typeName, UnmanagedType unmanagedType) => $@"
using System.Runtime.InteropServices;
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return: MarshalAs(UnmanagedType.{unmanagedType})]
    public static partial {typeName} Method(
        [MarshalAs(UnmanagedType.{unmanagedType})] {typeName} p,
        [MarshalAs(UnmanagedType.{unmanagedType})] in {typeName} pIn,
        [MarshalAs(UnmanagedType.{unmanagedType})] ref {typeName} pRef,
        [MarshalAs(UnmanagedType.{unmanagedType})] out {typeName} pOut);
}}
";

        /// <summary>
        /// Declaration with parameters with MarshalAs.
        /// </summary>
        public static string MarshalAsParametersAndModifiersUnsafe(string typeName, UnmanagedType unmanagedType) => $@"
using System.Runtime.InteropServices;
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return: MarshalAs(UnmanagedType.{unmanagedType})]
    public static unsafe partial {typeName} Method(
        [MarshalAs(UnmanagedType.{unmanagedType})] {typeName} p,
        [MarshalAs(UnmanagedType.{unmanagedType})] in {typeName} pIn,
        [MarshalAs(UnmanagedType.{unmanagedType})] ref {typeName} pRef,
        [MarshalAs(UnmanagedType.{unmanagedType})] out {typeName} pOut);
}}
";

        public static string MarshalAsParametersAndModifiers<T>(UnmanagedType unmanagedType) => MarshalAsParametersAndModifiers(typeof(T).ToString(), unmanagedType);

        /// <summary>
        /// Declaration with enum parameters.
        /// </summary>
        public static string EnumParameters => $@"
using System.Runtime.InteropServices;
using NS;

namespace NS
{{
    enum MyEnum {{ A, B, C }}
}}

partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial MyEnum Method(
        MyEnum p,
        in MyEnum pIn,
        ref MyEnum pRef,
        out MyEnum pOut);
}}";

        /// <summary>
        /// Declaration with pointer parameters.
        /// </summary>
        public static string PointerParameters<T>() => BasicParametersAndModifiersUnsafe($"{typeof(T)}*");

        /// <summary>
        /// Declaration with PreserveSig = false.
        /// </summary>
        public static string SetLastErrorTrue(string typeName) => $@"
using System.Runtime.InteropServices;
partial class Test
{{
    [LibraryImport(""DoesNotExist"", SetLastError = true)]
    public static partial {typeName} Method({typeName} p);
}}";

        public static string SetLastErrorTrue<T>() => SetLastErrorTrue(typeof(T).ToString());

        public static string DelegateParametersAndModifiers = BasicParametersAndModifiers("MyDelegate") + @"
delegate int MyDelegate(int a);";
        public static string DelegateMarshalAsParametersAndModifiers = MarshalAsParametersAndModifiers("MyDelegate", UnmanagedType.FunctionPtr) + @"
delegate int MyDelegate(int a);";

        private static string BlittableMyStruct(string modifier = "") => $@"#pragma warning disable CS0169
{modifier} unsafe struct MyStruct
{{
    private int i;
    private short s;
    private long l;
    private double d;
    private int* iptr;
    private short* sptr;
    private long* lptr;
    private double* dptr;
    private void* vptr;
}}";

        public static string BlittableStructParametersAndModifiers(string attr) => BasicParametersAndModifiers("MyStruct", attr) + $@"
{BlittableMyStruct()}
";

        public static string MarshalAsArrayParametersAndModifiers(string elementType, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return:MarshalAs(UnmanagedType.LPArray, SizeConst=10)]
    public static partial {elementType}[] Method(
        {elementType}[] p,
        in {elementType}[] pIn,
        int pRefSize,
        [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=2)] ref {elementType}[] pRef,
        [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=5, SizeConst=4)] out {elementType}[] pOut,
        out int pOutSize
        );
}}";

        public static string MarshalAsArrayParametersAndModifiers<T>(string preDeclaration = "") => MarshalAsArrayParametersAndModifiers(typeof(T).ToString(), preDeclaration);

        public static string MarshalAsArrayParameterWithSizeParam(string sizeParamType, bool isByRef) => $@"
using System.Runtime.InteropServices;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        {(isByRef ? "ref" : "")} {sizeParamType} pRefSize,
        [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=0)] ref int[] pRef
        );
}}";

        public static string MarshalAsArrayParameterWithSizeParam<T>(bool isByRef) => MarshalAsArrayParameterWithSizeParam(typeof(T).ToString(), isByRef);


        public static string MarshalAsArrayParameterWithNestedMarshalInfo(string elementType, UnmanagedType nestedMarshalInfo, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalAs(UnmanagedType.LPArray, ArraySubType=UnmanagedType.{nestedMarshalInfo})] {elementType}[] pRef
        );
}}";

        public static string MarshalAsArrayParameterWithNestedMarshalInfo<T>(UnmanagedType nestedMarshalType, string preDeclaration = "") => MarshalAsArrayParameterWithNestedMarshalInfo(typeof(T).ToString(), nestedMarshalType, preDeclaration);

        /// <summary>
        /// Declaration with parameters with MarshalAs.
        /// </summary>
        public static string MarshalUsingParametersAndModifiers(string typeName, string nativeTypeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return: MarshalUsing(typeof({nativeTypeName}))]
    public static partial {typeName} Method(
        [MarshalUsing(typeof({nativeTypeName}))] {typeName} p,
        [MarshalUsing(typeof({nativeTypeName}))] in {typeName} pIn,
        [MarshalUsing(typeof({nativeTypeName}))] ref {typeName} pRef,
        [MarshalUsing(typeof({nativeTypeName}))] out {typeName} pOut);
}}
";
        public static string BasicNonBlittableUserDefinedType(bool defineNativeMarshalling = true) => $@"
{(defineNativeMarshalling ? "[NativeMarshalling(typeof(Native))]" : string.Empty)}
struct S
{{
    public bool b;
}}

[CustomTypeMarshaller(typeof(S))]
struct Native
{{
    private int i;
    public Native(S s)
    {{
        i = s.b ? 1 : 0;
    }}

    public S ToManaged() => new S {{ b = i != 0 }};
}}
";

        public static string CustomStructMarshallingParametersAndModifiers = BasicParametersAndModifiers("S", DisableRuntimeMarshalling) + BasicNonBlittableUserDefinedType(defineNativeMarshalling: false);

        public static string CustomStructMarshallingMarshalUsingParametersAndModifiers = MarshalUsingParametersAndModifiers("S", "Native", DisableRuntimeMarshalling) + BasicNonBlittableUserDefinedType(defineNativeMarshalling: true);

        public static string CustomStructMarshallingStackallocParametersAndModifiersNoRef = BasicParametersAndModifiersNoRef("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Features = CustomTypeMarshallerFeatures.CallerAllocatedBuffer, BufferSize = 1)]
struct Native
{
    private int i;
    public Native(S s, System.Span<byte> b)
    {
        i = s.b ? 1 : 0;
    }

    public S ToManaged() => new S { b = i != 0 };
}
";
        public static string CustomStructMarshallingStackallocOnlyRefParameter = BasicParameterWithByRefModifier("ref", "S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.Out, Features = CustomTypeMarshallerFeatures.CallerAllocatedBuffer, BufferSize = 1)]
struct Native
{
    private int i;
    public Native(S s, System.Span<byte> b)
    {
        i = s.b ? 1 : 0;
    }

    public S ToManaged() => new S { b = i != 0 };
}
";
        public static string CustomStructMarshallingOptionalStackallocParametersAndModifiers = BasicParametersAndModifiers("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Features = CustomTypeMarshallerFeatures.CallerAllocatedBuffer, BufferSize = 1)]
struct Native
{
    private int i;
    public Native(S s, System.Span<byte> b)
    {
        i = s.b ? 1 : 0;
    }
    public Native(S s)
    {
        i = s.b ? 1 : 0;
    }

    public S ToManaged() => new S { b = i != 0 };
}
";

        public static string CustomStructMarshallingStackallocValuePropertyParametersAndModifiersNoRef = BasicParametersAndModifiersNoRef("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Features = CustomTypeMarshallerFeatures.CallerAllocatedBuffer | CustomTypeMarshallerFeatures.TwoStageMarshalling, BufferSize = 1)]
struct Native
{
    public Native(S s, System.Span<byte> b)
    {
    }

    public S ToManaged() => new S { b = true };

    public int ToNativeValue() => throw null;
    public void FromNativeValue(int value) => throw null;
}
";
        public static string CustomStructMarshallingValuePropertyParametersAndModifiers = BasicParametersAndModifiers("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
struct Native
{
    public Native(S s)
    {
    }

    public S ToManaged() => new S { b = true };

    public int ToNativeValue() => throw null;
    public void FromNativeValue(int value) => throw null;
}
";
        public static string CustomStructMarshallingPinnableParametersAndModifiers = BasicParametersAndModifiers("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
class S
{
    public int i;

    public ref int GetPinnableReference() => ref i;
}

[CustomTypeMarshaller(typeof(S), Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
unsafe struct Native
{
    private int* ptr;
    public Native(S s)
    {
        ptr = (int*)Marshal.AllocHGlobal(sizeof(int));
        *ptr = s.i;
    }

    public S ToManaged() => new S { i = *ptr };

    public nint ToNativeValue() => (nint)ptr;

    public void FromNativeValue(nint value) => ptr = (int*)value;
}
";

        public static string CustomStructMarshallingNativeTypePinnable = $@"
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System;

{DisableRuntimeMarshalling}

[NativeMarshalling(typeof(Native))]
class S
{{
    public byte c;
}}

[CustomTypeMarshaller(typeof(S), Features = CustomTypeMarshallerFeatures.CallerAllocatedBuffer | CustomTypeMarshallerFeatures.TwoStageMarshalling, BufferSize = 1)]
unsafe ref struct Native
{{
    private byte* ptr;
    private Span<byte> stackBuffer;

    public Native(S s) : this()
    {{
        ptr = (byte*)Marshal.AllocCoTaskMem(sizeof(byte));
        *ptr = s.c;
        stackBuffer = new Span<byte>(ptr, 1);
    }}

    public Native(S s, Span<byte> buffer) : this()
    {{
        stackBuffer = buffer;
        stackBuffer[0] = s.c;
    }}

    public ref byte GetPinnableReference() => ref stackBuffer.GetPinnableReference();

    public S ToManaged()
    {{
        return new S {{ c = *ptr }};
    }}

    public byte* ToNativeValue() => (byte*)Unsafe.AsPointer(ref GetPinnableReference());

    public void FromNativeValue(byte* value) => ptr = value;

    public void FreeNative()
    {{
        if (ptr != null)
        {{
            Marshal.FreeCoTaskMem((IntPtr)ptr);
        }}
    }}
}}

partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        S s,
        in S sIn);
}}
";

        public static string CustomStructMarshallingByRefValueProperty = BasicParametersAndModifiers("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
class S
{
    public byte c = 0;
}

[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.In, Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
unsafe struct Native
{
    private S value;

    public Native(S s) : this()
    {
        value = s;
    }

    public ref byte ToNativeValue() => ref value.c;
}
";

        public static string BasicParameterWithByRefModifier(string byRefKind, string typeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        {byRefKind} {typeName} p);
}}";

        public static string BasicParameterByValue(string typeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        {typeName} p);
}}";

        public static string BasicReturnType(string typeName, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial {typeName} Method();
}}";

        public static string BasicReturnAndParameterByValue(string returnType, string parameterType, string preDeclaration = "") => $@"
using System.Runtime.InteropServices;
{preDeclaration}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial {returnType} Method({parameterType} p);
}}";

        public static string CustomStructMarshallingManagedToNativeOnlyOutParameter => BasicParameterWithByRefModifier("out", "S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
[StructLayout(LayoutKind.Sequential)]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.In)]
struct Native
{
    private int i;
    public Native(S s)
    {
        i = s.b ? 1 : 0;
    }
}
";

        public static string CustomStructMarshallingNativeToManagedOnlyOutParameter => BasicParameterWithByRefModifier("out", "S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[StructLayout(LayoutKind.Sequential)]
[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.Out)]
struct Native
{
    private int i;
    public S ToManaged() => new S { b = i != 0 };
}
";

        public static string CustomStructMarshallingManagedToNativeOnlyReturnValue => BasicReturnType("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
[StructLayout(LayoutKind.Sequential)]
struct S
{
    public bool b;
}

[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.In)]
struct Native
{
    private int i;
    public Native(S s)
    {
        i = s.b ? 1 : 0;
    }
}
";

        public static string CustomStructMarshallingNativeToManagedOnlyReturnValue => BasicReturnType("S", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[StructLayout(LayoutKind.Sequential)]
[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.Out)]
struct Native
{
    private int i;
    public S ToManaged() => new S { b = i != 0 };
}
";

        public static string CustomStructMarshallingNativeToManagedOnlyInParameter => BasicParameterWithByRefModifier("in", "S", DisableRuntimeMarshalling)  + @"
[NativeMarshalling(typeof(Native))]
struct S
{
    public bool b;
}

[StructLayout(LayoutKind.Sequential)]
[CustomTypeMarshaller(typeof(S), Direction = CustomTypeMarshallerDirection.Out)]
struct Native
{
    private int i;
    public S ToManaged() => new S { b = i != 0 };
}
";

        public static string ArrayMarshallingWithCustomStructElementWithValueProperty => MarshalAsArrayParametersAndModifiers("IntStructWrapper", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(IntStructWrapperNative))]
public struct IntStructWrapper
{
    public int Value;
}

[CustomTypeMarshaller(typeof(IntStructWrapper), Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
public struct IntStructWrapperNative
{
    public IntStructWrapperNative(IntStructWrapper managed)
    {
    }

    public int ToNativeValue() => throw null;
    public void FromNativeValue(int value) => throw null;

    public IntStructWrapper ToManaged() => new IntStructWrapper { Value = 1 };
}
";

        public static string ArrayMarshallingWithCustomStructElement => MarshalAsArrayParametersAndModifiers("IntStructWrapper", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(IntStructWrapperNative))]
public struct IntStructWrapper
{
    public int Value;
}

[CustomTypeMarshaller(typeof(IntStructWrapper))]
public struct IntStructWrapperNative
{
    private int value;

    public IntStructWrapperNative(IntStructWrapper managed)
    {
        value = managed.Value;
    }

    public IntStructWrapper ToManaged() => new IntStructWrapper { Value = value };
}
";

        public static string SafeHandleWithCustomDefaultConstructorAccessibility(bool privateCtor) => BasicParametersAndModifiers("MySafeHandle") + $@"
class MySafeHandle : SafeHandle
{{
    {(privateCtor ? "private" : "public")} MySafeHandle() : base(System.IntPtr.Zero, true) {{ }}

    public override bool IsInvalid => handle == System.IntPtr.Zero;

    protected override bool ReleaseHandle() => true;
}}";

        public static string PreprocessorIfAroundFullFunctionDefinition(string define) =>
            $@"
partial class Test
{{
#if {define}
    [System.Runtime.InteropServices.LibraryImport(""DoesNotExist"")]
    public static partial int Method(
        int p,
        in int pIn,
        out int pOut);
#endif
}}";

        public static string PreprocessorIfAroundFullFunctionDefinitionWithFollowingFunction(string define) =>
            $@"
using System.Runtime.InteropServices;
partial class Test
{{
#if {define}
    [LibraryImport(""DoesNotExist"")]
    public static partial int Method(
        int p,
        in int pIn,
        out int pOut);
#endif
    public static int Method2(
        SafeHandle p) => throw null;
}}";

        public static string PreprocessorIfAfterAttributeAroundFunction(string define) =>
            $@"
using System.Runtime.InteropServices;
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
#if {define}
    public static partial int Method(
        int p,
        in int pIn,
        out int pOut);
#else
    public static partial int Method2(
        int p,
        in int pIn,
        out int pOut);
#endif
}}";

        public static string PreprocessorIfAfterAttributeAroundFunctionAdditionalFunctionAfter(string define) =>
            $@"
using System.Runtime.InteropServices;
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
#if {define}
    public static partial int Method(
        int p,
        in int pIn,
        out int pOut);
#else
    public static partial int Method2(
        int p,
        in int pIn,
        out int pOut);
#endif
    public static int Foo() => throw null;
}}";

        public static string MaybeBlittableGenericTypeParametersAndModifiers(string typeArgument) => BasicParametersAndModifiers($"Generic<{typeArgument}>", DisableRuntimeMarshalling) + @"
struct Generic<T>
{
#pragma warning disable CS0649
    public T field;
}
";

        public static string MaybeBlittableGenericTypeParametersAndModifiers<T>() =>
            MaybeBlittableGenericTypeParametersAndModifiers(typeof(T).ToString());

        public static string RecursiveImplicitlyBlittableStruct => BasicParametersAndModifiers("RecursiveStruct", DisableRuntimeMarshalling) + @"
struct RecursiveStruct
{
    RecursiveStruct s;
    int i;
}";
        public static string MutuallyRecursiveImplicitlyBlittableStruct => BasicParametersAndModifiers("RecursiveStruct1", DisableRuntimeMarshalling) + @"
struct RecursiveStruct1
{
    RecursiveStruct2 s;
    int i;
}

struct RecursiveStruct2
{
    RecursiveStruct1 s;
    int i;
}";

        public static string CollectionByValue(string elementType) => BasicParameterByValue($"TestCollection<{elementType}>", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Marshaller<>))]
class TestCollection<T> {}

[CustomTypeMarshaller(typeof(TestCollection<>), CustomTypeMarshallerKind.LinearCollection, Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
ref struct Marshaller<T>
{
    public Marshaller(int nativeElementSize) : this() {}
    public Marshaller(TestCollection<T> managed, int nativeElementSize) : this() {}
    public System.ReadOnlySpan<T> GetManagedValuesSource() => throw null;
    public System.Span<T> GetManagedValuesDestination(int length) => throw null;
    public System.ReadOnlySpan<byte> GetNativeValuesSource(int length) => throw null;
    public System.Span<byte> GetNativeValuesDestination() => throw null;
    public System.IntPtr ToNativeValue() => throw null;
    public void FromNativeValue(System.IntPtr value) => throw null;
    public TestCollection<T> ToManaged() => throw null;
}
";

        public static string CollectionByValue<T>() => CollectionByValue(typeof(T).ToString());

        public static string MarshalUsingCollectionCountInfoParametersAndModifiers(string collectionType) => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return:MarshalUsing(ConstantElementCount=10)]
    public static partial {collectionType} Method(
        {collectionType} p,
        in {collectionType} pIn,
        int pRefSize,
        [MarshalUsing(CountElementName = ""pRefSize"")] ref {collectionType} pRef,
        [MarshalUsing(CountElementName = ""pOutSize"")] out {collectionType} pOut,
        out int pOutSize
        );
}}";

        public static string CustomCollectionWithMarshaller(bool enableDefaultMarshalling)
        {
            string nativeMarshallingAttribute = enableDefaultMarshalling ? "[NativeMarshalling(typeof(Marshaller<>))]" : string.Empty;
            return nativeMarshallingAttribute + @"class TestCollection<T> {}

[CustomTypeMarshaller(typeof(TestCollection<>), CustomTypeMarshallerKind.LinearCollection, Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
ref struct Marshaller<T>
{
    public Marshaller(int nativeElementSize) : this() {}
    public Marshaller(TestCollection<T> managed, int nativeElementSize) : this() {}
    public System.ReadOnlySpan<T> GetManagedValuesSource() => throw null;
    public System.Span<T> GetManagedValuesDestination(int length) => throw null;
    public System.ReadOnlySpan<byte> GetNativeValuesSource(int length) => throw null;
    public System.Span<byte> GetNativeValuesDestination() => throw null;
    public System.IntPtr ToNativeValue() => throw null;
    public void FromNativeValue(System.IntPtr value) => throw null;
    public TestCollection<T> ToManaged() => throw null;
}";
        }

        public static string MarshalUsingCollectionCountInfoParametersAndModifiers<T>() => MarshalUsingCollectionCountInfoParametersAndModifiers(typeof(T).ToString());

        public static string CustomCollectionDefaultMarshallerParametersAndModifiers(string elementType) => MarshalUsingCollectionCountInfoParametersAndModifiers($"TestCollection<{elementType}>") + CustomCollectionWithMarshaller(enableDefaultMarshalling: true);

        public static string CustomCollectionDefaultMarshallerParametersAndModifiers<T>() => CustomCollectionDefaultMarshallerParametersAndModifiers(typeof(T).ToString());

        public static string MarshalUsingCollectionParametersAndModifiers(string collectionType, string marshallerType) => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return:MarshalUsing(typeof({marshallerType}), ConstantElementCount=10)]
    public static partial {collectionType} Method(
        [MarshalUsing(typeof({marshallerType}))] {collectionType} p,
        [MarshalUsing(typeof({marshallerType}))] in {collectionType} pIn,
        int pRefSize,
        [MarshalUsing(typeof({marshallerType}), CountElementName = ""pRefSize"")] ref {collectionType} pRef,
        [MarshalUsing(typeof({marshallerType}), CountElementName = ""pOutSize"")] out {collectionType} pOut,
        out int pOutSize
        );
}}";

        public static string CustomCollectionCustomMarshallerParametersAndModifiers(string elementType) => MarshalUsingCollectionParametersAndModifiers($"TestCollection<{elementType}>", $"Marshaller<{elementType}>") + CustomCollectionWithMarshaller(enableDefaultMarshalling: false);

        public static string CustomCollectionCustomMarshallerParametersAndModifiers<T>() => CustomCollectionCustomMarshallerParametersAndModifiers(typeof(T).ToString());

        public static string MarshalUsingCollectionReturnValueLength(string collectionType, string marshallerType) => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial int Method(
        [MarshalUsing(typeof({marshallerType}), CountElementName = MarshalUsingAttribute.ReturnsCountValue)] out {collectionType} pOut
        );
}}";

        public static string CustomCollectionCustomMarshallerReturnValueLength(string elementType) => MarshalUsingCollectionReturnValueLength($"TestCollection<{elementType}>", $"Marshaller<{elementType}>") + CustomCollectionWithMarshaller(enableDefaultMarshalling: false);

        public static string CustomCollectionCustomMarshallerReturnValueLength<T>() => CustomCollectionCustomMarshallerReturnValueLength(typeof(T).ToString());

        public static string MarshalUsingArrayParameterWithSizeParam(string sizeParamType, bool isByRef) => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        {(isByRef ? "ref" : "")} {sizeParamType} pRefSize,
        [MarshalUsing(CountElementName = ""pRefSize"")] ref int[] pRef
        );
}}";

        public static string MarshalUsingArrayParameterWithSizeParam<T>(bool isByRef) => MarshalUsingArrayParameterWithSizeParam(typeof(T).ToString(), isByRef);

        public static string MarshalUsingCollectionWithConstantAndElementCount => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        int pRefSize,
        [MarshalUsing(ConstantElementCount = 10, CountElementName = ""pRefSize"")] ref int[] pRef
        );
}}";

        public static string MarshalUsingCollectionWithNullElementName => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        int pRefSize,
        [MarshalUsing(CountElementName = null)] ref int[] pRef
        );
}}";

        public static string GenericCollectionMarshallingArityMismatch => BasicParameterByValue("TestCollection<int>", DisableRuntimeMarshalling) + @"
[NativeMarshalling(typeof(Marshaller<,>))]
class TestCollection<T> {}

[CustomTypeMarshaller(typeof(TestCollection<>), CustomTypeMarshallerKind.LinearCollection, Features = CustomTypeMarshallerFeatures.TwoStageMarshalling)]
ref struct Marshaller<T, U>
{
    public Marshaller(TestCollection<T> managed, int nativeElementSize) : this() {}

    public System.ReadOnlySpan<T> GetManagedValuesSource() => throw null;
    public System.Span<T> GetManagedValuesDestination(int length) => throw null;
    public System.ReadOnlySpan<byte> GetNativeValuesSource(int length) => throw null;
    public System.Span<byte> GetNativeValuesDestination() => throw null;
    public System.IntPtr ToNativeValue() => throw null;
    public void FromNativeValue(System.IntPtr value) => throw null;

    public TestCollection<T> ToManaged() => throw null;
}";

        public static string GenericCollectionWithCustomElementMarshalling => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return:MarshalUsing(ConstantElementCount=10)]
    [return:MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)]
    public static partial TestCollection<int> Method(
        [MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)] TestCollection<int> p,
        [MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)] in TestCollection<int> pIn,
        int pRefSize,
        [MarshalUsing(CountElementName = ""pRefSize""), MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)] ref TestCollection<int> pRef,
        [MarshalUsing(CountElementName = ""pOutSize"")][MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)] out TestCollection<int> pOut,
        out int pOutSize
        );
}}

struct IntWrapper
{{
    public IntWrapper(int i){{}}
    public int ToManaged() => throw null;
}}

" + CustomCollectionWithMarshaller(enableDefaultMarshalling: true);

        public static string GenericCollectionWithCustomElementMarshallingDuplicateElementIndirectionDepth => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)] [MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 1)] TestCollection<int> p);
}}

struct IntWrapper
{{
    public IntWrapper(int i){{}}
    public int ToManaged() => throw null;
}}

" + CustomCollectionWithMarshaller(enableDefaultMarshalling: true);

        public static string GenericCollectionWithCustomElementMarshallingUnusedElementIndirectionDepth => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalUsing(typeof(IntWrapper), ElementIndirectionDepth = 2)] TestCollection<int> p);
}}

struct IntWrapper
{{
    public IntWrapper(int i){{}}
    public int ToManaged() => throw null;
}}

" + CustomCollectionWithMarshaller(enableDefaultMarshalling: true);

        public static string MarshalAsAndMarshalUsingOnReturnValue => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return:MarshalUsing(ConstantElementCount=10)]
    [return:MarshalAs(UnmanagedType.LPArray, SizeConst=10)]
    public static partial int[] Method();
}}
";

        public static string RecursiveCountElementNameOnReturnValue => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    [return:MarshalUsing(CountElementName=MarshalUsingAttribute.ReturnsCountValue)]
    public static partial int[] Method();
}}
";

        public static string RecursiveCountElementNameOnParameter => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalUsing(CountElementName=""arr"")] ref int[] arr
    );
}}
";
        public static string MutuallyRecursiveCountElementNameOnParameter => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalUsing(CountElementName=""arr2"")] ref int[] arr,
        [MarshalUsing(CountElementName=""arr"")] ref int[] arr2
    );
}}
";
        public static string MutuallyRecursiveSizeParamIndexOnParameter => $@"
using System.Runtime.InteropServices;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=1)] ref int[] arr,
        [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=0)] ref int[] arr2
    );
}}
";

        public static string CollectionsOfCollectionsStress => $@"
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
{DisableRuntimeMarshalling}
partial class Test
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial void Method(
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]
        [MarshalUsing(CountElementName=""arr5"", ElementIndirectionDepth = 5)]
        [MarshalUsing(CountElementName=""arr6"", ElementIndirectionDepth = 6)]
        [MarshalUsing(CountElementName=""arr7"", ElementIndirectionDepth = 7)]
        [MarshalUsing(CountElementName=""arr8"", ElementIndirectionDepth = 8)]
        [MarshalUsing(CountElementName=""arr9"", ElementIndirectionDepth = 9)]
        [MarshalUsing(CountElementName=""arr10"", ElementIndirectionDepth = 10)] ref int[][][][][][][][][][][] arr11,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]
        [MarshalUsing(CountElementName=""arr5"", ElementIndirectionDepth = 5)]
        [MarshalUsing(CountElementName=""arr6"", ElementIndirectionDepth = 6)]
        [MarshalUsing(CountElementName=""arr7"", ElementIndirectionDepth = 7)]
        [MarshalUsing(CountElementName=""arr8"", ElementIndirectionDepth = 8)]
        [MarshalUsing(CountElementName=""arr9"", ElementIndirectionDepth = 9)]ref int[][][][][][][][][][][] arr10,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]
        [MarshalUsing(CountElementName=""arr5"", ElementIndirectionDepth = 5)]
        [MarshalUsing(CountElementName=""arr6"", ElementIndirectionDepth = 6)]
        [MarshalUsing(CountElementName=""arr7"", ElementIndirectionDepth = 7)]
        [MarshalUsing(CountElementName=""arr8"", ElementIndirectionDepth = 8)]ref int[][][][][][][][][][] arr9,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]
        [MarshalUsing(CountElementName=""arr5"", ElementIndirectionDepth = 5)]
        [MarshalUsing(CountElementName=""arr6"", ElementIndirectionDepth = 6)]
        [MarshalUsing(CountElementName=""arr7"", ElementIndirectionDepth = 7)]ref int[][][][][][][][][] arr8,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]
        [MarshalUsing(CountElementName=""arr5"", ElementIndirectionDepth = 5)]
        [MarshalUsing(CountElementName=""arr6"", ElementIndirectionDepth = 6)]ref int[][][][][][][] arr7,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]
        [MarshalUsing(CountElementName=""arr5"", ElementIndirectionDepth = 5)]ref int[][][][][][] arr6,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]
        [MarshalUsing(CountElementName=""arr4"", ElementIndirectionDepth = 4)]ref int[][][][][] arr5,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]
        [MarshalUsing(CountElementName=""arr3"", ElementIndirectionDepth = 3)]ref int[][][][] arr4,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]
        [MarshalUsing(CountElementName=""arr2"", ElementIndirectionDepth = 2)]ref int[][][] arr3,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]
        [MarshalUsing(CountElementName=""arr1"", ElementIndirectionDepth = 1)]ref int[][] arr2,
        [MarshalUsing(CountElementName=""arr0"", ElementIndirectionDepth = 0)]ref int[] arr1,
        ref int arr0
    );
}}
";

        public static string RefReturn(string typeName) => $@"
using System.Runtime.InteropServices;
partial struct Basic
{{
    [LibraryImport(""DoesNotExist"")]
    public static partial ref {typeName} RefReturn();
    [LibraryImport(""DoesNotExist"")]
    public static partial ref readonly {typeName} RefReadonlyReturn();
}}";

        public static string PartialPropertyName => @"
using System.Runtime.InteropServices;

partial struct Basic
{
    [LibraryImport(""DoesNotExist"", SetLa)]
    public static partial void Method();
}
";
        public static string InvalidConstantForModuleName => @"
using System.Runtime.InteropServices;

partial struct Basic
{
    [LibraryImport(DoesNotExist)]
    public static partial void Method();
}
";
        public static string IncorrectAttributeFieldType => @"
using System.Runtime.InteropServices;

partial struct Basic
{
    [LibraryImport(""DoesNotExist"", SetLastError = ""Foo"")]
    public static partial void Method();
}
";

        public static class ValidateDisableRuntimeMarshalling
        {
            public static string NonBlittableUserDefinedTypeWithNativeType = $@"
public struct S
{{
    public string s;
}}

[CustomTypeMarshaller(typeof(S))]
public struct Native
{{
    private System.IntPtr p;
    public Native(S s)
    {{
        p = System.IntPtr.Zero;
    }}

    public S ToManaged() => new S {{ s = string.Empty }};
}}
";

            public static string TypeUsage(string attr) => MarshalUsingParametersAndModifiers("S", "Native", attr);
        }
    }
}
