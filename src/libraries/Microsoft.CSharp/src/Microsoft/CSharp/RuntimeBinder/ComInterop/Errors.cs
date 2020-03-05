﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;

namespace Microsoft.CSharp.RuntimeBinder.ComInterop
{
    /// <summary>
    ///    Strongly-typed and parameterized exception factory.
    /// </summary>
    internal static partial class Error
    {
        /// <summary>
        /// InvalidOperationException with message like "Marshal.SetComObjectData failed."
        /// </summary>
        internal static Exception SetComObjectDataFailed()
        {
            return new InvalidOperationException(SR.COMSetComObjectDataFailed);
        }

        /// <summary>
        /// InvalidOperationException with message like "Unexpected VarEnum {0}."
        /// </summary>
        internal static Exception UnexpectedVarEnum(object p0)
        {
            return new InvalidOperationException(SR.Format(SR.COMUnexpectedVarEnum, p0));
        }

        /// <summary>
        /// System.Reflection.TargetParameterCountException with message like "Error while invoking {0}."
        /// </summary>
        internal static Exception DispBadParamCount(object p0)
        {
            return new System.Reflection.TargetParameterCountException(SR.Format(SR.COMDispatchInvokeError, p0));
        }

        /// <summary>
        /// MissingMemberException with message like "Error while invoking {0}."
        /// </summary>
        internal static Exception DispMemberNotFound(object p0)
        {
            return new MissingMemberException(SR.Format(SR.COMDispatchInvokeError, p0));
        }

        /// <summary>
        /// ArgumentException with message like "Error while invoking {0}. Named arguments are not supported."
        /// </summary>
        internal static Exception DispNoNamedArgs(object p0)
        {
            return new ArgumentException(SR.Format(SR.COMDispatchInvokeErrorNoNamedArgs, p0));
        }

        /// <summary>
        /// OverflowException with message like "Error while invoking {0}."
        /// </summary>
        internal static Exception DispOverflow(object p0)
        {
            return new OverflowException(SR.Format(SR.COMDispatchInvokeError, p0));
        }

        /// <summary>
        /// ArgumentException with message like "Could not convert argument {0} for call to {1}."
        /// </summary>
        internal static Exception DispTypeMismatch(object p0, object p1)
        {
            return new ArgumentException(SR.Format(SR.COMDispatchInvokeErrorTypeMismatch, p0, p1));
        }

        /// <summary>
        /// ArgumentException with message like "Error while invoking {0}. A required parameter was omitted."
        /// </summary>
        internal static Exception DispParamNotOptional(object p0)
        {
            return new ArgumentException(SR.Format(SR.COMDispatchInvokeErrorParamNotOptional, p0));
        }

        /// <summary>
        /// InvalidOperationException with message like "Cannot retrieve type information."
        /// </summary>
        internal static Exception CannotRetrieveTypeInformation()
        {
            return new InvalidOperationException(SR.COMCannotRetrieveTypeInfo);
        }

        /// <summary>
        /// ArgumentException with message like "IDispatch::GetIDsOfNames behaved unexpectedly for {0}."
        /// </summary>
        internal static Exception GetIDsOfNamesInvalid(object p0)
        {
            return new ArgumentException(SR.Format(SR.COMGetIDsOfNamesInvalid, p0));
        }

        /// <summary>
        /// InvalidOperationException with message like "Attempting to pass an event handler of an unsupported type."
        /// </summary>
        internal static Exception UnsupportedHandlerType()
        {
            return new InvalidOperationException(SR.COMUnsupportedEventHandlerType);
        }

        /// <summary>
        /// MissingMemberException with message like "Could not get dispatch ID for {0} (error: {1})."
        /// </summary>
        internal static Exception CouldNotGetDispId(object p0, object p1)
        {
            return new MissingMemberException(SR.Format(SR.COMGetDispatchIdFailed, p0, p1));
        }

        /// <summary>
        /// System.Reflection.AmbiguousMatchException with message like "There are valid conversions from {0} to {1}."
        /// </summary>
        internal static Exception AmbiguousConversion(object p0, object p1)
        {
            return new System.Reflection.AmbiguousMatchException(SR.Format(SR.COMAmbiguousConversion, p0, p1));
        }
    }
}
