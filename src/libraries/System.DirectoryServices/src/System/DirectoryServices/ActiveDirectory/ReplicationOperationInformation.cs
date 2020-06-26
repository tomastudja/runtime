// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System.DirectoryServices.ActiveDirectory
{
    public class ReplicationOperationInformation
    {
        internal DateTime startTime;
        internal ReplicationOperation currentOp;
        internal ReplicationOperationCollection collection;

        public ReplicationOperationInformation()
        {
        }

        public DateTime OperationStartTime => startTime;

        public ReplicationOperation CurrentOperation => currentOp;

        public ReplicationOperationCollection PendingOperations => collection;
    }
}
