﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;

namespace Microsoft.Extensions.DependencyInjection
{
    /// <summary>
    /// Default implementation of <see cref="IServiceProviderFactory{TContainerBuilder}"/>.
    /// </summary>
    public class DefaultServiceProviderFactory : IServiceProviderFactory<IServiceCollection>
    {
        private readonly ServiceProviderOptions _options;

        /// <summary>
        /// Initializes a new instance of the <see cref="DefaultServiceProviderFactory"/> class
        /// with default options.
        /// </summary>
        public DefaultServiceProviderFactory() : this(ServiceProviderOptions.Default)
        {

        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DefaultServiceProviderFactory"/> class
        /// with the specified <paramref name="options"/>.
        /// </summary>
        /// <param name="options">The options to use for this instance.</param>
        public DefaultServiceProviderFactory(ServiceProviderOptions options)
        {
            if (options == null)
            {
                throw new ArgumentNullException(nameof(options));
            }

            _options = options;
        }

        /// <inheritdoc />
        public IServiceCollection CreateBuilder(IServiceCollection services)
        {
            return services;
        }

        /// <inheritdoc />
        public IServiceProvider CreateServiceProvider(IServiceCollection containerBuilder)
        {
            return containerBuilder.BuildServiceProvider(_options);
        }
    }
}
