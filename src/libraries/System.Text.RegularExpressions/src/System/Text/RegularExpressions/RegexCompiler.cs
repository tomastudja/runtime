// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Reflection;
using System.Reflection.Emit;

namespace System.Text.RegularExpressions
{
    /// <summary>
    /// RegexCompiler translates a block of RegexCode to MSIL, and creates a
    /// subclass of the RegexRunner type.
    /// </summary>
    internal abstract class RegexCompiler
    {
        private static readonly FieldInfo s_runtextbegField = RegexRunnerField("runtextbeg");
        private static readonly FieldInfo s_runtextendField = RegexRunnerField("runtextend");
        private static readonly FieldInfo s_runtextstartField = RegexRunnerField("runtextstart");
        private static readonly FieldInfo s_runtextposField = RegexRunnerField("runtextpos");
        private static readonly FieldInfo s_runtextField = RegexRunnerField("runtext");
        private static readonly FieldInfo s_runtrackposField = RegexRunnerField("runtrackpos");
        private static readonly FieldInfo s_runtrackField = RegexRunnerField("runtrack");
        private static readonly FieldInfo s_runstackposField = RegexRunnerField("runstackpos");
        private static readonly FieldInfo s_runstackField = RegexRunnerField("runstack");
        private static readonly FieldInfo s_runtrackcountField = RegexRunnerField("runtrackcount");

        private static readonly MethodInfo s_doubleStackMethod = RegexRunnerMethod("DoubleStack");
        private static readonly MethodInfo s_doubleTrackMethod = RegexRunnerMethod("DoubleTrack");
        private static readonly MethodInfo s_captureMethod = RegexRunnerMethod("Capture");
        private static readonly MethodInfo s_transferCaptureMethod = RegexRunnerMethod("TransferCapture");
        private static readonly MethodInfo s_uncaptureMethod = RegexRunnerMethod("Uncapture");
        private static readonly MethodInfo s_isMatchedMethod = RegexRunnerMethod("IsMatched");
        private static readonly MethodInfo s_matchLengthMethod = RegexRunnerMethod("MatchLength");
        private static readonly MethodInfo s_matchIndexMethod = RegexRunnerMethod("MatchIndex");
        private static readonly MethodInfo s_isBoundaryMethod = RegexRunnerMethod("IsBoundary");
        private static readonly MethodInfo s_isECMABoundaryMethod = RegexRunnerMethod("IsECMABoundary");
        private static readonly MethodInfo s_crawlposMethod = RegexRunnerMethod("Crawlpos");
        private static readonly MethodInfo s_charInClassMethod = RegexRunnerMethod("CharInClass");
        private static readonly MethodInfo s_checkTimeoutMethod = RegexRunnerMethod("CheckTimeout");
#if DEBUG
        private static readonly MethodInfo s_dumpStateM = RegexRunnerMethod("DumpState");
#endif

        private static readonly MethodInfo s_charToLowerMethod = typeof(char).GetMethod("ToLower", new Type[] { typeof(char), typeof(CultureInfo) })!;
        private static readonly MethodInfo s_charToLowerInvariantMethod = typeof(char).GetMethod("ToLowerInvariant", new Type[] { typeof(char) })!;
        private static readonly MethodInfo s_charIsDigitMethod = typeof(char).GetMethod("IsDigit", new Type[] { typeof(char) })!;
        private static readonly MethodInfo s_charIsWhiteSpaceMethod = typeof(char).GetMethod("IsWhiteSpace", new Type[] { typeof(char) })!;
        private static readonly MethodInfo s_stringGetCharsMethod = typeof(string).GetMethod("get_Chars", new Type[] { typeof(int) })!;
        private static readonly MethodInfo s_stringAsSpanMethod = typeof(MemoryExtensions).GetMethod("AsSpan", new Type[] { typeof(string), typeof(int), typeof(int) })!;
        private static readonly MethodInfo s_stringIndexOf = typeof(string).GetMethod("IndexOf", new Type[] { typeof(char), typeof(int), typeof(int) })!;
        private static readonly MethodInfo s_spanIndexOf = typeof(MemoryExtensions).GetMethod("IndexOf", new Type[] { typeof(ReadOnlySpan<>).MakeGenericType(Type.MakeGenericMethodParameter(0)), Type.MakeGenericMethodParameter(0) })!.MakeGenericMethod(typeof(char));
        private static readonly MethodInfo s_spanIndexOfAnyCharChar = typeof(MemoryExtensions).GetMethod("IndexOfAny", new Type[] { typeof(ReadOnlySpan<>).MakeGenericType(Type.MakeGenericMethodParameter(0)), Type.MakeGenericMethodParameter(0), Type.MakeGenericMethodParameter(0) })!.MakeGenericMethod(typeof(char));
        private static readonly MethodInfo s_spanIndexOfAnyCharCharChar = typeof(MemoryExtensions).GetMethod("IndexOfAny", new Type[] { typeof(ReadOnlySpan<>).MakeGenericType(Type.MakeGenericMethodParameter(0)), Type.MakeGenericMethodParameter(0), Type.MakeGenericMethodParameter(0), Type.MakeGenericMethodParameter(0) })!.MakeGenericMethod(typeof(char));
        private static readonly MethodInfo s_spanGetItemMethod = typeof(ReadOnlySpan<char>).GetMethod("get_Item", new Type[] { typeof(int) })!;
        private static readonly MethodInfo s_spanGetLengthMethod = typeof(ReadOnlySpan<char>).GetMethod("get_Length")!;
        private static readonly MethodInfo s_spanSliceMethod = typeof(ReadOnlySpan<char>).GetMethod("Slice", new Type[] { typeof(int) })!;
        private static readonly MethodInfo s_cultureInfoGetCurrentCultureMethod = typeof(CultureInfo).GetMethod("get_CurrentCulture")!;
#if DEBUG
        private static readonly MethodInfo s_debugWriteLine = typeof(Debug).GetMethod("WriteLine", new Type[] { typeof(string) })!;
#endif

        protected ILGenerator? _ilg;

        // tokens representing local variables
        private LocalBuilder? _runtextbegLocal;
        private LocalBuilder? _runtextendLocal;
        private LocalBuilder? _runtextposLocal;
        private LocalBuilder? _runtextLocal;
        private LocalBuilder? _runtrackposLocal;
        private LocalBuilder? _runtrackLocal;
        private LocalBuilder? _runstackposLocal;
        private LocalBuilder? _runstackLocal;
        private LocalBuilder? _temp1Local;
        private LocalBuilder? _temp2Local;
        private LocalBuilder? _temp3Local;
        private LocalBuilder? _cultureLocal;  // current culture is cached in local variable to prevent many thread local storage accesses for CultureInfo.CurrentCulture
        private LocalBuilder? _loopTimeoutCounterLocal; // timeout counter for setrep and setloop

        protected RegexOptions _options;      // options
        protected RegexCode? _code;           // the RegexCode object
        protected int[]? _codes;              // the RegexCodes being translated
        protected string[]? _strings;         // the stringtable associated with the RegexCodes
        protected RegexPrefix? _fcPrefix;     // the possible first chars computed by RegexFCD
        protected RegexBoyerMoore? _bmPrefix; // a prefix as a boyer-moore machine
        protected int _anchors;               // the set of anchors
        protected bool _hasTimeout;           // whether the regex has a non-infinite timeout

        private Label[]? _labels;             // a label for every operation in _codes
        private BacktrackNote[]? _notes;      // a list of the backtracking states to be generated
        private int _notecount;               // true count of _notes (allocation grows exponentially)
        protected int _trackcount;            // count of backtracking states (used to reduce allocations)
        private Label _backtrack;             // label for backtracking

        private int _regexopcode;             // the current opcode being processed
        private int _codepos;                 // the current code being translated
        private int _backpos;                 // the current backtrack-note being translated

        // special code fragments
        private int[]? _uniquenote;           // _notes indices for code that should be emitted <= once
        private int[]? _goto;                 // indices for forward-jumps-through-switch (for allocations)

        // indices for unique code fragments
        private const int Stackpop = 0;       // pop one
        private const int Stackpop2 = 1;      // pop two
        private const int Capback = 3;        // uncapture
        private const int Capback2 = 4;       // uncapture 2
        private const int Branchmarkback2 = 5;      // back2 part of branchmark
        private const int Lazybranchmarkback2 = 6;  // back2 part of lazybranchmark
        private const int Branchcountback2 = 7;     // back2 part of branchcount
        private const int Lazybranchcountback2 = 8; // back2 part of lazybranchcount
        private const int Forejumpback = 9;         // back part of forejump
        private const int Uniquecount = 10;
        private const int LoopTimeoutCheckCount = 2048; // A conservative value to guarantee the correct timeout handling.

        private static FieldInfo RegexRunnerField(string fieldname) => typeof(RegexRunner).GetField(fieldname, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static)!;

        private static MethodInfo RegexRunnerMethod(string methname) => typeof(RegexRunner).GetMethod(methname, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static)!;

        /// <summary>
        /// Entry point to dynamically compile a regular expression.  The expression is compiled to
        /// an in-memory assembly.
        /// </summary>
        internal static RegexRunnerFactory Compile(RegexCode code, RegexOptions options, bool hasTimeout) => new RegexLWCGCompiler().FactoryInstanceFromCode(code, options, hasTimeout);

        /// <summary>
        /// Keeps track of an operation that needs to be referenced in the backtrack-jump
        /// switch table, and that needs backtracking code to be emitted (if flags != 0)
        /// </summary>
        private sealed class BacktrackNote
        {
            internal int _codepos;
            internal int _flags;
            internal Label _label;

            public BacktrackNote(int flags, Label label, int codepos)
            {
                _codepos = codepos;
                _flags = flags;
                _label = label;
            }
        }

        /// <summary>
        /// Adds a backtrack note to the list of them, and returns the index of the new
        /// note (which is also the index for the jump used by the switch table)
        /// </summary>
        private int AddBacktrackNote(int flags, Label l, int codepos)
        {
            if (_notes == null || _notecount >= _notes.Length)
            {
                var newnotes = new BacktrackNote[_notes == null ? 16 : _notes.Length * 2];
                if (_notes != null)
                {
                    Array.Copy(_notes, newnotes, _notecount);
                }
                _notes = newnotes;
            }

            _notes[_notecount] = new BacktrackNote(flags, l, codepos);

            return _notecount++;
        }

        /// <summary>
        /// Adds a backtrack note for the current operation; creates a new label for
        /// where the code will be, and returns the switch index.
        /// </summary>
        private int AddTrack() => AddTrack(RegexCode.Back);

        /// <summary>
        /// Adds a backtrack note for the current operation; creates a new label for
        /// where the code will be, and returns the switch index.
        /// </summary>
        private int AddTrack(int flags) => AddBacktrackNote(flags, DefineLabel(), _codepos);

        /// <summary>
        /// Adds a switchtable entry for the specified position (for the forward
        /// logic; does not cause backtracking logic to be generated)
        /// </summary>
        private int AddGoto(int destpos)
        {
            if (_goto![destpos] == -1)
            {
                _goto[destpos] = AddBacktrackNote(0, _labels![destpos], destpos);
            }

            return _goto[destpos];
        }

        /// <summary>
        /// Adds a note for backtracking code that only needs to be generated once;
        /// if it's already marked to be generated, returns the switch index
        /// for the unique piece of code.
        /// </summary>
        private int AddUniqueTrack(int i) => AddUniqueTrack(i, RegexCode.Back);

        /// <summary>
        /// Adds a note for backtracking code that only needs to be generated once;
        /// if it's already marked to be generated, returns the switch index
        /// for the unique piece of code.
        /// </summary>
        private int AddUniqueTrack(int i, int flags)
        {
            if (_uniquenote![i] == -1)
            {
                _uniquenote[i] = AddTrack(flags);
            }

            return _uniquenote[i];
        }

        /// <summary>A macro for _ilg.DefineLabel</summary>
        private Label DefineLabel() => _ilg!.DefineLabel();

        /// <summary>A macro for _ilg.MarkLabel</summary>
        private void MarkLabel(Label l) => _ilg!.MarkLabel(l);

        /// <summary>Returns the ith operand of the current operation.</summary>
        private int Operand(int i) => _codes![_codepos + i + 1];

        /// <summary>True if the current operation is marked for the leftward direction.</summary>
        private bool IsRightToLeft() => (_regexopcode & RegexCode.Rtl) != 0;

        /// <summary>True if the current operation is marked for case insensitive operation.</summary>
        private bool IsCaseInsensitive() => (_regexopcode & RegexCode.Ci) != 0;

        /// <summary>Returns the raw regex opcode (masking out Back and Rtl).</summary>
        private int Code() => _regexopcode & RegexCode.Mask;

        /// <summary>A macro for _ilg.Emit(Opcodes.Ldstr, str)</summary>
        private void Ldstr(string str) => _ilg!.Emit(OpCodes.Ldstr, str);

        /// <summary>A macro for the various forms of Ldc.</summary>
        private void Ldc(int i)
        {
            Debug.Assert(_ilg != null);

            if ((uint)i < 8)
            {
                _ilg.Emit(i switch
                {
                    0 => OpCodes.Ldc_I4_0,
                    1 => OpCodes.Ldc_I4_1,
                    2 => OpCodes.Ldc_I4_2,
                    3 => OpCodes.Ldc_I4_3,
                    4 => OpCodes.Ldc_I4_4,
                    5 => OpCodes.Ldc_I4_5,
                    6 => OpCodes.Ldc_I4_6,
                    _ => OpCodes.Ldc_I4_7,
                });
            }
            else if (i <= 127 && i >= -128)
            {
                _ilg.Emit(OpCodes.Ldc_I4_S, (byte)i);
            }
            else
            {
                _ilg.Emit(OpCodes.Ldc_I4, i);
            }
        }

        /// <summary>A macro for _ilg.Emit(OpCodes.Dup).</summary>
        private void Dup() => _ilg!.Emit(OpCodes.Dup);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ret).</summary>
        private void Ret() => _ilg!.Emit(OpCodes.Ret);

        /// <summary>A macro for _ilg.Emit(OpCodes.Rem_Un).</summary>
        private void RemUn() => _ilg!.Emit(OpCodes.Rem_Un);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ceq).</summary>
        private void Ceq() => _ilg!.Emit(OpCodes.Ceq);

        /// <summary>A macro for _ilg.Emit(OpCodes.Cgt_Un).</summary>
        private void CgtUn() => _ilg!.Emit(OpCodes.Cgt_Un);

        /// <summary>A macro for _ilg.Emit(OpCodes.Clt_Un).</summary>
        private void CltUn() => _ilg!.Emit(OpCodes.Clt_Un);

        /// <summary>A macro for _ilg.Emit(OpCodes.Pop).</summary>
        private void Pop() => _ilg!.Emit(OpCodes.Pop);

        /// <summary>A macro for _ilg.Emit(OpCodes.Add).</summary>
        private void Add() => _ilg!.Emit(OpCodes.Add);

        /// <summary>A macro for _ilg.Emit(OpCodes.Add); a true flag can turn it into a Sub.</summary>
        private void Add(bool negate) => _ilg!.Emit(negate ? OpCodes.Sub : OpCodes.Add);

        /// <summary>A macro for _ilg.Emit(OpCodes.Sub).</summary>
        private void Sub() => _ilg!.Emit(OpCodes.Sub);

        /// <summary>A macro for _ilg.Emit(OpCodes.Sub) or _ilg.Emit(OpCodes.Add).</summary>
        private void Sub(bool negate) => _ilg!.Emit(negate ? OpCodes.Add : OpCodes.Sub);

        /// <summary>A macro for _ilg.Emit(OpCodes.Mul).</summary>
        private void Mul() => _ilg!.Emit(OpCodes.Mul);

        /// <summary>A macro for _ilg.Emit(OpCodes.And).</summary>
        private void And() => _ilg!.Emit(OpCodes.And);

        /// <summary>A macro for _ilg.Emit(OpCodes.Shl).</summary>
        private void Shl() => _ilg!.Emit(OpCodes.Shl);

        /// <summary>A macro for _ilg.Emit(OpCodes.Shr).</summary>
        private void Shr() => _ilg!.Emit(OpCodes.Shr);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ldloc_S).</summary>
        private void Ldloc(LocalBuilder lt) => _ilg!.Emit(OpCodes.Ldloc_S, lt);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ldloca).</summary>
        private void Ldloca(LocalBuilder lt) => _ilg!.Emit(OpCodes.Ldloca, lt);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ldind_U2).</summary>
        private void LdindU2() => _ilg!.Emit(OpCodes.Ldind_U2);

        /// <summary>A macro for _ilg.Emit(OpCodes.Stloc_S).</summary>
        private void Stloc(LocalBuilder lt) => _ilg!.Emit(OpCodes.Stloc_S, lt);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ldarg_0).</summary>
        private void Ldthis() => _ilg!.Emit(OpCodes.Ldarg_0);

        /// <summary>A macro for Ldthis(); Ldfld();</summary>
        private void Ldthisfld(FieldInfo ft)
        {
            Ldthis();
            Ldfld(ft);
        }

        /// <summary>A macro for Ldthis(); Ldfld(); Stloc();</summary>
        private void Mvfldloc(FieldInfo ft, LocalBuilder lt)
        {
            Ldthisfld(ft);
            Stloc(lt);
        }

        /// <summary>A macro for Ldthis(); Ldloc(); Stfld();</summary>
        private void Mvlocfld(LocalBuilder lt, FieldInfo ft)
        {
            Ldthis();
            Ldloc(lt);
            Stfld(ft);
        }

        /// <summary>A macro for _ilg.Emit(OpCodes.Ldfld).</summary>
        private void Ldfld(FieldInfo ft) => _ilg!.Emit(OpCodes.Ldfld, ft);

        /// <summary>A macro for _ilg.Emit(OpCodes.Stfld).</summary>
        private void Stfld(FieldInfo ft) => _ilg!.Emit(OpCodes.Stfld, ft);

        /// <summary>A macro for _ilg.Emit(OpCodes.Callvirt, mt).</summary>
        private void Callvirt(MethodInfo mt) => _ilg!.Emit(OpCodes.Callvirt, mt);

        /// <summary>A macro for _ilg.Emit(OpCodes.Call, mt).</summary>
        private void Call(MethodInfo mt) => _ilg!.Emit(OpCodes.Call, mt);

        /// <summary>A macro for _ilg.Emit(OpCodes.Brfalse) (long form).</summary>
        private void BrfalseFar(Label l) => _ilg!.Emit(OpCodes.Brfalse, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Brtrue) (long form).</summary>
        private void BrtrueFar(Label l) => _ilg!.Emit(OpCodes.Brtrue, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Br) (long form).</summary>
        private void BrFar(Label l) => _ilg!.Emit(OpCodes.Br, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ble) (long form).</summary>
        private void BleFar(Label l) => _ilg!.Emit(OpCodes.Ble, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Blt) (long form).</summary>
        private void BltFar(Label l) => _ilg!.Emit(OpCodes.Blt, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Blt_Un) (long form).</summary>
        private void BltUnFar(Label l) => _ilg!.Emit(OpCodes.Blt_Un, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bge) (long form).</summary>
        private void BgeFar(Label l) => _ilg!.Emit(OpCodes.Bge, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bge_Un) (long form).</summary>
        private void BgeUnFar(Label l) => _ilg!.Emit(OpCodes.Bge_Un, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bgt) (long form).</summary>
        private void BgtFar(Label l) => _ilg!.Emit(OpCodes.Bgt, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bne) (long form).</summary>
        private void BneFar(Label l) => _ilg!.Emit(OpCodes.Bne_Un, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Beq) (long form).</summary>
        private void BeqFar(Label l) => _ilg!.Emit(OpCodes.Beq, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Brfalse_S) (short jump).</summary>
        private void Brfalse(Label l) => _ilg!.Emit(OpCodes.Brfalse_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Brtrue_S) (short jump).</summary>
        private void Brtrue(Label l) => _ilg!.Emit(OpCodes.Brtrue_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Br_S) (short jump).</summary>
        private void Br(Label l) => _ilg!.Emit(OpCodes.Br_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Ble_S) (short jump).</summary>
        private void Ble(Label l) => _ilg!.Emit(OpCodes.Ble_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Blt_S) (short jump).</summary>
        private void Blt(Label l) => _ilg!.Emit(OpCodes.Blt_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bge_S) (short jump).</summary>
        private void Bge(Label l) => _ilg!.Emit(OpCodes.Bge_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bgt_S) (short jump).</summary>
        private void Bgt(Label l) => _ilg!.Emit(OpCodes.Bgt_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bleun_S) (short jump).</summary>
        private void Bgtun(Label l) => _ilg!.Emit(OpCodes.Bgt_Un_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Bne_S) (short jump).</summary>
        private void Bne(Label l) => _ilg!.Emit(OpCodes.Bne_Un_S, l);

        /// <summary>A macro for _ilg.Emit(OpCodes.Beq_S) (short jump).</summary>
        private void Beq(Label l) => _ilg!.Emit(OpCodes.Beq_S, l);

        /// <summary>A macro for the Ldlen instruction.</summary>
        private void Ldlen() => _ilg!.Emit(OpCodes.Ldlen);

        /// <summary>A macro for the Ldelem_I4 instruction.</summary>
        private void LdelemI4() => _ilg!.Emit(OpCodes.Ldelem_I4);

        /// <summary>A macro for the Stelem_I4 instruction.</summary>
        private void StelemI4() => _ilg!.Emit(OpCodes.Stelem_I4);

        private void Switch(Label[] table) => _ilg!.Emit(OpCodes.Switch, table);

        /// <summary>Declares a local int.</summary>
        private LocalBuilder DeclareInt32() => _ilg!.DeclareLocal(typeof(int));

        /// <summary>Declares a local CultureInfo.</summary>
        private LocalBuilder? DeclareCultureInfo() => _ilg!.DeclareLocal(typeof(CultureInfo)); // cache local variable to avoid unnecessary TLS

        /// <summary>Declares a local int[].</summary>
        private LocalBuilder DeclareInt32Array() => _ilg!.DeclareLocal(typeof(int[]));

        /// <summary>Declares a local string.</summary>
        private LocalBuilder DeclareString() => _ilg!.DeclareLocal(typeof(string));

        private LocalBuilder DeclareReadOnlySpanChar() => _ilg!.DeclareLocal(typeof(ReadOnlySpan<char>));

        /// <summary>Loads the char to the right of the current position.</summary>
        private void Rightchar()
        {
            Ldloc(_runtextLocal!);
            Ldloc(_runtextposLocal!);
            Callvirt(s_stringGetCharsMethod);
        }

        /// <summary>Loads the char to the right of the current position and advances the current position.</summary>
        private void Rightcharnext()
        {
            Ldloc(_runtextLocal!);
            Ldloc(_runtextposLocal!);
            Callvirt(s_stringGetCharsMethod);
            Ldloc(_runtextposLocal!);
            Ldc(1);
            Add();
            Stloc(_runtextposLocal!);
        }

        /// <summary>Loads the char to the left of the current position.</summary>
        private void Leftchar()
        {
            Ldloc(_runtextLocal!);
            Ldloc(_runtextposLocal!);
            Ldc(1);
            Sub();
            Callvirt(s_stringGetCharsMethod);
        }

        /// <summary>Loads the char to the left of the current position and advances (leftward).</summary>
        private void Leftcharnext()
        {
            Ldloc(_runtextLocal!);
            Ldloc(_runtextposLocal!);
            Ldc(1);
            Sub();
            Dup();
            Stloc(_runtextposLocal!);
            Callvirt(s_stringGetCharsMethod);
        }

        /// <summary>Creates a backtrack note and pushes the switch index it on the tracking stack.</summary>
        private void Track()
        {
            ReadyPushTrack();
            Ldc(AddTrack());
            DoPush();
        }

        /// <summary>
        /// Pushes the current switch index on the tracking stack so the backtracking
        /// logic will be repeated again next time we backtrack here.
        /// </summary>
        private void Trackagain()
        {
            ReadyPushTrack();
            Ldc(_backpos);
            DoPush();
        }

        /// <summary>Saves the value of a local variable on the tracking stack.</summary>
        private void PushTrack(LocalBuilder lt)
        {
            ReadyPushTrack();
            Ldloc(lt);
            DoPush();
        }

        /// <summary>
        /// Creates a backtrack note for a piece of code that should only be generated once,
        /// and emits code that pushes the switch index on the backtracking stack.
        /// </summary>
        private void TrackUnique(int i)
        {
            ReadyPushTrack();
            Ldc(AddUniqueTrack(i));
            DoPush();
        }

        /// <summary>
        /// Creates a second-backtrack note for a piece of code that should only be
        /// generated once, and emits code that pushes the switch index on the
        /// backtracking stack.
        /// </summary>
        private void TrackUnique2(int i)
        {
            ReadyPushTrack();
            Ldc(AddUniqueTrack(i, RegexCode.Back2));
            DoPush();
        }

        /// <summary>Prologue to code that will push an element on the tracking stack.</summary>
        private void ReadyPushTrack()
        {
            Ldloc(_runtrackLocal!);
            Ldloc(_runtrackposLocal!);
            Ldc(1);
            Sub();
            Dup();
            Stloc(_runtrackposLocal!);
        }

        /// <summary>Pops an element off the tracking stack (leave it on the operand stack).</summary>
        private void PopTrack()
        {
            Ldloc(_runtrackLocal!);
            Ldloc(_runtrackposLocal!);
            LdelemI4();
            Ldloc(_runtrackposLocal!);
            Ldc(1);
            Add();
            Stloc(_runtrackposLocal!);
        }

        /// <summary>Retrieves the top entry on the tracking stack without popping.</summary>
        private void TopTrack()
        {
            Ldloc(_runtrackLocal!);
            Ldloc(_runtrackposLocal!);
            LdelemI4();
        }

        /// <summary>Saves the value of a local variable on the grouping stack.</summary>
        private void PushStack(LocalBuilder lt)
        {
            ReadyPushStack();
            Ldloc(lt);
            DoPush();
        }

        /// <summary>Prologue to code that will replace the ith element on the grouping stack.</summary>
        internal void ReadyReplaceStack(int i)
        {
            Ldloc(_runstackLocal!);
            Ldloc(_runstackposLocal!);
            if (i != 0)
            {
                Ldc(i);
                Add();
            }
        }

        /// <summary>Prologue to code that will push an element on the grouping stack.</summary>
        private void ReadyPushStack()
        {
            Ldloc(_runstackLocal!);
            Ldloc(_runstackposLocal!);
            Ldc(1);
            Sub();
            Dup();
            Stloc(_runstackposLocal!);
        }

        /// <summary>Retrieves the top entry on the stack without popping.</summary>
        private void TopStack()
        {
            Ldloc(_runstackLocal!);
            Ldloc(_runstackposLocal!);
            LdelemI4();
        }

        /// <summary>Pops an element off the grouping stack (leave it on the operand stack).</summary>
        private void PopStack()
        {
            Ldloc(_runstackLocal!);
            Ldloc(_runstackposLocal!);
            LdelemI4();
            Ldloc(_runstackposLocal!);
            Ldc(1);
            Add();
            Stloc(_runstackposLocal!);
        }

        /// <summary>Pops 1 element off the grouping stack and discards it.</summary>
        private void PopDiscardStack() => PopDiscardStack(1);

        /// <summary>Pops i elements off the grouping stack and discards them.</summary>
        private void PopDiscardStack(int i)
        {
            Ldloc(_runstackposLocal!);
            Ldc(i);
            Add();
            Stloc(_runstackposLocal!);
        }

        /// <summary>Epilogue to code that will replace an element on a stack (use Ld* in between).</summary>
        private void DoReplace() => StelemI4();

        /// <summary>Epilogue to code that will push an element on a stack (use Ld* in between).</summary>
        private void DoPush() => StelemI4();

        /// <summary>Jump to the backtracking switch.</summary>
        private void Back() => BrFar(_backtrack);

        /// <summary>
        /// Branch to the MSIL corresponding to the regex code at i
        /// </summary>
        /// <remarks>
        /// A trick: since track and stack space is gobbled up unboundedly
        /// only as a result of branching backwards, this is where we check
        /// for sufficient space and trigger reallocations.
        ///
        /// If the "goto" is backwards, we generate code that checks
        /// available space against the amount of space that would be needed
        /// in the worst case by code that will only go forward; if there's
        /// not enough, we push the destination on the tracking stack, then
        /// we jump to the place where we invoke the allocator.
        ///
        /// Since forward gotos pose no threat, they just turn into a Br.
        /// </remarks>
        private void Goto(int i)
        {
            if (i < _codepos)
            {
                Label l1 = DefineLabel();

                // When going backwards, ensure enough space.
                Ldloc(_runtrackposLocal!);
                Ldc(_trackcount * 4);
                Ble(l1);
                Ldloc(_runstackposLocal!);
                Ldc(_trackcount * 3);
                BgtFar(_labels![i]);
                MarkLabel(l1);
                ReadyPushTrack();
                Ldc(AddGoto(i));
                DoPush();
                BrFar(_backtrack);
            }
            else
            {
                BrFar(_labels![i]);
            }
        }

        /// <summary>
        /// Returns the position of the next operation in the regex code, taking
        /// into account the different numbers of arguments taken by operations
        /// </summary>
        private int NextCodepos() => _codepos + RegexCode.OpcodeSize(_codes![_codepos]);

        /// <summary>The label for the next (forward) operation.</summary>
        private Label AdvanceLabel() => _labels![NextCodepos()];

        /// <summary>Goto the next (forward) operation.</summary>
        private void Advance() => BrFar(AdvanceLabel());

        /// <summary>Sets the culture local to CultureInfo.CurrentCulture.</summary>
        private void InitLocalCultureInfo()
        {
            Debug.Assert(_cultureLocal != null);
            Call(s_cultureInfoGetCurrentCultureMethod);
            Stloc(_cultureLocal);
        }

        /// <summary>Whether ToLower operations should be performed with the invariant culture as opposed to the one in <see cref="_cultureLocal"/>.</summary>
        private bool UseToLowerInvariant => _cultureLocal == null || (_options & RegexOptions.CultureInvariant) != 0;

        /// <summary>Invokes either char.ToLower(..., _culture) or char.ToLowerInvariant(...).</summary>
        private void CallToLower()
        {
            if (UseToLowerInvariant)
            {
                Call(s_charToLowerInvariantMethod);
            }
            else
            {
                Ldloc(_cultureLocal!);
                Call(s_charToLowerMethod);
            }
        }

        /// <summary>
        /// Generates the first section of the MSIL. This section contains all
        /// the forward logic, and corresponds directly to the regex codes.
        /// In the absence of backtracking, this is all we would need.
        /// </summary>
        private void GenerateForwardSection()
        {
            _uniquenote = new int[Uniquecount];
            _labels = new Label[_codes!.Length];
            _goto = new int[_codes.Length];

            // initialize

            Array.Fill(_uniquenote, -1);
            for (int codepos = 0; codepos < _codes.Length; codepos += RegexCode.OpcodeSize(_codes[codepos]))
            {
                _goto[codepos] = -1;
                _labels[codepos] = DefineLabel();
            }

            // emit variable initializers

            Mvfldloc(s_runtextField, _runtextLocal!);
            Mvfldloc(s_runtextbegField, _runtextbegLocal!);
            Mvfldloc(s_runtextendField, _runtextendLocal!);
            Mvfldloc(s_runtextposField, _runtextposLocal!);
            Mvfldloc(s_runtrackField, _runtrackLocal!);
            Mvfldloc(s_runtrackposField, _runtrackposLocal!);
            Mvfldloc(s_runstackField, _runstackLocal!);
            Mvfldloc(s_runstackposField, _runstackposLocal!);

            _backpos = -1;

            for (int codepos = 0; codepos < _codes.Length; codepos += RegexCode.OpcodeSize(_codes[codepos]))
            {
                MarkLabel(_labels[codepos]);
                _codepos = codepos;
                _regexopcode = _codes[codepos];
                GenerateOneCode();
            }
        }

        /// <summary>
        /// Generates the middle section of the MSIL. This section contains the
        /// big switch jump that allows us to simulate a stack of addresses,
        /// and it also contains the calls that expand the tracking and the
        /// grouping stack when they get too full.
        /// </summary>
        private void GenerateMiddleSection()
        {
            LocalBuilder limitLocal = _temp1Local!;
            Label afterDoubleStack = DefineLabel();
            Label afterDoubleTrack = DefineLabel();

            // Backtrack:
            MarkLabel(_backtrack);

            // (Equivalent of EnsureStorage, but written to avoid unnecessary local spilling.)

            // int limitLocal = runtrackcount * 4;
            Ldthisfld(s_runtrackcountField);
            Ldc(4);
            Mul();
            Stloc(limitLocal);

            // if (runstackpos < limit)
            // {
            //     this.runstackpos = runstackpos;
            //     DoubleStack(); // might change runstackpos and runstack
            //     runstackpos = this.runstackpos;
            //     runstack = this.runstack;
            // }
            Ldloc(_runstackposLocal!);
            Ldloc(limitLocal);
            Bge(afterDoubleStack);
            Mvlocfld(_runstackposLocal!, s_runstackposField);
            Ldthis();
            Callvirt(s_doubleStackMethod);
            Mvfldloc(s_runstackposField, _runstackposLocal!);
            Mvfldloc(s_runstackField, _runstackLocal!);
            MarkLabel(afterDoubleStack);

            // if (runtrackpos < limit)
            // {
            //     this.runtrackpos = runtrackpos;
            //     DoubleTrack(); // might change runtrackpos and runtrack
            //     runtrackpos = this.runtrackpos;
            //     runtrack = this.runtrack;
            // }
            Ldloc(_runtrackposLocal!);
            Ldloc(limitLocal);
            Bge(afterDoubleTrack);
            Mvlocfld(_runtrackposLocal!, s_runtrackposField);
            Ldthis();
            Callvirt(s_doubleTrackMethod);
            Mvfldloc(s_runtrackposField, _runtrackposLocal!);
            Mvfldloc(s_runtrackField, _runtrackLocal!);
            MarkLabel(afterDoubleTrack);

            // runtrack[runtrackpos++]
            PopTrack();

            // Backtracking jump table
            var table = new Label[_notecount];
            for (int i = 0; i < _notecount; i++)
            {
                table[i] = _notes![i]._label;
            }
            Switch(table);
        }

        /// <summary>
        /// Generates the last section of the MSIL. This section contains all of
        /// the backtracking logic.
        /// </summary>
        private void GenerateBacktrackSection()
        {
            for (int i = 0; i < _notecount; i++)
            {
                BacktrackNote n = _notes![i];
                if (n._flags != 0)
                {
                    MarkLabel(n._label);
                    _codepos = n._codepos;
                    _backpos = i;
                    _regexopcode = _codes![n._codepos] | n._flags;
                    GenerateOneCode();
                }
            }
        }

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!! This function must be kept synchronized with FindFirstChar in      !!!!
        // !!!! RegexInterpreter.cs                                                !!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        /// <summary>
        /// Generates FindFirstChar.
        /// </summary>
        protected void GenerateFindFirstChar()
        {
            _runtextposLocal = DeclareInt32();
            _runtextLocal = DeclareString();
            _temp1Local = DeclareInt32();
            _temp2Local = DeclareInt32();
            _cultureLocal = null;
            if (!_options.HasFlag(RegexOptions.CultureInvariant))
            {
                if (_options.HasFlag(RegexOptions.IgnoreCase) ||
                    _bmPrefix?.CaseInsensitive == true ||
                    _fcPrefix.GetValueOrDefault().CaseInsensitive)
                {
                    _cultureLocal = DeclareCultureInfo();
                    InitLocalCultureInfo();
                }
            }

            if ((_anchors & (RegexFCD.Beginning | RegexFCD.Start | RegexFCD.EndZ | RegexFCD.End)) != 0)
            {
                if (!_code!.RightToLeft)
                {
                    if ((_anchors & RegexFCD.Beginning) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextbegField);
                        Ble(l1);
                        Ldthis();
                        Ldthisfld(s_runtextendField);
                        Stfld(s_runtextposField);
                        Ldc(0);
                        Ret();
                        MarkLabel(l1);
                    }

                    if ((_anchors & RegexFCD.Start) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextstartField);
                        Ble(l1);
                        Ldthis();
                        Ldthisfld(s_runtextendField);
                        Stfld(s_runtextposField);
                        Ldc(0);
                        Ret();
                        MarkLabel(l1);
                    }

                    if ((_anchors & RegexFCD.EndZ) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextendField);
                        Ldc(1);
                        Sub();
                        Bge(l1);
                        Ldthis();
                        Ldthisfld(s_runtextendField);
                        Ldc(1);
                        Sub();
                        Stfld(s_runtextposField);
                        MarkLabel(l1);
                    }

                    if ((_anchors & RegexFCD.End) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextendField);
                        Bge(l1);
                        Ldthis();
                        Ldthisfld(s_runtextendField);
                        Stfld(s_runtextposField);
                        MarkLabel(l1);
                    }
                }
                else
                {
                    if ((_anchors & RegexFCD.End) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextendField);
                        Bge(l1);
                        Ldthis();
                        Ldthisfld(s_runtextbegField);
                        Stfld(s_runtextposField);
                        Ldc(0);
                        Ret();
                        MarkLabel(l1);
                    }

                    if ((_anchors & RegexFCD.EndZ) != 0)
                    {
                        Label l1 = DefineLabel();
                        Label l2 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextendField);
                        Ldc(1);
                        Sub();
                        Blt(l1);
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextendField);
                        Beq(l2);
                        Ldthisfld(s_runtextField);
                        Ldthisfld(s_runtextposField);
                        Callvirt(s_stringGetCharsMethod);
                        Ldc('\n');
                        Beq(l2);
                        MarkLabel(l1);
                        Ldthis();
                        Ldthisfld(s_runtextbegField);
                        Stfld(s_runtextposField);
                        Ldc(0);
                        Ret();
                        MarkLabel(l2);
                    }

                    if ((_anchors & RegexFCD.Start) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextstartField);
                        Bge(l1);
                        Ldthis();
                        Ldthisfld(s_runtextbegField);
                        Stfld(s_runtextposField);
                        Ldc(0);
                        Ret();
                        MarkLabel(l1);
                    }

                    if ((_anchors & RegexFCD.Beginning) != 0)
                    {
                        Label l1 = DefineLabel();
                        Ldthisfld(s_runtextposField);
                        Ldthisfld(s_runtextbegField);
                        Ble(l1);
                        Ldthis();
                        Ldthisfld(s_runtextbegField);
                        Stfld(s_runtextposField);
                        MarkLabel(l1);
                    }
                }

                Ldc(1);
                Ret();
            }
            else if (_bmPrefix != null && _bmPrefix.NegativeUnicode == null)
            {
                // Compiled Boyer-Moore string matching

                LocalBuilder chLocal = _temp1Local;
                LocalBuilder testLocal = _temp1Local;
                LocalBuilder limitLocal = _temp2Local;
                Label lDefaultAdvance = DefineLabel();
                Label lAdvance = DefineLabel();
                Label lFail = DefineLabel();
                Label lStart = DefineLabel();
                Label lPartialMatch = DefineLabel();

                int beforefirst;
                int last;
                if (!_code!.RightToLeft)
                {
                    beforefirst = -1;
                    last = _bmPrefix.Pattern.Length - 1;
                }
                else
                {
                    beforefirst = _bmPrefix.Pattern.Length;
                    last = 0;
                }

                int chLast = _bmPrefix.Pattern[last];

                Mvfldloc(s_runtextField, _runtextLocal);
                Ldthisfld(_code.RightToLeft ? s_runtextbegField : s_runtextendField);
                Stloc(limitLocal);

                Ldthisfld(s_runtextposField);
                if (!_code.RightToLeft)
                {
                    Ldc(_bmPrefix.Pattern.Length - 1);
                    Add();
                }
                else
                {
                    Ldc(_bmPrefix.Pattern.Length);
                    Sub();
                }
                Stloc(_runtextposLocal);
                Br(lStart);

                MarkLabel(lDefaultAdvance);

                Ldc(_code.RightToLeft ? -_bmPrefix.Pattern.Length : _bmPrefix.Pattern.Length);

                MarkLabel(lAdvance);

                Ldloc(_runtextposLocal);
                Add();
                Stloc(_runtextposLocal);

                MarkLabel(lStart);

                Ldloc(_runtextposLocal);
                Ldloc(limitLocal);
                if (!_code.RightToLeft)
                {
                    BgeFar(lFail);
                }
                else
                {
                    BltFar(lFail);
                }

                Rightchar();
                if (_bmPrefix.CaseInsensitive)
                {
                    CallToLower();
                }

                Dup();
                Stloc(chLocal);
                Ldc(chLast);
                BeqFar(lPartialMatch);

                Ldloc(chLocal);
                Ldc(_bmPrefix.LowASCII);
                Sub();
                Dup();
                Stloc(chLocal);
                Ldc(_bmPrefix.HighASCII - _bmPrefix.LowASCII);
                Bgtun(lDefaultAdvance);

                var table = new Label[_bmPrefix.HighASCII - _bmPrefix.LowASCII + 1];

                for (int i = _bmPrefix.LowASCII; i <= _bmPrefix.HighASCII; i++)
                {
                    table[i - _bmPrefix.LowASCII] = (_bmPrefix.NegativeASCII[i] == beforefirst) ?
                        lDefaultAdvance :
                        DefineLabel();
                }

                Ldloc(chLocal);
                Switch(table);

                for (int i = _bmPrefix.LowASCII; i <= _bmPrefix.HighASCII; i++)
                {
                    if (_bmPrefix.NegativeASCII[i] == beforefirst)
                    {
                        continue;
                    }

                    MarkLabel(table[i - _bmPrefix.LowASCII]);

                    Ldc(_bmPrefix.NegativeASCII[i]);
                    BrFar(lAdvance);
                }

                MarkLabel(lPartialMatch);

                Ldloc(_runtextposLocal);
                Stloc(testLocal);

                for (int i = _bmPrefix.Pattern.Length - 2; i >= 0; i--)
                {
                    Label lNext = DefineLabel();
                    int charindex = _code.RightToLeft ?
                        _bmPrefix.Pattern.Length - 1 - i :
                        i;

                    Ldloc(_runtextLocal);
                    Ldloc(testLocal);
                    Ldc(1);
                    Sub(_code.RightToLeft);
                    Dup();
                    Stloc(testLocal);
                    Callvirt(s_stringGetCharsMethod);
                    if (_bmPrefix.CaseInsensitive)
                    {
                        CallToLower();
                    }

                    Ldc(_bmPrefix.Pattern[charindex]);
                    Beq(lNext);
                    Ldc(_bmPrefix.Positive[charindex]);
                    BrFar(lAdvance);

                    MarkLabel(lNext);
                }

                Ldthis();
                Ldloc(testLocal);
                if (_code.RightToLeft)
                {
                    Ldc(1);
                    Add();
                }
                Stfld(s_runtextposField);
                Ldc(1);
                Ret();

                MarkLabel(lFail);

                Ldthis();
                Ldthisfld(_code.RightToLeft ? s_runtextbegField : s_runtextendField);
                Stfld(s_runtextposField);
                Ldc(0);
                Ret();
            }
            else if (!_fcPrefix.HasValue)
            {
                Ldc(1);
                Ret();
            }
            else if (_code!.RightToLeft)
            {
                LocalBuilder charInClassLocal = _temp1Local;
                LocalBuilder cLocal = _temp2Local;

                Label l1 = DefineLabel();
                Label l2 = DefineLabel();
                Label l3 = DefineLabel();
                Label l4 = DefineLabel();
                Label l5 = DefineLabel();

                Mvfldloc(s_runtextposField, _runtextposLocal);
                Mvfldloc(s_runtextField, _runtextLocal);

                Ldloc(_runtextposLocal);
                Ldthisfld(s_runtextbegField);
                Sub();
                Stloc(cLocal);

                Ldloc(cLocal);
                Ldc(0);
                BleFar(l4);

                MarkLabel(l1);

                Ldloc(cLocal);
                Ldc(1);
                Sub();
                Stloc(cLocal);

                Leftcharnext();

                if (!RegexCharClass.IsSingleton(_fcPrefix.GetValueOrDefault().Prefix))
                {
                    EmitCallCharInClass(_fcPrefix.GetValueOrDefault().Prefix, _fcPrefix.GetValueOrDefault().CaseInsensitive, charInClassLocal);
                    BrtrueFar(l2);
                }
                else
                {
                    Ldc(RegexCharClass.SingletonChar(_fcPrefix.GetValueOrDefault().Prefix));
                    Beq(l2);
                }

                MarkLabel(l5);

                Ldloc(cLocal);
                Ldc(0);
                if (!RegexCharClass.IsSingleton(_fcPrefix.GetValueOrDefault().Prefix))
                {
                    BgtFar(l1);
                }
                else
                {
                    Bgt(l1);
                }

                Ldc(0);
                BrFar(l3);

                MarkLabel(l2);

                Ldloc(_runtextposLocal);
                Ldc(1);
                Sub(_code.RightToLeft);
                Stloc(_runtextposLocal);
                Ldc(1);

                MarkLabel(l3);

                Mvlocfld(_runtextposLocal, s_runtextposField);
                Ret();

                MarkLabel(l4);
                Ldc(0);
                Ret();
            }
            else // for left-to-right, we can take advantage of vectorization and JIT optimizations
            {
                LocalBuilder iLocal = _temp2Local;
                Label returnFalseLabel = DefineLabel();
                Label updatePosAndReturnFalse = DefineLabel();

                // if (runtextend - runtextpos > 0)
                Ldthisfld(s_runtextendField);
                Ldthisfld(s_runtextposField);
                Sub();
                Ldc(0);
                BleFar(returnFalseLabel);

                // string runtext = this.runtext
                Mvfldloc(s_runtextField, _runtextLocal);

                Span<char> setChars = stackalloc char[3];
                int setCharsCount;
                if (!_fcPrefix.GetValueOrDefault().CaseInsensitive &&
                    (setCharsCount = RegexCharClass.GetSetChars(_fcPrefix.GetValueOrDefault().Prefix, setChars)) > 0)
                {
                    // This is a case-sensitive class with a small number of characters in the class, small enough
                    // that we can generate an IndexOf{Any} call.  That takes advantage of optimizations in
                    // IndexOf{Any}, such as vectorization, which our open-coded loop through the span doesn't have.
                    switch (setCharsCount)
                    {
                        case 1:
                            // int i = runtext.IndexOf(setChars[0], runtextpos, runtextend - runtextpos);
                            Ldloc(_runtextLocal);
                            Ldc(setChars[0]);
                            Ldthisfld(s_runtextposField);
                            Ldthisfld(s_runtextendField);
                            Ldthisfld(s_runtextposField);
                            Sub();
                            Call(s_stringIndexOf);
                            Stloc(iLocal);

                            // if (i >= 0)
                            Ldloc(iLocal);
                            Ldc(0);
                            BltFar(updatePosAndReturnFalse);

                            // runtextpos = i; return true;
                            Mvlocfld(iLocal, s_runtextposField);
                            Ldc(1);
                            Ret();
                            break;

                        case 2:
                        case 3:
                            // int i = runtext.AsSpan(runtextpos, runtextend - runtextpos).IndexOfAny(setChars[0], setChars[1]{, setChars[2]});
                            Ldloc(_runtextLocal);
                            Ldthisfld(s_runtextposField);
                            Ldthisfld(s_runtextendField);
                            Ldthisfld(s_runtextposField);
                            Sub();
                            Call(s_stringAsSpanMethod);
                            Ldc(setChars[0]);
                            Ldc(setChars[1]);
                            if (setCharsCount == 3)
                            {
                                Ldc(setChars[2]);
                                Call(s_spanIndexOfAnyCharCharChar);
                            }
                            else
                            {
                                Call(s_spanIndexOfAnyCharChar);
                            }
                            Stloc(iLocal);

                            // if (i >= 0)
                            Ldloc(iLocal);
                            Ldc(0);
                            BltFar(updatePosAndReturnFalse);

                            // runtextpos = i; return true;
                            Ldthis();
                            Ldthisfld(s_runtextposField);
                            Ldloc(iLocal);
                            Add();
                            Stfld(s_runtextposField);
                            Ldc(1);
                            Ret();
                            break;

                        default:
                            Debug.Fail("Unexpected setCharsCount: " + setCharsCount);
                            break;
                    }
                }
                else
                {
                    // Either this isn't a class with just a few characters in it, or this is case insensitive.
                    // Either way, create a span and iterate through it rather than the original string in order
                    // to avoid bounds checks on each access.

                    LocalBuilder charInClassLocal = _temp1Local;
                    _temp3Local = DeclareReadOnlySpanChar();
                    LocalBuilder textSpanLocal = _temp3Local;

                    Label checkSpanLengthLabel = DefineLabel();
                    Label charNotInClassLabel = DefineLabel();
                    Label loopBody = DefineLabel();

                    // ReadOnlySpan<char> span = runtext.AsSpan(runtextpos, runtextend - runtextpos);
                    Ldloc(_runtextLocal);
                    Ldthisfld(s_runtextposField);
                    Ldthisfld(s_runtextendField);
                    Ldthisfld(s_runtextposField);
                    Sub();
                    Call(s_stringAsSpanMethod);
                    Stloc(textSpanLocal);

                    // for (int i = 0;
                    Ldc(0);
                    Stloc(iLocal);
                    BrFar(checkSpanLengthLabel);

                    // if (CharInClass(span[i], "..."))
                    MarkLabel(loopBody);
                    Ldloca(textSpanLocal);
                    Ldloc(iLocal);
                    Call(s_spanGetItemMethod);
                    LdindU2();
                    EmitCallCharInClass(_fcPrefix.GetValueOrDefault().Prefix, _fcPrefix.GetValueOrDefault().CaseInsensitive, charInClassLocal);
                    BrfalseFar(charNotInClassLabel);

                    // runtextpos += i; return true;
                    Ldthis();
                    Ldthisfld(s_runtextposField);
                    Ldloc(iLocal);
                    Add();
                    Stfld(s_runtextposField);
                    Ldc(1);
                    Ret();

                    // for (...; ...; i++)
                    MarkLabel(charNotInClassLabel);
                    Ldloc(iLocal);
                    Ldc(1);
                    Add();
                    Stloc(iLocal);

                    // for (...; i < span.Length; ...);
                    MarkLabel(checkSpanLengthLabel);
                    Ldloc(iLocal);
                    Ldloca(textSpanLocal);
                    Call(s_spanGetLengthMethod);
                    BltFar(loopBody);
                }

                // runtextpos = runtextend;
                MarkLabel(updatePosAndReturnFalse);
                Ldthis();
                Ldthisfld(s_runtextendField);
                Stfld(s_runtextposField);

                // return false;
                MarkLabel(returnFalseLabel);
                Ldc(0);
                Ret();
            }
        }

        /// <summary>Generates a very simple method that sets the _trackcount field.</summary>
        protected void GenerateInitTrackCount()
        {
            Ldthis();
            Ldc(_trackcount);
            Stfld(s_runtrackcountField);
            Ret();
        }

        private bool TryGenerateNonBacktrackingGo(RegexNode node)
        {
            Debug.Assert(node.Type == RegexNode.Capture && node.ChildCount() == 1,
                "Every generated tree should begin with a capture node that has a single child.");

            // RightToLeft is rare and not worth adding a lot of custom code to handle in this path.
            if ((node.Options & RegexOptions.RightToLeft) != 0)
            {
                return false;
            }

            // Skip the Capture node.  This path only supports the implicit capture of the whole match,
            // which we handle implicitly at the end of the generated code in one location.
            node = node.Child(0);

            // Alternate is supported only at the root (a global alternate). That way, any failures in a
            // branch can be unwound by just resetting everything, rather than keeping track of a backtracking stack.
            if (node.Type == RegexNode.Alternate)
            {
                int branchCount = node.ChildCount();
                Debug.Assert(branchCount >= 2);
                for (int i = 0; i < branchCount; i++)
                {
                    if (!NodeSupportsNonBacktrackingImplementation(node.Child(i)))
                    {
                        return false;
                    }
                }
            }
            else if (!NodeSupportsNonBacktrackingImplementation(node))
            {
                return false;
            }

            // We've determined that the RegexNode can be handled with this optimized path.  Generate the code.
#if DEBUG
            if ((_options & RegexOptions.Debug) != 0)
            {
                Debug.WriteLine("Using optimized non-backtracking code gen.");
            }
#endif

            // Declare some locals.
            LocalBuilder runtextposLocal = DeclareInt32();
            LocalBuilder originalruntextposLocal = DeclareInt32();
            LocalBuilder textSpanLocal = DeclareReadOnlySpanChar();
            LocalBuilder originalTextSpanLocal = DeclareReadOnlySpanChar();
            LocalBuilder setScratchLocal = DeclareInt32();
            LocalBuilder? iterationLocal = null;
            Label stopSuccessLabel = DefineLabel();
            Label doneLabel = DefineLabel();
            if (_hasTimeout)
            {
                _loopTimeoutCounterLocal = DeclareInt32();
            }

            // CultureInfo culture = CultureInfo.CurrentCulture;
            // (only if the whole expression or any subportion is ignoring case, and we're not using invariant)
            InitializeCultureForGoIfNecessary();

            // int runtextpos = this.runtextpos;
            // int originalruntextpos = runtextpos;
            Ldthisfld(s_runtextposField);
            Dup();
            Stloc(runtextposLocal);
            Stloc(originalruntextposLocal);

            // ReadOnlySpan<char> textSpan = this.runtext.AsSpan(this.runtextpos, this.runtextend - this.runtextpos);
            // ReadOnlySpan<char> originalTextSpan = textSpan;
            Ldthisfld(s_runtextField);
            Ldthisfld(s_runtextposField);
            Ldthisfld(s_runtextendField);
            Ldthisfld(s_runtextposField);
            Sub();
            Call(s_stringAsSpanMethod);
            Dup();
            Stloc(textSpanLocal);
            Stloc(originalTextSpanLocal);

            // The implementation tries to use const indexes into the span wherever possible, which we can do
            // in all places except for variable-length loops.  For everything else, we know at any point in
            // the regex exactly how far into it we are, and we can use that to index into the span created
            // at the beginning of the routine to begin at exactly where we're starting in the input.  For
            // variable-length loops, we index at this position + i, and then after the loop we slice the input
            // by i so that this position is still accurate for everything after it.
            int textSpanPos = 0;

            // Emit the code for all nodes in the tree.
            if (node.Type == RegexNode.Alternate)
            {
                EmitGlobalAlternate(node);
            }
            else
            {
                EmitNode(node);
            }

            // Success:
            // this.runtextpos = runtextpos + textSpanPos;
            MarkLabel(stopSuccessLabel);
            Ldthis();
            Ldloc(runtextposLocal);
            Ldc(textSpanPos);
            Add();
            Stfld(s_runtextposField);

            // Capture(0, originalruntextposLocal, this.runtextpos);
            Ldthis();
            Ldc(0);
            Ldloc(originalruntextposLocal);
            Ldthisfld(s_runtextposField);
            Callvirt(s_captureMethod);

            // Done:
            // return;
            MarkLabel(doneLabel);
            Ret();

            // Generated code successfully with non-backtracking implementation.
            return true;

            // Determines whether the node supports an optimized implementation that doesn't allow for backtracking.
            static bool NodeSupportsNonBacktrackingImplementation(RegexNode node)
            {
                bool supported = false;

                // We only support the default left-to-right, not right-to-left, which requires more complication in the gerated code.
                if ((node.Options & RegexOptions.RightToLeft) == 0)
                {
                    switch (node.Type)
                    {
                        // One/Notone/Set/Multi don't involve any repetition and are easily supported.
                        case RegexNode.One:
                        case RegexNode.Notone:
                        case RegexNode.Set:
                        case RegexNode.Multi:
                            supported = true;
                            break;

                        // Boundaries are like set checks and don't involve repetition, either.
                        case RegexNode.Boundary:
                        case RegexNode.Nonboundary:
                        case RegexNode.ECMABoundary:
                        case RegexNode.NonECMABoundary:
                            supported = true;
                            break;

                        // {Set/One}loopgreedy are optimized nodes that represent non-backtracking variable-length loops.
                        // These consume their {Set/One} inputs as long as they match, and don't give up anything they
                        // matched, which means we can support them without backtracking.
                        case RegexNode.Oneloopgreedy:
                        case RegexNode.Notoneloopgreedy:
                        case RegexNode.Setloopgreedy:
                            // TODO: Add support for greedy {Lazy}Loop around supported elements, namely Concatenate.
                            //       Nested loops will require multiple iteration variables to be defined.
                            supported = true;
                            break;

                        // Repeaters don't require backtracking as long as their min and max are equal.
                        // At that point they're just a shorthand for writing out the One/Notone/Set
                        // that number of times.
                        case RegexNode.Oneloop:
                        case RegexNode.Onelazy:
                        case RegexNode.Notoneloop:
                        case RegexNode.Notonelazy:
                        case RegexNode.Setloop:
                        case RegexNode.Setlazy:
                            // TODO: Add support for {Lazy}Loop with M == N.
                            //       Nested loops will require adding a recursion cut-off to avoid stack dives for bad regexes.
                            supported = node.M == node.N;
                            break;

                        // Concatenation doesn't require backtracking as long as its children don't.
                        case RegexNode.Concatenate:
                            supported = true;
                            int childCount = node.ChildCount();
                            for (int i = 0; i < childCount; i++)
                            {
                                Debug.Assert(node.Child(i).Type != RegexNode.Concatenate, "Should have been eliminated by parser / tree reducer. Failure to do so could lead to stack dives here.");
                                if (!NodeSupportsNonBacktrackingImplementation(node.Child(i)))
                                {
                                    supported = false;
                                    break;
                                }
                            }
                            break;

                        // "Empty" is easy: nothing is emitted for it.
                        // "Nothing" is also easy: it doesn't match anything.
                        case RegexNode.Empty:
                        case RegexNode.Nothing:
                            supported = true;
                            break;
                    }
                }

                if (supported)
                {
                    return true;
                }

                // This node can't be supported today by the current non-backtracking implementation.
#if DEBUG
                if ((node.Options & RegexOptions.Debug) != 0)
                {
                    Debug.WriteLine($"Unable to use non-backtracking code gen: node {node.Description()} isn't supported.");
                }
#endif
                return false;
            }

            static bool IsCaseInsensitive(RegexNode node) => (node.Options & RegexOptions.IgnoreCase) != 0;

            // Emits a check that the span is large enough at the currently known static position to handle the required additional length.
            void EmitSpanLengthCheck(int requiredLength)
            {
                // if ((uint)(textSpanPos + requiredLength - 1) >= (uint)textSpan.Length) goto Done;
                Debug.Assert(requiredLength > 0);
                Ldc(textSpanPos + requiredLength - 1);
                Ldloca(textSpanLocal);
                Call(s_spanGetLengthMethod);
                BgeUnFar(doneLabel);
            }

            // Emits the code for an Alternate at the root of the tree.  This amounts to generating the code for each branch,
            // with failures in a branch resetting state to what it was initially and then jumping to the next branch.
            // We don't need to worry about uncapturing, because capturing is only allowed for the implicit capture that
            // happens for the whole match at the end.
            void EmitGlobalAlternate(RegexNode node)
            {
                // Branch0(); // jumps to NextBranch1 on failure
                // goto Success;
                //
                // NextBranch1:
                // runtextpos = originalruntextpos;
                // textSpan = originalTextSpan;
                // Branch1(); // jumps to NextBranch2 on failure
                // goto Success;
                //
                // ...
                //
                // NextBranchN:
                // runtextpos = originalruntextpos;
                // textSpan = originalTextSpan;
                // BranchN(); // jumps to Done on failure

                Label postAlternateDone = doneLabel;
                int childCount = node.ChildCount();
                for (int i = 0; i < childCount - 1; i++)
                {
                    Label nextBranch = DefineLabel();
                    doneLabel = nextBranch;

                    EmitNode(node.Child(i));
                    BrFar(stopSuccessLabel);

                    MarkLabel(nextBranch);

                    // Reset state for next branch
                    textSpanPos = 0;
                    Ldloc(originalruntextposLocal);
                    Stloc(runtextposLocal);
                    Ldloc(originalTextSpanLocal);
                    Stloc(textSpanLocal);
                }

                doneLabel = postAlternateDone;
                EmitNode(node.Child(childCount - 1));
            }

            // Emits the code for the node.
            void EmitNode(RegexNode node)
            {
                switch (node.Type)
                {
                    case RegexNode.One:
                    case RegexNode.Notone:
                    case RegexNode.Set:
                        EmitSingleChar(node);
                        break;

                    case RegexNode.Boundary:
                    case RegexNode.Nonboundary:
                    case RegexNode.ECMABoundary:
                    case RegexNode.NonECMABoundary:
                        EmitBoundary(node);
                        break;

                    case RegexNode.Multi:
                        EmitMultiChar(node);
                        break;

                    case RegexNode.Oneloopgreedy:
                    case RegexNode.Notoneloopgreedy:
                    case RegexNode.Setloopgreedy:
                        EmitGreedyLoop(node);
                        break;

                    case RegexNode.Oneloop:
                    case RegexNode.Onelazy:
                    case RegexNode.Notoneloop:
                    case RegexNode.Notonelazy:
                    case RegexNode.Setloop:
                    case RegexNode.Setlazy:
                        EmitRepeater(node);
                        break;

                    case RegexNode.Concatenate:
                        int childCount = node.ChildCount();
                        for (int i = 0; i < childCount; i++)
                        {
                            EmitNode(node.Child(i));
                        }
                        break;

                    case RegexNode.Nothing:
                        BrFar(doneLabel);
                        break;

                    case RegexNode.Empty:
                        // Emit nothing.
                        break;

                    default:
                        Debug.Fail($"Unexpected node type: {node.Type}");
                        break;
                }
            }

            // Emits the code to handle a single-character match.
            void EmitSingleChar(RegexNode node)
            {
                // if (textSpanPos >= textSpan.Length || textSpan[textSpanPos] != ch) goto Done;
                EmitSpanLengthCheck(1);
                Ldloca(textSpanLocal);
                Ldc(textSpanPos);
                Call(s_spanGetItemMethod);
                LdindU2();
                switch (node.Type)
                {
                    // This only emits a single check, but it's called from the looping constructs in a loop
                    // to generate the code for a single check, so we map those looping constructs to the
                    // appropriate single check.

                    case RegexNode.Set:
                    case RegexNode.Setlazy:
                    case RegexNode.Setloop:
                    case RegexNode.Setloopgreedy:
                        EmitCallCharInClass(node.Str!, IsCaseInsensitive(node), setScratchLocal);
                        BrfalseFar(doneLabel);
                        break;

                    case RegexNode.One:
                    case RegexNode.Onelazy:
                    case RegexNode.Oneloop:
                    case RegexNode.Oneloopgreedy:
                        if (IsCaseInsensitive(node)) CallToLower();
                        Ldc(node.Ch);
                        BneFar(doneLabel);
                        break;

                    default:
                        Debug.Assert(node.Type == RegexNode.Notone || node.Type == RegexNode.Notonelazy || node.Type == RegexNode.Notoneloop || node.Type == RegexNode.Notoneloopgreedy);
                        if (IsCaseInsensitive(node)) CallToLower();
                        Ldc(node.Ch);
                        BeqFar(doneLabel);
                        break;
                }

                textSpanPos++;
            }

            // Emits the code to handle a boundary check on a character.
            void EmitBoundary(RegexNode node)
            {
                // if (!IsBoundary(runtextpos + textSpanPos, this.runtextbeg, this.runtextend)) goto doneLabel;
                Ldthis();
                Ldloc(runtextposLocal);
                Ldc(textSpanPos);
                Add();
                Ldthisfld(s_runtextbegField!);
                Ldthisfld(s_runtextendField!);
                switch (node.Type)
                {
                    case RegexNode.Boundary:
                        Callvirt(s_isBoundaryMethod);
                        BrfalseFar(doneLabel);
                        break;

                    case RegexNode.Nonboundary:
                        Callvirt(s_isBoundaryMethod);
                        BrtrueFar(doneLabel);
                        break;

                    case RegexNode.ECMABoundary:
                        Callvirt(s_isECMABoundaryMethod);
                        BrfalseFar(doneLabel);
                        break;

                    default:
                        Debug.Assert(node.Type == RegexNode.NonECMABoundary);
                        Callvirt(s_isECMABoundaryMethod);
                        BrtrueFar(doneLabel);
                        break;
                }
            }

            // Emits the code to handle a multiple-character match.
            void EmitMultiChar(RegexNode node)
            {
                // if (textSpanPos + node.Str.Length >= textSpan.Length) goto doneLabel;
                // if (node.Str[0] != textSpan[textSpanPos]) goto doneLabel;
                // if (node.Str[1] != textSpan[textSpanPos+1]) goto doneLabel;
                // ...
                EmitSpanLengthCheck(node.Str!.Length);
                for (int i = 0; i < node.Str!.Length; i++)
                {
                    Ldloca(textSpanLocal);
                    Ldc(textSpanPos + i);
                    Call(s_spanGetItemMethod);
                    LdindU2();
                    if (IsCaseInsensitive(node)) CallToLower();
                    Ldc(node.Str[i]);
                    BneFar(doneLabel);
                }

                textSpanPos += node.Str.Length;
            }

            // Emits the code to handle a loop (repeater) with a fixed number of iterations.
            // This is used both to handle the case of A{5, 5} where the min and max are equal,
            // and also to handle part of the case of A{3, 5}, where this method is called to
            // handle the A{3, 3} portion, and then remaining A{0, 2} is handled separately.
            void EmitRepeater(RegexNode node, int iterations = -1)
            {
                if (iterations == -1)
                {
                    Debug.Assert(node.M > 0 && node.M == node.N);
                    iterations = node.M;
                }
                Debug.Assert(iterations > 0);

                // for (int i = 0; i < iterations; i++)
                // {
                //     TimeoutCheck();
                //     if (textSpan[textSpanPos] != ch) goto Done;
                // }

                Label conditionLabel = DefineLabel();
                Label bodyLabel = DefineLabel();

                iterationLocal ??= DeclareInt32();

                Ldc(0);
                Stloc(iterationLocal);
                BrFar(conditionLabel);

                MarkLabel(bodyLabel);
                EmitTimeoutCheck();
                EmitSingleChar(node);
                Ldloc(iterationLocal);
                Ldc(1);
                Add();
                Stloc(iterationLocal);

                MarkLabel(conditionLabel);
                Ldloc(iterationLocal);
                Ldc(iterations);
                BltFar(bodyLabel);

                textSpanPos += (iterations - 1); // EmitSingleChar already incremented +1
            }

            // Emits the code to handle a non-backtracking, variable-length loop (Oneloopgreedy or Setloopgreedy).
            void EmitGreedyLoop(RegexNode node)
            {
                Debug.Assert(node.Type == RegexNode.Oneloopgreedy || node.Type == RegexNode.Notoneloopgreedy || node.Type == RegexNode.Setloopgreedy);
                Debug.Assert(node.M < int.MaxValue);

                iterationLocal ??= DeclareInt32();

                // First generate the code to handle the required number of iterations.
                if (node.M > 0)
                {
                    EmitRepeater(node, node.M);
                }

                // Then generate the code to handle the 0 or more remaining optional iterations.
                if (node.N > node.M)
                {
                    Label originalDoneLabel = doneLabel;
                    doneLabel = DefineLabel();

                    if (node.Type == RegexNode.Notoneloopgreedy && node.N == int.MaxValue && !IsCaseInsensitive(node))
                    {
                        // For Notoneloopgreedy, we're looking for a specific character, as everything until we find
                        // it is consumed by the loop.  If we're unbounded, such as with ".*" and if we're case-sensitive,
                        // we can use the vectorized IndexOf to do the search, rather than open-coding it. (In the future,
                        // we could consider using IndexOf with StringComparison for case insensitivity.)

                        // int i = textSpan.Slice(textSpanPos).IndexOf(char);
                        Ldloca(textSpanLocal);
                        Ldc(textSpanPos);
                        Call(s_spanSliceMethod);
                        Ldc(node.Ch);
                        Call(s_spanIndexOf);
                        Stloc(iterationLocal);

                        // if (i != -1) goto doneLabel;
                        Ldloc(iterationLocal);
                        Ldc(-1);
                        BneFar(doneLabel);

                        // i = textSpan.Length - textSpanPos;
                        Ldloca(textSpanLocal);
                        Call(s_spanGetLengthMethod);
                        Ldc(textSpanPos);
                        Sub();
                        Stloc(iterationLocal);
                    }
                    else
                    {
                        // For everything else, do a normal loop.

                        Label conditionLabel = DefineLabel();
                        Label bodyLabel = DefineLabel();

                        int maxIterations = node.N == int.MaxValue ? int.MaxValue : node.N - node.M;

                        // int i = 0;
                        Ldc(0);
                        Stloc(iterationLocal);
                        BrFar(conditionLabel);

                        // Body:
                        // TimeoutCheck();
                        // if (!match) goto Done;
                        MarkLabel(bodyLabel);
                        EmitTimeoutCheck();

                        // if (textSpan[textSpanPos + i] != ch) goto Done;
                        Ldloca(textSpanLocal);
                        Ldc(textSpanPos);
                        Ldloc(iterationLocal);
                        Add();
                        Call(s_spanGetItemMethod);
                        LdindU2();
                        switch (node.Type)
                        {
                            case RegexNode.Oneloopgreedy:
                                if (IsCaseInsensitive(node)) CallToLower();
                                Ldc(node.Ch);
                                BneFar(doneLabel);
                                break;
                            case RegexNode.Notoneloopgreedy:
                                if (IsCaseInsensitive(node)) CallToLower();
                                Ldc(node.Ch);
                                BeqFar(doneLabel);
                                break;
                            case RegexNode.Setloopgreedy:
                                EmitCallCharInClass(node.Str!, IsCaseInsensitive(node), setScratchLocal);
                                BrfalseFar(doneLabel);
                                break;
                        }

                        // i++;
                        Ldloc(iterationLocal);
                        Ldc(1);
                        Add();
                        Stloc(iterationLocal);

                        // if ((uint)(textSpanPos + i) >= (uint)textSpan.Length || i >= maxIterations) goto Done;
                        MarkLabel(conditionLabel);
                        Ldc(textSpanPos);
                        Ldloc(iterationLocal);
                        Add();
                        Ldloca(textSpanLocal);
                        Call(s_spanGetLengthMethod);
                        BgeUnFar(doneLabel);
                        if (maxIterations != int.MaxValue)
                        {
                            Ldloc(iterationLocal);
                            Ldc(maxIterations);
                            BltFar(bodyLabel);
                        }
                        else
                        {
                            BrFar(bodyLabel);
                        }
                    }

                    // Done:
                    MarkLabel(doneLabel);
                    doneLabel = originalDoneLabel; // Restore the original done label

                    // textSpan = textSpan.Slice(i);
                    Ldloca(textSpanLocal);
                    Ldloc(iterationLocal);
                    Call(s_spanSliceMethod);
                    Stloc(textSpanLocal);

                    // runtextpos += i;
                    Ldloc(runtextposLocal);
                    Ldloc(iterationLocal);
                    Add();
                    Stloc(runtextposLocal);
                }
            }
        }

        /// <summary>Generates the code for "RegexRunner.Go".</summary>
        protected void GenerateGo()
        {
            // Generate backtrack-free code when we're dealing with simpler regexes.
            if (TryGenerateNonBacktrackingGo(_code!.Tree.Root))
            {
                return;
            }

            // We're dealing with a regex more complicated that the fast-path non-backtracking
            // implementation can handle.  Do the full-fledged thing.

            // declare some locals

            _runtextposLocal = DeclareInt32();
            _runtextLocal = DeclareString();
            _runtrackposLocal = DeclareInt32();
            _runtrackLocal = DeclareInt32Array();
            _runstackposLocal = DeclareInt32();
            _runstackLocal = DeclareInt32Array();
            _temp1Local = DeclareInt32();
            _temp2Local = DeclareInt32();
            _temp3Local = DeclareInt32();
            if (_hasTimeout)
            {
                _loopTimeoutCounterLocal = DeclareInt32();
            }
            _runtextbegLocal = DeclareInt32();
            _runtextendLocal = DeclareInt32();

            InitializeCultureForGoIfNecessary();

            // clear some tables

            _labels = null;
            _notes = null;
            _notecount = 0;

            // globally used labels

            _backtrack = DefineLabel();

            // emit the code!

            GenerateForwardSection();
            GenerateMiddleSection();
            GenerateBacktrackSection();
        }

        private void InitializeCultureForGoIfNecessary()
        {
            _cultureLocal = null;
            if ((_options & RegexOptions.CultureInvariant) == 0)
            {
                bool needsCulture = (_options & RegexOptions.IgnoreCase) != 0;
                if (!needsCulture)
                {
                    for (int codepos = 0; codepos < _codes!.Length; codepos += RegexCode.OpcodeSize(_codes[codepos]))
                    {
                        if ((_codes[codepos] & RegexCode.Ci) == RegexCode.Ci)
                        {
                            needsCulture = true;
                            break;
                        }
                    }
                }

                if (needsCulture)
                {
                    // cache CultureInfo in local variable which saves excessive thread local storage accesses
                    _cultureLocal = DeclareCultureInfo();
                    InitLocalCultureInfo();
                }
            }
        }

#if DEBUG
        /// <summary>Debug only: emit code to print out a message.</summary>
        private void Message(string str)
        {
            Ldstr(str);
            Call(s_debugWriteLine!);
        }
#endif

        /// <summary>
        /// The main translation function. It translates the logic for a single opcode at
        /// the current position. The structure of this function exactly mirrors
        /// the structure of the inner loop of RegexInterpreter.Go().
        /// </summary>
        /// <remarks>
        /// The C# code from RegexInterpreter.Go() that corresponds to each case is
        /// included as a comment.
        ///
        /// Note that since we're generating code, we can collapse many cases that are
        /// dealt with one-at-a-time in RegexIntepreter. We can also unroll loops that
        /// iterate over constant strings or sets.
        /// </remarks>
        private void GenerateOneCode()
        {
#if DEBUG
            if ((_options & RegexOptions.Debug) != 0)
            {
                Mvlocfld(_runtextposLocal!, s_runtextposField);
                Mvlocfld(_runtrackposLocal!, s_runtrackposField);
                Mvlocfld(_runstackposLocal!, s_runstackposField);
                Ldthis();
                Callvirt(s_dumpStateM);

                var sb = new StringBuilder();
                if (_backpos > 0)
                {
                    sb.AppendFormat("{0:D6} ", _backpos);
                }
                else
                {
                    sb.Append("       ");
                }
                sb.Append(_code!.OpcodeDescription(_codepos));

                if ((_regexopcode & RegexCode.Back) != 0)
                {
                    sb.Append(" Back");
                }

                if ((_regexopcode & RegexCode.Back2) != 0)
                {
                    sb.Append(" Back2");
                }

                Message(sb.ToString());
            }
#endif
            LocalBuilder charInClassLocal;

            // Before executing any RegEx code in the unrolled loop,
            // we try checking for the match timeout:

            if (_hasTimeout)
            {
                Ldthis();
                Callvirt(s_checkTimeoutMethod);
            }

            // Now generate the IL for the RegEx code saved in _regexopcode.
            // We unroll the loop done by the RegexCompiler creating as very long method
            // that is longer if the pattern is longer:

            switch (_regexopcode)
            {
                case RegexCode.Stop:
                    //: return;
                    Mvlocfld(_runtextposLocal!, s_runtextposField);       // update _textpos
                    Ret();
                    break;

                case RegexCode.Nothing:
                    //: break Backward;
                    Back();
                    break;

                case RegexCode.Goto:
                    //: Goto(Operand(0));
                    Goto(Operand(0));
                    break;

                case RegexCode.Testref:
                    //: if (!_match.IsMatched(Operand(0)))
                    //:     break Backward;
                    Ldthis();
                    Ldc(Operand(0));
                    Callvirt(s_isMatchedMethod);
                    BrfalseFar(_backtrack);
                    break;

                case RegexCode.Lazybranch:
                    //: Track(Textpos());
                    PushTrack(_runtextposLocal!);
                    Track();
                    break;

                case RegexCode.Lazybranch | RegexCode.Back:
                    //: Trackframe(1);
                    //: Textto(Tracked(0));
                    //: Goto(Operand(0));
                    PopTrack();
                    Stloc(_runtextposLocal!);
                    Goto(Operand(0));
                    break;

                case RegexCode.Nullmark:
                    //: Stack(-1);
                    //: Track();
                    ReadyPushStack();
                    Ldc(-1);
                    DoPush();
                    TrackUnique(Stackpop);
                    break;

                case RegexCode.Setmark:
                    //: Stack(Textpos());
                    //: Track();
                    PushStack(_runtextposLocal!);
                    TrackUnique(Stackpop);
                    break;

                case RegexCode.Nullmark | RegexCode.Back:
                case RegexCode.Setmark | RegexCode.Back:
                    //: Stackframe(1);
                    //: break Backward;
                    PopDiscardStack();
                    Back();
                    break;

                case RegexCode.Getmark:
                    //: Stackframe(1);
                    //: Track(Stacked(0));
                    //: Textto(Stacked(0));
                    ReadyPushTrack();
                    PopStack();
                    Dup();
                    Stloc(_runtextposLocal!);
                    DoPush();

                    Track();
                    break;

                case RegexCode.Getmark | RegexCode.Back:
                    //: Trackframe(1);
                    //: Stack(Tracked(0));
                    //: break Backward;
                    ReadyPushStack();
                    PopTrack();
                    DoPush();
                    Back();
                    break;

                case RegexCode.Capturemark:
                    //: if (!IsMatched(Operand(1)))
                    //:     break Backward;
                    //: Stackframe(1);
                    //: if (Operand(1) != -1)
                    //:     TransferCapture(Operand(0), Operand(1), Stacked(0), Textpos());
                    //: else
                    //:     Capture(Operand(0), Stacked(0), Textpos());
                    //: Track(Stacked(0));

                    //: Stackframe(1);
                    //: Capture(Operand(0), Stacked(0), Textpos());
                    //: Track(Stacked(0));

                    if (Operand(1) != -1)
                    {
                        Ldthis();
                        Ldc(Operand(1));
                        Callvirt(s_isMatchedMethod);
                        BrfalseFar(_backtrack);
                    }

                    PopStack();
                    Stloc(_temp1Local!);

                    if (Operand(1) != -1)
                    {
                        Ldthis();
                        Ldc(Operand(0));
                        Ldc(Operand(1));
                        Ldloc(_temp1Local!);
                        Ldloc(_runtextposLocal!);
                        Callvirt(s_transferCaptureMethod);
                    }
                    else
                    {
                        Ldthis();
                        Ldc(Operand(0));
                        Ldloc(_temp1Local!);
                        Ldloc(_runtextposLocal!);
                        Callvirt(s_captureMethod);
                    }

                    PushTrack(_temp1Local!);

                    TrackUnique(Operand(0) != -1 && Operand(1) != -1 ? Capback2 : Capback);
                    break;


                case RegexCode.Capturemark | RegexCode.Back:
                    //: Trackframe(1);
                    //: Stack(Tracked(0));
                    //: Uncapture();
                    //: if (Operand(0) != -1 && Operand(1) != -1)
                    //:     Uncapture();
                    //: break Backward;
                    ReadyPushStack();
                    PopTrack();
                    DoPush();
                    Ldthis();
                    Callvirt(s_uncaptureMethod);
                    if (Operand(0) != -1 && Operand(1) != -1)
                    {
                        Ldthis();
                        Callvirt(s_uncaptureMethod);
                    }
                    Back();
                    break;

                case RegexCode.Branchmark:
                    //: Stackframe(1);
                    //:
                    //: if (Textpos() != Stacked(0))
                    //: {                                   // Nonempty match -> loop now
                    //:     Track(Stacked(0), Textpos());   // Save old mark, textpos
                    //:     Stack(Textpos());               // Make new mark
                    //:     Goto(Operand(0));               // Loop
                    //: }
                    //: else
                    //: {                                   // Empty match -> straight now
                    //:     Track2(Stacked(0));             // Save old mark
                    //:     Advance(1);                     // Straight
                    //: }
                    //: continue Forward;
                    {
                        LocalBuilder mark = _temp1Local!;
                        Label l1 = DefineLabel();

                        PopStack();
                        Dup();
                        Stloc(mark!);                            // Stacked(0) -> temp
                        PushTrack(mark!);
                        Ldloc(_runtextposLocal!);
                        Beq(l1);                                // mark == textpos -> branch

                        // (matched != 0)

                        PushTrack(_runtextposLocal!);
                        PushStack(_runtextposLocal!);
                        Track();
                        Goto(Operand(0));                       // Goto(Operand(0))

                        // else

                        MarkLabel(l1);
                        TrackUnique2(Branchmarkback2);
                        break;
                    }

                case RegexCode.Branchmark | RegexCode.Back:
                    //: Trackframe(2);
                    //: Stackframe(1);
                    //: Textto(Tracked(1));                     // Recall position
                    //: Track2(Tracked(0));                     // Save old mark
                    //: Advance(1);
                    PopTrack();
                    Stloc(_runtextposLocal!);
                    PopStack();
                    Pop();
                    // track spot 0 is already in place
                    TrackUnique2(Branchmarkback2);
                    Advance();
                    break;

                case RegexCode.Branchmark | RegexCode.Back2:
                    //: Trackframe(1);
                    //: Stack(Tracked(0));                      // Recall old mark
                    //: break Backward;                         // Backtrack
                    ReadyPushStack();
                    PopTrack();
                    DoPush();
                    Back();
                    break;

                case RegexCode.Lazybranchmark:
                    //: StackPop();
                    //: int oldMarkPos = StackPeek();
                    //:
                    //: if (Textpos() != oldMarkPos) {         // Nonempty match -> next loop
                    //: {                                   // Nonempty match -> next loop
                    //:     if (oldMarkPos != -1)
                    //:         Track(Stacked(0), Textpos());   // Save old mark, textpos
                    //:     else
                    //:         TrackPush(Textpos(), Textpos());
                    //: }
                    //: else
                    //: {                                   // Empty match -> no loop
                    //:     Track2(Stacked(0));             // Save old mark
                    //: }
                    //: Advance(1);
                    //: continue Forward;
                    {
                        LocalBuilder mark = _temp1Local!;
                        Label l1 = DefineLabel();
                        Label l2 = DefineLabel();
                        Label l3 = DefineLabel();

                        PopStack();
                        Dup();
                        Stloc(mark!);                      // Stacked(0) -> temp

                        // if (oldMarkPos != -1)
                        Ldloc(mark);
                        Ldc(-1);
                        Beq(l2);                                // mark == -1 -> branch
                        PushTrack(mark);
                        Br(l3);
                        // else
                        MarkLabel(l2);
                        PushTrack(_runtextposLocal!);
                        MarkLabel(l3);

                        // if (Textpos() != mark)
                        Ldloc(_runtextposLocal!);
                        Beq(l1);                                // mark == textpos -> branch
                        PushTrack(_runtextposLocal!);
                        Track();
                        Br(AdvanceLabel());                 // Advance (near)
                                                            // else
                        MarkLabel(l1);
                        ReadyPushStack();                   // push the current textPos on the stack.
                                                            // May be ignored by 'back2' or used by a true empty match.
                        Ldloc(mark);

                        DoPush();
                        TrackUnique2(Lazybranchmarkback2);

                        break;
                    }

                case RegexCode.Lazybranchmark | RegexCode.Back:
                    //: Trackframe(2);
                    //: Track2(Tracked(0));                     // Save old mark
                    //: Stack(Textpos());                       // Make new mark
                    //: Textto(Tracked(1));                     // Recall position
                    //: Goto(Operand(0));                       // Loop

                    PopTrack();
                    Stloc(_runtextposLocal!);
                    PushStack(_runtextposLocal!);
                    TrackUnique2(Lazybranchmarkback2);
                    Goto(Operand(0));
                    break;

                case RegexCode.Lazybranchmark | RegexCode.Back2:
                    //: Stackframe(1);
                    //: Trackframe(1);
                    //: Stack(Tracked(0));                  // Recall old mark
                    //: break Backward;
                    ReadyReplaceStack(0);
                    PopTrack();
                    DoReplace();
                    Back();
                    break;

                case RegexCode.Nullcount:
                    //: Stack(-1, Operand(0));
                    //: Track();
                    ReadyPushStack();
                    Ldc(-1);
                    DoPush();
                    ReadyPushStack();
                    Ldc(Operand(0));
                    DoPush();
                    TrackUnique(Stackpop2);
                    break;

                case RegexCode.Setcount:
                    //: Stack(Textpos(), Operand(0));
                    //: Track();
                    PushStack(_runtextposLocal!);
                    ReadyPushStack();
                    Ldc(Operand(0));
                    DoPush();
                    TrackUnique(Stackpop2);
                    break;

                case RegexCode.Nullcount | RegexCode.Back:
                case RegexCode.Setcount | RegexCode.Back:
                    //: Stackframe(2);
                    //: break Backward;
                    PopDiscardStack(2);
                    Back();
                    break;

                case RegexCode.Branchcount:
                    //: Stackframe(2);
                    //: int mark = Stacked(0);
                    //: int count = Stacked(1);
                    //:
                    //: if (count >= Operand(1) || Textpos() == mark && count >= 0)
                    //: {                                   // Max loops or empty match -> straight now
                    //:     Track2(mark, count);            // Save old mark, count
                    //:     Advance(2);                     // Straight
                    //: }
                    //: else
                    //: {                                   // Nonempty match -> count+loop now
                    //:     Track(mark);                    // remember mark
                    //:     Stack(Textpos(), count + 1);    // Make new mark, incr count
                    //:     Goto(Operand(0));               // Loop
                    //: }
                    //: continue Forward;
                    {
                        LocalBuilder count = _temp1Local!;
                        LocalBuilder mark = _temp2Local!;
                        Label l1 = DefineLabel();
                        Label l2 = DefineLabel();

                        PopStack();
                        Stloc(count);                           // count -> temp
                        PopStack();
                        Dup();
                        Stloc(mark);                            // mark -> temp2
                        PushTrack(mark);

                        Ldloc(_runtextposLocal!);
                        Bne(l1);                                // mark != textpos -> l1
                        Ldloc(count);
                        Ldc(0);
                        Bge(l2);                                // count >= 0 && mark == textpos -> l2

                        MarkLabel(l1);
                        Ldloc(count);
                        Ldc(Operand(1));
                        Bge(l2);                                // count >= Operand(1) -> l2

                        // else
                        PushStack(_runtextposLocal!);
                        ReadyPushStack();
                        Ldloc(count);                           // mark already on track
                        Ldc(1);
                        Add();
                        DoPush();
                        Track();
                        Goto(Operand(0));

                        // if (count >= Operand(1) || Textpos() == mark)
                        MarkLabel(l2);
                        PushTrack(count);                       // mark already on track
                        TrackUnique2(Branchcountback2);
                        break;
                    }

                case RegexCode.Branchcount | RegexCode.Back:
                    //: Trackframe(1);
                    //: Stackframe(2);
                    //: if (Stacked(1) > 0)                     // Positive -> can go straight
                    //: {
                    //:     Textto(Stacked(0));                 // Zap to mark
                    //:     Track2(Tracked(0), Stacked(1) - 1); // Save old mark, old count
                    //:     Advance(2);                         // Straight
                    //:     continue Forward;
                    //: }
                    //: Stack(Tracked(0), Stacked(1) - 1);      // recall old mark, old count
                    //: break Backward;
                    {

                        LocalBuilder count = _temp1Local!;
                        Label l1 = DefineLabel();
                        PopStack();
                        Ldc(1);
                        Sub();
                        Dup();
                        Stloc(count!);
                        Ldc(0);
                        Blt(l1);

                        // if (count >= 0)
                        PopStack();
                        Stloc(_runtextposLocal!);
                        PushTrack(count);                       // Tracked(0) is alredy on the track
                        TrackUnique2(Branchcountback2);
                        Advance();

                        // else
                        MarkLabel(l1);
                        ReadyReplaceStack(0);
                        PopTrack();
                        DoReplace();
                        PushStack(count);
                        Back();
                        break;
                    }

                case RegexCode.Branchcount | RegexCode.Back2:
                    //: Trackframe(2);
                    //: Stack(Tracked(0), Tracked(1));      // Recall old mark, old count
                    //: break Backward;                     // Backtrack

                    PopTrack();
                    Stloc(_temp1Local!);
                    ReadyPushStack();
                    PopTrack();
                    DoPush();
                    PushStack(_temp1Local!);
                    Back();
                    break;

                case RegexCode.Lazybranchcount:
                    //: Stackframe(2);
                    //: int mark = Stacked(0);
                    //: int count = Stacked(1);
                    //:
                    //: if (count < 0)
                    //: {                                   // Negative count -> loop now
                    //:     Track2(mark);                   // Save old mark
                    //:     Stack(Textpos(), count + 1);    // Make new mark, incr count
                    //:     Goto(Operand(0));               // Loop
                    //: }
                    //: else
                    //: {                                   // Nonneg count or empty match -> straight now
                    //:     Track(mark, count, Textpos());  // Save mark, count, position
                    //: }
                    {
                        LocalBuilder count = _temp1Local!;
                        LocalBuilder mark = _temp2Local!;
                        Label l1 = DefineLabel();

                        PopStack();
                        Stloc(count);                           // count -> temp
                        PopStack();
                        Stloc(mark);                            // mark -> temp2

                        Ldloc(count);
                        Ldc(0);
                        Bge(l1);                                // count >= 0 -> l1

                        // if (count < 0)
                        PushTrack(mark);
                        PushStack(_runtextposLocal!);
                        ReadyPushStack();
                        Ldloc(count);
                        Ldc(1);
                        Add();
                        DoPush();
                        TrackUnique2(Lazybranchcountback2);
                        Goto(Operand(0));

                        // else
                        MarkLabel(l1);
                        PushTrack(mark);
                        PushTrack(count);
                        PushTrack(_runtextposLocal!);
                        Track();
                        break;
                    }

                case RegexCode.Lazybranchcount | RegexCode.Back:
                    //: Trackframe(3);
                    //: int mark = Tracked(0);
                    //: int textpos = Tracked(2);
                    //: if (Tracked(1) < Operand(1) && textpos != mark)
                    //: {                                       // Under limit and not empty match -> loop
                    //:     Textto(Tracked(2));                 // Recall position
                    //:     Stack(Textpos(), Tracked(1) + 1);   // Make new mark, incr count
                    //:     Track2(Tracked(0));                 // Save old mark
                    //:     Goto(Operand(0));                   // Loop
                    //:     continue Forward;
                    //: }
                    //: else
                    //: {
                    //:     Stack(Tracked(0), Tracked(1));      // Recall old mark, count
                    //:     break Backward;                     // backtrack
                    //: }
                    {
                        Label l1 = DefineLabel();
                        LocalBuilder cLocal = _temp1Local!;
                        PopTrack();
                        Stloc(_runtextposLocal!);
                        PopTrack();
                        Dup();
                        Stloc(cLocal);
                        Ldc(Operand(1));
                        Bge(l1);                                // Tracked(1) >= Operand(1) -> l1

                        Ldloc(_runtextposLocal!);
                        TopTrack();
                        Beq(l1);                                // textpos == mark -> l1

                        PushStack(_runtextposLocal!);
                        ReadyPushStack();
                        Ldloc(cLocal);
                        Ldc(1);
                        Add();
                        DoPush();
                        TrackUnique2(Lazybranchcountback2);
                        Goto(Operand(0));

                        MarkLabel(l1);
                        ReadyPushStack();
                        PopTrack();
                        DoPush();
                        PushStack(cLocal);
                        Back();
                        break;
                    }

                case RegexCode.Lazybranchcount | RegexCode.Back2:
                    // <
                    ReadyReplaceStack(1);
                    PopTrack();
                    DoReplace();
                    ReadyReplaceStack(0);
                    TopStack();
                    Ldc(1);
                    Sub();
                    DoReplace();
                    Back();
                    break;

                case RegexCode.Setjump:
                    //: Stack(Trackpos(), Crawlpos());
                    //: Track();
                    ReadyPushStack();
                    Ldthisfld(s_runtrackField);
                    Ldlen();
                    Ldloc(_runtrackposLocal!);
                    Sub();
                    DoPush();
                    ReadyPushStack();
                    Ldthis();
                    Callvirt(s_crawlposMethod);
                    DoPush();
                    TrackUnique(Stackpop2);
                    break;

                case RegexCode.Setjump | RegexCode.Back:
                    //: Stackframe(2);
                    PopDiscardStack(2);
                    Back();
                    break;

                case RegexCode.Backjump:
                    //: Stackframe(2);
                    //: Trackto(Stacked(0));
                    //: while (Crawlpos() != Stacked(1))
                    //:     Uncapture();
                    //: break Backward;
                    {
                        Label l1 = DefineLabel();
                        Label l2 = DefineLabel();

                        PopStack();
                        Ldthisfld(s_runtrackField);
                        Ldlen();
                        PopStack();
                        Sub();
                        Stloc(_runtrackposLocal!);
                        Dup();
                        Ldthis();
                        Callvirt(s_crawlposMethod);
                        Beq(l2);

                        MarkLabel(l1);
                        Ldthis();
                        Callvirt(s_uncaptureMethod);
                        Dup();
                        Ldthis();
                        Callvirt(s_crawlposMethod);
                        Bne(l1);

                        MarkLabel(l2);
                        Pop();
                        Back();
                        break;
                    }

                case RegexCode.Forejump:
                    //: Stackframe(2);
                    //: Trackto(Stacked(0));
                    //: Track(Stacked(1));
                    PopStack();
                    Stloc(_temp1Local!);
                    Ldthisfld(s_runtrackField);
                    Ldlen();
                    PopStack();
                    Sub();
                    Stloc(_runtrackposLocal!);
                    PushTrack(_temp1Local!);
                    TrackUnique(Forejumpback);
                    break;

                case RegexCode.Forejump | RegexCode.Back:
                    //: Trackframe(1);
                    //: while (Crawlpos() != Tracked(0))
                    //:     Uncapture();
                    //: break Backward;
                    {
                        Label l1 = DefineLabel();
                        Label l2 = DefineLabel();

                        PopTrack();

                        Dup();
                        Ldthis();
                        Callvirt(s_crawlposMethod);
                        Beq(l2);

                        MarkLabel(l1);
                        Ldthis();
                        Callvirt(s_uncaptureMethod);
                        Dup();
                        Ldthis();
                        Callvirt(s_crawlposMethod);
                        Bne(l1);

                        MarkLabel(l2);
                        Pop();
                        Back();
                        break;
                    }

                case RegexCode.Bol:
                    //: if (Leftchars() > 0 && CharAt(Textpos() - 1) != '\n')
                    //:     break Backward;
                    {
                        Label l1 = _labels![NextCodepos()];
                        Ldloc(_runtextposLocal!);
                        Ldloc(_runtextbegLocal!);
                        Ble(l1);
                        Leftchar();
                        Ldc('\n');
                        BneFar(_backtrack);
                        break;
                    }

                case RegexCode.Eol:
                    //: if (Rightchars() > 0 && CharAt(Textpos()) != '\n')
                    //:     break Backward;
                    {
                        Label l1 = _labels![NextCodepos()];
                        Ldloc(_runtextposLocal!);
                        Ldloc(_runtextendLocal!);
                        Bge(l1);
                        Rightchar();
                        Ldc('\n');
                        BneFar(_backtrack);
                        break;
                    }

                case RegexCode.Boundary:
                case RegexCode.Nonboundary:
                    //: if (!IsBoundary(Textpos(), _textbeg, _textend))
                    //:     break Backward;
                    Ldthis();
                    Ldloc(_runtextposLocal!);
                    Ldloc(_runtextbegLocal!);
                    Ldloc(_runtextendLocal!);
                    Callvirt(s_isBoundaryMethod);
                    if (Code() == RegexCode.Boundary)
                    {
                        BrfalseFar(_backtrack);
                    }
                    else
                    {
                        BrtrueFar(_backtrack);
                    }
                    break;

                case RegexCode.ECMABoundary:
                case RegexCode.NonECMABoundary:
                    //: if (!IsECMABoundary(Textpos(), _textbeg, _textend))
                    //:     break Backward;
                    Ldthis();
                    Ldloc(_runtextposLocal!);
                    Ldloc(_runtextbegLocal!);
                    Ldloc(_runtextendLocal!);
                    Callvirt(s_isECMABoundaryMethod);
                    if (Code() == RegexCode.ECMABoundary)
                    {
                        BrfalseFar(_backtrack);
                    }
                    else
                    {
                        BrtrueFar(_backtrack);
                    }
                    break;

                case RegexCode.Beginning:
                    //: if (Leftchars() > 0)
                    //:    break Backward;
                    Ldloc(_runtextposLocal!);
                    Ldloc(_runtextbegLocal!);
                    BgtFar(_backtrack);
                    break;

                case RegexCode.Start:
                    //: if (Textpos() != Textstart())
                    //:    break Backward;
                    Ldloc(_runtextposLocal!);
                    Ldthisfld(s_runtextstartField);
                    BneFar(_backtrack);
                    break;

                case RegexCode.EndZ:
                    //: if (Rightchars() > 1 || Rightchars() == 1 && CharAt(Textpos()) != '\n')
                    //:    break Backward;
                    Ldloc(_runtextposLocal!);
                    Ldloc(_runtextendLocal!);
                    Ldc(1);
                    Sub();
                    BltFar(_backtrack);
                    Ldloc(_runtextposLocal!);
                    Ldloc(_runtextendLocal!);
                    Bge(_labels![NextCodepos()]);
                    Rightchar();
                    Ldc('\n');
                    BneFar(_backtrack);
                    break;

                case RegexCode.End:
                    //: if (Rightchars() > 0)
                    //:    break Backward;
                    Ldloc(_runtextposLocal!);
                    Ldloc(_runtextendLocal!);
                    BltFar(_backtrack);
                    break;

                case RegexCode.One:
                case RegexCode.Notone:
                case RegexCode.Set:
                case RegexCode.One | RegexCode.Rtl:
                case RegexCode.Notone | RegexCode.Rtl:
                case RegexCode.Set | RegexCode.Rtl:
                case RegexCode.One | RegexCode.Ci:
                case RegexCode.Notone | RegexCode.Ci:
                case RegexCode.Set | RegexCode.Ci:
                case RegexCode.One | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Notone | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Set | RegexCode.Ci | RegexCode.Rtl:

                    //: if (Rightchars() < 1 || Rightcharnext() != (char)Operand(0))
                    //:    break Backward;

                    charInClassLocal = _temp1Local!;

                    Ldloc(_runtextposLocal!);

                    if (!IsRightToLeft())
                    {
                        Ldloc(_runtextendLocal!);
                        BgeFar(_backtrack);
                        Rightcharnext();
                    }
                    else
                    {
                        Ldloc(_runtextbegLocal!);
                        BleFar(_backtrack);
                        Leftcharnext();
                    }

                    if (Code() == RegexCode.Set)
                    {
                        EmitCallCharInClass(_strings![Operand(0)], IsCaseInsensitive(), charInClassLocal);
                        BrfalseFar(_backtrack);
                    }
                    else
                    {
                        if (IsCaseInsensitive())
                        {
                            CallToLower();
                        }

                        Ldc(Operand(0));
                        if (Code() == RegexCode.One)
                        {
                            BneFar(_backtrack);
                        }
                        else
                        {
                            BeqFar(_backtrack);
                        }
                    }
                    break;

                case RegexCode.Multi:
                case RegexCode.Multi | RegexCode.Ci:
                    //: String Str = _strings[Operand(0)];
                    //: int i, c;
                    //: if (Rightchars() < (c = Str.Length))
                    //:     break Backward;
                    //: for (i = 0; c > 0; i++, c--)
                    //:     if (Str[i] != Rightcharnext())
                    //:         break Backward;
                    {
                        string str = _strings![Operand(0)];

                        Ldc(str.Length);
                        Ldloc(_runtextendLocal!);
                        Ldloc(_runtextposLocal!);
                        Sub();
                        BgtFar(_backtrack);

                        // unroll the string
                        for (int i = 0; i < str.Length; i++)
                        {
                            Ldloc(_runtextLocal!);
                            Ldloc(_runtextposLocal!);
                            if (i != 0)
                            {
                                Ldc(i);
                                Add();
                            }
                            Callvirt(s_stringGetCharsMethod);
                            if (IsCaseInsensitive())
                            {
                                CallToLower();
                            }

                            Ldc(str[i]);
                            BneFar(_backtrack);
                        }

                        Ldloc(_runtextposLocal!);
                        Ldc(str.Length);
                        Add();
                        Stloc(_runtextposLocal!);
                        break;
                    }

                case RegexCode.Multi | RegexCode.Rtl:
                case RegexCode.Multi | RegexCode.Ci | RegexCode.Rtl:
                    //: String Str = _strings[Operand(0)];
                    //: int c;
                    //: if (Leftchars() < (c = Str.Length))
                    //:     break Backward;
                    //: while (c > 0)
                    //:     if (Str[--c] != Leftcharnext())
                    //:         break Backward;
                    {
                        string str = _strings![Operand(0)];

                        Ldc(str.Length);
                        Ldloc(_runtextposLocal!);
                        Ldloc(_runtextbegLocal!);
                        Sub();
                        BgtFar(_backtrack);

                        // unroll the string
                        for (int i = str.Length; i > 0;)
                        {
                            i--;
                            Ldloc(_runtextLocal!);
                            Ldloc(_runtextposLocal!);
                            Ldc(str.Length - i);
                            Sub();
                            Callvirt(s_stringGetCharsMethod);
                            if (IsCaseInsensitive())
                            {
                                CallToLower();
                            }
                            Ldc(str[i]);
                            BneFar(_backtrack);
                        }

                        Ldloc(_runtextposLocal!);
                        Ldc(str.Length);
                        Sub();
                        Stloc(_runtextposLocal!);

                        break;
                    }

                case RegexCode.Ref:
                case RegexCode.Ref | RegexCode.Rtl:
                case RegexCode.Ref | RegexCode.Ci:
                case RegexCode.Ref | RegexCode.Ci | RegexCode.Rtl:
                    //: int capnum = Operand(0);
                    //: int j, c;
                    //: if (!_match.IsMatched(capnum)) {
                    //:     if (!RegexOptions.ECMAScript)
                    //:         break Backward;
                    //: } else {
                    //:     if (Rightchars() < (c = _match.MatchLength(capnum)))
                    //:         break Backward;
                    //:     for (j = _match.MatchIndex(capnum); c > 0; j++, c--)
                    //:         if (CharAt(j) != Rightcharnext())
                    //:             break Backward;
                    //: }
                    {
                        LocalBuilder lenLocal = _temp1Local!;
                        LocalBuilder indexLocal = _temp2Local!;
                        Label l1 = DefineLabel();

                        Ldthis();
                        Ldc(Operand(0));
                        Callvirt(s_isMatchedMethod);
                        if ((_options & RegexOptions.ECMAScript) != 0)
                        {
                            Brfalse(AdvanceLabel());
                        }
                        else
                        {
                            BrfalseFar(_backtrack); // !IsMatched() -> back
                        }

                        Ldthis();
                        Ldc(Operand(0));
                        Callvirt(s_matchLengthMethod);
                        Dup();
                        Stloc(lenLocal);
                        if (!IsRightToLeft())
                        {
                            Ldloc(_runtextendLocal!);
                            Ldloc(_runtextposLocal!);
                        }
                        else
                        {
                            Ldloc(_runtextposLocal!);
                            Ldloc(_runtextbegLocal!);
                        }
                        Sub();
                        BgtFar(_backtrack);         // Matchlength() > Rightchars() -> back

                        Ldthis();
                        Ldc(Operand(0));
                        Callvirt(s_matchIndexMethod);
                        if (!IsRightToLeft())
                        {
                            Ldloc(lenLocal);
                            Add(IsRightToLeft());
                        }
                        Stloc(indexLocal);              // index += len

                        Ldloc(_runtextposLocal!);
                        Ldloc(lenLocal);
                        Add(IsRightToLeft());
                        Stloc(_runtextposLocal!);           // texpos += len

                        MarkLabel(l1);
                        Ldloc(lenLocal);
                        Ldc(0);
                        Ble(AdvanceLabel());
                        Ldloc(_runtextLocal!);
                        Ldloc(indexLocal);
                        Ldloc(lenLocal);
                        if (IsRightToLeft())
                        {
                            Ldc(1);
                            Sub();
                            Dup();
                            Stloc(lenLocal);
                        }
                        Sub(IsRightToLeft());
                        Callvirt(s_stringGetCharsMethod);
                        if (IsCaseInsensitive())
                        {
                            CallToLower();
                        }

                        Ldloc(_runtextLocal!);
                        Ldloc(_runtextposLocal!);
                        Ldloc(lenLocal);
                        if (!IsRightToLeft())
                        {
                            Dup();
                            Ldc(1);
                            Sub();
                            Stloc(lenLocal);
                        }
                        Sub(IsRightToLeft());
                        Callvirt(s_stringGetCharsMethod);
                        if (IsCaseInsensitive())
                        {
                            CallToLower();
                        }

                        Beq(l1);
                        Back();
                        break;
                    }

                case RegexCode.Onerep:
                case RegexCode.Notonerep:
                case RegexCode.Setrep:
                case RegexCode.Onerep | RegexCode.Rtl:
                case RegexCode.Notonerep | RegexCode.Rtl:
                case RegexCode.Setrep | RegexCode.Rtl:
                case RegexCode.Onerep | RegexCode.Ci:
                case RegexCode.Notonerep | RegexCode.Ci:
                case RegexCode.Setrep | RegexCode.Ci:
                case RegexCode.Onerep | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Notonerep | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Setrep | RegexCode.Ci | RegexCode.Rtl:
                    //: int c = Operand(1);
                    //: if (Rightchars() < c)
                    //:     break Backward;
                    //: char ch = (char)Operand(0);
                    //: while (c-- > 0)
                    //:     if (Rightcharnext() != ch)
                    //:         break Backward;
                    {
                        LocalBuilder lenLocal = _temp1Local!;
                        charInClassLocal = _temp2Local!;
                        Label l1 = DefineLabel();

                        int c = Operand(1);

                        if (c == 0)
                            break;

                        Ldc(c);
                        if (!IsRightToLeft())
                        {
                            Ldloc(_runtextendLocal!);
                            Ldloc(_runtextposLocal!);
                        }
                        else
                        {
                            Ldloc(_runtextposLocal!);
                            Ldloc(_runtextbegLocal!);
                        }
                        Sub();
                        BgtFar(_backtrack);         // Matchlength() > Rightchars() -> back

                        Ldloc(_runtextposLocal!);
                        Ldc(c);
                        Add(IsRightToLeft());
                        Stloc(_runtextposLocal!);           // texpos += len

                        Ldc(c);
                        Stloc(lenLocal);

                        MarkLabel(l1);
                        Ldloc(_runtextLocal!);
                        Ldloc(_runtextposLocal!);
                        Ldloc(lenLocal);
                        if (IsRightToLeft())
                        {
                            Ldc(1);
                            Sub();
                            Dup();
                            Stloc(lenLocal);
                            Add();
                        }
                        else
                        {
                            Dup();
                            Ldc(1);
                            Sub();
                            Stloc(lenLocal);
                            Sub();
                        }
                        Callvirt(s_stringGetCharsMethod);

                        if (Code() == RegexCode.Setrep)
                        {
                            EmitTimeoutCheck();
                            EmitCallCharInClass(_strings![Operand(0)], IsCaseInsensitive(), charInClassLocal);
                            BrfalseFar(_backtrack);
                        }
                        else
                        {
                            if (IsCaseInsensitive())
                            {
                                CallToLower();
                            }

                            Ldc(Operand(0));
                            if (Code() == RegexCode.Onerep)
                            {
                                BneFar(_backtrack);
                            }
                            else
                            {
                                BeqFar(_backtrack);
                            }
                        }
                        Ldloc(lenLocal);
                        Ldc(0);
                        if (Code() == RegexCode.Setrep)
                        {
                            BgtFar(l1);
                        }
                        else
                        {
                            Bgt(l1);
                        }
                        break;
                    }

                case RegexCode.Oneloop:
                case RegexCode.Notoneloop:
                case RegexCode.Setloop:
                case RegexCode.Oneloop | RegexCode.Rtl:
                case RegexCode.Notoneloop | RegexCode.Rtl:
                case RegexCode.Setloop | RegexCode.Rtl:
                case RegexCode.Oneloop | RegexCode.Ci:
                case RegexCode.Notoneloop | RegexCode.Ci:
                case RegexCode.Setloop | RegexCode.Ci:
                case RegexCode.Oneloop | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Notoneloop | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Setloop | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Oneloopgreedy:
                case RegexCode.Notoneloopgreedy:
                case RegexCode.Setloopgreedy:
                case RegexCode.Oneloopgreedy | RegexCode.Rtl:
                case RegexCode.Notoneloopgreedy | RegexCode.Rtl:
                case RegexCode.Setloopgreedy | RegexCode.Rtl:
                case RegexCode.Oneloopgreedy | RegexCode.Ci:
                case RegexCode.Notoneloopgreedy | RegexCode.Ci:
                case RegexCode.Setloopgreedy | RegexCode.Ci:
                case RegexCode.Oneloopgreedy | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Notoneloopgreedy | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Setloopgreedy | RegexCode.Ci | RegexCode.Rtl:
                    //: int c = Operand(1);
                    //: if (c > Rightchars())
                    //:     c = Rightchars();
                    //: char ch = (char)Operand(0);
                    //: int i;
                    //: for (i = c; i > 0; i--)
                    //: {
                    //:     if (Rightcharnext() != ch)
                    //:     {
                    //:         Leftnext();
                    //:         break;
                    //:     }
                    //: }
                    //: if (c > i)
                    //:     Track(c - i - 1, Textpos() - 1);
                    {
                        LocalBuilder cLocal = _temp1Local!;
                        LocalBuilder lenLocal = _temp2Local!;
                        charInClassLocal = _temp3Local!;
                        Label l1 = DefineLabel();
                        Label l2 = DefineLabel();

                        int c = Operand(1);
                        if (c == 0)
                        {
                            break;
                        }

                        if (!IsRightToLeft())
                        {
                            Ldloc(_runtextendLocal!);
                            Ldloc(_runtextposLocal!);
                        }
                        else
                        {
                            Ldloc(_runtextposLocal!);
                            Ldloc(_runtextbegLocal!);
                        }
                        Sub();
                        if (c != int.MaxValue)
                        {
                            Label l4 = DefineLabel();
                            Dup();
                            Ldc(c);
                            Blt(l4);
                            Pop();
                            Ldc(c);
                            MarkLabel(l4);
                        }
                        Dup();
                        Stloc(lenLocal);
                        Ldc(1);
                        Add();
                        Stloc(cLocal);

                        MarkLabel(l1);
                        Ldloc(cLocal);
                        Ldc(1);
                        Sub();
                        Dup();
                        Stloc(cLocal);
                        Ldc(0);
                        if (Code() == RegexCode.Setloop || Code() == RegexCode.Setloopgreedy)
                        {
                            BleFar(l2);
                        }
                        else
                        {
                            Ble(l2);
                        }

                        if (IsRightToLeft())
                        {
                            Leftcharnext();
                        }
                        else
                        {
                            Rightcharnext();
                        }

                        if (Code() == RegexCode.Setloop || Code() == RegexCode.Setloopgreedy)
                        {
                            EmitTimeoutCheck();
                            EmitCallCharInClass(_strings![Operand(0)], IsCaseInsensitive(), charInClassLocal);
                            BrtrueFar(l1);
                        }
                        else
                        {
                            if (IsCaseInsensitive())
                            {
                                CallToLower();
                            }

                            Ldc(Operand(0));
                            if (Code() == RegexCode.Oneloop || Code() == RegexCode.Oneloopgreedy)
                            {
                                Beq(l1);
                            }
                            else
                            {
                                Debug.Assert(Code() == RegexCode.Notoneloop || Code() == RegexCode.Notoneloopgreedy);
                                Bne(l1);
                            }
                        }

                        Ldloc(_runtextposLocal!);
                        Ldc(1);
                        Sub(IsRightToLeft());
                        Stloc(_runtextposLocal!);

                        MarkLabel(l2);

                        if (Code() != RegexCode.Oneloopgreedy && Code() != RegexCode.Notoneloopgreedy && Code() != RegexCode.Setloopgreedy)
                        {
                            Ldloc(lenLocal);
                            Ldloc(cLocal);
                            Ble(AdvanceLabel());

                            ReadyPushTrack();
                            Ldloc(lenLocal);
                            Ldloc(cLocal);
                            Sub();
                            Ldc(1);
                            Sub();
                            DoPush();

                            ReadyPushTrack();
                            Ldloc(_runtextposLocal!);
                            Ldc(1);
                            Sub(IsRightToLeft());
                            DoPush();

                            Track();
                        }
                        break;
                    }

                case RegexCode.Oneloop | RegexCode.Back:
                case RegexCode.Notoneloop | RegexCode.Back:
                case RegexCode.Setloop | RegexCode.Back:
                case RegexCode.Oneloop | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Notoneloop | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Setloop | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Oneloop | RegexCode.Ci | RegexCode.Back:
                case RegexCode.Notoneloop | RegexCode.Ci | RegexCode.Back:
                case RegexCode.Setloop | RegexCode.Ci | RegexCode.Back:
                case RegexCode.Oneloop | RegexCode.Ci | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Notoneloop | RegexCode.Ci | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Setloop | RegexCode.Ci | RegexCode.Rtl | RegexCode.Back:
                    //: Trackframe(2);
                    //: int i   = Tracked(0);
                    //: int pos = Tracked(1);
                    //: Textto(pos);
                    //: if (i > 0)
                    //:     Track(i - 1, pos - 1);
                    //: Advance(2);
                    PopTrack();
                    Stloc(_runtextposLocal!);
                    PopTrack();
                    Stloc(_temp1Local!);
                    Ldloc(_temp1Local!);
                    Ldc(0);
                    BleFar(AdvanceLabel());
                    ReadyPushTrack();
                    Ldloc(_temp1Local!);
                    Ldc(1);
                    Sub();
                    DoPush();
                    ReadyPushTrack();
                    Ldloc(_runtextposLocal!);
                    Ldc(1);
                    Sub(IsRightToLeft());
                    DoPush();
                    Trackagain();
                    Advance();
                    break;

                case RegexCode.Onelazy:
                case RegexCode.Notonelazy:
                case RegexCode.Setlazy:
                case RegexCode.Onelazy | RegexCode.Rtl:
                case RegexCode.Notonelazy | RegexCode.Rtl:
                case RegexCode.Setlazy | RegexCode.Rtl:
                case RegexCode.Onelazy | RegexCode.Ci:
                case RegexCode.Notonelazy | RegexCode.Ci:
                case RegexCode.Setlazy | RegexCode.Ci:
                case RegexCode.Onelazy | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Notonelazy | RegexCode.Ci | RegexCode.Rtl:
                case RegexCode.Setlazy | RegexCode.Ci | RegexCode.Rtl:
                    //: int c = Operand(1);
                    //: if (c > Rightchars())
                    //:     c = Rightchars();
                    //: if (c > 0)
                    //:     Track(c - 1, Textpos());
                    {
                        LocalBuilder cLocal = _temp1Local!;

                        int c = Operand(1);
                        if (c == 0)
                        {
                            break;
                        }

                        if (!IsRightToLeft())
                        {
                            Ldloc(_runtextendLocal!);
                            Ldloc(_runtextposLocal!);
                        }
                        else
                        {
                            Ldloc(_runtextposLocal!);
                            Ldloc(_runtextbegLocal!);
                        }
                        Sub();
                        if (c != int.MaxValue)
                        {
                            Label l4 = DefineLabel();
                            Dup();
                            Ldc(c);
                            Blt(l4);
                            Pop();
                            Ldc(c);
                            MarkLabel(l4);
                        }
                        Dup();
                        Stloc(cLocal);
                        Ldc(0);
                        Ble(AdvanceLabel());
                        ReadyPushTrack();
                        Ldloc(cLocal);
                        Ldc(1);
                        Sub();
                        DoPush();
                        PushTrack(_runtextposLocal!);
                        Track();
                        break;
                    }

                case RegexCode.Onelazy | RegexCode.Back:
                case RegexCode.Notonelazy | RegexCode.Back:
                case RegexCode.Setlazy | RegexCode.Back:
                case RegexCode.Onelazy | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Notonelazy | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Setlazy | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Onelazy | RegexCode.Ci | RegexCode.Back:
                case RegexCode.Notonelazy | RegexCode.Ci | RegexCode.Back:
                case RegexCode.Setlazy | RegexCode.Ci | RegexCode.Back:
                case RegexCode.Onelazy | RegexCode.Ci | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Notonelazy | RegexCode.Ci | RegexCode.Rtl | RegexCode.Back:
                case RegexCode.Setlazy | RegexCode.Ci | RegexCode.Rtl | RegexCode.Back:
                    //: Trackframe(2);
                    //: int pos = Tracked(1);
                    //: Textto(pos);
                    //: if (Rightcharnext() != (char)Operand(0))
                    //:     break Backward;
                    //: int i = Tracked(0);
                    //: if (i > 0)
                    //:     Track(i - 1, pos + 1);

                    charInClassLocal = _temp1Local!;

                    PopTrack();
                    Stloc(_runtextposLocal!);
                    PopTrack();
                    Stloc(_temp2Local!);

                    if (!IsRightToLeft())
                    {
                        Rightcharnext();
                    }
                    else
                    {
                        Leftcharnext();
                    }

                    if (Code() == RegexCode.Setlazy)
                    {
                        EmitCallCharInClass(_strings![Operand(0)], IsCaseInsensitive(), charInClassLocal);
                        BrfalseFar(_backtrack);
                    }
                    else
                    {
                        if (IsCaseInsensitive())
                        {
                            CallToLower();
                        }

                        Ldc(Operand(0));
                        if (Code() == RegexCode.Onelazy)
                        {
                            BneFar(_backtrack);
                        }
                        else
                        {
                            BeqFar(_backtrack);
                        }
                    }

                    Ldloc(_temp2Local!);
                    Ldc(0);
                    BleFar(AdvanceLabel());
                    ReadyPushTrack();
                    Ldloc(_temp2Local!);
                    Ldc(1);
                    Sub();
                    DoPush();
                    PushTrack(_runtextposLocal!);
                    Trackagain();
                    Advance();
                    break;

                default:
                    throw new NotImplementedException(SR.UnimplementedState);
            }
        }

        /// <summary>Emits a call to RegexRunner.CharInClass or a functional equivalent.</summary>
        private void EmitCallCharInClass(string charClass, bool caseInsensitive, LocalBuilder tempLocal)
        {
            // We need to perform the equivalent of calling RegexRunner.CharInClass(ch, charClass),
            // but that call is relatively expensive.  Before we fall back to it, we try to optimize
            // some common cases for which we can do much better, such as known character classes
            // for which we can call a dedicated method, or a fast-path for ASCII using a lookup table.

            // First, see if the char class is a built-in one for which there's a better function
            // we can just call directly.  Everything in this section must work correctly for both case
            // sensitive and case insensitive modes, regardless of current culture or invariant.
            switch (charClass)
            {
                case RegexCharClass.AnyClass:
                    // true
                    Pop();
                    Ldc(1);
                    return;

                case RegexCharClass.DigitClass:
                    // char.IsDigit(ch)
                    Call(s_charIsDigitMethod);
                    return;

                case RegexCharClass.NotDigitClass:
                    // !char.IsDigit(ch)
                    Call(s_charIsDigitMethod);
                    Ldc(0);
                    Ceq();
                    return;

                case RegexCharClass.SpaceClass:
                    // char.IsWhiteSpace(ch)
                    Call(s_charIsWhiteSpaceMethod);
                    return;

                case RegexCharClass.NotSpaceClass:
                    // !char.IsWhiteSpace(ch)
                    Call(s_charIsWhiteSpaceMethod);
                    Ldc(0);
                    Ceq();
                    return;
            }

            // If we're meant to be doing a case-insensitive lookup, and if we're not using the invariant culture,
            // lowercase the input.  If we're using the invariant culture, we may still end up calling ToLower later
            // on, but we may also be able to avoid it, in particular in the case of our lookup table, where we can
            // generate the lookup table already factoring in the invariant case sensitivity.
            bool invariant = false;
            if (caseInsensitive)
            {
                invariant = UseToLowerInvariant;
                if (!invariant)
                {
                    CallToLower();
                }
            }

            // Next, handle simple sets of one range, e.g. [A-Z], [0-9], etc.  This includes some built-in classes, like ECMADigitClass.
            if (!invariant && // if we're being asked to do a case insensitive comparison with the invariant culture, just use the lookup table
                charClass.Length == RegexCharClass.SetStartIndex + 2 && // one set of two values
                charClass[RegexCharClass.SetLengthIndex] == 2 && // validate we have the right number of ranges
                charClass[RegexCharClass.CategoryLengthIndex] == 0 && // must not have any categories
                charClass[RegexCharClass.SetStartIndex] < charClass[RegexCharClass.SetStartIndex + 1]) // valid range
            {
                if (RegexCharClass.IsSingleton(charClass) || RegexCharClass.IsSingletonInverse(charClass))
                {
                    // ch == charClass[3]
                    Ldc(charClass[3]);
                    Ceq();
                }
                else
                {
                    // (uint)ch - charClass[3] < charClass[4] - charClass[3]
                    Ldc(charClass[RegexCharClass.SetStartIndex]);
                    Sub();
                    Ldc(charClass[RegexCharClass.SetStartIndex + 1] - charClass[RegexCharClass.SetStartIndex]);
                    CltUn();
                }

                // Negate the answer if the negation flag was set
                if (RegexCharClass.IsNegated(charClass))
                {
                    Ldc(0);
                    Ceq();
                }

                return;
            }

            // Next, special-case ASCII inputs.  If the character class contains only ASCII inputs, then we
            // can satisfy the entire operation via a small lookup table, e.g.
            //     ch < 128 && lookup(ch)
            // If the character class contains values outside of the ASCII range, we can still optimize for
            // ASCII inputs, using the table for values < 128, and falling back to calling CharInClass
            // for anything outside of the ASCII range, e.g.
            //     if (ch < 128) lookup(ch)
            //     else ...
            // Either way, we need to generate the lookup table for the ASCII range.
            // We use a const string instead of a byte[] / static data property because
            // it lets IL emit handle all the gory details for us.  It also is ok from an
            // endianness perspective because the compilation happens on the same machine
            // that runs the compiled code.  If that were to ever change, this would need
            // to be revisited. String length is 8 chars == 16 bytes == 128 bits.
            string bitVectorString = string.Create(8, (charClass, invariant), (dest, state) =>
            {
                for (int i = 0; i < 128; i++)
                {
                    char c = (char)i;
                    if (RegexCharClass.CharInClass(c, state.charClass) ||
                        (state.invariant && char.IsUpper(c) && RegexCharClass.CharInClass(char.ToLowerInvariant(c), state.charClass)))
                    {
                        dest[i >> 4] |= (char)(1 << (i & 0xF));
                    }
                }
            });

            // In order to determine whether we need the non-ASCII fallback, we have a few options:
            // 1. Interpret the char class.  This would require fully understanding all of the ins and outs of the design,
            //    and is a lot of code (in the future it's possible the parser could pass this information along).
            // 2. Employ a heuristic to approximate (1), allowing for false positives (saying we need the fallback when
            //    we don't) but no false negatives (saying we don't need the fallback when we do).
            // 3. Evaluate CharInClass on all ~65K inputs.  This is relatively expensive, impacting startup costs.
            // We currently go with (2).  We may sometimes generate a fallback when we don't need one, but the cost of
            // doing so once in a while is minimal.
            bool asciiOnly = RegexCharClass.CanEasilyEnumerateSetContents(charClass);
            if (asciiOnly)
            {
                for (int i = RegexCharClass.SetStartIndex; i < charClass.Length; i++)
                {
                    if (charClass[i] >= 128) // validate all characters in the set are ASCII
                    {
                        asciiOnly = false;
                        break;
                    }
                }
            }

            Label nonAsciiLabel = DefineLabel(); // jumped to when input is >= 128
            Label doneLabel = DefineLabel(); // jumped to when answer has been computed

            // Store the input character so we can read it multiple times.
            Stloc(tempLocal);

            // ch < 128
            Ldloc(tempLocal);
            Ldc(128);
            Bge(nonAsciiLabel);

            // (bitVectorString[ch >> 4] & (1 << (ch & 0xF))) != 0
            Ldstr(bitVectorString);
            Ldloc(tempLocal);
            Ldc(4);
            Shr();
            Call(s_stringGetCharsMethod);
            Ldc(1);
            Ldloc(tempLocal);
            Ldc(15);
            And();
            Ldc(31);
            And();
            Shl();
            And();
            Ldc(0);
            CgtUn();
            Br(doneLabel);

            MarkLabel(nonAsciiLabel);
            if (asciiOnly)
            {
                // The whole class was ASCII, so if the character is >= 128, it's not in the class:
                // false
                Ldc(0);
            }
            else
            {
                // The whole class wasn't ASCII, so if the character is >= 128, we need to fall back to calling:
                // CharInClass(ch, charClass).  If case insensitivity is required, we will have already called
                // ToLower, but only if !invariant, so we need to do so here for invariant as well.
                Ldloc(tempLocal);
                if (invariant)
                {
                    CallToLower();
                }
                Ldstr(charClass);
                Call(s_charInClassMethod);
            }

            MarkLabel(doneLabel);
        }

        /// <summary>Emits a timeout check.</summary>
        private void EmitTimeoutCheck()
        {
            if (!_hasTimeout)
            {
                return;
            }

            Debug.Assert(_loopTimeoutCounterLocal != null);

            // Increment counter for each loop iteration.
            Ldloc(_loopTimeoutCounterLocal);
            Ldc(1);
            Add();
            Stloc(_loopTimeoutCounterLocal);

            // Emit code to check the timeout every 2048th iteration.
            Label label = DefineLabel();
            Ldloc(_loopTimeoutCounterLocal);
            Ldc(LoopTimeoutCheckCount);
            RemUn();
            Brtrue(label);
            Ldthis();
            Callvirt(s_checkTimeoutMethod);
            MarkLabel(label);
        }
    }
}
