﻿// <auto-generated/>
#nullable enable

// Suppress warnings about [Obsolete] member usage in generated code.
#pragma warning disable CS0618

namespace System.Text.Json.Tests.SourceGenRegressionTests.Net60
{
    public partial class Net60GeneratedContext
    {
        private global::System.Text.Json.Serialization.Metadata.JsonTypeInfo<global::System.Collections.Generic.List<global::System.DateTimeOffset>>? _ListDateTimeOffset;
        public global::System.Text.Json.Serialization.Metadata.JsonTypeInfo<global::System.Collections.Generic.List<global::System.DateTimeOffset>> ListDateTimeOffset
        {
            get
            {
                if (_ListDateTimeOffset == null)
                {
                    global::System.Text.Json.Serialization.JsonConverter? customConverter;
                    if (Options.Converters.Count > 0 && (customConverter = GetRuntimeProvidedCustomConverter(typeof(global::System.Collections.Generic.List<global::System.DateTimeOffset>))) != null)
                    {
                        _ListDateTimeOffset = global::System.Text.Json.Serialization.Metadata.JsonMetadataServices.CreateValueInfo<global::System.Collections.Generic.List<global::System.DateTimeOffset>>(Options, customConverter);
                    }
                    else
                    {
                        global::System.Text.Json.Serialization.Metadata.JsonCollectionInfoValues<global::System.Collections.Generic.List<global::System.DateTimeOffset>> info = new global::System.Text.Json.Serialization.Metadata.JsonCollectionInfoValues<global::System.Collections.Generic.List<global::System.DateTimeOffset>>()
                            {
                                ObjectCreator = () => new global::System.Collections.Generic.List<global::System.DateTimeOffset>(),
                                KeyInfo = null,
                                ElementInfo = this.DateTimeOffset,
                                NumberHandling = default,
                                SerializeHandler = ListDateTimeOffsetSerializeHandler
                            };
            
                            _ListDateTimeOffset = global::System.Text.Json.Serialization.Metadata.JsonMetadataServices.CreateListInfo<global::System.Collections.Generic.List<global::System.DateTimeOffset>, global::System.DateTimeOffset>(Options, info);
            
                    }
                }
        
                return _ListDateTimeOffset;
            }
        }
        
        private static void ListDateTimeOffsetSerializeHandler(global::System.Text.Json.Utf8JsonWriter writer, global::System.Collections.Generic.List<global::System.DateTimeOffset>? value)
        {
            if (value == null)
            {
                writer.WriteNullValue();
                return;
            }
        
            writer.WriteStartArray();
        
            for (int i = 0; i < value.Count; i++)
            {
                writer.WriteStringValue(value[i]);
            }
        
            writer.WriteEndArray();
        }
    }
}
