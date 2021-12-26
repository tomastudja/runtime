// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;

using Internal.TypeSystem;

using Debug = System.Diagnostics.Debug;

namespace Internal.Runtime
{
    /// <summary>
    /// Helper class to create exceptions. This is expected to be used from two places:
    /// from the throw helpers generated by the compiler after encountering a method that fails to
    /// compile, and from the runtime type loader (living in a universe with a separate
    /// System.Object) to create classlib exceptions.
    /// </summary>
    internal static class TypeLoaderExceptionHelper
    {
        public static Exception CreateBadImageFormatException(ExceptionStringID id)
        {
            return new BadImageFormatException(GetFormatString(id));
        }

        public static Exception CreateTypeLoadException(ExceptionStringID id, string typeName, string moduleName)
        {
            return new TypeLoadException(SR.Format(GetFormatString(id), typeName, moduleName), typeName);
        }

        public static Exception CreateTypeLoadException(ExceptionStringID id, string typeName, string moduleName, string messageArg)
        {
            return new TypeLoadException(SR.Format(GetFormatString(id), typeName, moduleName, messageArg), typeName);
        }

        public static Exception CreateMissingFieldException(ExceptionStringID id, string fieldName)
        {
            return new MissingFieldException(SR.Format(GetFormatString(id), fieldName));
        }

        public static Exception CreateMissingMethodException(ExceptionStringID id, string methodName)
        {
            throw new MissingMethodException(SR.Format(GetFormatString(id), methodName));
        }

        public static Exception CreateFileNotFoundException(ExceptionStringID id, string fileName)
        {
            throw new System.IO.FileNotFoundException(SR.Format(GetFormatString(id), fileName), fileName);
        }

        public static Exception CreateInvalidProgramException(ExceptionStringID id)
        {
            throw new InvalidProgramException(GetFormatString(id));
        }

        public static Exception CreateInvalidProgramException(ExceptionStringID id, string methodName)
        {
            throw new InvalidProgramException(SR.Format(GetFormatString(id), methodName));
        }

        public static Exception CreateMarshalDirectiveException(ExceptionStringID id)
        {
            throw new MarshalDirectiveException(GetFormatString(id));
        }

        // TODO: move to a place where we can share this with the compiler
        private static string GetFormatString(ExceptionStringID id)
        {
            switch (id)
            {
                case ExceptionStringID.ClassLoadGeneral:
                    return SR.ClassLoad_General;
                case ExceptionStringID.ClassLoadExplicitGeneric:
                    return SR.ClassLoad_ExplicitGeneric;
                case ExceptionStringID.ClassLoadBadFormat:
                    return SR.ClassLoad_BadFormat;
                case ExceptionStringID.ClassLoadValueClassTooLarge:
                    return SR.ClassLoad_ValueClassTooLarge;
                case ExceptionStringID.ClassLoadExplicitLayout:
                    return SR.ClassLoad_ExplicitLayout;
                case ExceptionStringID.ClassLoadRankTooLarge:
                    return SR.ClassLoad_RankTooLarge;
                case ExceptionStringID.InvalidProgramDefault:
                    return SR.InvalidProgram_Default;
                case ExceptionStringID.InvalidProgramSpecific:
                    return SR.InvalidProgram_Specific;
                case ExceptionStringID.InvalidProgramVararg:
                    return SR.InvalidProgram_Vararg;
                case ExceptionStringID.InvalidProgramCallVirtFinalize:
                    return SR.InvalidProgram_CallVirtFinalize;
                case ExceptionStringID.MissingField:
                    return SR.EE_MissingField;
                case ExceptionStringID.MissingMethod:
                    return SR.EE_MissingMethod;
                case ExceptionStringID.FileLoadErrorGeneric:
                    return SR.IO_FileNotFound_FileName;
                case ExceptionStringID.BadImageFormatGeneric:
                    return SR.Arg_BadImageFormatException;
                case ExceptionStringID.MarshalDirectiveGeneric:
                    return SR.Arg_MarshalDirectiveException;
                default:
                    Debug.Assert(false);
                    return "";
            }
        }
    }
}
