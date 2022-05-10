// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.CodeAnalysis;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json.Linq;
using System.Text.RegularExpressions;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.CSharp;
using System.Reflection;
using System.Text;
using System.Runtime.CompilerServices;
using BrowserDebugProxy;

namespace Microsoft.WebAssembly.Diagnostics
{
    internal enum TokenType
    {
        MdtModule               = 0x00000000,       //
        MdtTypeRef              = 0x01000000,       //
        MdtTypeDef              = 0x02000000,       //
        MdtFieldDef             = 0x04000000,       //
        MdtMethodDef            = 0x06000000,       //
        MdtParamDef             = 0x08000000,       //
        MdtInterfaceImpl        = 0x09000000,       //
        MdtMemberRef            = 0x0a000000,       //
        MdtCustomAttribute      = 0x0c000000,       //
        MdtPermission           = 0x0e000000,       //
        MdtSignature            = 0x11000000,       //
        MdtEvent                = 0x14000000,       //
        MdtProperty             = 0x17000000,       //
        MdtModuleRef            = 0x1a000000,       //
        MdtTypeSpec             = 0x1b000000,       //
        MdtAssembly             = 0x20000000,       //
        MdtAssemblyRef          = 0x23000000,       //
        MdtFile                 = 0x26000000,       //
        MdtExportedType         = 0x27000000,       //
        MdtManifestResource     = 0x28000000,       //
        MdtGenericParam         = 0x2a000000,       //
        MdtMethodSpec           = 0x2b000000,       //
        MdtGenericParamConstraint = 0x2c000000,
        MdtString               = 0x70000000,       //
        MdtName                 = 0x71000000,       //
        MdtBaseType             = 0x72000000,       // Leave this on the high end value. This does not correspond to metadata table
    }

    [Flags]
    internal enum GetObjectCommandOptions
    {
        None = 0,
        WithSetter = 1,
        AccessorPropertiesOnly = 2,
        OwnProperties = 4,
        ForDebuggerProxyAttribute = 8,
        ForDebuggerDisplayAttribute = 16,
        WithProperties = 32
    }

    internal enum CommandSet {
        Vm = 1,
        ObjectRef = 9,
        StringRef = 10,
        Thread = 11,
        ArrayRef = 13,
        EventRequest = 15,
        StackFrame = 16,
        AppDomain = 20,
        Assembly = 21,
        Method = 22,
        Type = 23,
        Module = 24,
        Field = 25,
        Event = 64,
        Pointer = 65
    }

    internal enum EventKind {
        VmStart = 0,
        VmDeath = 1,
        ThreadStart = 2,
        ThreadDeath = 3,
        AppDomainCreate = 4,
        AppDomainUnload = 5,
        MethodEntry = 6,
        MethodExit = 7,
        AssemblyLoad = 8,
        AssemblyUnload = 9,
        Breakpoint = 10,
        Step = 11,
        TypeLoad = 12,
        Exception = 13,
        KeepAlive = 14,
        UserBreak = 15,
        UserLog = 16,
        Crash = 17,
        EnC = 18,
        MethodUpdate = 19
    }

    internal enum ModifierKind {
        Count = 1,
        ThreadOnly = 3,
        LocationOnly = 7,
        ExceptionOnly = 8,
        Step = 10,
        AssemblyOnly = 11,
        SourceFileOnly = 12,
        TypeNameOnly = 13
    }


    internal enum SuspendPolicy {
        None = 0,
        EventThread = 1,
        All = 2
    }

    internal enum CmdVM {
        Version = 1,
        AllThreads = 2,
        Suspend = 3,
        Resume = 4,
        Exit = 5,
        Dispose = 6,
        InvokeMethod = 7,
        SetProtocolVersion = 8,
        AbortInvoke = 9,
        SetKeepAlive = 10,
        GetTypesForSourceFile = 11,
        GetTypes = 12,
        InvokeMethods = 13,
        StartBuffering = 14,
        StopBuffering = 15,
        VmReadMemory = 16,
        VmWriteMemory = 17,
        GetAssemblyByName = 18,
        GetModuleByGUID = 19
    }

    internal enum CmdFrame {
        GetValues = 1,
        GetThis = 2,
        SetValues = 3,
        GetDomain = 4,
        SetThis = 5,
        GetArgument = 6,
        GetArguments = 7
    }

    internal enum CmdEvent {
        Composite = 100
    }

    internal enum CmdThread {
        GetFrameInfo = 1,
        GetName = 2,
        GetState = 3,
        GetInfo = 4,
        /* FIXME: Merge into GetInfo when the major protocol version is increased */
        GetId = 5,
        /* Ditto */
        GetTid = 6,
        SetIp = 7,
        GetElapsedTime = 8
    }

    internal enum CmdEventRequest {
        Set = 1,
        Clear = 2,
        ClearAllBreakpoints = 3
    }

    internal enum CmdAppDomain {
        GetRootDomain = 1,
        GetFriendlyName = 2,
        GetAssemblies = 3,
        GetEntryAssembly = 4,
        CreateString = 5,
        GetCorLib = 6,
        CreateBoxedValue = 7,
        CreateByteArray = 8,
    }

    internal enum CmdAssembly {
        GetLocation = 1,
        GetEntryPoint = 2,
        GetManifestModule = 3,
        GetObject = 4,
        GetType = 5,
        GetName = 6,
        GetDomain = 7,
        GetMetadataBlob = 8,
        GetIsDynamic = 9,
        GetPdbBlob = 10,
        GetTypeFromToken = 11,
        GetMethodFromToken = 12,
        HasDebugInfo = 13,
    }

    internal enum CmdModule {
        GetInfo = 1,
        ApplyChanges = 2,
    }

    internal enum CmdPointer{
        GetValue = 1
    }

    internal enum CmdMethod {
        GetName = 1,
        GetDeclaringType = 2,
        GetDebugInfo = 3,
        GetParamInfo = 4,
        GetLocalsInfo = 5,
        GetInfo = 6,
        GetBody = 7,
        ResolveToken = 8,
        GetCattrs = 9,
        MakeGenericMethod = 10,
        Token = 11,
        Assembly = 12,
        ClassToken = 13,
        AsyncDebugInfo = 14,
        GetNameFull = 15
    }

    internal enum CmdType {
        GetInfo = 1,
        GetMethods = 2,
        GetFields = 3,
        GetValues = 4,
        GetObject = 5,
        GetSourceFiles = 6,
        SetValues = 7,
        IsAssignableFrom = 8,
        GetProperties = 9,
        GetCattrs = 10,
        GetFieldCattrs = 11,
        GetPropertyCattrs = 12,
        /* FIXME: Merge into GetSourceFiles when the major protocol version is increased */
        GetSourceFiles2 = 13,
        /* FIXME: Merge into GetValues when the major protocol version is increased */
        GetValues2 = 14,
        GetMethodsByNameFlags = 15,
        GetInterfaces = 16,
        GetInterfacesMap = 17,
        IsInitialized = 18,
        CreateInstance = 19,
        GetValueSize = 20,
        GetValuesICorDbg = 21,
        GetParents = 22,
        Initialize = 23,
    }

    internal enum CmdArray {
        GetLength = 1,
        GetValues = 2,
        SetValues = 3,
        RefGetType = 4
    }


    internal enum CmdField {
        GetInfo = 1
    }

    internal enum CmdString {
        GetValue = 1,
        GetLength = 2,
        GetChars = 3
    }

    internal enum CmdObject {
        RefGetType = 1,
        RefGetValues = 2,
        RefIsCollected = 3,
        RefGetAddress = 4,
        RefGetDomain = 5,
        RefSetValues = 6,
        RefGetInfo = 7,
        GetValuesICorDbg = 8,
        RefDelegateGetMethod = 9,
        RefIsDelegate = 10
    }

    internal enum ElementType {
        End             = 0x00,
        Void            = 0x01,
        Boolean         = 0x02,
        Char            = 0x03,
        I1              = 0x04,
        U1              = 0x05,
        I2              = 0x06,
        U2              = 0x07,
        I4              = 0x08,
        U4              = 0x09,
        I8              = 0x0a,
        U8              = 0x0b,
        R4              = 0x0c,
        R8              = 0x0d,
        String          = 0x0e,
        Ptr             = 0x0f,
        ByRef           = 0x10,
        ValueType       = 0x11,
        Class           = 0x12,
        Var             = 0x13,
        Array           = 0x14,
        GenericInst     = 0x15,
        TypedByRef      = 0x16,
        I               = 0x18,
        U               = 0x19,
        FnPtr           = 0x1b,
        Object          = 0x1c,
        SzArray         = 0x1d,
        MVar            = 0x1e,
        CModReqD        = 0x1f,
        CModOpt         = 0x20,
        Internal        = 0x21,
        Modifier        = 0x40,
        Sentinel        = 0x41,
        Pinned          = 0x45,

        Type            = 0x50,
        Boxed           = 0x51,
        Enum            = 0x55
    }

    internal enum ValueTypeId {
        Null = 0xf0,
        Type = 0xf1,
        VType = 0xf2,
        FixedArray = 0xf3
    }
    internal enum MonoTypeNameFormat{
        FormatIL,
        FormatReflection,
        FullName,
        AssemblyQualified
    }

    internal enum StepFilter {
        None = 0,
        StaticCtor = 1,
        DebuggerHidden = 2,
        DebuggerStepThrough = 4,
        DebuggerNonUserCode = 8
    }

    internal enum StepSize
    {
        Minimal,
        Line
    }

    internal sealed record ArrayDimensions
    {
        internal int Rank { get; }
        internal int [] Bounds { get; }
        internal int TotalLength { get; }
        public ArrayDimensions(int [] rank)
        {
            Rank = rank.Length;
            Bounds = rank;
            TotalLength = 1;
            for (int i = 0 ; i < Rank ; i++)
                TotalLength *= Bounds[i];
        }

        public override string ToString()
        {
            return $"{string.Join(", ", Bounds)}";
        }
        internal string GetArrayIndexString(int idx)
        {
            if (idx < 0 || idx >= TotalLength)
                return "Invalid Index";
            int[] arrayStr = new int[Rank];
            int rankStart = 0;
            while (idx > 0)
            {
                int boundLimit = 1;
                for (int i = Rank - 1; i >= rankStart; i--)
                {
                    int lastBoundLimit = boundLimit;
                    boundLimit *= Bounds[i];
                    if (idx < boundLimit)
                    {
                        arrayStr[i] = (int)(idx / lastBoundLimit);
                        idx -= arrayStr[i] * lastBoundLimit;
                        rankStart = i;
                        break;
                    }
                }
            }
            return $"{string.Join(", ", arrayStr)}";
        }
    }

    internal sealed record MethodInfoWithDebugInformation(MethodInfo Info, int DebugId, string Name);

    internal sealed class TypeInfoWithDebugInformation
    {
        public TypeInfo Info { get; }
        public int DebugId { get; }
        public string Name { get; }
        public List<FieldTypeClass> FieldsList { get; set; }
        public byte[] PropertiesBuffer { get; set; }
        public List<int> TypeParamsOrArgsForGenericType { get; set; }

        public TypeInfoWithDebugInformation(TypeInfo typeInfo, int debugId, string name)
        {
            Info = typeInfo;
            DebugId = debugId;
            Name = name;
        }
    }

    internal sealed class MonoBinaryReader : BinaryReader
    {
        public bool HasError { get; }

        private MonoBinaryReader(Stream stream, bool hasError = false) : base(stream)
        {
            HasError = hasError;
        }

        public MonoBinaryReader(byte [] data) : base(new MemoryStream(data)) {}

        public static MonoBinaryReader From(Result result)
        {
            byte[] newBytes = Array.Empty<byte>();
            if (result.IsOk) {
                newBytes = Convert.FromBase64String(result.Value?["result"]?["value"]?["value"]?.Value<string>());
            }
            return new MonoBinaryReader(new MemoryStream(newBytes), !result.IsOk);
        }

        public override string ReadString()
        {
            var valueLen = ReadInt32();
            byte[] value = new byte[valueLen];
            Read(value, 0, valueLen);

            return new string(Encoding.UTF8.GetChars(value, 0, valueLen));
        }

        // SDB encodes these as 4 bytes
        public override sbyte ReadSByte() => (sbyte)ReadInt32();
        public byte ReadUByte() => (byte)ReadUInt32();
        public ushort ReadUShort() => (ushort)ReadUInt32();

        // Big endian overrides
        public override int ReadInt32() => ReadBigEndian<int>();
        public override double ReadDouble() => ReadBigEndian<double>();
        public override uint ReadUInt32() => ReadBigEndian<uint>();
        public override float ReadSingle() => ReadBigEndian<float>();
        public override ulong ReadUInt64() => ReadBigEndian<ulong>();
        public override long ReadInt64() => ReadBigEndian<long>();

        private unsafe T ReadBigEndian<T>() where T : struct
        {
            Span<byte> data = stackalloc byte[Unsafe.SizeOf<T>()];
            T ret = default;
            Read(data);
            if (BitConverter.IsLittleEndian)
            {
                data.Reverse();
            }
            data.CopyTo(new Span<byte>(Unsafe.AsPointer(ref ret), data.Length));
            return ret;
        }
    }

    internal sealed class MonoBinaryWriter : BinaryWriter
    {
        public MonoBinaryWriter() : base(new MemoryStream(20)) {}

        public override void Write(string val)
        {
            var bytes = Encoding.UTF8.GetBytes(val);
            WriteByteArray(bytes);
        }

        public override void Write(long val) => WriteBigEndian<long>(val);
        public override void Write(int val) => WriteBigEndian<int>(val);

        private unsafe void WriteBigEndian<T>(T val) where T : struct
        {
            Span<byte> data = stackalloc byte[Unsafe.SizeOf<T>()];
            new Span<byte>(Unsafe.AsPointer(ref val), data.Length).CopyTo(data);
            if (BitConverter.IsLittleEndian)
            {
                data.Reverse();
            }
            base.Write(data);
        }

        internal void Write<T>(ElementType type, T value) where T : struct => Write((byte)type, value);

        private void Write<T1, T2>(T1 type, T2 value) where T1 : struct where T2 : struct
        {
            WriteBigEndian(type);
            WriteBigEndian(value);
        }

        public void WriteObj(DotnetObjectId objectId, MonoSDBHelper SdbHelper)
        {
            if (objectId.Scheme == "object")
            {
                Write(ElementType.Class, objectId.Value);
            }
            else if (objectId.Scheme == "valuetype")
            {
                Write(SdbHelper.valueTypes[objectId.Value].Buffer);
            }
        }

        public void WriteByteArray(byte[] bytes)
        {
            Write(bytes.Length);
            Write(bytes);
        }

        public async Task<bool> WriteConst(ElementType? type, object value, MonoSDBHelper SdbHelper, CancellationToken token)
        {
            switch (type)
            {
                case ElementType.Boolean:
                case ElementType.Char:
                case ElementType.U1:
                case ElementType.I2:
                case ElementType.I4:
                    Write((ElementType)type, (int)value);
                    return true;
                case ElementType.I1:
                case ElementType.U2:
                case ElementType.U4:
                    Write((ElementType)type, (uint)value);
                    return true;
                case ElementType.I8:
                    Write((ElementType)type, (long)value);
                    return true;
                case ElementType.U8:
                    Write((ElementType)type, (ulong)value);
                    return true;
                case ElementType.R4:
                    Write((ElementType)type, (float)value);
                    return true;
                case ElementType.R8:
                    Write((ElementType)type, (double)value);
                    return true;
                case ElementType.String:
                    int stringId = await SdbHelper.CreateString((string)value, token);
                    Write(ElementType.String, stringId);
                    return true;
                case null:
                    if (value == null)
                        return false;
                    //ConstantTypeCode.NullReference
                    Write((byte)value);
                    Write((byte)0); //not used
                    Write((int)0);  //not used
                    return true;
            }
            return false;
        }

        public async Task<bool> WriteConst(LiteralExpressionSyntax constValue, MonoSDBHelper SdbHelper, CancellationToken token)
        {
            switch (constValue.Kind())
            {
                case SyntaxKind.NumericLiteralExpression:
                {
                    switch (constValue.Token.Value) {
                        case double d:
                            Write(ElementType.R8, d);
                            break;
                        case float f:
                            Write(ElementType.R4, f);
                            break;
                        case long l:
                            Write(ElementType.I8, l);
                            break;
                        case ulong ul:
                            Write(ElementType.U8, ul);
                            break;
                        case byte b:
                            Write(ElementType.U1, (int)b);
                            break;
                        case sbyte sb:
                            Write(ElementType.I1, (uint)sb);
                            break;
                        case ushort us:
                            Write(ElementType.U2, (int)us);
                            break;
                        case short s:
                            Write(ElementType.I2, (uint)s);
                            break;
                        case uint ui:
                            Write(ElementType.U4, ui);
                            break;
                        case IntPtr ip:
                            Write(ElementType.I, (int)ip);
                            break;
                        case UIntPtr up:
                            Write(ElementType.U, (uint)up);
                            break;
                        default:
                            Write(ElementType.I4, (int)constValue.Token.Value);
                            break;
                    }
                    return true;
                }
                case SyntaxKind.StringLiteralExpression:
                {
                    int stringId = await SdbHelper.CreateString((string)constValue.Token.Value, token);
                    Write(ElementType.String, stringId);
                    return true;
                }
                case SyntaxKind.TrueLiteralExpression:
                {
                    Write(ElementType.Boolean, 1);
                    return true;
                }
                case SyntaxKind.FalseLiteralExpression:
                {
                    Write(ElementType.Boolean, 0);
                    return true;
                }
                case SyntaxKind.NullLiteralExpression:
                {
                    Write((byte)ValueTypeId.Null);
                    Write((byte)0); //not used
                    Write((int)0);  //not used
                    return true;
                }
                case SyntaxKind.CharacterLiteralExpression:
                {
                    Write(ElementType.Char, (int)(char)constValue.Token.Value);
                    return true;
                }
            }
            return false;
        }

        public async Task<bool> WriteJsonValue(JObject objValue, MonoSDBHelper SdbHelper, CancellationToken token)
        {
            switch (objValue["type"].Value<string>())
            {
                case "number":
                {
                    Write(ElementType.I4, objValue["value"].Value<int>());
                    return true;
                }
                case "string":
                {
                    int stringId = await SdbHelper.CreateString(objValue["value"].Value<string>(), token);
                    Write(ElementType.String, stringId);
                    return true;
                }
                case "boolean":
                {
                    Write(ElementType.Boolean, objValue["value"].Value<bool>() ? 1 : 0);
                    return true;
                }
                case "object":
                {
                    DotnetObjectId.TryParse(objValue["objectId"]?.Value<string>(), out DotnetObjectId objectId);
                    WriteObj(objectId, SdbHelper);
                    return true;
                }
            }
            return false;
        }

        public ArraySegment<byte> GetParameterBuffer()
        {
            ((MemoryStream)BaseStream).TryGetBuffer(out var segment);
            return segment;
        }

        public (string data, int length) ToBase64() {
            var segment = GetParameterBuffer();
            return (Convert.ToBase64String(segment), segment.Count);
        }
    }
    internal sealed class FieldTypeClass
    {
        public int Id { get; }
        public string Name { get; }
        public int TypeId { get; }
        public bool IsNotPrivate { get; }
        public bool IsBackingField { get; }
        public FieldAttributes Attributes { get; }
        public FieldTypeClass(int id, string name, int typeId, bool isBackingField, FieldAttributes attributes)
        {
            Id = id;
            Name = name;
            TypeId = typeId;
            IsNotPrivate = (Attributes & FieldAttributes.FieldAccessMask & FieldAttributes.Public) != 0;
            Attributes = attributes;
            IsBackingField = isBackingField;
        }
    }

    internal sealed class PointerValue
    {
        public long address;
        public int typeId;
        public string varName;
        private JObject _value;

        public PointerValue(long address, int typeId, string varName)
        {
            this.address = address;
            this.typeId = typeId;
            this.varName = varName;
        }

        public async Task<JObject> GetValue(MonoSDBHelper sdbHelper, CancellationToken token)
        {
            if (_value == null)
            {
                using var commandParamsWriter = new MonoBinaryWriter();
                commandParamsWriter.Write(address);
                commandParamsWriter.Write(typeId);
                using var retDebuggerCmdReader = await sdbHelper.SendDebuggerAgentCommand(CmdPointer.GetValue, commandParamsWriter, token);
                string displayVarName = varName;
                if (int.TryParse(varName, out _))
                    displayVarName = $"[{varName}]";
                _value = await sdbHelper.CreateJObjectForVariableValue(retDebuggerCmdReader, "*" + displayVarName, token);
            }

            return _value;
        }
    }
    internal sealed class MonoSDBHelper
    {
        private static int debuggerObjectId;
        private static int cmdId = 1; //cmdId == 0 is used by events which come from runtime
        private static int MINOR_VERSION = 61;
        private static int MAJOR_VERSION = 2;

        private Dictionary<int, MethodInfoWithDebugInformation> methods;
        private Dictionary<int, AssemblyInfo> assemblies;
        private Dictionary<int, TypeInfoWithDebugInformation> types;

        internal Dictionary<int, ValueTypeClass> valueTypes = new Dictionary<int, ValueTypeClass>();
        internal Dictionary<int, PointerValue> pointerValues = new Dictionary<int, PointerValue>();

        private MonoProxy proxy;
        private DebugStore store;
        private SessionId sessionId;

        private readonly ILogger logger;
        private Regex regexForAsyncLocals = new Regex(@"\<([^)]*)\>", RegexOptions.Singleline);

        public static int GetNewId() { return cmdId++; }
        public static int GetNewObjectId() => Interlocked.Increment(ref debuggerObjectId);

        public MonoSDBHelper(MonoProxy proxy, ILogger logger, SessionId sessionId)
        {
            this.proxy = proxy;
            this.logger = logger;
            this.sessionId = sessionId;
            ResetStore(null);
        }

        public void ResetStore(DebugStore store)
        {
            this.store = store;
            this.methods = new();
            this.assemblies = new();
            this.types = new();
            ClearCache();
        }

        public async Task<AssemblyInfo> GetAssemblyInfo(int assemblyId, CancellationToken token)
        {
            if (assemblies.TryGetValue(assemblyId, out AssemblyInfo asm))
            {
                return asm;
            }
            var assemblyName = await GetAssemblyName(assemblyId, token);

            asm = store.GetAssemblyByName(assemblyName);

            if (asm == null)
            {
                assemblyName = await GetAssemblyFileNameFromId(assemblyId, token); //maybe is a lazy loaded assembly
                asm = store.GetAssemblyByName(assemblyName);
                if (asm == null)
                {
                    logger.LogDebug($"Unable to find assembly: {assemblyName}");
                    return null;
                }
            }
            asm.SetDebugId(assemblyId);
            assemblies[assemblyId] = asm;
            return asm;
        }

        public async Task<MethodInfoWithDebugInformation> GetMethodInfo(int methodId, CancellationToken token)
        {
            if (methods.TryGetValue(methodId, out MethodInfoWithDebugInformation methodDebugInfo))
            {
                return methodDebugInfo;
            }
            var methodToken = await GetMethodToken(methodId, token);
            var assemblyId = await GetAssemblyIdFromMethod(methodId, token);

            var asm = await GetAssemblyInfo(assemblyId, token);

            if (asm == null)
            {
                logger.LogDebug($"Unable to find assembly: {assemblyId}");
                return null;
            }

            var method = asm.GetMethodByToken(methodToken);

            if (method == null && !asm.HasSymbols)
            {
                try
                {
                    method = await proxy.LoadSymbolsOnDemand(asm, methodToken, sessionId, token);
                }
                catch (Exception e)
                {
                    logger.LogDebug($"Unable to find method token: {methodToken} assembly name: {asm.Name} exception: {e}");
                    return null;
                }
            }

            if (method == null)
            {
                logger.LogDebug($"Unable to find method token: {methodToken} assembly name: {asm.Name}");
                return null;
            }

            string methodName = await GetMethodName(methodId, token);
            methods[methodId] = new MethodInfoWithDebugInformation(method, methodId, methodName);
            return methods[methodId];
        }

        public async Task<TypeInfoWithDebugInformation> GetTypeInfo(int typeId, CancellationToken token)
        {
            if (types.TryGetValue(typeId, out TypeInfoWithDebugInformation typeDebugInfo))
            {
                return typeDebugInfo;
            }

            var typeToken = await GetTypeToken(typeId, token);
            var typeName = await GetTypeName(typeId, token);
            var assemblyId = await GetAssemblyFromType(typeId, token);
            var asm = await GetAssemblyInfo(assemblyId, token);

            if (asm == null)
            {
                logger.LogDebug($"Unable to find assembly: {assemblyId}");
                return null;
            }

            asm.TypesByToken.TryGetValue(typeToken, out TypeInfo type);

            if (type == null)
            {
                logger.LogDebug($"Unable to find type token: {typeName} assembly name: {asm.Name}");
                return null;
            }

            types[typeId] = new TypeInfoWithDebugInformation(type, typeId, typeName);
            return types[typeId];
        }

        public void ClearCache()
        {
            valueTypes = new Dictionary<int, ValueTypeClass>();
            pointerValues = new Dictionary<int, PointerValue>();
        }

        public async Task<bool> SetProtocolVersion(CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(MAJOR_VERSION);
            commandParamsWriter.Write(MINOR_VERSION);
            commandParamsWriter.Write((byte)0);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdVM.SetProtocolVersion, commandParamsWriter, token);
            return true;
        }

        public async Task<bool> EnableReceiveRequests(EventKind eventKind, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((byte)eventKind);
            commandParamsWriter.Write((byte)SuspendPolicy.None);
            commandParamsWriter.Write((byte)0);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdEventRequest.Set, commandParamsWriter, token);
            return true;
        }

        internal async Task<MonoBinaryReader> SendDebuggerAgentCommand<T>(T command, MonoBinaryWriter arguments, CancellationToken token, bool throwOnError = true)
        {
            Result res = await proxy.SendMonoCommand(sessionId, MonoCommands.SendDebuggerAgentCommand(proxy.RuntimeId, GetNewId(), (int)GetCommandSetForCommand(command), (int)(object)command, arguments?.ToBase64().data ?? string.Empty), token);
            return !res.IsOk && throwOnError
                        ? throw new DebuggerAgentException($"SendDebuggerAgentCommand failed for {command}: {res}")
                        : MonoBinaryReader.From(res);
        }

        private static CommandSet GetCommandSetForCommand<T>(T command) =>
            command switch {
                CmdVM => CommandSet.Vm,
                CmdObject => CommandSet.ObjectRef,
                CmdString => CommandSet.StringRef,
                CmdThread => CommandSet.Thread,
                CmdArray => CommandSet.ArrayRef,
                CmdEventRequest => CommandSet.EventRequest,
                CmdFrame => CommandSet.StackFrame,
                CmdAppDomain => CommandSet.AppDomain,
                CmdAssembly => CommandSet.Assembly,
                CmdMethod => CommandSet.Method,
                CmdType => CommandSet.Type,
                CmdModule => CommandSet.Module,
                CmdField => CommandSet.Field,
                CmdEvent => CommandSet.Event,
                CmdPointer => CommandSet.Pointer,
                _ => throw new Exception ("Unknown CommandSet")
            };

        internal async Task<MonoBinaryReader> SendDebuggerAgentCommandWithParms<T>(T command, (string data, int length) encoded, int type, string extraParm, CancellationToken token, bool throwOnError = true)
        {
            Result res = await proxy.SendMonoCommand(sessionId, MonoCommands.SendDebuggerAgentCommandWithParms(proxy.RuntimeId, GetNewId(), (int)GetCommandSetForCommand(command), (int)(object)command, encoded.data, encoded.length, type, extraParm), token);
            return !res.IsOk && throwOnError
                        ? throw new DebuggerAgentException($"SendDebuggerAgentCommand failed for {command}: {res.Error}")
                        : MonoBinaryReader.From(res);
        }

        public async Task<int> CreateString(string value, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAppDomain.GetRootDomain, commandParamsWriter, token);
            var root = retDebuggerCmdReader.ReadInt32();
            commandParamsWriter.Write(root);
            commandParamsWriter.Write(value);
            using var stringDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAppDomain.CreateString, commandParamsWriter, token);
            return stringDebuggerCmdReader.ReadInt32();
        }

        public async Task<int> GetMethodToken(int methodId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.Token, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32() & 0xffffff; //token
        }

        public async Task<int> MakeGenericMethod(int methodId, List<int> genericTypes, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);
            commandParamsWriter.Write(genericTypes.Count);
            foreach (var genericType in genericTypes)
            {
                commandParamsWriter.Write(genericType);
            }
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.MakeGenericMethod, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<int> GetMethodIdByToken(int assembly_id, int method_token, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(assembly_id);
            commandParamsWriter.Write(method_token | (int)TokenType.MdtMethodDef);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAssembly.GetMethodFromToken, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<int> GetAssemblyIdFromType(int type_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(type_id);
            commandParamsWriter.Write((int) MonoTypeNameFormat.FormatReflection);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetInfo, commandParamsWriter, token);
            retDebuggerCmdReader.ReadString(); //namespace
            retDebuggerCmdReader.ReadString(); //name
            retDebuggerCmdReader.ReadString(); //formatted name
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<List<int>> GetTypeParamsOrArgsForGenericType(int typeId, CancellationToken token)
        {
            var typeInfo = await GetTypeInfo(typeId, token);

            if (typeInfo is null)
                return null;

            if (typeInfo.TypeParamsOrArgsForGenericType != null)
                return typeInfo.TypeParamsOrArgsForGenericType;

            var ret = new List<int>();
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);
            commandParamsWriter.Write((int) MonoTypeNameFormat.FormatReflection);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetInfo, commandParamsWriter, token);

            retDebuggerCmdReader.ReadString(); //namespace
            retDebuggerCmdReader.ReadString(); //name
            retDebuggerCmdReader.ReadString(); //name full
            retDebuggerCmdReader.ReadInt32(); //assembly_id
            retDebuggerCmdReader.ReadInt32(); //module_id
            retDebuggerCmdReader.ReadInt32(); //type_id
            retDebuggerCmdReader.ReadInt32(); //rank type
            retDebuggerCmdReader.ReadInt32(); //type token
            retDebuggerCmdReader.ReadByte(); //rank
            retDebuggerCmdReader.ReadInt32(); //flags
            retDebuggerCmdReader.ReadByte();
            int nested = retDebuggerCmdReader.ReadInt32();
            for (int i = 0 ; i < nested; i++)
            {
                retDebuggerCmdReader.ReadInt32(); //nested type
            }
            retDebuggerCmdReader.ReadInt32(); //typeid
            int generics = retDebuggerCmdReader.ReadInt32();
            for (int i = 0 ; i < generics; i++)
            {
                ret.Add(retDebuggerCmdReader.ReadInt32()); //generic type
            }

            typeInfo.TypeParamsOrArgsForGenericType = ret;

            return ret;
        }

        public async Task<int> GetAssemblyIdFromMethod(int methodId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.Assembly, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32(); //assembly_id
        }

        public async Task<int> GetAssemblyId(string asm_name, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(asm_name);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdVM.GetAssemblyByName, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<int> GetModuleId(string moduleGuid, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            var guidArray = Convert.FromBase64String(moduleGuid);
            commandParamsWriter.WriteByteArray(guidArray);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdVM.GetModuleByGUID, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<string> GetAssemblyNameFromModule(int moduleId, CancellationToken token)
        {
            using var command_params_writer = new MonoBinaryWriter();
            command_params_writer.Write(moduleId);

            using var ret_debugger_cmd_reader = await SendDebuggerAgentCommand(CmdModule.GetInfo, command_params_writer, token);
            ret_debugger_cmd_reader.ReadString();
            return ret_debugger_cmd_reader.ReadString();
        }

        public async Task<string> GetAssemblyName(int assembly_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(assembly_id);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAssembly.GetLocation, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadString();
        }

        public async Task<string> GetFullAssemblyName(int assemblyId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(assemblyId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAssembly.GetName, commandParamsWriter, token);
            var name = retDebuggerCmdReader.ReadString();
            return name;
        }

        public async Task<string> GetAssemblyFileNameFromId(int assemblyId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(assemblyId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAssembly.GetName, commandParamsWriter, token);
            var name = retDebuggerCmdReader.ReadString();
            return name.Remove(name.IndexOf(",")) + ".dll";
        }

        public async Task<string> GetMethodName(int methodId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.GetNameFull, commandParamsWriter, token);
            var methodName = retDebuggerCmdReader.ReadString();
            return methodName.Substring(methodName.IndexOf(":")+1);
        }

        public async Task<bool> MethodIsStatic(int methodId, CancellationToken token)
        {
            var methodInfo = await GetMethodInfo(methodId, token);
            if (methodInfo != null)
                return methodInfo.Info.IsStatic();

            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.GetInfo, commandParamsWriter, token);
            var flags = retDebuggerCmdReader.ReadInt32();
            return (flags & 0x0010) > 0; //check method is static
        }

        public async Task<int> GetParamCount(int methodId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.GetParamInfo, commandParamsWriter, token);
            retDebuggerCmdReader.ReadInt32();
            int param_count = retDebuggerCmdReader.ReadInt32();
            return param_count;
        }

        public async Task<string> GetReturnType(int methodId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.GetParamInfo, commandParamsWriter, token);
            retDebuggerCmdReader.ReadInt32();
            retDebuggerCmdReader.ReadInt32();
            retDebuggerCmdReader.ReadInt32();
            var retType = retDebuggerCmdReader.ReadInt32();
            var ret = await GetTypeName(retType, token);
            return ret;
        }

        public async Task<string> GetParameters(int methodId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.GetParamInfo, commandParamsWriter, token);
            retDebuggerCmdReader.ReadInt32();
            var paramCount = retDebuggerCmdReader.ReadInt32();
            retDebuggerCmdReader.ReadInt32();
            var retType = retDebuggerCmdReader.ReadInt32();
            var parameters = "(";
            for (int i = 0 ; i < paramCount; i++)
            {
                var paramType = retDebuggerCmdReader.ReadInt32();
                parameters += await GetTypeName(paramType, token);
                parameters = parameters.Replace("System.Func", "Func");
                if (i + 1 < paramCount)
                    parameters += ",";
            }
            parameters += ")";
            return parameters;
        }

        public async Task<int> SetBreakpoint(int methodId, long il_offset, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((byte)EventKind.Breakpoint);
            commandParamsWriter.Write((byte)SuspendPolicy.None);
            commandParamsWriter.Write((byte)1);
            commandParamsWriter.Write((byte)ModifierKind.LocationOnly);
            commandParamsWriter.Write(methodId);
            commandParamsWriter.Write(il_offset);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdEventRequest.Set, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<bool> RemoveBreakpoint(int breakpoint_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((byte)EventKind.Breakpoint);
            commandParamsWriter.Write((int) breakpoint_id);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdEventRequest.Clear, commandParamsWriter, token);

            if (retDebuggerCmdReader != null)
                return true;
            return false;
        }

        public async Task<bool> Step(int thread_id, StepKind kind, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((byte)EventKind.Step);
            commandParamsWriter.Write((byte)SuspendPolicy.None);
            commandParamsWriter.Write((byte)1);
            commandParamsWriter.Write((byte)ModifierKind.Step);
            commandParamsWriter.Write(thread_id);
            commandParamsWriter.Write((int)StepSize.Line);
            commandParamsWriter.Write((int)kind);
            commandParamsWriter.Write((int)(StepFilter.StaticCtor)); //filter
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdEventRequest.Set, commandParamsWriter, token, throwOnError: false);
            if (retDebuggerCmdReader.HasError)
                return false;
            var isBPOnManagedCode = retDebuggerCmdReader.ReadInt32();
            if (isBPOnManagedCode == 0)
                return false;
            return true;
        }

        public async Task<bool> ClearSingleStep(int req_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((byte)EventKind.Step);
            commandParamsWriter.Write((int) req_id);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdEventRequest.Clear, commandParamsWriter, token, throwOnError: false);
            return !retDebuggerCmdReader.HasError ? true : false;
        }

        public async Task<JObject> GetFieldValue(int typeId, int fieldId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);
            commandParamsWriter.Write(1);
            commandParamsWriter.Write(fieldId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetValues, commandParamsWriter, token);
            return await CreateJObjectForVariableValue(retDebuggerCmdReader, "", token);
        }

        public async Task<int> TypeIsInitialized(int typeId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.IsInitialized, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<int> TypeInitialize(int typeId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.Initialize, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<MonoBinaryReader> GetTypePropertiesReader(int typeId, CancellationToken token)
        {
            var typeInfo = await GetTypeInfo(typeId, token);

            if (typeInfo is null)
                return null;

            if (typeInfo.PropertiesBuffer is not null)
                return new MonoBinaryReader(typeInfo.PropertiesBuffer);

            var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);

            var reader = await SendDebuggerAgentCommand(CmdType.GetProperties, commandParamsWriter, token);
            typeInfo.PropertiesBuffer = ((MemoryStream)reader.BaseStream).ToArray();
            return reader;
        }

        public async Task<List<FieldTypeClass>> GetTypeFields(int typeId, CancellationToken token)
        {
            var typeInfo = await GetTypeInfo(typeId, token);

            if (typeInfo.FieldsList != null) {
                return typeInfo.FieldsList;
            }

            var ret = new List<FieldTypeClass>();
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetFields, commandParamsWriter, token);
            var nFields = retDebuggerCmdReader.ReadInt32();

            for (int i = 0 ; i < nFields; i++)
            {
                int fieldId = retDebuggerCmdReader.ReadInt32(); //fieldId
                string fieldNameStr = retDebuggerCmdReader.ReadString();
                int fieldTypeId = retDebuggerCmdReader.ReadInt32(); //typeId
                int attrs = retDebuggerCmdReader.ReadInt32(); //attrs
                FieldAttributes fieldAttrs = (FieldAttributes)attrs;
                int isSpecialStatic = retDebuggerCmdReader.ReadInt32(); //is_special_static
                if (isSpecialStatic == 1)
                    continue;

                bool isBackingField = false;
                if (fieldNameStr.Contains("k__BackingField"))
                {
                    isBackingField = true;
                    fieldNameStr = fieldNameStr.Replace("k__BackingField", "");
                    fieldNameStr = fieldNameStr.Replace("<", "");
                    fieldNameStr = fieldNameStr.Replace(">", "");
                }
                ret.Add(new FieldTypeClass(fieldId, fieldNameStr, fieldTypeId, isBackingField, fieldAttrs));
            }
            typeInfo.FieldsList = ret;
            return ret;
        }

        private static string ReplaceCommonClassNames(string className) =>
            new StringBuilder(className)
                .Replace("System.String", "string")
                .Replace("System.Boolean", "bool")
                .Replace("System.Char", "char")
                .Replace("System.SByte", "sbyte")
                .Replace("System.Int32", "int")
                .Replace("System.Int64", "long")
                .Replace("System.Single", "float")
                .Replace("System.Double", "double")
                .Replace("System.Byte", "byte")
                .Replace("System.UInt16", "ushort")
                .Replace("System.UInt32", "uint")
                .Replace("System.UInt64", "ulong")
                .Replace("System.Object", "object")
                .Replace("System.Void", "void")
                //.Replace("System.Decimal", "decimal")
                .ToString();

        internal async Task<MonoBinaryReader> GetCAttrsFromType(int typeId, string attrName, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);
            commandParamsWriter.Write(0);
            var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetCattrs, commandParamsWriter, token);
            var count = retDebuggerCmdReader.ReadInt32();
            if (count == 0)
                return null;
            for (int i = 0 ; i < count; i++)
            {
                var methodId = retDebuggerCmdReader.ReadInt32();
                using var commandCattrParamsWriter = new MonoBinaryWriter();
                commandCattrParamsWriter.Write(methodId);
                using var retDebuggerCmdReader2 = await SendDebuggerAgentCommand(CmdMethod.GetDeclaringType, commandCattrParamsWriter, token);
                var customAttributeTypeId = retDebuggerCmdReader2.ReadInt32();
                var customAttributeName = await GetTypeName(customAttributeTypeId, token);
                if (customAttributeName == attrName)
                    return retDebuggerCmdReader;

                //reading buffer only to advance the reader to the next cattr
                for (int k = 0 ; k < 2; k++)
                {
                    var parmCount = retDebuggerCmdReader.ReadInt32();
                    for (int j = 0; j < parmCount; j++)
                    {
                        //to typed_args
                        await CreateJObjectForVariableValue(retDebuggerCmdReader, "varName", token);
                    }
                }
            }
            return null;
        }

        public async Task<int> GetAssemblyFromType(int type_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(type_id);
            commandParamsWriter.Write((int) MonoTypeNameFormat.FormatReflection);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetInfo, commandParamsWriter, token);

            retDebuggerCmdReader.ReadString();
            retDebuggerCmdReader.ReadString();
            retDebuggerCmdReader.ReadString();

            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<string> GetValueFromDebuggerDisplayAttribute(DotnetObjectId dotnetObjectId, int typeId, CancellationToken token)
        {
            string expr = "";
            try {
                var getCAttrsRetReader = await GetCAttrsFromType(typeId, "System.Diagnostics.DebuggerDisplayAttribute", token);
                if (getCAttrsRetReader == null)
                    return null;

                var parmCount = getCAttrsRetReader.ReadInt32();
                var monoType = (ElementType)getCAttrsRetReader.ReadByte(); //MonoTypeEnum -> MONO_TYPE_STRING
                if (monoType != ElementType.String)
                    return null;

                var stringId = getCAttrsRetReader.ReadInt32();
                var dispAttrStr = await GetStringValue(stringId, token);
                ExecutionContext context = proxy.GetContext(sessionId);
                GetMembersResult members = await GetTypeMemberValues(
                    dotnetObjectId,
                    GetObjectCommandOptions.WithProperties | GetObjectCommandOptions.ForDebuggerDisplayAttribute,
                    token);
                JArray objectValues = new JArray(members.Flatten());

                var thisObj = CreateJObject<string>(value: "", type: "object", description: "", writable: false, objectId: dotnetObjectId.ToString());
                thisObj["name"] = "this";
                objectValues.Add(thisObj);

                var resolver = new MemberReferenceResolver(proxy, context, sessionId, objectValues, logger);
                if (dispAttrStr.Length == 0)
                    return null;

                if (dispAttrStr.Contains(", nq"))
                {
                    dispAttrStr = dispAttrStr.Replace(", nq", "");
                }
                if (dispAttrStr.Contains(",nq"))
                {
                    dispAttrStr = dispAttrStr.Replace(",nq", "");
                }
                expr = "$\"" + dispAttrStr + "\"";
                JObject retValue = await resolver.Resolve(expr, token);
                if (retValue == null)
                    retValue = await EvaluateExpression.CompileAndRunTheExpression(expr, resolver, token);

                return retValue?["value"]?.Value<string>();
            }
            catch (Exception ex)
            {
                logger.LogDebug($"Could not evaluate DebuggerDisplayAttribute - {expr} - {await GetTypeName(typeId, token)}: {ex}");
            }
            return null;
        }

        public async Task<string> GetTypeName(int typeId, CancellationToken token)
        {
            string className = await GetTypeNameOriginal(typeId, token);
            className = className.Replace("+", ".");
            className = Regex.Replace(className, @"`\d+", "");
            className = Regex.Replace(className, @"[[, ]+]", "__SQUARED_BRACKETS__");
            //className = className.Replace("[]", "__SQUARED_BRACKETS__");
            className = className.Replace("[", "<");
            className = className.Replace("]", ">");
            className = className.Replace("__SQUARED_BRACKETS__", "[]");
            className = className.Replace(",", ", ");
            className = ReplaceCommonClassNames(className);
            return className;
        }

        public async Task<string> GetTypeNameOriginal(int typeId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);
            commandParamsWriter.Write((int) MonoTypeNameFormat.FormatReflection);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetInfo, commandParamsWriter, token);
            retDebuggerCmdReader.ReadString(); //namespace
            retDebuggerCmdReader.ReadString(); //class name
            return retDebuggerCmdReader.ReadString(); //class name formatted
        }

        public async Task<int> GetTypeToken(int typeId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeId);
            commandParamsWriter.Write((int) MonoTypeNameFormat.FormatReflection);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetInfo, commandParamsWriter, token);
            retDebuggerCmdReader.ReadString(); //namespace
            retDebuggerCmdReader.ReadString(); //class name
            retDebuggerCmdReader.ReadString(); //class name formatted
            retDebuggerCmdReader.ReadInt32(); //assemblyid
            retDebuggerCmdReader.ReadInt32(); //moduleId
            retDebuggerCmdReader.ReadInt32(); //parent typeId
            retDebuggerCmdReader.ReadInt32(); //array typeId
            return retDebuggerCmdReader.ReadInt32(); //token
        }

        public async Task<string> GetStringValue(int string_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(string_id);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdString.GetValue, commandParamsWriter, token);
            var isUtf16 = retDebuggerCmdReader.ReadByte();
            if (isUtf16 == 0) {
                return retDebuggerCmdReader.ReadString();
            }
            return null;
        }
        public async Task<ArrayDimensions> GetArrayDimensions(int object_id, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(object_id);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdArray.GetLength, commandParamsWriter, token);
            var length = retDebuggerCmdReader.ReadInt32();
            var rank = new int[length];
            for (int i = 0 ; i < length; i++)
            {
                rank[i] = retDebuggerCmdReader.ReadInt32();
                retDebuggerCmdReader.ReadInt32(); //lower_bound
            }
            return new ArrayDimensions(rank);
        }

        public async Task<List<int>> GetTypeIdsForObject(int object_id, bool withParents, CancellationToken token)
        {
            List<int> ret = new List<int>();
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(object_id);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdObject.RefGetType, commandParamsWriter, token);
            var type_id = retDebuggerCmdReader.ReadInt32();
            ret.Add(type_id);
            if (withParents)
            {
                using var commandParentsParamsWriter = new MonoBinaryWriter();
                commandParentsParamsWriter.Write(type_id);
                using var parentsCmdReader = await SendDebuggerAgentCommand(CmdType.GetParents, commandParentsParamsWriter, token);
                var parentsCount = parentsCmdReader.ReadInt32();
                for (int i = 0 ; i < parentsCount; i++)
                {
                    ret.Add(parentsCmdReader.ReadInt32());
                }
            }
            return ret;
        }

        public async Task<string> GetClassNameFromObject(int object_id, CancellationToken token)
        {
            var type_id = await GetTypeIdsForObject(object_id, false, token);
            return await GetTypeName(type_id[0], token);
        }

        public async Task<int> GetTypeIdFromToken(int assemblyId, int typeToken, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((int)assemblyId);
            commandParamsWriter.Write((int)typeToken);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAssembly.GetTypeFromToken, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<int> GetMethodIdByName(int type_id, string method_name, CancellationToken token)
        {
            if (type_id <= 0)
                throw new DebuggerAgentException($"Invalid type_id {type_id} (method_name: {method_name}");

            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((int)type_id);
            commandParamsWriter.Write(method_name);
            commandParamsWriter.Write((int)(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static));
            commandParamsWriter.Write((int)1); //case sensitive
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdType.GetMethodsByNameFlags, commandParamsWriter, token);
            var nMethods = retDebuggerCmdReader.ReadInt32();
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<bool> IsDelegate(int objectId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((int)objectId);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdObject.RefIsDelegate, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadByte() == 1;
        }

        public async Task<int> GetDelegateMethod(int objectId, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((int)objectId);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdObject.RefDelegateGetMethod, commandParamsWriter, token);
            return retDebuggerCmdReader.ReadInt32();
        }

        public async Task<string> GetDelegateMethodDescription(int objectId, CancellationToken token)
        {
            var methodId = await GetDelegateMethod(objectId, token);

            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);
            //Console.WriteLine("methodId - " + methodId);
            if (methodId == 0)
                return "";
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.GetName, commandParamsWriter, token);
            var methodName = retDebuggerCmdReader.ReadString();

            var returnType = await GetReturnType(methodId, token);
            var parameters = await GetParameters(methodId, token);

            return $"{returnType} {methodName} {parameters}";
        }

        public async Task<JObject> InvokeMethod(ArraySegment<byte> argsBuffer, int methodId, CancellationToken token, string name = null)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);
            commandParamsWriter.Write(argsBuffer);
            commandParamsWriter.Write(0);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdVM.InvokeMethod, commandParamsWriter, token);
            retDebuggerCmdReader.ReadByte(); //number of objects returned.
            return await CreateJObjectForVariableValue(retDebuggerCmdReader, name, token);
        }

        public Task<JObject> InvokeMethod(int objectId, int methodId, bool isValueType, CancellationToken token)
        {
            if (isValueType)
            {
                return valueTypes.TryGetValue(objectId, out var valueType)
                        ? InvokeMethod(valueType.Buffer, methodId, token)
                        : throw new ArgumentException($"Could not find valuetype with id {objectId}, for method id: {methodId}", nameof(objectId));
            }
            else
            {
                using var commandParamsObjWriter = new MonoBinaryWriter();
                commandParamsObjWriter.Write(ElementType.Class, objectId);
                return InvokeMethod(commandParamsObjWriter.GetParameterBuffer(), methodId, token);
            }
        }

        public Task<JObject> InvokeMethod(DotnetObjectId dotnetObjectId, CancellationToken token, int methodId = -1)
        {
            if (dotnetObjectId.Scheme == "method")
            {
                JObject args = dotnetObjectId.ValueAsJson;
                int? objectId = args["containerId"]?.Value<int>();
                int? embeddedMethodId = args["methodId"]?.Value<int>();

                return objectId == null || embeddedMethodId == null
                    ? throw new ArgumentException($"Invalid object id for a method, with missing container, or methodId", nameof(dotnetObjectId))
                    : InvokeMethod(objectId.Value, embeddedMethodId.Value, isValueType: args["isValueType"]?.Value<bool>() == true, token);
            }

            return dotnetObjectId.Scheme is "object" or "valuetype"
                ? InvokeMethod(dotnetObjectId.Value, methodId, isValueType: dotnetObjectId.IsValueType, token)
                : throw new ArgumentException($"Cannot invoke method with id {methodId} on {dotnetObjectId}", nameof(dotnetObjectId));
        }

        public async Task<int> GetPropertyMethodIdByName(int typeId, string propertyName, CancellationToken token)
        {
            using var retDebuggerCmdReader =  await GetTypePropertiesReader(typeId, token);
            if (retDebuggerCmdReader == null)
                return -1;

            var nProperties = retDebuggerCmdReader.ReadInt32();
            for (int i = 0 ; i < nProperties; i++)
            {
                retDebuggerCmdReader.ReadInt32(); //propertyId
                string propertyNameStr = retDebuggerCmdReader.ReadString();
                var getMethodId = retDebuggerCmdReader.ReadInt32();
                retDebuggerCmdReader.ReadInt32(); //setmethod
                var attrs = retDebuggerCmdReader.ReadInt32(); //attrs
                if (propertyNameStr == propertyName)
                {
                    return getMethodId;
                }
            }
            return -1;
        }

        public async Task<JObject> GetPointerContent(int pointerId, CancellationToken token)
        {
            if (!pointerValues.TryGetValue(pointerId, out PointerValue pointerValue))
                throw new ArgumentException($"Could not find any pointer with id: {pointerId}", nameof(pointerId));

            return await pointerValue.GetValue(this, token);
        }

        public static bool AutoExpandable(string className) {
            if (className == "System.DateTime" ||
                className == "System.DateTimeOffset" ||
                className == "System.TimeSpan")
                return true;
            return false;
        }

        private static bool AutoInvokeToString(string className) {
            if (className == "System.DateTime" ||
                className == "System.DateTimeOffset" ||
                className == "System.TimeSpan" ||
                className == "System.Decimal"  ||
                className == "System.Guid")
                return true;
            return false;
        }

        public static JObject CreateJObject<T>(T value, string type, string description, bool writable, string className = null, string objectId = null, string __custom_type = null, string subtype = null, bool isValueType = false, bool expanded = false, bool isEnum = false)
        {
            var ret = JObject.FromObject(new {
                    value = new
                    {
                        type,
                        value,
                        description
                    },
                    writable
                });
            if (__custom_type != null)
                ret["value"]["__custom_type"] = __custom_type;
            if (className != null)
                ret["value"]["className"] = className;
            if (objectId != null)
                ret["value"]["objectId"] = objectId;
            if (subtype != null)
                ret["value"]["subtype"] = subtype;
            if (isValueType)
                ret["value"]["isValueType"] = isValueType;
            if (expanded)
                ret["value"]["expanded"] = expanded;
            if (isEnum)
                ret["value"]["isEnum"] = isEnum;
            return ret;

        }

        private static JObject CreateJObjectForBoolean(int value)
        {
            return CreateJObject<bool>(value == 0 ? false : true, "boolean", value == 0 ? "false" : "true", true);
        }

        private static JObject CreateJObjectForNumber<T>(T value)
        {
            return CreateJObject<T>(value, "number", value.ToString(), true);
        }

        private static JObject CreateJObjectForChar(int value)
        {
            char charValue = Convert.ToChar(value);
            var description = $"{value} '{charValue}'";
            return CreateJObject<char>(charValue, "symbol", description, true);
        }

        public async Task<JObject> CreateJObjectForPtr(ElementType etype, MonoBinaryReader retDebuggerCmdReader, string name, CancellationToken token)
        {
            string type;
            string value;
            long valueAddress = retDebuggerCmdReader.ReadInt64();
            var typeId = retDebuggerCmdReader.ReadInt32();
            string className;
            if (etype == ElementType.FnPtr)
                className = "(*())"; //to keep the old behavior
            else
                className = "(" + await GetTypeName(typeId, token) + ")";

            int pointerId = -1;
            if (valueAddress != 0 && className != "(void*)")
            {
                pointerId = GetNewObjectId();
                type = "object";
                value =  className;
                pointerValues[pointerId] = new PointerValue(valueAddress, typeId, name);
            }
            else
            {
                type = "symbol";
                value = className + " " + valueAddress;
            }
            return CreateJObject<string>(value, type, value, false, className, $"dotnet:pointer:{pointerId}", "pointer");
        }

        public async Task<JObject> CreateJObjectForString(MonoBinaryReader retDebuggerCmdReader, CancellationToken token)
        {
            var string_id = retDebuggerCmdReader.ReadInt32();
            var value = await GetStringValue(string_id, token);
            return CreateJObject<string>(value, "string", value, false);
        }

        public async Task<JObject> CreateJObjectForArray(MonoBinaryReader retDebuggerCmdReader, CancellationToken token)
        {
            var objectId = retDebuggerCmdReader.ReadInt32();
            var className = await GetClassNameFromObject(objectId, token);
            var arrayType = className.ToString();
            var length = await GetArrayDimensions(objectId, token);
            if (arrayType.LastIndexOf('[') > 0)
                arrayType = arrayType.Insert(arrayType.LastIndexOf('[')+1, length.ToString());
            if (className.LastIndexOf('[') > 0)
                className = className.Insert(arrayType.LastIndexOf('[')+1, new string(',', length.Rank-1));
            return CreateJObject<string>(null, "object", description : arrayType, writable : false, className.ToString(), "dotnet:array:" + objectId, null, subtype : length.Rank == 1 ? "array" : null);
        }

        public async Task<JObject> CreateJObjectForObject(MonoBinaryReader retDebuggerCmdReader, int typeIdFromAttribute, bool forDebuggerDisplayAttribute, CancellationToken token)
        {
            var objectId = retDebuggerCmdReader.ReadInt32();
            var type_id = await GetTypeIdsForObject(objectId, false, token);
            string className = await GetTypeName(type_id[0], token);
            string debuggerDisplayAttribute = null;
            if (!forDebuggerDisplayAttribute)
                debuggerDisplayAttribute = await GetValueFromDebuggerDisplayAttribute(new DotnetObjectId("object", objectId), type_id[0], token);
            var description = className.ToString();

            if (debuggerDisplayAttribute != null)
                description = debuggerDisplayAttribute;

            if (await IsDelegate(objectId, token))
            {
                if (typeIdFromAttribute != -1)
                    className = await GetTypeName(typeIdFromAttribute, token);

                description = await GetDelegateMethodDescription(objectId, token);
                if (description == "")
                    return CreateJObject<string>(className.ToString(), "symbol", className.ToString(), false);
            }
            return CreateJObject<string>(null, "object", description, false, className, $"dotnet:object:{objectId}");
        }

        private static readonly string[] s_primitiveTypeNames = new[]
        {
            "bool",
            "char",
            "string",
            "byte",
            "sbyte",
            "int",
            "uint",
            "long",
            "ulong",
            "short",
            "ushort",
            "float",
            "double",
        };

        public static bool IsPrimitiveType(string simplifiedClassName)
            => s_primitiveTypeNames.Contains(simplifiedClassName);

        public async Task<JObject> CreateJObjectForValueType(
            MonoBinaryReader retDebuggerCmdReader, string name, long initialPos, bool forDebuggerDisplayAttribute, CancellationToken token)
        {
            // FIXME: debugger proxy
            var isEnum = retDebuggerCmdReader.ReadByte() == 1;
            var isBoxed = retDebuggerCmdReader.ReadByte() == 1;
            var typeId = retDebuggerCmdReader.ReadInt32();
            var className = await GetTypeName(typeId, token);
            var numValues = retDebuggerCmdReader.ReadInt32();

            if (className.IndexOf("System.Nullable<", StringComparison.Ordinal) == 0) //should we call something on debugger-agent to check???
            {
                retDebuggerCmdReader.ReadByte(); //ignoring the boolean type
                var isNull = retDebuggerCmdReader.ReadInt32();

                // Read the value, even if isNull==true, to correctly advance the reader
                var value = await CreateJObjectForVariableValue(retDebuggerCmdReader, name, token);
                if (isNull != 0)
                    return value;
                else
                    return CreateJObject<string>(null, "object", className, false, className, null, null, "null", true);
            }
            if (isBoxed && numValues == 1)
            {
                if (IsPrimitiveType(className))
                {
                    var value = await CreateJObjectForVariableValue(retDebuggerCmdReader, name: null, token);
                    return value;
                }
            }

            ValueTypeClass valueType = await ValueTypeClass.CreateFromReader(
                                                        this,
                                                        retDebuggerCmdReader,
                                                        initialPos,
                                                        className,
                                                        typeId,
                                                        numValues,
                                                        isEnum,
                                                        token);
            valueTypes[valueType.Id.Value] = valueType;
            return await valueType.ToJObject(this, forDebuggerDisplayAttribute, token);
        }

        public async Task<JObject> CreateJObjectForNull(MonoBinaryReader retDebuggerCmdReader, CancellationToken token)
        {
            string className;
            ElementType variableType = (ElementType)retDebuggerCmdReader.ReadByte();
            switch (variableType)
            {
                case ElementType.String:
                case ElementType.Class:
                {
                    var type_id = retDebuggerCmdReader.ReadInt32();
                    className = await GetTypeName(type_id, token);
                    break;

                }
                case ElementType.SzArray:
                case ElementType.Array:
                {
                    ElementType byte_type = (ElementType)retDebuggerCmdReader.ReadByte();
                    retDebuggerCmdReader.ReadInt32(); // rank
                    if (byte_type == ElementType.Class) {
                        retDebuggerCmdReader.ReadInt32(); // internal_type_id
                    }
                    var type_id = retDebuggerCmdReader.ReadInt32();
                    className = await GetTypeName(type_id, token);
                    break;
                }
                default:
                {
                    var type_id = retDebuggerCmdReader.ReadInt32();
                    className = await GetTypeName(type_id, token);
                    break;
                }
            }
            return CreateJObject<string>(null, "object", className, false, className, null, null, "null");
        }

        public async Task<JObject> CreateJObjectForVariableValue(MonoBinaryReader retDebuggerCmdReader,
                                                                 string name,
                                                                 CancellationToken token,
                                                                 bool isOwn = false,
                                                                 int typeIdForObject = -1,
                                                                 bool forDebuggerDisplayAttribute = false)
        {
            long initialPos =  /*retDebuggerCmdReader == null ? 0 : */retDebuggerCmdReader.BaseStream.Position;
            ElementType etype = (ElementType)retDebuggerCmdReader.ReadByte();
            JObject ret = null;
            switch (etype) {
                case ElementType.I:
                case ElementType.U:
                case ElementType.Void:
                case (ElementType)ValueTypeId.VType:
                case (ElementType)ValueTypeId.FixedArray:
                    ret = JObject.FromObject(new {
                        value = new
                        {
                            type = "void",
                            value = "void",
                            description = "void"
                        }});
                    break;
                case ElementType.Boolean:
                {
                    var value = retDebuggerCmdReader.ReadInt32();
                    ret = CreateJObjectForBoolean(value);
                    break;
                }
                case ElementType.I1:
                {
                    var value = retDebuggerCmdReader.ReadSByte();
                    ret = CreateJObjectForNumber<int>(value);
                    break;
                }
                case ElementType.I2:
                case ElementType.I4:
                {
                    var value = retDebuggerCmdReader.ReadInt32();
                    ret = CreateJObjectForNumber<int>(value);
                    break;
                }
                case ElementType.U1:
                {
                    var value = retDebuggerCmdReader.ReadUByte();
                    ret = CreateJObjectForNumber<int>(value);
                    break;
                }
                case ElementType.U2:
                {
                    var value = retDebuggerCmdReader.ReadUShort();
                    ret = CreateJObjectForNumber<int>(value);
                    break;
                }
                case ElementType.U4:
                {
                    var value = retDebuggerCmdReader.ReadUInt32();
                    ret = CreateJObjectForNumber<uint>(value);
                    break;
                }
                case ElementType.R4:
                {
                    float value = retDebuggerCmdReader.ReadSingle();
                    ret = CreateJObjectForNumber<float>(value);
                    break;
                }
                case ElementType.Char:
                {
                    var value = retDebuggerCmdReader.ReadInt32();
                    ret = CreateJObjectForChar(value);
                    break;
                }
                case ElementType.I8:
                {
                    long value = retDebuggerCmdReader.ReadInt64();
                    ret = CreateJObjectForNumber<long>(value);
                    break;
                }
                case ElementType.U8:
                {
                    ulong value = retDebuggerCmdReader.ReadUInt64();
                    ret = CreateJObjectForNumber<ulong>(value);
                    break;
                }
                case ElementType.R8:
                {
                    double value = retDebuggerCmdReader.ReadDouble();
                    ret = CreateJObjectForNumber<double>(value);
                    break;
                }
                case ElementType.FnPtr:
                case ElementType.Ptr:
                {
                    ret = await CreateJObjectForPtr(etype, retDebuggerCmdReader, name, token);
                    break;
                }
                case ElementType.String:
                {
                    ret = await CreateJObjectForString(retDebuggerCmdReader, token);
                    break;
                }
                case ElementType.SzArray:
                case ElementType.Array:
                {
                    ret = await CreateJObjectForArray(retDebuggerCmdReader, token);
                    break;
                }
                case ElementType.Class:
                case ElementType.Object:
                {
                    ret = await CreateJObjectForObject(retDebuggerCmdReader, typeIdForObject, forDebuggerDisplayAttribute, token);
                    break;
                }
                case ElementType.ValueType:
                {
                    ret = await CreateJObjectForValueType(retDebuggerCmdReader, name, initialPos, forDebuggerDisplayAttribute, token);
                    break;
                }
                case (ElementType)ValueTypeId.Null:
                {
                    ret = await CreateJObjectForNull(retDebuggerCmdReader, token);
                    break;
                }
                case (ElementType)ValueTypeId.Type:
                {
                    retDebuggerCmdReader.ReadInt32();
                    break;
                }
                default:
                {
                    logger.LogDebug($"Could not evaluate CreateJObjectForVariableValue invalid type {etype}");
                    break;
                }
            }
            if (ret != null)
            {
                if (isOwn)
                    ret["isOwn"] = true;
                if (!string.IsNullOrEmpty(name))
                    ret["name"] = name;
            }
            return ret;
        }

        public async Task<bool> IsAsyncMethod(int methodId, CancellationToken token)
        {
            var methodInfo = await GetMethodInfo(methodId, token);
            if (methodInfo != null && methodInfo.Info.IsAsync != -1)
            {
                return methodInfo.Info.IsAsync == 1;
            }

            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(methodId);

            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdMethod.AsyncDebugInfo, commandParamsWriter, token);
            methodInfo.Info.IsAsync = retDebuggerCmdReader.ReadByte();
            return methodInfo.Info.IsAsync == 1;
        }

        private static bool IsClosureReferenceField (string fieldName)
        {
            // mcs is "$locvar"
            // old mcs is "<>f__ref"
            // csc is "CS$<>"
            // roslyn is "<>8__"
            return fieldName.StartsWith ("CS$<>", StringComparison.Ordinal) ||
                        fieldName.StartsWith ("<>f__ref", StringComparison.Ordinal) ||
                        fieldName.StartsWith ("$locvar", StringComparison.Ordinal) ||
                        fieldName.StartsWith ("<>8__", StringComparison.Ordinal);
        }

        public async Task<JArray> GetHoistedLocalVariables(int objectId, IEnumerable<JToken> asyncLocals, CancellationToken token)
        {
            JArray asyncLocalsFull = new JArray();
            List<int> objectsAlreadyRead = new();
            objectsAlreadyRead.Add(objectId);
            foreach (var asyncLocal in asyncLocals)
            {
                var fieldName = asyncLocal["name"].Value<string>();
                if (fieldName.EndsWith("__this", StringComparison.Ordinal))
                {
                    asyncLocal["name"] = "this";
                    asyncLocalsFull.Add(asyncLocal);
                }
                else if (IsClosureReferenceField(fieldName)) //same code that has on debugger-libs
                {
                    if (DotnetObjectId.TryParse(asyncLocal?["value"]?["objectId"]?.Value<string>(), out DotnetObjectId dotnetObjectId))
                    {
                        if (!objectsAlreadyRead.Contains(dotnetObjectId.Value))
                        {
                            var asyncProxyMembersFromObject = await MemberObjectsExplorer.GetObjectMemberValues(
                                this, dotnetObjectId.Value, GetObjectCommandOptions.WithProperties, token);
                            var hoistedLocalVariable = await GetHoistedLocalVariables(dotnetObjectId.Value, asyncProxyMembersFromObject.Flatten(), token);
                            asyncLocalsFull = new JArray(asyncLocalsFull.Union(hoistedLocalVariable));
                        }
                    }
                }
                else if (fieldName.StartsWith("<>", StringComparison.Ordinal)) //examples: <>t__builder, <>1__state
                {
                    continue;
                }
                else if (fieldName.StartsWith('<')) //examples: <code>5__2
                {
                    var match = regexForAsyncLocals.Match(fieldName);
                    if (match.Success)
                        asyncLocal["name"] = match.Groups[1].Value;
                    asyncLocalsFull.Add(asyncLocal);
                }
                else
                {
                    asyncLocalsFull.Add(asyncLocal);
                }
            }
            return asyncLocalsFull;
        }

        public async Task<JArray> StackFrameGetValues(MethodInfoWithDebugInformation method, int thread_id, int frame_id, VarInfo[] varIds, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(thread_id);
            commandParamsWriter.Write(frame_id);
            commandParamsWriter.Write(varIds.Length);
            foreach (var var in varIds)
            {
                commandParamsWriter.Write(var.Index);
            }

            if (await IsAsyncMethod(method.DebugId, token))
            {
                using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdFrame.GetThis, commandParamsWriter, token);
                retDebuggerCmdReader.ReadByte(); //ignore type
                var objectId = retDebuggerCmdReader.ReadInt32();
                GetMembersResult asyncProxyMembers = await MemberObjectsExplorer.GetObjectMemberValues(this, objectId, GetObjectCommandOptions.WithProperties, token);
                var asyncLocals = await GetHoistedLocalVariables(objectId, asyncProxyMembers.Flatten(), token);
                return asyncLocals;
            }

            JArray locals = new JArray();
            using var localsDebuggerCmdReader = await SendDebuggerAgentCommand(CmdFrame.GetValues, commandParamsWriter, token);
            foreach (var var in varIds)
            {
                try
                {
                    var var_json = await CreateJObjectForVariableValue(localsDebuggerCmdReader, var.Name, token);
                    locals.Add(var_json);
                }
                catch (Exception ex)
                {
                    logger.LogDebug($"Failed to create value for local var {var}: {ex}");
                    continue;
                }
            }
            if (!method.Info.IsStatic())
            {
                using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdFrame.GetThis, commandParamsWriter, token);
                var var_json = await CreateJObjectForVariableValue(retDebuggerCmdReader, "this", token);
                var_json.Add("fieldOffset", -1);
                locals.Add(var_json);
            }
            return locals;

        }

        public ValueTypeClass GetValueTypeClass(int valueTypeId)
        {
            if (valueTypes.TryGetValue(valueTypeId, out ValueTypeClass value))
                return value;
            return null;
        }

        public async Task<JArray> GetArrayValues(int arrayId, CancellationToken token)
        {
            var dimensions = await GetArrayDimensions(arrayId, token);
            var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(arrayId);
            commandParamsWriter.Write(0);
            commandParamsWriter.Write(dimensions.TotalLength);
            var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdArray.GetValues, commandParamsWriter, token);
            JArray array = new JArray();
            for (int i = 0 ; i < dimensions.TotalLength; i++)
            {
                var var_json = await CreateJObjectForVariableValue(retDebuggerCmdReader, dimensions.GetArrayIndexString(i), token);
                array.Add(var_json);
            }
            return array;
        }

        public async Task<JObject> GetArrayValuesProxy(int arrayId, CancellationToken token)
        {
            var length = await GetArrayDimensions(arrayId, token);
            var arrayProxy = JObject.FromObject(new
            {
                items = await GetArrayValues(arrayId, token),
                dimensionsDetails = length.Bounds
            });
            return arrayProxy;
        }

        public async Task<bool> EnableExceptions(PauseOnExceptionsKind state, CancellationToken token)
        {
            if (state == PauseOnExceptionsKind.Unset)
            {
                logger.LogDebug($"Trying to setPauseOnExceptions using status Unset");
                return false;
            }

            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write((byte)EventKind.Exception);
            commandParamsWriter.Write((byte)SuspendPolicy.None);
            commandParamsWriter.Write((byte)1);
            commandParamsWriter.Write((byte)ModifierKind.ExceptionOnly);
            commandParamsWriter.Write(0); //exc_class
            if (state == PauseOnExceptionsKind.All)
                commandParamsWriter.Write((byte)1); //caught
            else
                commandParamsWriter.Write((byte)0); //caught

            if (state == PauseOnExceptionsKind.Uncaught || state == PauseOnExceptionsKind.All)
                commandParamsWriter.Write((byte)1); //uncaught
            else
                commandParamsWriter.Write((byte)0); //uncaught

            commandParamsWriter.Write((byte)1);//subclasses
            commandParamsWriter.Write((byte)0);//not_filtered_feature
            commandParamsWriter.Write((byte)0);//everything_else
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdEventRequest.Set, commandParamsWriter, token);
            return true;
        }

        public async Task<int> GetTypeByName(string typeToSearch, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(typeToSearch);
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdVM.GetTypes, commandParamsWriter, token);
            var count = retDebuggerCmdReader.ReadInt32(); //count ret
            return retDebuggerCmdReader.ReadInt32();
        }

        // FIXME: support valuetypes
        public async Task<GetMembersResult> GetValuesFromDebuggerProxyAttribute(int objectId, int typeId, CancellationToken token)
        {
            try
            {
                int methodId = await FindDebuggerProxyConstructorIdFor(typeId, token);
                if (methodId == -1)
                    return null;

                using var ctorArgsWriter = new MonoBinaryWriter();
                ctorArgsWriter.Write((byte)ValueTypeId.Null);

                // FIXME: move method invocation to valueTypeclass?
                if (valueTypes.TryGetValue(objectId, out var valueType))
                {
                    //FIXME: Issue #68390
                    //ctorArgsWriter.Write((byte)0); //not used but needed
                    //ctorArgsWriter.Write(0); //not used but needed
                    //ctorArgsWriter.Write((int)1); // num args
                    //ctorArgsWriter.Write(valueType.Buffer);
                    return null;
                }
                else
                {
                    ctorArgsWriter.Write((byte)0); //not used
                    ctorArgsWriter.Write(0); //not used
                    ctorArgsWriter.Write((int)1); // num args
                    ctorArgsWriter.Write((byte)ElementType.Object);
                    ctorArgsWriter.Write(objectId);
                }

                var retMethod = await InvokeMethod(ctorArgsWriter.GetParameterBuffer(), methodId, token);
                if (!DotnetObjectId.TryParse(retMethod?["value"]?["objectId"]?.Value<string>(), out DotnetObjectId dotnetObjectId))
                    throw new Exception($"Invoking .ctor ({methodId}) for DebuggerTypeProxy on type {typeId} returned {retMethod}");

                GetMembersResult members = await GetTypeMemberValues(dotnetObjectId,
                                                                            GetObjectCommandOptions.WithProperties | GetObjectCommandOptions.ForDebuggerProxyAttribute,
                                                                            token);

                return members;
            }
            catch (Exception e)
            {
                logger.LogDebug($"Could not evaluate DebuggerTypeProxyAttribute of type {await GetTypeName(typeId, token)} - {e}");
            }

            return null;
        }

        private async Task<int> FindDebuggerProxyConstructorIdFor(int typeId, CancellationToken token)
        {
            try
            {
                var getCAttrsRetReader = await GetCAttrsFromType(typeId, "System.Diagnostics.DebuggerTypeProxyAttribute", token);
                if (getCAttrsRetReader == null)
                    return -1;

                var methodId = -1;
                var parmCount = getCAttrsRetReader.ReadInt32();
                for (int j = 0; j < parmCount; j++)
                {
                    var monoTypeId = getCAttrsRetReader.ReadByte();
                    // FIXME: DebuggerTypeProxyAttribute(string) - not supported
                    if ((ValueTypeId)monoTypeId != ValueTypeId.Type)
                        continue;

                    var cAttrTypeId = getCAttrsRetReader.ReadInt32();
                    using var commandParamsWriter = new MonoBinaryWriter();
                    commandParamsWriter.Write(cAttrTypeId);
                    var className = await GetTypeNameOriginal(cAttrTypeId, token);
                    if (className.IndexOf('[') > 0)
                    {
                        className = className.Remove(className.IndexOf('['));
                        var assemblyId = await GetAssemblyIdFromType(cAttrTypeId, token);
                        var assemblyName = await GetFullAssemblyName(assemblyId, token);
                        var typeToSearch = className;
                        typeToSearch += "[["; //System.Collections.Generic.List`1[[System.Int32,mscorlib,Version=4.0.0.0,Culture=neutral,PublicKeyToken=b77a5c561934e089]],mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089
                        List<int> genericTypeArgs = await GetTypeParamsOrArgsForGenericType(typeId, token);
                        for (int k = 0; k < genericTypeArgs.Count; k++)
                        {
                            var assemblyIdArg = await GetAssemblyIdFromType(genericTypeArgs[k], token);
                            var assemblyNameArg = await GetFullAssemblyName(assemblyIdArg, token);
                            var classNameArg = await GetTypeNameOriginal(genericTypeArgs[k], token);
                            typeToSearch += classNameArg + ", " + assemblyNameArg;
                            if (k + 1 < genericTypeArgs.Count)
                                typeToSearch += "], [";
                            else
                                typeToSearch += "]";
                        }
                        typeToSearch += "]";
                        typeToSearch += ", " + assemblyName;
                        var genericTypeId = await GetTypeByName(typeToSearch, token);
                        if (genericTypeId < 0)
                            break;
                        cAttrTypeId = genericTypeId;
                    }
                    methodId = await GetMethodIdByName(cAttrTypeId, ".ctor", token);
                    break;
                }

                return methodId;
            }
            catch (Exception e)
            {
                logger.LogDebug($"Could not evaluate DebuggerTypeProxyAttribute of type {await GetTypeName(typeId, token)} - {e}");
            }

            return -1;
        }

        public Task<GetMembersResult> GetTypeMemberValues(DotnetObjectId dotnetObjectId, GetObjectCommandOptions getObjectOptions, CancellationToken token, bool sortByAccessLevel = false)
            => dotnetObjectId.IsValueType
                    ? MemberObjectsExplorer.GetValueTypeMemberValues(this, dotnetObjectId.Value, getObjectOptions, token)
                    : MemberObjectsExplorer.GetObjectMemberValues(this, dotnetObjectId.Value, getObjectOptions, token);


        public async Task<JObject> GetMethodProxy(JObject objectId, CancellationToken token)
        {
            var containerId = objectId["containerId"].Value<int>();
            var methodId = objectId["methodId"].Value<int>();
            var isValueType = objectId["isValueType"].Value<bool>();
            return await InvokeMethod(containerId, methodId, isValueType, token);
        }

        public async Task<JArray> GetObjectProxy(int objectId, CancellationToken token)
        {
            GetMembersResult members = await MemberObjectsExplorer.GetObjectMemberValues(this, objectId, GetObjectCommandOptions.WithSetter, token);
            JArray ret = members.Flatten();
            var typeIds = await GetTypeIdsForObject(objectId, true, token);
            foreach (var typeId in typeIds)
            {
                var retDebuggerCmdReader =  await GetTypePropertiesReader(typeId, token);
                if (retDebuggerCmdReader == null)
                    return null;

                var nProperties = retDebuggerCmdReader.ReadInt32();
                for (int i = 0 ; i < nProperties; i++)
                {
                    retDebuggerCmdReader.ReadInt32(); //propertyId
                    string propertyNameStr = retDebuggerCmdReader.ReadString();
                    var getMethodId = retDebuggerCmdReader.ReadInt32();
                    var setMethodId = retDebuggerCmdReader.ReadInt32(); //setmethod
                    var attrValue = retDebuggerCmdReader.ReadInt32(); //attrs
                    //Console.WriteLine($"{propertyNameStr} - {attrValue}");
                    if (ret.Where(attribute => attribute["name"].Value<string>().Equals(propertyNameStr)).Any())
                    {
                        var attr = ret.Where(attribute => attribute["name"].Value<string>().Equals(propertyNameStr)).First();

                        using var command_params_writer_to_set = new MonoBinaryWriter();
                        command_params_writer_to_set.Write(setMethodId);
                        command_params_writer_to_set.Write((byte)ElementType.Class);
                        command_params_writer_to_set.Write(objectId);
                        command_params_writer_to_set.Write(1);
                        var (data, length) = command_params_writer_to_set.ToBase64();

                        if (attr["set"] != null)
                        {
                            attr["set"] = JObject.FromObject(new {
                                        commandSet = CommandSet.Vm,
                                        command = CmdVM.InvokeMethod,
                                        buffer = data,
                                        valtype = attr["set"]["valtype"],
                                        length,
                                        id = GetNewId()
                                });
                        }
                        continue;
                    }
                    else
                    {
                        var command_params_writer_to_get = new MonoBinaryWriter();
                        command_params_writer_to_get.Write(getMethodId);
                        command_params_writer_to_get.Write((byte)ElementType.Class);
                        command_params_writer_to_get.Write(objectId);
                        command_params_writer_to_get.Write(0);
                        var (data, length) = command_params_writer_to_get.ToBase64();

                        ret.Add(JObject.FromObject(new {
                                get = JObject.FromObject(new {
                                    commandSet = CommandSet.Vm,
                                    command = CmdVM.InvokeMethod,
                                    buffer = data,
                                    length = length,
                                    id = GetNewId()
                                    }),
                                name = propertyNameStr
                            }));
                    }
                    if (await MethodIsStatic(getMethodId, token))
                        continue;
                }
            }
            return ret;
        }

        public async Task<bool> SetVariableValue(int thread_id, int frame_id, int varId, string newValue, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(thread_id);
            commandParamsWriter.Write(frame_id);
            commandParamsWriter.Write(1);
            commandParamsWriter.Write(varId);
            JArray locals = new JArray();
            using var getDebuggerCmdReader = await SendDebuggerAgentCommand(CmdFrame.GetValues, commandParamsWriter, token);
            int etype = getDebuggerCmdReader.ReadByte();
            using var setDebuggerCmdReader = await SendDebuggerAgentCommandWithParms(CmdFrame.SetValues, commandParamsWriter.ToBase64(), etype, newValue, token, throwOnError: false);
            return !setDebuggerCmdReader.HasError;
        }

        public async Task<bool> SetNextIP(MethodInfoWithDebugInformation method, int threadId, IlLocation ilOffset, CancellationToken token)
        {
            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(threadId);
            commandParamsWriter.Write(method.DebugId);
            commandParamsWriter.Write((long)ilOffset.Offset);
            using var getDebuggerCmdReader = await SendDebuggerAgentCommand(CmdThread.SetIp, commandParamsWriter, token);
            return !getDebuggerCmdReader.HasError;
        }

        public async Task<int> CreateByteArray(string diff, CancellationToken token)
        {
            var diffArr = Convert.FromBase64String(diff);
            using var commandParamsWriter = new MonoBinaryWriter();
            using var retDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAppDomain.GetRootDomain, commandParamsWriter, token);
            var root = retDebuggerCmdReader.ReadInt32();

            commandParamsWriter.Write(root);
            commandParamsWriter.WriteByteArray(diffArr);
            using var arrayDebuggerCmdReader = await SendDebuggerAgentCommand(CmdAppDomain.CreateByteArray, commandParamsWriter, token);
            return arrayDebuggerCmdReader.ReadInt32();
        }

        public async Task<bool> ApplyUpdates(int moduleId, string dmeta, string dil, string dpdb, CancellationToken token)
        {
            int dpdbId = -1;
            var dmetaId = await CreateByteArray(dmeta, token);
            var dilId = await CreateByteArray(dil, token);
            if (dpdb != null)
                dpdbId = await CreateByteArray(dpdb, token);

            using var commandParamsWriter = new MonoBinaryWriter();
            commandParamsWriter.Write(moduleId);
            commandParamsWriter.Write(dmetaId);
            commandParamsWriter.Write(dilId);
            if (dpdbId != -1)
                commandParamsWriter.Write(dpdbId);
            else
                commandParamsWriter.Write((byte)ValueTypeId.Null);
            await SendDebuggerAgentCommand(CmdModule.ApplyChanges, commandParamsWriter, token);
            return true;
        }
    }

    internal static class HelperExtensions
    {
        public static void AddRange(this JArray arr, JArray addedArr)
        {
            foreach (var item in addedArr)
                arr.Add(item);
        }

        public static bool IsNullValuedObject(this JObject obj)
            => obj != null && obj["type"]?.Value<string>() == "object" && obj["subtype"]?.Value<string>() == "null";

    }
}
