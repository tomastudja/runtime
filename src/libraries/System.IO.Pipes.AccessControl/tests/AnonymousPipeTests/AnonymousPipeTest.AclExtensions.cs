// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Microsoft.Win32.SafeHandles;

namespace System.IO.Pipes.Tests
{
    public class AnonymousPipeTest_AclExtensions : PipeTest_AclExtensions
    {
        protected override ServerClientPair CreateServerClientPair()
        {
            ServerClientPair ret = new ServerClientPair();
            ret.readablePipe = new AnonymousPipeServerStream(PipeDirection.In);
            ret.writeablePipe = new AnonymousPipeClientStream(PipeDirection.Out, ((AnonymousPipeServerStream)ret.readablePipe).ClientSafePipeHandle);
            return ret;
        }
    }
}
