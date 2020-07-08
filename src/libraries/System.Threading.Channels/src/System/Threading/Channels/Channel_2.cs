// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Threading.Channels
{
    /// <summary>
    /// Provides a base class for channels that support reading elements of type <typeparamref name="TRead"/>
    /// and writing elements of type <typeparamref name="TWrite"/>.
    /// </summary>
    /// <typeparam name="TWrite">Specifies the type of data that may be written to the channel.</typeparam>
    /// <typeparam name="TRead">Specifies the type of data that may be read from the channel.</typeparam>
    public abstract class Channel<TWrite, TRead>
    {
        /// <summary>Gets the readable half of this channel.</summary>
        public ChannelReader<TRead> Reader { get; protected set; } = null!; // derived types should always set the Reader as part of construction

        /// <summary>Gets the writable half of this channel.</summary>
        public ChannelWriter<TWrite> Writer { get; protected set; } = null!; // derived types should always set the Writer as part of construction

        /// <summary>Implicit cast from a channel to its readable half.</summary>
        /// <param name="channel">The channel being cast.</param>
        public static implicit operator ChannelReader<TRead>(Channel<TWrite, TRead> channel) => channel.Reader;

        /// <summary>Implicit cast from a channel to its writable half.</summary>
        /// <param name="channel">The channel being cast.</param>
        public static implicit operator ChannelWriter<TWrite>(Channel<TWrite, TRead> channel) => channel.Writer;
    }
}
