// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Configuration
{
    /// <summary>
    /// Description for a particular setting.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class SettingsDescriptionAttribute : Attribute
    {
        private readonly string _description;

        /// <summary>
        /// Constructor takes the description string.
        /// </summary>
        public SettingsDescriptionAttribute(string description)
        {
            _description = description;
        }

        /// <summary>
        /// Description string.
        /// </summary>
        public string Description
        {
            get
            {
                return _description;
            }
        }
    }

}
