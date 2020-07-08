// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Data
{
    public sealed class DataTableClearEventArgs : EventArgs
    {
        public DataTableClearEventArgs(DataTable dataTable)
        {
            Table = dataTable;
        }

        public DataTable Table { get; }
        public string TableName => Table.TableName;
        public string TableNamespace => Table.Namespace;
    }
}
