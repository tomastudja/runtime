// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using Internal.TypeSystem;

namespace Internal.IL.Stubs
{
    partial class PInvokeTargetNativeMethod : IPrefixMangledMethod
    {
        MethodDesc IPrefixMangledMethod.BaseMethod
        {
            get
            {
                return _declMethod;
            }
        }

        string IPrefixMangledMethod.Prefix
        {
            get
            {
                return "rawpinvoke";
            }
        }
    }
}
