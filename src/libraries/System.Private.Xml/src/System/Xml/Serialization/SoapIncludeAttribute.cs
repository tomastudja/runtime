// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#nullable enable
namespace System.Xml.Serialization
{
    using System;

    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Interface | AttributeTargets.Method, AllowMultiple = true)]
    public class SoapIncludeAttribute : System.Attribute
    {
        private Type _type;

        public SoapIncludeAttribute(Type type)
        {
            _type = type;
        }

        public Type Type
        {
            get { return _type; }
            set { _type = value; }
        }
    }
}
