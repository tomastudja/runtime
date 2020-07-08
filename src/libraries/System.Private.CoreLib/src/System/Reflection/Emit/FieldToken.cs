// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Reflection.Emit
{
    /// <summary>
    /// The FieldToken class is an opaque representation of the Token returned
    /// by the Metadata to represent the field.  FieldTokens are generated by
    /// Module.GetFieldToken().  There are no meaningful accessors on this class,
    /// but it can be passed to ILGenerator which understands it's internals.
    /// </summary>
    public struct FieldToken
    {
        public static readonly FieldToken Empty;

        private readonly object _class;

        internal FieldToken(int fieldToken, Type fieldClass)
        {
            Token = fieldToken;
            _class = fieldClass;
        }

        public int Token { get; }

        public override int GetHashCode() => Token;

        public override bool Equals(object? obj) => obj is FieldToken ft && Equals(ft);

        public bool Equals(FieldToken obj) => obj.Token == Token && obj._class == _class;

        public static bool operator ==(FieldToken a, FieldToken b) => a.Equals(b);

        public static bool operator !=(FieldToken a, FieldToken b) => !(a == b);
    }
}
