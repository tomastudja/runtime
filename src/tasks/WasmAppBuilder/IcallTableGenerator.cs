// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Reflection;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

public class IcallTableGenerator : Task
{
    [Required]
    public string? RuntimeIcallTableFile { get; set; }
    [Required]
    public ITaskItem[]? Assemblies { get; set; }
    [Required, NotNull]
    public string? OutputPath { get; set; }

    [Output]
    public string? FileWrites { get; private set; } = "";

    private List<Icall> _icalls = new List<Icall> ();
    private Dictionary<string, IcallClass> _runtimeIcalls = new Dictionary<string, IcallClass> ();

    public override bool Execute()
    {
        GenIcallTable(RuntimeIcallTableFile!, Assemblies!.Select(item => item.ItemSpec).ToArray());
        return true;
    }

    //
    // Given the runtime generated icall table, and a set of assemblies, generate
    // a smaller linked icall table mapping tokens to C function names
    // The runtime icall table should be generated using
    // mono --print-icall-table
    //
    public void GenIcallTable(string runtimeIcallTableFile, string[] assemblies)
    {
        ReadTable (runtimeIcallTableFile);
        var resolver = new PathAssemblyResolver(assemblies);
        var mlc = new MetadataLoadContext(resolver, "System.Private.CoreLib");
        foreach (var aname in assemblies)
        {
            var a = mlc.LoadFromAssemblyPath(aname);
            foreach (var type in a.GetTypes())
                ProcessType(type);
        }

        string tmpFileName = Path.GetTempFileName();
        using (var w = File.CreateText(tmpFileName))
            EmitTable (w);

        if (Utils.CopyIfDifferent(tmpFileName, OutputPath, useHash: false))
            Log.LogMessage(MessageImportance.Low, $"Generating icall table to '{OutputPath}'.");
        else
            Log.LogMessage(MessageImportance.Low, $"Icall table in {OutputPath} is unchanged.");
        FileWrites = OutputPath;

        File.Delete(tmpFileName);
    }

    private void EmitTable (StreamWriter w)
    {
        var assemblyMap = new Dictionary<string, string> ();
        foreach (var icall in _icalls)
            assemblyMap [icall.Assembly!] = icall.Assembly!;

        foreach (var assembly in assemblyMap.Keys)
        {
            var sorted = _icalls.Where (i => i.Assembly == assembly).ToArray ();
            Array.Sort (sorted);

            string aname;
            if (assembly == "System.Private.CoreLib")
                aname = "corlib";
            else
                aname = assembly.Replace (".", "_");
            w.WriteLine ($"#define ICALL_TABLE_{aname} 1\n");

            w.WriteLine ($"static int {aname}_icall_indexes [] = {{");
            foreach (var icall in sorted)
                w.WriteLine (string.Format ("{0},", icall.TokenIndex));
            w.WriteLine ("};");
            foreach (var icall in sorted)
                w.WriteLine (GenIcallDecl (icall));
            w.WriteLine ($"static void *{aname}_icall_funcs [] = {{");
            foreach (var icall in sorted)
            {
                w.WriteLine (string.Format ("// token {0},", icall.TokenIndex));
                w.WriteLine (string.Format ("{0},", icall.Func));
            }
            w.WriteLine ("};");
            w.WriteLine ($"static uint8_t {aname}_icall_handles [] = {{");
            foreach (var icall in sorted)
                w.WriteLine (string.Format ("{0},", icall.Handles ? "1" : "0"));
            w.WriteLine ("};");
        }
    }

    // Read the icall table generated by mono --print-icall-table
    private void ReadTable (string filename)
    {
        JsonDocument json;
        using (var stream = File.Open (filename, FileMode.Open))
        {
            json = JsonDocument.Parse (stream);
        }

        var arr = json.RootElement;
        foreach (var v in arr.EnumerateArray ())
        {
            var className = v.GetProperty ("klass").GetString ()!;
            if (className == "")
                // Dummy value
                continue;
            var icallClass = new IcallClass (className);
            _runtimeIcalls [icallClass.Name] = icallClass;
            foreach (var icall_j in v.GetProperty ("icalls").EnumerateArray ())
            {
                if (!icall_j.TryGetProperty ("name", out var nameElem))
                    continue;
                string name = nameElem.GetString ()!;
                string func = icall_j.GetProperty ("func").GetString ()!;
                bool handles = icall_j.GetProperty ("handles").GetBoolean ();

                icallClass.Icalls.Add (name, new Icall (name, func, handles));
            }
        }
    }

    private void ProcessType (Type type)
    {
        foreach (var method in type.GetMethods(BindingFlags.DeclaredOnly|BindingFlags.Public|BindingFlags.NonPublic|BindingFlags.Static|BindingFlags.Instance))
        {
            if ((method.GetMethodImplementationFlags() & MethodImplAttributes.InternalCall) == 0)
                continue;

            var className = method.DeclaringType!.FullName!;
            if (!_runtimeIcalls.ContainsKey (className))
                // Registered at runtime
                continue;

            var icallClass = _runtimeIcalls [className];

            Icall? icall = null;

            // Try name first
            icallClass.Icalls.TryGetValue (method.Name, out icall);
            if (icall == null)
            {
                // Then with signature
                var sig = new StringBuilder (method.Name + "(");
                int pindex = 0;
                foreach (var par in method.GetParameters())
                {
                    if (pindex > 0)
                        sig.Append (',');
                    var t = par.ParameterType;
                    AppendType (sig, t);
                    pindex++;
                }
                sig.Append (')');
                if (icallClass.Icalls.ContainsKey (sig.ToString ()))
                    icall = icallClass.Icalls [sig.ToString ()];
            }
            if (icall == null)
                // Registered at runtime
                continue;
            icall.Method = method;
            icall.TokenIndex = (int)method.MetadataToken & 0xffffff;
            icall.Assembly = method.DeclaringType.Module.Assembly.GetName().Name;
            _icalls.Add (icall);
         }

        foreach (var nestedType in type.GetNestedTypes())
            ProcessType(nestedType);
    }

    // Append the type name used by the runtime icall tables
    private void AppendType (StringBuilder sb, Type t) {
        if (t.IsArray)
        {
            AppendType (sb, t.GetElementType()!);
            sb.Append ("[]");
        }
        else if (t.IsByRef)
        {
            AppendType (sb, t.GetElementType()!);
            sb.Append ('&');
        }
        else if (t.IsPointer)
        {
            AppendType (sb, t.GetElementType()!);
            sb.Append ('*');
        }
        else
        {
            string name = t.Name;

            switch (name)
            {
            case "Char": sb.Append ("char"); break;
            case "Boolean": sb.Append ("bool"); break;
            case "Int8": sb.Append ("sbyte"); break;
            case "UInt8": sb.Append ("byte"); break;
            case "Int16": sb.Append ("int16"); break;
            case "UInt16": sb.Append ("uint16"); break;
            case "Int32": sb.Append ("int"); break;
            case "UInt32": sb.Append ("uint"); break;
            case "Int64": sb.Append ("long"); break;
            case "UInt64": sb.Append ("ulong"); break;
            case "IntPtr": sb.Append ("intptr"); break;
            case "UIntPtr": sb.Append ("uintptr"); break;
            case "Single": sb.Append ("single"); break;
            case "Double": sb.Append ("double"); break;
            case "Object": sb.Append ("object"); break;
            case "String": sb.Append ("string"); break;
            default:
                throw new NotImplementedException (t.FullName);
            }
        }
    }

    private static string MapType (Type t)
    {
        switch (t.Name)
        {
        case "Void":
            return "void";
        case "Double":
            return "double";
        case "Single":
            return "float";
        case "Int64":
            return "int64_t";
        case "UInt64":
            return "uint64_t";
        default:
            return "int";
        }
    }

    private static string GenIcallDecl (Icall icall)
    {
        var sb = new StringBuilder ();
        var method = icall.Method!;
        sb.Append (MapType (method.ReturnType));
        sb.Append ($" {icall.Func} (");
        int pindex = 0;
        int aindex = 0;
        if (!method.IsStatic)
        {
            sb.Append ("int");
            aindex++;
        }
        foreach (var p in method.GetParameters ())
        {
            if (aindex > 0)
                sb.Append (',');
            sb.Append (MapType (p.ParameterType));
            pindex++;
            aindex++;
        }
        if (icall.Handles)
        {
            if (aindex > 0)
                sb.Append (',');
            sb.Append ("int");
            pindex++;
        }
        sb.Append (");");
        return sb.ToString ();
    }

    private class Icall : IComparable<Icall>
    {
        public Icall (string name, string func, bool handles)
        {
            Name = name;
            Func = func;
            Handles = handles;
            TokenIndex = 0;
        }

        public string Name;
        public string Func;
        public string? Assembly;
        public bool Handles;
        public int TokenIndex;
        public MethodInfo? Method;

        public int CompareTo (Icall? other) {
            return TokenIndex - other!.TokenIndex;
        }
    }

    private class IcallClass
    {
        public IcallClass (string name)
        {
            Name = name;
            Icalls = new Dictionary<string, Icall> ();
        }

        public string Name;
        public Dictionary<string, Icall> Icalls;
    }
}
