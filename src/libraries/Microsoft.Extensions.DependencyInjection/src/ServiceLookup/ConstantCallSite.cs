// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;

namespace Microsoft.Extensions.DependencyInjection.ServiceLookup
{
    internal class ConstantCallSite : ServiceCallSite
    {
        internal object DefaultValue { get; }

        public ConstantCallSite(Type serviceType, object defaultValue): base(ResultCache.None)
        {
            if (defaultValue != null && !serviceType.IsInstanceOfType(defaultValue))
            {
                throw new ArgumentException(Resources.FormatConstantCantBeConvertedToServiceType(defaultValue.GetType(), serviceType));
            }

            DefaultValue = defaultValue;
        }

        public override Type ServiceType => DefaultValue.GetType();
        public override Type ImplementationType => DefaultValue.GetType();
        public override CallSiteKind Kind { get; } = CallSiteKind.Constant;
    }
}
