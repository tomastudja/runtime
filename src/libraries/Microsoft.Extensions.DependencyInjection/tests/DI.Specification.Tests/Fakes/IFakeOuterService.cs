// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Collections.Generic;

namespace Microsoft.Extensions.DependencyInjection.Specification.Fakes
{
    public interface IFakeOuterService
    {
        IFakeService SingleService { get; }

        IEnumerable<IFakeMultipleService> MultipleServices { get; }
    }
}