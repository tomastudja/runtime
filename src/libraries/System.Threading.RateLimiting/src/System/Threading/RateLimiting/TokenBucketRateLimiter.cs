// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Threading.Tasks;

namespace System.Threading.RateLimiting
{
    /// <summary>
    /// <see cref="RateLimiter"/> implementation that replenishes tokens periodically instead of via a release mechanism.
    /// </summary>
    public sealed class TokenBucketRateLimiter : RateLimiter
    {
        private int _tokenCount;
        private int _queueCount;
        private uint _lastReplenishmentTick = (uint)Environment.TickCount;
        private bool _disposed;

        private readonly Timer? _renewTimer;
        private readonly TokenBucketRateLimiterOptions _options;
        private readonly Deque<RequestRegistration> _queue = new Deque<RequestRegistration>();

        // Use the queue as the lock field so we don't need to allocate another object for a lock and have another field in the object
        private object Lock => _queue;

        private static readonly RateLimitLease SuccessfulLease = new TokenBucketLease(true, null);
        private static readonly RateLimitLease FailedLease = new TokenBucketLease(false, null);

        /// <summary>
        /// Initializes the <see cref="TokenBucketRateLimiter"/>.
        /// </summary>
        /// <param name="options">Options to specify the behavior of the <see cref="TokenBucketRateLimiter"/>.</param>
        public TokenBucketRateLimiter(TokenBucketRateLimiterOptions options)
        {
            _options = options ?? throw new ArgumentNullException(nameof(options));
            _tokenCount = options.TokenLimit;

            if (_options.AutoReplenishment)
            {
                _renewTimer = new Timer(Replenish, this, _options.ReplenishmentPeriod, _options.ReplenishmentPeriod);
            }
        }

        /// <inheritdoc/>
        public override int GetAvailablePermits() => _tokenCount;

        /// <inheritdoc/>
        protected override RateLimitLease AcquireCore(int tokenCount)
        {
            // These amounts of resources can never be acquired
            if (tokenCount > _options.TokenLimit)
            {
                throw new ArgumentOutOfRangeException(nameof(tokenCount), tokenCount, SR.Format(SR.TokenLimitExceeded, tokenCount, _options.TokenLimit));
            }

            // Return SuccessfulLease or FailedLease depending to indicate limiter state
            if (tokenCount == 0 && !_disposed)
            {
                if (_tokenCount > 0)
                {
                    return SuccessfulLease;
                }

                return CreateFailedTokenLease(tokenCount);
            }

            lock (Lock)
            {
                if (TryLeaseUnsynchronized(tokenCount, out RateLimitLease? lease))
                {
                    return lease;
                }

                return CreateFailedTokenLease(tokenCount);
            }
        }

        /// <inheritdoc/>
        protected override ValueTask<RateLimitLease> WaitAsyncCore(int tokenCount, CancellationToken cancellationToken = default)
        {
            // These amounts of resources can never be acquired
            if (tokenCount > _options.TokenLimit)
            {
                throw new ArgumentOutOfRangeException(nameof(tokenCount), tokenCount, SR.Format(SR.TokenLimitExceeded, tokenCount, _options.TokenLimit));
            }

            ThrowIfDisposed();

            // Return SuccessfulAcquisition if requestedCount is 0 and resources are available
            if (tokenCount == 0 && _tokenCount > 0)
            {
                return new ValueTask<RateLimitLease>(SuccessfulLease);
            }

            lock (Lock)
            {
                if (TryLeaseUnsynchronized(tokenCount, out RateLimitLease? lease))
                {
                    return new ValueTask<RateLimitLease>(lease);
                }

                // Avoid integer overflow by using subtraction instead of addition
                Debug.Assert(_options.QueueLimit >= _queueCount);
                if (_options.QueueLimit - _queueCount < tokenCount)
                {
                    if (_options.QueueProcessingOrder == QueueProcessingOrder.NewestFirst && tokenCount <= _options.QueueLimit)
                    {
                        // remove oldest items from queue until there is space for the newest acquisition request
                        do
                        {
                            RequestRegistration oldestRequest = _queue.DequeueHead();
                            _queueCount -= oldestRequest.Count;
                            Debug.Assert(_queueCount >= 0);
                            oldestRequest.Tcs.TrySetResult(FailedLease);
                        }
                        while (_options.QueueLimit - _queueCount < tokenCount);
                    }
                    else
                    {
                        // Don't queue if queue limit reached and QueueProcessingOrder is OldestFirst
                        return new ValueTask<RateLimitLease>(CreateFailedTokenLease(tokenCount));
                    }
                }

                TaskCompletionSource<RateLimitLease> tcs = new TaskCompletionSource<RateLimitLease>(TaskCreationOptions.RunContinuationsAsynchronously);

                CancellationTokenRegistration ctr = default;
                if (cancellationToken.CanBeCanceled)
                {
                    ctr = cancellationToken.Register(static obj =>
                    {
                        ((TaskCompletionSource<RateLimitLease>)obj!).TrySetException(new OperationCanceledException());
                    }, tcs);
                }

                RequestRegistration registration = new RequestRegistration(tokenCount, tcs, ctr);
                _queue.EnqueueTail(registration);
                _queueCount += tokenCount;
                Debug.Assert(_queueCount <= _options.QueueLimit);

                // handle cancellation
                return new ValueTask<RateLimitLease>(registration.Tcs.Task);
            }
        }

        private RateLimitLease CreateFailedTokenLease(int tokenCount)
        {
            int replenishAmount = tokenCount - _tokenCount + _queueCount;
            // can't have 0 replenish periods, that would mean it should be a successful lease
            // if TokensPerPeriod is larger than the replenishAmount needed then it would be 0
            Debug.Assert(_options.TokensPerPeriod > 0);
            int replenishPeriods = Math.Max(replenishAmount / _options.TokensPerPeriod, 1);

            return new TokenBucketLease(false, TimeSpan.FromTicks(_options.ReplenishmentPeriod.Ticks * replenishPeriods));
        }

        private bool TryLeaseUnsynchronized(int tokenCount, [NotNullWhen(true)] out RateLimitLease? lease)
        {
            ThrowIfDisposed();

            // if permitCount is 0 we want to queue it if there are no available permits
            if (_tokenCount >= tokenCount && _tokenCount != 0)
            {
                if (tokenCount == 0)
                {
                    // Edge case where the check before the lock showed 0 available permits but when we got the lock some permits were now available
                    lease = SuccessfulLease;
                    return true;
                }

                // a. if there are no items queued we can lease
                // b. if there are items queued but the processing order is newest first, then we can lease the incoming request since it is the newest
                if (_queueCount == 0 || (_queueCount > 0 && _options.QueueProcessingOrder == QueueProcessingOrder.NewestFirst))
                {
                    _tokenCount -= tokenCount;
                    Debug.Assert(_tokenCount >= 0);
                    lease = SuccessfulLease;
                    return true;
                }
            }

            lease = null;
            return false;
        }

        /// <summary>
        /// Attempts to replenish the bucket.
        /// </summary>
        /// <returns>
        /// False if <see cref="TokenBucketRateLimiterOptions.AutoReplenishment"/> is enabled, otherwise true.
        /// Does not reflect if tokens were replenished.
        /// </returns>
        public bool TryReplenish()
        {
            if (_options.AutoReplenishment)
            {
                return false;
            }
            Replenish(this);
            return true;
        }

        private static void Replenish(object? state)
        {
            TokenBucketRateLimiter limiter = (state as TokenBucketRateLimiter)!;
            Debug.Assert(limiter is not null);

            // Use Environment.TickCount instead of DateTime.UtcNow to avoid issues on systems where the clock can change
            uint nowTicks = (uint)Environment.TickCount;
            limiter!.ReplenishInternal(nowTicks);
        }

        // Used in tests that test behavior with specific time intervals
        private void ReplenishInternal(uint nowTicks)
        {
            bool wrapped = false;
            // (uint)TickCount will wrap every ~50 days, we can detect that by checking if the new ticks is less than the last replenishment
            if (nowTicks < _lastReplenishmentTick)
            {
                wrapped = true;
            }

            // method is re-entrant (from Timer), lock to avoid multiple simultaneous replenishes
            lock (Lock)
            {
                if (_disposed)
                {
                    return;
                }

                // Fix the wrapping by using a long and adding uint.MaxValue in the wrapped case
                long nonWrappedTicks = wrapped ? (long)nowTicks + uint.MaxValue : nowTicks;
                if (nonWrappedTicks - _lastReplenishmentTick < _options.ReplenishmentPeriod.TotalMilliseconds)
                {
                    return;
                }

                _lastReplenishmentTick = nowTicks;

                int availablePermits = _tokenCount;
                TokenBucketRateLimiterOptions options = _options;
                int maxPermits = options.TokenLimit;
                int resourcesToAdd;

                if (availablePermits < maxPermits)
                {
                    resourcesToAdd = Math.Min(options.TokensPerPeriod, maxPermits - availablePermits);
                }
                else
                {
                    // All tokens available, nothing to do
                    return;
                }

                // Process queued requests
                Deque<RequestRegistration> queue = _queue;

                _tokenCount += resourcesToAdd;
                Debug.Assert(_tokenCount <= _options.TokenLimit);
                while (queue.Count > 0)
                {
                    RequestRegistration nextPendingRequest =
                          options.QueueProcessingOrder == QueueProcessingOrder.OldestFirst
                          ? queue.PeekHead()
                          : queue.PeekTail();

                    if (_tokenCount >= nextPendingRequest.Count)
                    {
                        // Request can be fulfilled
                        nextPendingRequest =
                            options.QueueProcessingOrder == QueueProcessingOrder.OldestFirst
                            ? queue.DequeueHead()
                            : queue.DequeueTail();

                        _queueCount -= nextPendingRequest.Count;
                        _tokenCount -= nextPendingRequest.Count;
                        Debug.Assert(_queueCount >= 0);
                        Debug.Assert(_tokenCount >= 0);

                        if (!nextPendingRequest.Tcs.TrySetResult(SuccessfulLease))
                        {
                            // Queued item was canceled so add count back
                            _tokenCount += nextPendingRequest.Count;
                        }
                        nextPendingRequest.CancellationTokenRegistration.Dispose();
                    }
                    else
                    {
                        // Request cannot be fulfilled
                        break;
                    }
                }
            }
        }

        protected override void Dispose(bool disposing)
        {
            if (!disposing)
            {
                return;
            }

            lock (Lock)
            {
                if (_disposed)
                {
                    return;
                }
                _disposed = true;
                _renewTimer?.Dispose();
                while (_queue.Count > 0)
                {
                    RequestRegistration next = _options.QueueProcessingOrder == QueueProcessingOrder.OldestFirst
                        ? _queue.DequeueHead()
                        : _queue.DequeueTail();
                    next.CancellationTokenRegistration.Dispose();
                    next.Tcs.SetResult(FailedLease);
                }
            }
        }

        protected override ValueTask DisposeAsyncCore()
        {
            Dispose(true);

            return default;
        }

        private void ThrowIfDisposed()
        {
            if (_disposed)
            {
                throw new ObjectDisposedException(nameof(TokenBucketRateLimiter));
            }
        }

        private sealed class TokenBucketLease : RateLimitLease
        {
            private static readonly string[] s_allMetadataNames = new[] { MetadataName.RetryAfter.Name };

            private readonly TimeSpan? _retryAfter;

            public TokenBucketLease(bool isAcquired, TimeSpan? retryAfter)
            {
                IsAcquired = isAcquired;
                _retryAfter = retryAfter;
            }

            public override bool IsAcquired { get; }

            public override IEnumerable<string> MetadataNames => s_allMetadataNames;

            public override bool TryGetMetadata(string metadataName, out object? metadata)
            {
                if (metadataName == MetadataName.RetryAfter.Name && _retryAfter.HasValue)
                {
                    metadata = _retryAfter.Value;
                    return true;
                }

                metadata = default;
                return false;
            }
        }

        private readonly struct RequestRegistration
        {
            public RequestRegistration(int tokenCount, TaskCompletionSource<RateLimitLease> tcs, CancellationTokenRegistration cancellationTokenRegistration)
            {
                Count = tokenCount;
                // Use VoidAsyncOperationWithData<T> instead
                Tcs = tcs;
                CancellationTokenRegistration = cancellationTokenRegistration;
            }

            public int Count { get; }

            public TaskCompletionSource<RateLimitLease> Tcs { get; }

            public CancellationTokenRegistration CancellationTokenRegistration { get; }

        }
    }
}
