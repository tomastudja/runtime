// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Xunit;
using Xunit.Abstractions;

namespace System.Net.Sockets.Tests
{
    public abstract class ReceiveMessageFrom<T> : SocketTestHelperBase<T> where T : SocketHelperBase, new()
    {
        protected static IPEndPoint GetGetDummyTestEndpoint(AddressFamily addressFamily = AddressFamily.InterNetwork) =>
            addressFamily == AddressFamily.InterNetwork ? new IPEndPoint(IPAddress.Parse("1.2.3.4"), 1234) : new IPEndPoint(IPAddress.Parse("1:2:3::4"), 1234);

        protected static readonly TimeSpan CancellationTestTimeout = TimeSpan.FromSeconds(30);

        protected ReceiveMessageFrom(ITestOutputHelper output) : base(output) { }

        [PlatformSpecific(TestPlatforms.AnyUnix)]
        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public async Task ReceiveSent_TCP_Success(bool ipv6)
        {
            if (ipv6 && PlatformDetection.IsOSX)
            {
                // [ActiveIssue("https://github.com/dotnet/runtime/issues/47335")]
                // accept() will create a (seemingly) DualMode socket on Mac,
                // but since recvmsg() does not work with DualMode on that OS, we throw PNSE CheckDualModeReceiveSupport().
                // Weirdly, the flag is readable, but an attempt to write it leads to EINVAL.
                // The best option seems to be to skip this test for the Mac+IPV6 case
                return;
            }

            (Socket sender, Socket receiver) = SocketTestExtensions.CreateConnectedSocketPair(ipv6);
            using (sender)
            using (receiver)
            {
                byte[] sendBuffer = { 1, 2, 3 };
                sender.Send(sendBuffer);

                byte[] receiveBuffer = new byte[3];
                var r = await ReceiveMessageFromAsync(receiver, receiveBuffer, sender.LocalEndPoint);
                Assert.Equal(3, r.ReceivedBytes);
                AssertExtensions.SequenceEqual(sendBuffer, receiveBuffer);
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public async Task ReceiveSent_UDP_Success(bool ipv4)
        {
            const int Offset = 10;
            const int DatagramSize = 256;
            const int DatagramsToSend = 16;

            IPAddress address = ipv4 ? IPAddress.Loopback : IPAddress.IPv6Loopback;
            using Socket receiver = new Socket(address.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            using Socket sender = new Socket(address.AddressFamily, SocketType.Dgram, ProtocolType.Udp);

            receiver.SetSocketOption(ipv4 ? SocketOptionLevel.IP : SocketOptionLevel.IPv6, SocketOptionName.PacketInformation, true);
            ConfigureNonBlocking(sender);
            ConfigureNonBlocking(receiver);

            receiver.BindToAnonymousPort(address);
            sender.BindToAnonymousPort(address);

            byte[] sendBuffer = new byte[DatagramSize];
            var receiveInternalBuffer = new byte[DatagramSize + Offset];
            var emptyBuffer = new byte[Offset];
            ArraySegment<byte> receiveBuffer = new ArraySegment<byte>(receiveInternalBuffer, Offset, DatagramSize);

            Random rnd = new Random(0);

            IPEndPoint remoteEp = new IPEndPoint(ipv4 ? IPAddress.Any : IPAddress.IPv6Any, 0);

            for (int i = 0; i < DatagramsToSend; i++)
            {
                rnd.NextBytes(sendBuffer);
                sender.SendTo(sendBuffer, receiver.LocalEndPoint);

                SocketReceiveMessageFromResult result = await ReceiveMessageFromAsync(receiver, receiveBuffer, remoteEp);
                IPPacketInformation packetInformation = result.PacketInformation;

                Assert.Equal(DatagramSize, result.ReceivedBytes);
                AssertExtensions.SequenceEqual(emptyBuffer, new ReadOnlySpan<byte>(receiveInternalBuffer, 0, Offset));
                AssertExtensions.SequenceEqual(sendBuffer, new ReadOnlySpan<byte>(receiveInternalBuffer, Offset, DatagramSize));
                Assert.Equal(sender.LocalEndPoint, result.RemoteEndPoint);
                Assert.Equal(((IPEndPoint)sender.LocalEndPoint).Address, packetInformation.Address);
            }
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        public async Task ClosedBeforeOperation_Throws_ObjectDisposedException(bool closeOrDispose)
        {
            using var socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            socket.BindToAnonymousPort(IPAddress.Any);
            if (closeOrDispose) socket.Close();
            else socket.Dispose();

            await Assert.ThrowsAsync<ObjectDisposedException>(() => ReceiveMessageFromAsync(socket, new byte[1], GetGetDummyTestEndpoint()));
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        [ActiveIssue("https://github.com/dotnet/runtime/issues/47561")]
        public async Task ClosedDuringOperation_Throws_ObjectDisposedExceptionOrSocketException(bool closeOrDispose)
        {
            if (UsesSync && PlatformDetection.IsOSX)
            {
                // [ActiveIssue("https://github.com/dotnet/runtime/issues/47342")]
                // On Mac, Close/Dispose hangs when invoked concurrently with a blocking UDP receive.
                return;
            }

            using var socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            socket.BindToAnonymousPort(IPAddress.Any);

            Task receiveTask = ReceiveMessageFromAsync(socket, new byte[1], GetGetDummyTestEndpoint());
            await Task.Delay(100);
            if (closeOrDispose) socket.Close();
            else socket.Dispose();

            if (UsesApm)
            {
                await Assert.ThrowsAsync<ObjectDisposedException>(() => receiveTask)
                    .TimeoutAfter(CancellationTestTimeout);
            }
            else
            {
                SocketException ex = await Assert.ThrowsAsync<SocketException>(() => receiveTask)
                    .TimeoutAfter(CancellationTestTimeout);
                SocketError expectedError = UsesSync ? SocketError.Interrupted : SocketError.OperationAborted;
                Assert.Equal(expectedError, ex.SocketErrorCode);
            }
        }

        [PlatformSpecific(TestPlatforms.Windows)] // It's allowed to shutdown() UDP sockets on Windows, however on Unix this will lead to ENOTCONN
        [Theory]
        [InlineData(SocketShutdown.Both)]
        [InlineData(SocketShutdown.Receive)]
        public async Task ShutdownReceiveBeforeOperation_ThrowsSocketException(SocketShutdown shutdown)
        {
            using var socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            socket.BindToAnonymousPort(IPAddress.Any);
            socket.Shutdown(shutdown);

            // [ActiveIssue("https://github.com/dotnet/runtime/issues/47469")]
            // Shutdown(Both) does not seem to take immediate effect for Receive(Message)From in a consistent manner, trying to workaround with a delay:
            if (shutdown == SocketShutdown.Both) await Task.Delay(50);

            SocketException exception = await Assert.ThrowsAnyAsync<SocketException>(() => ReceiveMessageFromAsync(socket, new byte[1], GetGetDummyTestEndpoint()))
                .TimeoutAfter(CancellationTestTimeout);

            Assert.Equal(SocketError.Shutdown, exception.SocketErrorCode);
        }

        [PlatformSpecific(TestPlatforms.Windows)] // It's allowed to shutdown() UDP sockets on Windows, however on Unix this will lead to ENOTCONN
        [Fact]
        public async Task ShutdownSend_ReceiveFromShouldSucceed()
        {
            using var receiver = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            receiver.BindToAnonymousPort(IPAddress.Loopback);
            receiver.Shutdown(SocketShutdown.Send);

            using var sender = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            sender.BindToAnonymousPort(IPAddress.Loopback);
            sender.SendTo(new byte[1], receiver.LocalEndPoint);

            var r = await ReceiveMessageFromAsync(receiver, new byte[1], sender.LocalEndPoint);
            Assert.Equal(1, r.ReceivedBytes);
        }
    }

    public sealed class ReceiveMessageFrom_Sync : ReceiveMessageFrom<SocketHelperArraySync>
    {
        public ReceiveMessageFrom_Sync(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_SyncForceNonBlocking : ReceiveMessageFrom<SocketHelperSyncForceNonBlocking>
    {
        public ReceiveMessageFrom_SyncForceNonBlocking(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_Apm : ReceiveMessageFrom<SocketHelperApm>
    {
        public ReceiveMessageFrom_Apm(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_Task : ReceiveMessageFrom<SocketHelperTask>
    {
        public ReceiveMessageFrom_Task(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_CancellableTask : ReceiveMessageFrom<SocketHelperCancellableTask>
    {
        public ReceiveMessageFrom_CancellableTask(ITestOutputHelper output) : base(output) { }

        [Theory]
        [MemberData(nameof(LoopbacksAndBuffers))]
        public async Task WhenCanceled_Throws(IPAddress loopback, bool precanceled)
        {
            using var socket = new Socket(loopback.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            using var dummy = new Socket(loopback.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            socket.BindToAnonymousPort(loopback);
            dummy.BindToAnonymousPort(loopback);
            Memory<byte> buffer = new byte[1];

            CancellationTokenSource cts = new CancellationTokenSource();
            if (precanceled) cts.Cancel();
            else cts.CancelAfter(100);

            OperationCanceledException ex = await Assert.ThrowsAnyAsync<OperationCanceledException>(
                () => socket.ReceiveMessageFromAsync(buffer, SocketFlags.None, dummy.LocalEndPoint, cts.Token).AsTask())
                .TimeoutAfter(CancellationTestTimeout);
            Assert.Equal(cts.Token, ex.CancellationToken);
        }
    }

    public sealed class ReceiveMessageFrom_Eap : ReceiveMessageFrom<SocketHelperEap>
    {
        public ReceiveMessageFrom_Eap(ITestOutputHelper output) : base(output) { }

        [Theory]
        [InlineData(false, 0)]
        [InlineData(false, 1)]
        [InlineData(false, 2)]
        [InlineData(true, 0)]
        [InlineData(true, 1)]
        [InlineData(true, 2)]
        public void ReceiveSentMessages_ReuseEventArgs_Success(bool ipv4, int bufferMode)
        {
            const int DatagramsToSend = 5;
            const int TimeoutMs = 30_000;

            AddressFamily family;
            IPAddress loopback, any;
            SocketOptionLevel level;
            if (ipv4)
            {
                family = AddressFamily.InterNetwork;
                loopback = IPAddress.Loopback;
                any = IPAddress.Any;
                level = SocketOptionLevel.IP;
            }
            else
            {
                family = AddressFamily.InterNetworkV6;
                loopback = IPAddress.IPv6Loopback;
                any = IPAddress.IPv6Any;
                level = SocketOptionLevel.IPv6;
            }

            using var receiver = new Socket(family, SocketType.Dgram, ProtocolType.Udp);
            using var sender = new Socket(family, SocketType.Dgram, ProtocolType.Udp);
            using var saea = new SocketAsyncEventArgs();
            var completed = new ManualResetEventSlim();
            saea.Completed += delegate { completed.Set(); };

            int port = receiver.BindToAnonymousPort(loopback);
            receiver.SetSocketOption(level, SocketOptionName.PacketInformation, true);
            sender.Bind(new IPEndPoint(loopback, 0));
            saea.RemoteEndPoint = new IPEndPoint(any, 0);

            Random random = new Random(0);
            byte[] sendBuffer = new byte[1024];
            random.NextBytes(sendBuffer);

            for (int i = 0; i < DatagramsToSend; i++)
            {
                byte[] receiveBuffer = new byte[1024];
                switch (bufferMode)
                {
                    case 0: // single buffer
                        saea.SetBuffer(receiveBuffer, 0, 1024);
                        break;
                    case 1: // single buffer in buffer list
                        saea.BufferList = new List<ArraySegment<byte>>
                        {
                            new ArraySegment<byte>(receiveBuffer)
                        };
                        break;
                    case 2: // multiple buffers in buffer list
                        saea.BufferList = new List<ArraySegment<byte>>
                        {
                            new ArraySegment<byte>(receiveBuffer, 0, 512),
                            new ArraySegment<byte>(receiveBuffer, 512, 512)
                        };
                        break;
                }

                bool pending = receiver.ReceiveMessageFromAsync(saea);
                sender.SendTo(sendBuffer, new IPEndPoint(loopback, port));
                if (pending) Assert.True(completed.Wait(TimeoutMs), "Expected operation to complete within timeout");
                completed.Reset();

                Assert.Equal(1024, saea.BytesTransferred);
                AssertExtensions.SequenceEqual(sendBuffer, receiveBuffer);
                Assert.Equal(sender.LocalEndPoint, saea.RemoteEndPoint);
                Assert.Equal(((IPEndPoint)sender.LocalEndPoint).Address, saea.ReceiveMessageFromPacketInfo.Address);
            }
        }
    }

    public sealed class ReceiveMessageFrom_SpanSync : ReceiveMessageFrom<SocketHelperSpanSync>
    {
        public ReceiveMessageFrom_SpanSync(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_SpanSyncForceNonBlocking : ReceiveMessageFrom<SocketHelperSpanSyncForceNonBlocking>
    {
        public ReceiveMessageFrom_SpanSyncForceNonBlocking(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_MemoryArrayTask : ReceiveMessageFrom<SocketHelperMemoryArrayTask>
    {
        public ReceiveMessageFrom_MemoryArrayTask(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ReceiveMessageFrom_MemoryNativeTask : ReceiveMessageFrom<SocketHelperMemoryNativeTask>
    {
        public ReceiveMessageFrom_MemoryNativeTask(ITestOutputHelper output) : base(output) { }
    }
}
