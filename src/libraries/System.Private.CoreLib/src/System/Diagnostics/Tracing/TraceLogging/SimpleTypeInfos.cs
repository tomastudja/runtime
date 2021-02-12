// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#if ES_BUILD_STANDALONE
using System;
using System.Diagnostics;
using System.Runtime.Serialization;
#endif
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.CompilerServices;

#if ES_BUILD_STANDALONE
namespace Microsoft.Diagnostics.Tracing
#else
namespace System.Diagnostics.Tracing
#endif
{
    /// <summary>
    /// TraceLogging: Type handler for empty or unsupported types.
    /// </summary>
    internal sealed class NullTypeInfo : TraceLoggingTypeInfo
    {
        public NullTypeInfo() : base(typeof(EmptyStruct)) { }

        public override void WriteMetadata(
            TraceLoggingMetadataCollector collector,
            string? name,
            EventFieldFormat format)
        {
            collector.AddGroup(name);
        }

        public override void WriteData(PropertyValue value)
        {
        }

        public override object? GetData(object? value)
        {
            return null;
        }
    }

    /// <summary>
    /// Type handler for simple scalar types.
    /// </summary>
    internal sealed class ScalarTypeInfo : TraceLoggingTypeInfo
    {
        private readonly Func<EventFieldFormat, TraceLoggingDataType, TraceLoggingDataType> formatFunc;
        private readonly TraceLoggingDataType nativeFormat;

        private ScalarTypeInfo(
            Type type,
            Func<EventFieldFormat, TraceLoggingDataType, TraceLoggingDataType> formatFunc,
            TraceLoggingDataType nativeFormat)
            : base(type)
        {
            this.formatFunc = formatFunc;
            this.nativeFormat = nativeFormat;
        }

        public override void WriteMetadata(TraceLoggingMetadataCollector collector, string? name, EventFieldFormat format)
        {
            collector.AddScalar(name!, formatFunc(format, nativeFormat));
        }

        public override void WriteData(PropertyValue value)
        {
            TraceLoggingDataCollector.AddScalar(value);
        }

        public static TraceLoggingTypeInfo Boolean() { return new ScalarTypeInfo(typeof(bool), Statics.Format8, TraceLoggingDataType.Boolean8); }
        public static TraceLoggingTypeInfo Byte() { return new ScalarTypeInfo(typeof(byte), Statics.Format8, TraceLoggingDataType.UInt8); }
        public static TraceLoggingTypeInfo SByte() { return new ScalarTypeInfo(typeof(sbyte), Statics.Format8, TraceLoggingDataType.Int8); }
        public static TraceLoggingTypeInfo Char() { return new ScalarTypeInfo(typeof(char), Statics.Format16, TraceLoggingDataType.Char16); }
        public static TraceLoggingTypeInfo Int16() { return new ScalarTypeInfo(typeof(short), Statics.Format16, TraceLoggingDataType.Int16); }
        public static TraceLoggingTypeInfo UInt16() { return new ScalarTypeInfo(typeof(ushort), Statics.Format16, TraceLoggingDataType.UInt16); }
        public static TraceLoggingTypeInfo Int32() { return new ScalarTypeInfo(typeof(int), Statics.Format32, TraceLoggingDataType.Int32); }
        public static TraceLoggingTypeInfo UInt32() { return new ScalarTypeInfo(typeof(uint), Statics.Format32, TraceLoggingDataType.UInt32); }
        public static TraceLoggingTypeInfo Int64() { return new ScalarTypeInfo(typeof(long), Statics.Format64, TraceLoggingDataType.Int64); }
        public static TraceLoggingTypeInfo UInt64() { return new ScalarTypeInfo(typeof(ulong), Statics.Format64, TraceLoggingDataType.UInt64); }
        public static TraceLoggingTypeInfo IntPtr() { return new ScalarTypeInfo(typeof(IntPtr), Statics.FormatPtr, Statics.IntPtrType); }
        public static TraceLoggingTypeInfo UIntPtr() { return new ScalarTypeInfo(typeof(UIntPtr), Statics.FormatPtr, Statics.UIntPtrType); }
        public static TraceLoggingTypeInfo Single() { return new ScalarTypeInfo(typeof(float), Statics.Format32, TraceLoggingDataType.Float); }
        public static TraceLoggingTypeInfo Double() { return new ScalarTypeInfo(typeof(double), Statics.Format64, TraceLoggingDataType.Double); }
        public static TraceLoggingTypeInfo Guid() { return new ScalarTypeInfo(typeof(Guid), (f, t) => Statics.MakeDataType(TraceLoggingDataType.Guid, f), TraceLoggingDataType.Guid); }
    }


    /// <summary>
    /// Type handler for arrays of scalars
    /// </summary>
    internal sealed class ScalarArrayTypeInfo : TraceLoggingTypeInfo
    {
        private readonly Func<EventFieldFormat, TraceLoggingDataType, TraceLoggingDataType> formatFunc;
        private readonly TraceLoggingDataType nativeFormat;
        private readonly int elementSize;

        private ScalarArrayTypeInfo(
            Type type,
            Func<EventFieldFormat, TraceLoggingDataType, TraceLoggingDataType> formatFunc,
            TraceLoggingDataType nativeFormat,
            int elementSize)
            : base(type)
        {
            this.formatFunc = formatFunc;
            this.nativeFormat = nativeFormat;
            this.elementSize = elementSize;
        }

        public override void WriteMetadata(TraceLoggingMetadataCollector collector, string? name, EventFieldFormat format)
        {
            collector.AddArray(name!, formatFunc(format, nativeFormat));
        }

        public override void WriteData(PropertyValue value)
        {
            TraceLoggingDataCollector.AddArray(value, elementSize);
        }

        public static TraceLoggingTypeInfo Boolean() { return new ScalarArrayTypeInfo(typeof(bool[]), Statics.Format8, TraceLoggingDataType.Boolean8, sizeof(bool)); }
        public static TraceLoggingTypeInfo Byte() { return new ScalarArrayTypeInfo(typeof(byte[]), Statics.Format8, TraceLoggingDataType.UInt8, sizeof(byte)); }
        public static TraceLoggingTypeInfo SByte() { return new ScalarArrayTypeInfo(typeof(sbyte[]), Statics.Format8, TraceLoggingDataType.Int8, sizeof(sbyte)); }
        public static TraceLoggingTypeInfo Char() { return new ScalarArrayTypeInfo(typeof(char[]), Statics.Format16, TraceLoggingDataType.Char16, sizeof(char)); }
        public static TraceLoggingTypeInfo Int16() { return new ScalarArrayTypeInfo(typeof(short[]), Statics.Format16, TraceLoggingDataType.Int16, sizeof(short)); }
        public static TraceLoggingTypeInfo UInt16() { return new ScalarArrayTypeInfo(typeof(ushort[]), Statics.Format16, TraceLoggingDataType.UInt16, sizeof(ushort)); }
        public static TraceLoggingTypeInfo Int32() { return new ScalarArrayTypeInfo(typeof(int[]), Statics.Format32, TraceLoggingDataType.Int32, sizeof(int)); }
        public static TraceLoggingTypeInfo UInt32() { return new ScalarArrayTypeInfo(typeof(uint[]), Statics.Format32, TraceLoggingDataType.UInt32, sizeof(uint)); }
        public static TraceLoggingTypeInfo Int64() { return new ScalarArrayTypeInfo(typeof(long[]), Statics.Format64, TraceLoggingDataType.Int64, sizeof(long)); }
        public static TraceLoggingTypeInfo UInt64() { return new ScalarArrayTypeInfo(typeof(ulong[]), Statics.Format64, TraceLoggingDataType.UInt64, sizeof(ulong)); }
        public static TraceLoggingTypeInfo IntPtr() { return new ScalarArrayTypeInfo(typeof(IntPtr[]), Statics.FormatPtr, Statics.IntPtrType, System.IntPtr.Size); }
        public static TraceLoggingTypeInfo UIntPtr() { return new ScalarArrayTypeInfo(typeof(UIntPtr[]), Statics.FormatPtr, Statics.UIntPtrType, System.IntPtr.Size); }
        public static TraceLoggingTypeInfo Single() { return new ScalarArrayTypeInfo(typeof(float[]), Statics.Format32, TraceLoggingDataType.Float, sizeof(float)); }
        public static TraceLoggingTypeInfo Double() { return new ScalarArrayTypeInfo(typeof(double[]), Statics.Format64, TraceLoggingDataType.Double, sizeof(double)); }
        public static unsafe TraceLoggingTypeInfo Guid() { return new ScalarArrayTypeInfo(typeof(Guid), (f, t) => Statics.MakeDataType(TraceLoggingDataType.Guid, f), TraceLoggingDataType.Guid, sizeof(Guid)); }
    }

    /// <summary>
    /// TraceLogging: Type handler for String.
    /// </summary>
    internal sealed class StringTypeInfo : TraceLoggingTypeInfo
    {
        public StringTypeInfo() : base(typeof(string)) { }

        public override void WriteMetadata(
            TraceLoggingMetadataCollector collector,
            string? name,
            EventFieldFormat format)
        {
            // name can be null if the string was used as a top-level object in an event.
            // In that case, use 'message' as the name of the field.
            name ??= "message";

            collector.AddNullTerminatedString(name, Statics.MakeDataType(TraceLoggingDataType.Utf16String, format));
        }

        public override void WriteData(PropertyValue value)
        {
            TraceLoggingDataCollector.AddNullTerminatedString((string?)value.ReferenceValue);
        }

        public override object GetData(object? value)
        {
            if (value == null)
            {
                return "";
            }

            return value;
        }
    }

    /// <summary>
    /// TraceLogging: Type handler for DateTime.
    /// </summary>
    internal sealed class DateTimeTypeInfo : TraceLoggingTypeInfo
    {
        public DateTimeTypeInfo() : base(typeof(DateTime)) { }

        public override void WriteMetadata(
            TraceLoggingMetadataCollector collector,
            string? name,
            EventFieldFormat format)
        {
            collector.AddScalar(name!, Statics.MakeDataType(TraceLoggingDataType.FileTime, format));
        }

        public override void WriteData(PropertyValue value)
        {
            DateTime dateTime = value.ScalarValue.AsDateTime;
            const long UTCMinTicks = 504911232000000000;
            long dateTimeTicks = 0;
            // We cannot translate dates sooner than 1/1/1601 in UTC.
            // To avoid getting an ArgumentOutOfRangeException we compare with 1/1/1601 DateTime ticks
            if (dateTime.Ticks > UTCMinTicks)
                dateTimeTicks = dateTime.ToFileTimeUtc();
            TraceLoggingDataCollector.AddScalar(dateTimeTicks);
        }
    }

    /// <summary>
    /// TraceLogging: Type handler for DateTimeOffset.
    /// </summary>
    internal sealed class DateTimeOffsetTypeInfo : TraceLoggingTypeInfo
    {
        public DateTimeOffsetTypeInfo() : base(typeof(DateTimeOffset)) { }

        public override void WriteMetadata(TraceLoggingMetadataCollector collector, string? name, EventFieldFormat format)
        {
            TraceLoggingMetadataCollector group = collector.AddGroup(name);
            group.AddScalar("Ticks", Statics.MakeDataType(TraceLoggingDataType.FileTime, format));
            group.AddScalar("Offset", TraceLoggingDataType.Int64);
        }

        public override void WriteData(PropertyValue value)
        {
            DateTimeOffset dateTimeOffset = value.ScalarValue.AsDateTimeOffset;
            long ticks = dateTimeOffset.Ticks;
            TraceLoggingDataCollector.AddScalar(ticks < 504911232000000000 ? 0 : ticks - 504911232000000000);
            TraceLoggingDataCollector.AddScalar(dateTimeOffset.Offset.Ticks);
        }
    }

    /// <summary>
    /// TraceLogging: Type handler for TimeSpan.
    /// </summary>
    internal sealed class TimeSpanTypeInfo : TraceLoggingTypeInfo
    {
        public TimeSpanTypeInfo() : base(typeof(TimeSpan)) { }

        public override void WriteMetadata(
            TraceLoggingMetadataCollector collector,
            string? name,
            EventFieldFormat format)
        {
            collector.AddScalar(name!, Statics.MakeDataType(TraceLoggingDataType.Int64, format));
        }

        public override void WriteData(PropertyValue value)
        {
            TraceLoggingDataCollector.AddScalar(value.ScalarValue.AsTimeSpan.Ticks);
        }
    }

    /// <summary>
    /// TraceLogging: Type handler for decimal. (Note: not full-fidelity, exposed as Double.)
    /// </summary>
    internal sealed class DecimalTypeInfo : TraceLoggingTypeInfo
    {
        public DecimalTypeInfo() : base(typeof(decimal)) { }

        public override void WriteMetadata(
            TraceLoggingMetadataCollector collector,
            string? name,
            EventFieldFormat format)
        {
            collector.AddScalar(name!, Statics.MakeDataType(TraceLoggingDataType.Double, format));
        }

        public override void WriteData(PropertyValue value)
        {
            TraceLoggingDataCollector.AddScalar((double)value.ScalarValue.AsDecimal);
        }
    }

    /// <summary>
    /// TraceLogging: Type handler for Nullable.
    /// </summary>
    internal sealed class NullableTypeInfo : TraceLoggingTypeInfo
    {
        private readonly TraceLoggingTypeInfo valueInfo;

        public NullableTypeInfo(Type type, List<Type> recursionCheck)
            : base(type)
        {
            Type[] typeArgs = type.GenericTypeArguments;
            Debug.Assert(typeArgs.Length == 1);
            this.valueInfo = TraceLoggingTypeInfo.GetInstance(typeArgs[0], recursionCheck);
        }

        public override void WriteMetadata(
            TraceLoggingMetadataCollector collector,
            string? name,
            EventFieldFormat format)
        {
            TraceLoggingMetadataCollector group = collector.AddGroup(name);
            group.AddScalar("HasValue", TraceLoggingDataType.Boolean8);
            this.valueInfo.WriteMetadata(group, "Value", format);
        }

#if !ES_BUILD_STANDALONE
        [UnconditionalSuppressMessage("ReflectionAnalysis", "IL2072:UnrecognizedReflectionPattern",
                Justification = "The underlying type of Nullable<T> must be defaultable")]
#endif
        public override void WriteData(PropertyValue value)
        {
            object? refVal = value.ReferenceValue;
            bool hasValue = refVal is not null;
            TraceLoggingDataCollector.AddScalar(hasValue);
            PropertyValue val = valueInfo.PropertyValueFactory(hasValue
                ? refVal
#if ES_BUILD_STANDALONE
                : FormatterServices.GetUninitializedObject(valueInfo.DataType));
 #else
                : RuntimeHelpers.GetUninitializedObject(valueInfo.DataType));
 #endif
            this.valueInfo.WriteData(val);
        }
    }
}
