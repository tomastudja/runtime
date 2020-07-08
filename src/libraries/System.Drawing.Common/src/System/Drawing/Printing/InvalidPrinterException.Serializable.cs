// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// This file isn't built into the .csproj in the runtime libraries but is consumed by Mono.

using System.Runtime.Serialization;

namespace System.Drawing.Printing
{
    partial class InvalidPrinterException
    {
        protected InvalidPrinterException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
            _settings = (PrinterSettings)info.GetValue("settings", typeof(PrinterSettings));
        }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            base.GetObjectData(info, context);
            info.AddValue("settings", _settings);
        }
    }
}
