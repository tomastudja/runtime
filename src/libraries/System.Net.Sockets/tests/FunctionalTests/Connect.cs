// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using Xunit;
using Xunit.Abstractions;
using Xunit.Sdk;
using System.Linq;

namespace System.Net.Sockets.Tests
{
    public abstract class Connect<T> : SocketTestHelperBase<T> where T : SocketHelperBase, new()
    {
        public Connect(ITestOutputHelper output) : base(output) {}

        [OuterLoop]
        [Theory]
        [MemberData(nameof(Loopbacks))]
        public async Task Connect_Success(IPAddress listenAt)
        {
            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, listenAt, out port))
            {
                using (Socket client = new Socket(listenAt.AddressFamily, SocketType.Stream, ProtocolType.Tcp))
                {
                    Task connectTask = ConnectAsync(client, new IPEndPoint(listenAt, port));
                    await connectTask;
                    Assert.True(client.Connected);
                }
            }
        }

        [Theory]
        [MemberData(nameof(Loopbacks))]
        public async Task Connect_Udp_Success(IPAddress listenAt)
        {
            using Socket listener = new Socket(listenAt.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            using Socket client = new Socket(listenAt.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            listener.Bind(new IPEndPoint(listenAt, 0));

            await ConnectAsync(client, new IPEndPoint(listenAt, ((IPEndPoint)listener.LocalEndPoint).Port));
            Assert.True(client.Connected);
        }

        [Theory]
        [MemberData(nameof(Loopbacks))]
        public async Task Connect_Dns_Success(IPAddress listenAt)
        {
            // On some systems (like Ubuntu 16.04 and Ubuntu 18.04) "localhost" doesn't resolve to '::1'.
            if (Array.IndexOf(Dns.GetHostAddresses("localhost"), listenAt) == -1)
            {
                return;
            }

            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, listenAt, out port))
            {
                using (Socket client = new Socket(listenAt.AddressFamily, SocketType.Stream, ProtocolType.Tcp))
                {
                    Task connectTask = ConnectAsync(client, new DnsEndPoint("localhost", port));
                    await connectTask;
                    Assert.True(client.Connected);
                }
            }
        }

        [OuterLoop]
        [Theory]
        [MemberData(nameof(Loopbacks))]
        public async Task Connect_MultipleIPAddresses_Success(IPAddress listenAt)
        {
            if (!SupportsMultiConnect)
                return;

            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, listenAt, out port))
            using (Socket client = new Socket(listenAt.AddressFamily, SocketType.Stream, ProtocolType.Tcp))
            {
                Task connectTask = MultiConnectAsync(client, new IPAddress[] { IPAddress.Loopback, IPAddress.IPv6Loopback }, port);
                await connectTask;
                Assert.True(client.Connected);
            }
        }

        [Fact]
        public async Task Connect_OnConnectedSocket_Fails()
        {
            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, IPAddress.Loopback, out port))
            using (Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                await ConnectAsync(client, new IPEndPoint(IPAddress.Loopback, port));

                // In the sync case, we throw a derived exception here, so need to use ThrowsAnyAsync
                SocketException se = await Assert.ThrowsAnyAsync<SocketException>(() => ConnectAsync(client, new IPEndPoint(IPAddress.Loopback, port)));
                Assert.Equal(SocketError.IsConnected, se.SocketErrorCode);
            }
        }

        [PlatformSpecific(TestPlatforms.Windows)] // Unix currently does not support Disconnect
        [OuterLoop]
        [Fact]
        public async Task Connect_AfterDisconnect_Fails()
        {
            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, IPAddress.Loopback, out port))
            using (Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                await ConnectAsync(client, new IPEndPoint(IPAddress.Loopback, port));
                client.Disconnect(reuseSocket: false);

                if (ConnectAfterDisconnectResultsInInvalidOperationException)
                {
                    await Assert.ThrowsAsync<InvalidOperationException>(() => ConnectAsync(client, new IPEndPoint(IPAddress.Loopback, port)));
                }
                else
                {
                    SocketException se = await Assert.ThrowsAsync<SocketException>(() => ConnectAsync(client, new IPEndPoint(IPAddress.Loopback, port)));
                    Assert.Equal(SocketError.IsConnected, se.SocketErrorCode);
                }
            }
        }

        [OuterLoop("Connects to external server")]
        [SkipOnPlatform(TestPlatforms.OSX | TestPlatforms.MacCatalyst | TestPlatforms.iOS | TestPlatforms.tvOS | TestPlatforms.FreeBSD, "Not supported on BSD like OSes.")]
        [Theory]
        [InlineData("1.1.1.1", false, true)]
        [InlineData("1.1.1.1", true, true)]
        [InlineData("[::ffff:1.1.1.1]", false, true)]
        [InlineData("[::ffff:1.1.1.1]", true, true)]
        [InlineData("1.1.1.1", false, false)]
        [InlineData("1.1.1.1", true, false)]
        [InlineData("[::ffff:1.1.1.1]", false, false)]
        [InlineData("[::ffff:1.1.1.1]", true, false)]
        public async Task ConnectGetsCanceledByDispose(string addressString, bool useDns, bool owning)
        {
            // Aborting sync operations for non-owning handles is not supported on Unix.
            if (!owning && UsesSync && !PlatformDetection.IsWindows)
            {
                return;
            }

            IPAddress address = IPAddress.Parse(addressString);

            // We try this a couple of times to deal with a timing race: if the Dispose happens
            // before the operation is started, we won't see a SocketException.
            int msDelay = 100;
            await RetryHelper.ExecuteAsync(async () =>
            {
                var client = new Socket(address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                using SafeSocketHandle? owner = ReplaceWithNonOwning(ref client, owning);

                if (address.IsIPv4MappedToIPv6) client.DualMode = true;

                Task connectTask = ConnectAsync(client, useDns ?
                    new DnsEndPoint("one.one.one.one", 23) :
                    new IPEndPoint(address, 23));

                // Wait a little so the operation is started.
                await Task.Delay(Math.Min(msDelay, 1000));
                msDelay *= 2;
                Task disposeTask = Task.Run(() => client.Dispose());

                await Task.WhenAny(disposeTask, connectTask).WaitAsync(TimeSpan.FromSeconds(30));
                await disposeTask;

                SocketError? localSocketError = null;
                bool disposedException = false;
                try
                {
                    await connectTask;
                }
                catch (SocketException se)
                {
                    // On connection timeout, retry.
                    Assert.NotEqual(SocketError.TimedOut, se.SocketErrorCode);

                    localSocketError = se.SocketErrorCode;
                }
                catch (ObjectDisposedException)
                {
                    disposedException = true;
                }

                if (UsesSync)
                {
                    Assert.True(disposedException || localSocketError == SocketError.NotSocket, $"{disposedException} {localSocketError}");
                }
                else
                {
                    Assert.Equal(SocketError.OperationAborted, localSocketError);
                }
            }, maxAttempts: 10, retryWhen: e => e is XunitException);
        }

        [OuterLoop("Connection failure takes long on Windows.")]
        [Fact]
        public async Task Connect_WithoutListener_ThrowSocketExceptionWithAppropriateInfo()
        {
            using PortBlocker portBlocker = new PortBlocker(() =>
            {
                Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                socket.BindToAnonymousPort(IPAddress.Loopback);
                return socket;
            });
            Socket a = portBlocker.MainSocket;
            using Socket b = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            SocketException ex = await Assert.ThrowsAsync<SocketException>(() => ConnectAsync(b, a.LocalEndPoint));
            Assert.Contains(Marshal.GetPInvokeErrorMessage(ex.NativeErrorCode), ex.Message);

            if (UsesSync)
            {
                Assert.Contains(a.LocalEndPoint.ToString(), ex.Message);
            }
        }

        [Theory]
        [MemberData(nameof(LoopbacksAndAny))]
        public async Task Connect_DatagramSockets_DontThrowConnectedException_OnSecondAttempt(IPAddress listenAt, IPAddress secondConnection)
        {
            using Socket listener = new Socket(listenAt.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            using Socket s = new Socket(listenAt.AddressFamily, SocketType.Dgram, ProtocolType.Udp);
            listener.Bind(new IPEndPoint(listenAt, 0));

            await ConnectAsync(s, new IPEndPoint(listenAt, ((IPEndPoint)listener.LocalEndPoint).Port));
            Assert.True(s.Connected);
            // According to the OSX man page, it's enough connecting to an invalid address to dissolve the connection. (0 port connection returns error on OSX)
            await ConnectAsync(s, new IPEndPoint(secondConnection, PlatformDetection.IsOSX ? 1 : 0));
            Assert.True(s.Connected);
        }
    }

    public sealed class ConnectSync : Connect<SocketHelperArraySync>
    {
        public ConnectSync(ITestOutputHelper output) : base(output) {}
    }

    public sealed class ConnectSyncForceNonBlocking : Connect<SocketHelperSyncForceNonBlocking>
    {
        public ConnectSyncForceNonBlocking(ITestOutputHelper output) : base(output) {}
    }

    public sealed class ConnectApm : Connect<SocketHelperApm>
    {
        public ConnectApm(ITestOutputHelper output) : base(output) {}
    }

    public sealed class ConnectTask : Connect<SocketHelperTask>
    {
        public ConnectTask(ITestOutputHelper output) : base(output) {}
    }

    public sealed class ConnectEap : Connect<SocketHelperEap>
    {
        public ConnectEap(ITestOutputHelper output) : base(output) {}

        [Theory]
        [PlatformSpecific(TestPlatforms.Windows)]
        [InlineData(true)]
        [InlineData(false)]
        public async Task ConnectAsync_WithData_DataReceived(bool useArrayApi)
        {
            using Socket listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            listener.Bind(new IPEndPoint(IPAddress.Loopback, 0));
            IPEndPoint serverEp = (IPEndPoint)listener.LocalEndPoint!;
            listener.Listen();

            var serverTask = Task.Run(async () =>
            {
                using Socket handler = await listener.AcceptAsync();
                using var cts = new CancellationTokenSource(TestSettings.PassingTestTimeout);
                byte[] recvBuffer = new byte[6];
                int received = await handler.ReceiveAsync(recvBuffer, SocketFlags.None, cts.Token);
                Assert.True(received == 4);

                recvBuffer.AsSpan(0, 4).SequenceEqual(new byte[] { 2, 3, 4, 5 });
            });

            using var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            byte[] buffer = new byte[] { 0, 1, 2, 3, 4, 5, 6, 7 };
            
            var mre = new ManualResetEventSlim(false);
            var saea = new SocketAsyncEventArgs();
            saea.RemoteEndPoint = serverEp;

            // Slice the buffer to test the offset management:
            if (useArrayApi)
            {
                saea.SetBuffer(buffer, 2, 4);
            }
            else
            {
                saea.SetBuffer(buffer.AsMemory(2, 4));
            }
            
            saea.Completed += (_, __) => mre.Set();

            if (client.ConnectAsync(saea))
            {
                Assert.True(mre.Wait(TestSettings.PassingTestTimeout), "Timed out while waiting for connection");
            }

            Assert.Equal(SocketError.Success, saea.SocketError);

            await serverTask;
        }
    }

    public sealed class ConnectCancellableTask : Connect<SocketHelperCancellableTask>
    {
        public ConnectCancellableTask(ITestOutputHelper output) : base(output) { }

        [Fact]
        public async Task ConnectEndPoint_Precanceled_Throws()
        {
            EndPoint ep = new IPEndPoint(IPAddress.Parse("1.2.3.4"), 1);

            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();
                cts.Cancel();

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await client.ConnectAsync(ep, cts.Token));
            }
        }

        [Fact]
        public async Task ConnectAddressAndPort_Precanceled_Throws()
        {
            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();
                cts.Cancel();

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await client.ConnectAsync(IPAddress.Parse("1.2.3.4"), 1, cts.Token));
            }
        }

        [Fact]
        public async Task ConnectMultiAddressAndPort_Precanceled_Throws()
        {
            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();
                cts.Cancel();

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await client.ConnectAsync(new IPAddress[] { IPAddress.Parse("1.2.3.4"), IPAddress.Parse("1.2.3.5") }, 1, cts.Token));
            }
        }

        [Fact]
        public async Task ConnectHostNameAndPort_Precanceled_Throws()
        {
            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();
                cts.Cancel();

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await client.ConnectAsync("1.2.3.4", 1, cts.Token));
            }
        }

        [Fact]
        [OuterLoop("Uses Task.Delay")]
        [PlatformSpecific(TestPlatforms.Windows)]   // Linux will not even attempt to connect to the invalid IP address
        public async Task ConnectEndPoint_CancelDuringConnect_Throws()
        {
            EndPoint ep = new IPEndPoint(IPAddress.Parse("1.2.3.4"), 1);

            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();

                ValueTask t = client.ConnectAsync(ep, cts.Token);

                // Delay cancellation a bit to try to ensure the OS actually attempts to connect
                cts.CancelAfter(100);

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await t);
            }
        }

        [Fact]
        [OuterLoop("Uses Task.Delay")]
        [PlatformSpecific(TestPlatforms.Windows)]   // Linux will not even attempt to connect to the invalid IP address
        public async Task ConnectAddressAndPort_CancelDuringConnect_Throws()
        {
            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();

                ValueTask t = client.ConnectAsync(IPAddress.Parse("1.2.3.4"), 1, cts.Token);

                // Delay cancellation a bit to try to ensure the OS actually attempts to connect
                cts.CancelAfter(100);

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await t);
            }
        }

        [Fact]
        [OuterLoop("Uses Task.Delay")]
        [PlatformSpecific(TestPlatforms.Windows)]   // Linux will not even attempt to connect to the invalid IP address
        public async Task ConnectMultiAddressAndPort_CancelDuringConnect_Throws()
        {
            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();

                ValueTask t = client.ConnectAsync(new IPAddress[] { IPAddress.Parse("1.2.3.4"), IPAddress.Parse("1.2.3.5") }, 1, cts.Token);

                // Delay cancellation a bit to try to ensure the OS actually attempts to connect
                cts.CancelAfter(100);

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await t);
            }
        }

        [Fact]
        [OuterLoop("Uses Task.Delay")]
        [PlatformSpecific(TestPlatforms.Windows)]   // Linux will not even attempt to connect to the invalid IP address
        public async Task ConnectHostNameAndPort_CancelDuringConnect_Throws()
        {
            using (var client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                var cts = new CancellationTokenSource();

                ValueTask t = client.ConnectAsync("1.2.3.4", 1, cts.Token);

                // Delay cancellation a bit to try to ensure the OS actually attempts to connect
                cts.CancelAfter(100);

                await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await t);
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public async Task FailedConnect_ConnectedReturnsFalse(bool useTimeSpan)
        {
            using Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            // Connect to port 1 where we expect no server to be listening.
            SocketException se = await Assert.ThrowsAnyAsync<SocketException>(() => ConnectAsync(socket, new IPEndPoint(IPAddress.Loopback, 1)));

            if (se.SocketErrorCode != SocketError.ConnectionRefused)
            {
                Assert.Equal(SocketError.WouldBlock, se.SocketErrorCode);

                // Give the non-blocking connect some time to complete.
                if (useTimeSpan)
                {
                    socket.Poll(TimeSpan.FromMilliseconds(5000), SelectMode.SelectWrite);
                }
                else
                {
                    socket.Poll(5_000_000 /* microSeconds */, SelectMode.SelectWrite);
                }
            }

            Assert.False(socket.Connected);
        }
    }

    // The test class is declared non-parallel because of possible IPv4/IPv6 port-collision on Unix:
    // When running these tests in parallel with other tests, there is some chance that the DualMode client
    // will connect to an IPv4 server of a parallel test case.
    [Collection(nameof(DisableParallelization))]
    public abstract class Connect_NonParallel<T> : SocketTestHelperBase<T> where T : SocketHelperBase, new()
    {
        protected Connect_NonParallel(ITestOutputHelper output) : base(output)
        {
        }

        [Fact]
        public async Task Connect_DualMode_MultiAddressFamilyConnect_RetrievedEndPoints_Success()
        {
            if (!SupportsMultiConnect)
                return;

            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, IPAddress.Loopback, out port))
            using (Socket client = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                Assert.True(client.DualMode);

                await MultiConnectAsync(client, new IPAddress[] { IPAddress.IPv6Loopback, IPAddress.Loopback }, port);

                CheckIsIpv6LoopbackEndPoint(client.LocalEndPoint);
                CheckIsIpv6LoopbackEndPoint(client.RemoteEndPoint);
            }
        }

        [Fact]
        public async Task Connect_DualMode_DnsConnect_RetrievedEndPoints_Success()
        {
            var localhostAddresses = Dns.GetHostAddresses("localhost");
            if (Array.IndexOf(localhostAddresses, IPAddress.Loopback) == -1 ||
                Array.IndexOf(localhostAddresses, IPAddress.IPv6Loopback) == -1)
            {
                return;
            }

            int port;
            using (SocketTestServer.SocketTestServerFactory(SocketImplementationType.Async, IPAddress.Loopback, out port))
            using (Socket client = new Socket(SocketType.Stream, ProtocolType.Tcp))
            {
                Assert.True(client.DualMode);

                await ConnectAsync(client, new DnsEndPoint("localhost", port));

                CheckIsIpv6LoopbackEndPoint(client.LocalEndPoint);
                CheckIsIpv6LoopbackEndPoint(client.RemoteEndPoint);
            }
        }

        private static void CheckIsIpv6LoopbackEndPoint(EndPoint endPoint)
        {
            IPEndPoint ep = endPoint as IPEndPoint;
            Assert.NotNull(ep);
            Assert.True(ep.Address.Equals(IPAddress.IPv6Loopback) || ep.Address.Equals(IPAddress.Loopback.MapToIPv6()));
        }
    }

    public sealed class ConnectSync_NonParallel : Connect_NonParallel<SocketHelperArraySync>
    {
        public ConnectSync_NonParallel(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ConnectSyncForceNonBlocking_NonParallel : Connect_NonParallel<SocketHelperSyncForceNonBlocking>
    {
        public ConnectSyncForceNonBlocking_NonParallel(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ConnectApm_NonParallel : Connect_NonParallel<SocketHelperApm>
    {
        public ConnectApm_NonParallel(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ConnectTask_NonParallel : Connect_NonParallel<SocketHelperTask>
    {
        public ConnectTask_NonParallel(ITestOutputHelper output) : base(output) { }
    }

    public sealed class ConnectEap_NonParallel : Connect_NonParallel<SocketHelperEap>
    {
        public ConnectEap_NonParallel(ITestOutputHelper output) : base(output) { }
    }
}
