// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

// =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// NullableDoubleAverageAggregationOperator.cs
//
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace System.Linq.Parallel
{
    /// <summary>
    /// An inlined average aggregation operator and its enumerator, for Nullable doubles.
    /// </summary>
    internal sealed class NullableDoubleAverageAggregationOperator : InlinedAggregationOperator<double?, Pair<double, long>, double?>
    {
        //---------------------------------------------------------------------------------------
        // Constructs a new instance of an average associative operator.
        //

        internal NullableDoubleAverageAggregationOperator(IEnumerable<double?> child) : base(child)
        {
        }

        //---------------------------------------------------------------------------------------
        // Executes the entire query tree, and aggregates the intermediate results into the
        // final result based on the binary operators and final reduction.
        //
        // Return Value:
        //     The single result of aggregation.
        //

        protected override double? InternalAggregate(ref Exception singularExceptionToThrow)
        {
            // Because the final reduction is typically much cheaper than the intermediate
            // reductions over the individual partitions, and because each parallel partition
            // will do a lot of work to produce a single output element, we prefer to turn off
            // pipelining, and process the final reductions serially.
            using (IEnumerator<Pair<double, long>> enumerator = GetEnumerator(ParallelMergeOptions.FullyBuffered, true))
            {
                // If the sequence was empty, return null right away.
                if (!enumerator.MoveNext())
                {
                    return null;
                }

                Pair<double, long> result = enumerator.Current;

                // Simply add together the sums and totals.
                while (enumerator.MoveNext())
                {
                    checked
                    {
                        result.First += enumerator.Current.First;
                        result.Second += enumerator.Current.Second;
                    }
                }

                // And divide the sum by the total to obtain the final result.
                return result.First / result.Second;
            }
        }

        //---------------------------------------------------------------------------------------
        // Creates an enumerator that is used internally for the final aggregation step.
        //

        protected override QueryOperatorEnumerator<Pair<double, long>, int> CreateEnumerator<TKey>(
            int index, int count, QueryOperatorEnumerator<double?, TKey> source, object sharedData, CancellationToken cancellationToken)
        {
            return new NullableDoubleAverageAggregationOperatorEnumerator<TKey>(source, index, cancellationToken);
        }

        //---------------------------------------------------------------------------------------
        // This enumerator type encapsulates the intermediary aggregation over the underlying
        // (possibly partitioned) data source.
        //

        private class NullableDoubleAverageAggregationOperatorEnumerator<TKey> : InlinedAggregationOperatorEnumerator<Pair<double, long>>
        {
            private readonly QueryOperatorEnumerator<double?, TKey> _source; // The source data.

            //---------------------------------------------------------------------------------------
            // Instantiates a new aggregation operator.
            //

            internal NullableDoubleAverageAggregationOperatorEnumerator(QueryOperatorEnumerator<double?, TKey> source, int partitionIndex,
                CancellationToken cancellationToken) :
                base(partitionIndex, cancellationToken)
            {
                Debug.Assert(source != null);
                _source = source;
            }

            //---------------------------------------------------------------------------------------
            // Tallies up the average of the underlying data source, walking the entire thing the first
            // time MoveNext is called on this object.
            //

            protected override bool MoveNextCore(ref Pair<double, long> currentElement)
            {
                // The temporary result contains the running sum and count, respectively.
                double sum = 0.0;
                long count = 0;

                QueryOperatorEnumerator<double?, TKey> source = _source;
                double? current = default(double?);
                TKey keyUnused = default(TKey);
                int i = 0;
                while (source.MoveNext(ref current, ref keyUnused))
                {
                    if (current.HasValue)
                    {
                        if ((i++ & CancellationState.POLL_INTERVAL) == 0)
                            CancellationState.ThrowIfCanceled(_cancellationToken);

                        checked
                        {
                            sum += current.GetValueOrDefault();
                            count++;
                        }
                    }
                }

                currentElement = new Pair<double, long>(sum, count);
                return count > 0;
            }

            //---------------------------------------------------------------------------------------
            // Dispose of resources associated with the underlying enumerator.
            //

            protected override void Dispose(bool disposing)
            {
                Debug.Assert(_source != null);
                _source.Dispose();
            }
        }
    }
}
