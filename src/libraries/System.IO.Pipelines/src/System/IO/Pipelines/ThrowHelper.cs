// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.CompilerServices;
using System.Diagnostics.CodeAnalysis;

namespace System.IO.Pipelines
{
    internal static class ThrowHelper
    {
        [DoesNotReturn]
        internal static void ThrowArgumentOutOfRangeException(ExceptionArgument argument) => throw CreateArgumentOutOfRangeException(argument);
        [MethodImpl(MethodImplOptions.NoInlining)]
        private static Exception CreateArgumentOutOfRangeException(ExceptionArgument argument) => new ArgumentOutOfRangeException(argument.ToString());

        [DoesNotReturn]
        internal static void ThrowArgumentNullException(ExceptionArgument argument) => throw CreateArgumentNullException(argument);
        [MethodImpl(MethodImplOptions.NoInlining)]
        private static Exception CreateArgumentNullException(ExceptionArgument argument) => new ArgumentNullException(argument.ToString());

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_AlreadyReading() => throw CreateInvalidOperationException_AlreadyReading();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_AlreadyReading() => new InvalidOperationException(SR.ReadingIsInProgress);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_NoReadToComplete() => throw CreateInvalidOperationException_NoReadToComplete();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_NoReadToComplete() => new InvalidOperationException(SR.NoReadingOperationToComplete);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_NoConcurrentOperation() => throw CreateInvalidOperationException_NoConcurrentOperation();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_NoConcurrentOperation() => new InvalidOperationException(SR.ConcurrentOperationsNotSupported);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_GetResultNotCompleted() => throw CreateInvalidOperationException_GetResultNotCompleted();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_GetResultNotCompleted() => new InvalidOperationException(SR.GetResultBeforeCompleted);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_NoWritingAllowed() => throw CreateInvalidOperationException_NoWritingAllowed();

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_NoWritingAllowed() => new InvalidOperationException(SR.WritingAfterCompleted);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_NoReadingAllowed() => throw CreateInvalidOperationException_NoReadingAllowed();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_NoReadingAllowed() => new InvalidOperationException(SR.ReadingAfterCompleted);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_InvalidExaminedPosition() => throw CreateInvalidOperationException_InvalidExaminedPosition();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_InvalidExaminedPosition() => new InvalidOperationException(SR.InvalidExaminedPosition);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_InvalidExaminedOrConsumedPosition() => throw CreateInvalidOperationException_InvalidExaminedOrConsumedPosition();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_InvalidExaminedOrConsumedPosition() => new InvalidOperationException(SR.InvalidExaminedOrConsumedPosition);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_AdvanceToInvalidCursor() => throw CreateInvalidOperationException_AdvanceToInvalidCursor();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_AdvanceToInvalidCursor() => new InvalidOperationException(SR.AdvanceToInvalidCursor);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_ResetIncompleteReaderWriter() => throw CreateInvalidOperationException_ResetIncompleteReaderWriter();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_ResetIncompleteReaderWriter() => new InvalidOperationException(SR.ReaderAndWriterHasToBeCompleted);

        [DoesNotReturn]
        public static void ThrowOperationCanceledException_ReadCanceled() => throw CreateOperationCanceledException_ReadCanceled();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateOperationCanceledException_ReadCanceled() => new OperationCanceledException(SR.ReadCanceledOnPipeReader);

        [DoesNotReturn]
        public static void ThrowOperationCanceledException_FlushCanceled() => throw CreateOperationCanceledException_FlushCanceled();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateOperationCanceledException_FlushCanceled() => new OperationCanceledException(SR.FlushCanceledOnPipeWriter);

        [DoesNotReturn]
        public static void ThrowInvalidOperationException_InvalidZeroByteRead() => throw CreateInvalidOperationException_InvalidZeroByteRead();
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static Exception CreateInvalidOperationException_InvalidZeroByteRead() => new InvalidOperationException(SR.InvalidZeroByteRead);
    }

    internal enum ExceptionArgument
    {
        minimumSize,
        bytes,
        callback,
        options,
        pauseWriterThreshold,
        resumeWriterThreshold,
        sizeHint
    }
}
