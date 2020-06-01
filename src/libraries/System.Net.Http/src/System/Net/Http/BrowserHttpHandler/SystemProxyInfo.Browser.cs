// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System.Net.Http
{
    internal static partial class SystemProxyInfo
    {
        // On Browser we do not support proxy.
        public static IWebProxy ConstructSystemProxy()
        {
            return new HttpNoProxy();
        }
    }
}
