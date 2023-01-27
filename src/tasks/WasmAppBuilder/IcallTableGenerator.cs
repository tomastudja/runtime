// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Reflection;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

internal sealed class IcallTableGenerator
{
    public string[]? Cookies { get; private set; }

    private List<Icall> _icalls = new List<Icall>();
    private List<string> _signatures = new List<string>();
    private Dictionary<string, IcallClass> _runtimeIcalls = new Dictionary<string, IcallClass>();

    private TaskLoggingHelper Log { get; set; }
    private readonly Func<string, string> _fixupSymbolName;

    public IcallTableGenerator(Func<string, string> fixupSymbolName, TaskLoggingHelper log)
    {
        Log = log;
        _fixupSymbolName = fixupSymbolName;
    }

    //
    // Given the runtime generated icall table, and a set of assemblies, generate
    // a smaller linked icall table mapping tokens to C function names
    // The runtime icall table should be generated using
    // mono --print-icall-table
    //
    public IEnumerable<string> Generate(string? runtimeIcallTableFile, IEnumerable<string> assemblies, string? outputPath)
    {
        _icalls.Clear();
        _signatures.Clear();

        if (runtimeIcallTableFile != null)
            ReadTable(runtimeIcallTableFile);

        var resolver = new PathAssemblyResolver(assemblies);
        using var mlc = new MetadataLoadContext(resolver, "System.Private.CoreLib");
        foreach (var asmPath in assemblies)
        {
            if (!File.Exists(asmPath))
                throw new LogAsErrorException($"Cannot find assembly {asmPath}");

            Log.LogMessage(MessageImportance.Low, $"Loading {asmPath} to scan for icalls");
            var a = mlc.LoadFromAssemblyPath(asmPath);
            foreach (var type in a.GetTypes())
                ProcessType(type);
        }

        if (outputPath != null)
        {
            string tmpFileName = Path.GetTempFileName();
            try
            {
                using (var w = File.CreateText(tmpFileName))
                    EmitTable(w);

                if (Utils.CopyIfDifferent(tmpFileName, outputPath, useHash: false))
                    Log.LogMessage(MessageImportance.Low, $"Generating icall table to '{outputPath}'.");
                else
                    Log.LogMessage(MessageImportance.Low, $"Icall table in {outputPath} is unchanged.");
            }
            finally
            {
                File.Delete(tmpFileName);
            }
        }

        return _signatures;
    }

    private void EmitTable(StreamWriter w)
    {
        var assemblyMap = new Dictionary<string, string>();
        foreach (var icall in _icalls)
            assemblyMap[icall.Assembly!] = icall.Assembly!;

        foreach (var assembly in assemblyMap.Keys)
        {
            var sorted = _icalls.Where(i => i.Assembly == assembly).ToArray();
            Array.Sort(sorted);

            string aname;
            if (assembly == "System.Private.CoreLib")
                aname = "corlib";
            else
                aname = _fixupSymbolName(assembly);
            w.WriteLine($"#define ICALL_TABLE_{aname} 1\n");

            w.WriteLine($"static int {aname}_icall_indexes [] = {{");
            foreach (var icall in sorted)
                w.WriteLine(string.Format("{0},", icall.TokenIndex));
            w.WriteLine("};");
            foreach (var icall in sorted)
                w.WriteLine(GenIcallDecl(icall));
            w.WriteLine($"static void *{aname}_icall_funcs [] = {{");
            foreach (var icall in sorted)
            {
                w.WriteLine(string.Format("// token {0},", icall.TokenIndex));
                w.WriteLine(string.Format("{0},", icall.Func));
            }
            w.WriteLine("};");
            w.WriteLine($"static uint8_t {aname}_icall_flags [] = {{");
            foreach (var icall in sorted)
                w.WriteLine(string.Format("{0},", icall.Flags));
            w.WriteLine("};");
        }
    }

    // Read the icall table generated by mono --print-icall-table
    private void ReadTable(string filename)
    {
        using var stream = File.OpenRead(filename);
        using JsonDocument json = JsonDocument.Parse(stream);

        var arr = json.RootElement;
        foreach (var v in arr.EnumerateArray())
        {
            var className = v.GetProperty("klass").GetString()!;
            if (className == "")
                // Dummy value
                continue;

            var icallClass = new IcallClass(className);
            _runtimeIcalls[icallClass.Name] = icallClass;
            foreach (var icall_j in v.GetProperty("icalls").EnumerateArray())
            {
                if (!icall_j.TryGetProperty("name", out var nameElem))
                    continue;

                string name = nameElem.GetString()!;
                string func = icall_j.GetProperty("func").GetString()!;
                bool handles = icall_j.GetProperty("handles").GetBoolean();
                int flags = icall_j.TryGetProperty ("flags", out var _) ? int.Parse (icall_j.GetProperty("flags").GetString()!) : 0;

                icallClass.Icalls.Add(name, new Icall(name, func, handles, flags));
            }
        }
    }

    private void ProcessType(Type type)
    {
        foreach (var method in type.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance))
        {
            if ((method.GetMethodImplementationFlags() & MethodImplAttributes.InternalCall) == 0)
                continue;

            try
            {
                AddSignature(type, method);
            }
            catch (Exception ex) when (ex is not LogAsErrorException)
            {
                Log.LogWarning(null, "WASM0001", "", "", 0, 0, 0, 0, $"Could not get icall, or callbacks for method '{type.FullName}::{method.Name}' because '{ex.Message}'");
                continue;
            }

            var className = method.DeclaringType!.FullName!;
            if (!_runtimeIcalls.ContainsKey(className))
                // Registered at runtime
                continue;

            var icallClass = _runtimeIcalls[className];

            Icall? icall = null;

            // Try name first
            icallClass.Icalls.TryGetValue(method.Name, out icall);
            if (icall == null)
            {
                string? methodSig = BuildSignature(method, className);
                if (methodSig != null)
                    icallClass.Icalls.TryGetValue(methodSig, out icall);

                if (icall == null)
                    // Registered at runtime
                    continue;
            }

            icall.Method = method;
            icall.TokenIndex = (int)method.MetadataToken & 0xffffff;
            icall.Assembly = method.DeclaringType.Module.Assembly.GetName().Name;
            _icalls.Add(icall);
        }

        foreach (var nestedType in type.GetNestedTypes())
            ProcessType(nestedType);

        string? BuildSignature(MethodInfo method, string className)
        {
            // Then with signature
            var sig = new StringBuilder(method.Name + "(");
            int pindex = 0;
            foreach (var par in method.GetParameters())
            {
                if (pindex > 0)
                    sig.Append(',');

                var t = par.ParameterType;
                try
                {
                    AppendType(sig, t);
                }
                catch (NotImplementedException nie)
                {
                    Log.LogWarning(null, "WASM0001", "", "", 0, 0, 0, 0, $"Failed to generate icall function for method '[{method.DeclaringType!.Assembly.GetName().Name}] {className}::{method.Name}'" +
                                    $" because type '{nie.Message}' is not supported for parameter named '{par.Name}'. Ignoring.");
                    return null;
                }
                pindex++;
            }
            sig.Append(')');

            return sig.ToString();
        }

        void AddSignature(Type type, MethodInfo method)
        {
            string? signature = SignatureMapper.MethodToSignature(method);
            if (signature == null)
            {
                throw new LogAsErrorException($"Unsupported parameter type in method '{type.FullName}.{method.Name}'");
            }

            Log.LogMessage(MessageImportance.Low, $"Adding icall signature {signature} for method '{type.FullName}.{method.Name}'");
            _signatures.Add(signature);
        }
    }

    // Append the type name used by the runtime icall tables
    private void AppendType(StringBuilder sb, Type t)
    {
        if (t.IsArray)
        {
            AppendType(sb, t.GetElementType()!);
            sb.Append("[]");
        }
        else if (t.IsByRef)
        {
            AppendType(sb, t.GetElementType()!);
            sb.Append('&');
        }
        else if (t.IsPointer)
        {
            AppendType(sb, t.GetElementType()!);
            sb.Append('*');
        }
        else if (t.IsEnum)
        {
            AppendType(sb, Enum.GetUnderlyingType(t));
        }
        else
        {
            sb.Append(t.Name switch
            {
                nameof(Char) => "char",
                nameof(Boolean) => "bool",
                nameof(SByte) => "sbyte",
                nameof(Byte) => "byte",
                nameof(Int16) => "int16",
                nameof(UInt16) => "uint16",
                nameof(Int32) => "int",
                nameof(UInt32) => "uint",
                nameof(Int64) => "long",
                nameof(UInt64) => "ulong",
                nameof(IntPtr) => "intptr",
                nameof(UIntPtr) => "uintptr",
                nameof(Single) => "single",
                nameof(Double) => "double",
                nameof(Object) => "object",
                nameof(String) => "string",
                _ => throw new NotImplementedException(t.FullName)
            });
        }
    }

    private static string MapType(Type t) => t.Name switch
    {
        "Void" => "void",
        nameof(Double) => "double",
        nameof(Single) => "float",
        nameof(Int64) => "int64_t",
        nameof(UInt64) => "uint64_t",
        _ => "int",
    };

    private static string GenIcallDecl(Icall icall)
    {
        var sb = new StringBuilder();
        var method = icall.Method!;
        sb.Append(MapType(method.ReturnType));
        sb.Append($" {icall.Func} (");
        int aindex = 0;
        if (!method.IsStatic)
        {
            sb.Append("int");
            aindex++;
        }
        foreach (var p in method.GetParameters())
        {
            if (aindex > 0)
                sb.Append(',');
            sb.Append(MapType(p.ParameterType));
            aindex++;
        }
        if (icall.Handles)
        {
            if (aindex > 0)
                sb.Append(',');
            sb.Append("int");
        }
        sb.Append(");");
        return sb.ToString();
    }

    private sealed class Icall : IComparable<Icall>
    {
        public Icall(string name, string func, bool handles, int flags)
        {
            Name = name;
            Func = func;
            Flags = flags;
            Handles = handles;
            TokenIndex = 0;
        }

        public string Name;
        public string Func;
        public string? Assembly;
        public bool Handles;
        public int Flags;
        public int TokenIndex;
        public MethodInfo? Method;

        public int CompareTo(Icall? other)
        {
            return TokenIndex - other!.TokenIndex;
        }
    }

    private sealed class IcallClass
    {
        public IcallClass(string name)
        {
            Name = name;
            Icalls = new Dictionary<string, Icall>();
        }

        public string Name;
        public Dictionary<string, Icall> Icalls;
    }
}
