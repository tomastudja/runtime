// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text.Json.Serialization;

namespace System.Text.Json
{
    [DebuggerDisplay("ClassType.{ClassType}, {Type.Name}")]
    internal sealed partial class JsonClassInfo
    {
        public delegate object? ConstructorDelegate();

        public delegate T ParameterizedConstructorDelegate<T>(object[] arguments);

        public delegate T ParameterizedConstructorDelegate<T, TArg0, TArg1, TArg2, TArg3>(TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3);

        public ConstructorDelegate? CreateObject { get; private set; }

        public object? CreateObjectWithArgs { get; set; }

        public ClassType ClassType { get; private set; }

        public JsonPropertyInfo? DataExtensionProperty { get; private set; }

        // If enumerable, the JsonClassInfo for the element type.
        private JsonClassInfo? _elementClassInfo;

        /// <summary>
        /// Return the JsonClassInfo for the element type, or null if the type is not an enumerable or dictionary.
        /// </summary>
        /// <remarks>
        /// This should not be called during warm-up (initial creation of JsonClassInfos) to avoid recursive behavior
        /// which could result in a StackOverflowException.
        /// </remarks>
        public JsonClassInfo? ElementClassInfo
        {
            get
            {
                if (_elementClassInfo == null && ElementType != null)
                {
                    Debug.Assert(ClassType == ClassType.Enumerable ||
                        ClassType == ClassType.Dictionary);

                    _elementClassInfo = Options.GetOrAddClass(ElementType);
                }

                return _elementClassInfo;
            }
        }

        public Type? ElementType { get; set; }

        public JsonSerializerOptions Options { get; private set; }

        public Type Type { get; private set; }

        /// <summary>
        /// The JsonPropertyInfo for this JsonClassInfo. It is used to obtain the converter for the ClassInfo.
        /// </summary>
        /// <remarks>
        /// The returned JsonPropertyInfo does not represent a real property; instead it represents either:
        /// a collection element type,
        /// a generic type parameter,
        /// a property type (if pushed to a new stack frame),
        /// or the root type passed into the root serialization APIs.
        /// For example, for a property returning <see cref="Collections.Generic.List{T}"/> where T is a string,
        /// a JsonClassInfo will be created with .Type=typeof(string) and .PropertyInfoForClassInfo=JsonPropertyInfo{string}.
        /// Without this property, a "Converter" property would need to be added to JsonClassInfo and there would be several more
        /// `if` statements to obtain the converter from either the actual JsonPropertyInfo (for a real property) or from the
        /// ClassInfo (for the cases mentioned above). In addition, methods that have a JsonPropertyInfo argument would also likely
        /// need to add an argument for JsonClassInfo.
        /// </remarks>
        public JsonPropertyInfo PropertyInfoForClassInfo { get; private set; }

        public JsonClassInfo(Type type, JsonSerializerOptions options)
        {
            Type = type;
            Options = options;

            JsonConverter converter = GetConverter(
                Type,
                parentClassType: null, // A ClassInfo never has a "parent" class.
                memberInfo: null, // A ClassInfo never has a "parent" property.
                out Type runtimeType,
                Options);

            ClassType = converter.ClassType;
            PropertyInfoForClassInfo = CreatePropertyInfoForClassInfo(Type, runtimeType, converter, Options);

            switch (ClassType)
            {
                case ClassType.Object:
                    {
                        CreateObject = Options.MemberAccessorStrategy.CreateConstructor(type);
                        Dictionary<string, JsonPropertyInfo> cache = new Dictionary<string, JsonPropertyInfo>(
                            Options.PropertyNameCaseInsensitive
                                ? StringComparer.OrdinalIgnoreCase
                                : StringComparer.Ordinal);

                        Dictionary<string, MemberInfo>? ignoredMembers = null;

                        // We start from the most derived type.
                        for (Type? currentType = type; currentType != null; currentType = currentType.BaseType)
                        {
                            const BindingFlags bindingFlags =
                                BindingFlags.Instance |
                                BindingFlags.Public |
                                BindingFlags.NonPublic |
                                BindingFlags.DeclaredOnly;

                            foreach (PropertyInfo propertyInfo in currentType.GetProperties(bindingFlags))
                            {
                                // Ignore indexers and virtual properties that have overrides that were [JsonIgnore]d.
                                if (propertyInfo.GetIndexParameters().Length > 0 || PropertyIsOverridenAndIgnored(propertyInfo, ignoredMembers))
                                {
                                    continue;
                                }

                                // For now we only support public properties (i.e. setter and/or getter is public).
                                if (propertyInfo.GetMethod?.IsPublic == true ||
                                    propertyInfo.SetMethod?.IsPublic == true)
                                {
                                    CacheMember(currentType, propertyInfo.PropertyType, propertyInfo, cache, ref ignoredMembers);
                                }
                                else
                                {
                                    if (JsonPropertyInfo.GetAttribute<JsonIncludeAttribute>(propertyInfo) != null)
                                    {
                                        ThrowHelper.ThrowInvalidOperationException_JsonIncludeOnNonPublicInvalid(propertyInfo, currentType);
                                    }

                                    // Non-public properties should not be included for (de)serialization.
                                }
                            }

                            foreach (FieldInfo fieldInfo in currentType.GetFields(bindingFlags))
                            {
                                if (PropertyIsOverridenAndIgnored(fieldInfo, ignoredMembers))
                                {
                                    continue;
                                }

                                bool hasJsonInclude = JsonPropertyInfo.GetAttribute<JsonIncludeAttribute>(fieldInfo) != null;

                                if (fieldInfo.IsPublic)
                                {
                                    if (hasJsonInclude || Options.IncludeFields)
                                    {
                                        CacheMember(currentType, fieldInfo.FieldType, fieldInfo, cache, ref ignoredMembers);
                                    }
                                }
                                else
                                {
                                    if (hasJsonInclude)
                                    {
                                        ThrowHelper.ThrowInvalidOperationException_JsonIncludeOnNonPublicInvalid(fieldInfo, currentType);
                                    }

                                    // Non-public fields should not be included for (de)serialization.
                                }
                            }
                        }

                        JsonPropertyInfo[] cacheArray;
                        if (DetermineExtensionDataProperty(cache))
                        {
                            // Remove from cache since it is handled independently.
                            cache.Remove(DataExtensionProperty!.NameAsString);

                            cacheArray = new JsonPropertyInfo[cache.Count + 1];

                            // Set the last element to the extension property.
                            cacheArray[cache.Count] = DataExtensionProperty;
                        }
                        else
                        {
                            cacheArray = new JsonPropertyInfo[cache.Count];
                        }

                        // Copy the dictionary cache to the array cache.
                        cache.Values.CopyTo(cacheArray, 0);

                        // Set the array cache field at this point since it is completely initialized.
                        // It can now be safely accessed by other threads.
                        PropertyCacheArray = cacheArray;

                        // Allow constructor parameter logic to remove items from the dictionary since the JSON
                        // property values will be passed to the constructor and do not call a property setter.
                        if (converter.ConstructorIsParameterized)
                        {
                            InitializeConstructorParameters(cache, converter.ConstructorInfo!);
                        }

                        // Set the dictionary cache field at this point since it is completely initialized.
                        // It can now be safely accessed by other threads.
                        PropertyCache = cache;
                    }
                    break;
                case ClassType.Enumerable:
                case ClassType.Dictionary:
                    {
                        ElementType = converter.ElementType;
                        CreateObject = Options.MemberAccessorStrategy.CreateConstructor(runtimeType);
                    }
                    break;
                case ClassType.Value:
                case ClassType.NewValue:
                    {
                        CreateObject = Options.MemberAccessorStrategy.CreateConstructor(type);
                    }
                    break;
                case ClassType.None:
                    {
                        ThrowHelper.ThrowNotSupportedException_SerializationNotSupported(type);
                    }
                    break;
                default:
                    Debug.Fail($"Unexpected class type: {ClassType}");
                    throw new InvalidOperationException();
            }
        }

        private void CacheMember(
            Type declaringType,
            Type memberType,
            MemberInfo memberInfo,
            Dictionary<string, JsonPropertyInfo> cache,
            ref Dictionary<string, MemberInfo>? ignoredMembers)
        {
            JsonPropertyInfo jsonPropertyInfo = AddProperty(memberInfo, memberType, declaringType, Options);
            Debug.Assert(jsonPropertyInfo.NameAsString != null);

            string memberName = memberInfo.Name;

            // The JsonPropertyNameAttribute or naming policy resulted in a collision.
            if (!JsonHelpers.TryAdd(cache, jsonPropertyInfo.NameAsString, jsonPropertyInfo))
            {
                JsonPropertyInfo other = cache[jsonPropertyInfo.NameAsString];

                if (other.IsIgnored)
                {
                    // Overwrite previously cached property since it has [JsonIgnore].
                    cache[jsonPropertyInfo.NameAsString] = jsonPropertyInfo;
                }
                else if (
                    // Does the current property have `JsonIgnoreAttribute`?
                    !jsonPropertyInfo.IsIgnored &&
                    // Is the current property hidden by the previously cached property
                    // (with `new` keyword, or by overriding)?
                    other.MemberInfo!.Name != memberName &&
                    // Was a property with the same CLR name was ignored? That property hid the current property,
                    // thus, if it was ignored, the current property should be ignored too.
                    ignoredMembers?.ContainsKey(memberName) != true)
                {
                    // We throw if we have two public properties that have the same JSON property name, and neither have been ignored.
                    ThrowHelper.ThrowInvalidOperationException_SerializerPropertyNameConflict(Type, jsonPropertyInfo);
                }
                // Ignore the current property.
            }

            if (jsonPropertyInfo.IsIgnored)
            {
                (ignoredMembers ??= new Dictionary<string, MemberInfo>()).Add(memberName, memberInfo);
            }
        }

        private void InitializeConstructorParameters(Dictionary<string, JsonPropertyInfo> propertyCache, ConstructorInfo constructorInfo)
        {
            ParameterInfo[] parameters = constructorInfo!.GetParameters();
            Dictionary<string, JsonParameterInfo> parameterCache = new Dictionary<string, JsonParameterInfo>(
                parameters.Length, Options.PropertyNameCaseInsensitive ? StringComparer.OrdinalIgnoreCase : null);

            foreach (ParameterInfo parameterInfo in parameters)
            {
                MemberInfo? firstMatch = null;
                bool isBound = false;

                foreach (JsonPropertyInfo jsonPropertyInfo in PropertyCacheArray!)
                {
                    // This is not null because it is an actual
                    // property on a type, not a "policy property".
                    MemberInfo memberInfo = jsonPropertyInfo.MemberInfo!;
                    Debug.Assert(memberInfo is PropertyInfo || memberInfo is FieldInfo);

                    Type memberType = memberInfo is PropertyInfo propertyInfo
                        ? propertyInfo.PropertyType
                        : Unsafe.As<FieldInfo>(memberInfo).FieldType;

                    string camelCasePropName = JsonNamingPolicy.CamelCase.ConvertName(memberInfo.Name);

                    if (parameterInfo.Name == camelCasePropName &&
                        parameterInfo.ParameterType == memberType)
                    {
                        if (isBound)
                        {
                            Debug.Assert(firstMatch != null);

                            // Multiple object properties cannot bind to the same
                            // constructor parameter.
                            ThrowHelper.ThrowInvalidOperationException_MultiplePropertiesBindToConstructorParameters(
                                Type,
                                parameterInfo,
                                firstMatch,
                                memberInfo,
                                constructorInfo);
                        }

                        JsonParameterInfo jsonParameterInfo = AddConstructorParameter(parameterInfo, jsonPropertyInfo, Options);

                        // One object property cannot map to multiple constructor
                        // parameters (ConvertName above can't return multiple strings).
                        parameterCache.Add(jsonPropertyInfo.NameAsString, jsonParameterInfo);

                        // Remove property from deserialization cache to reduce the number of JsonPropertyInfos considered during JSON matching.
                        propertyCache.Remove(jsonPropertyInfo.NameAsString);

                        isBound = true;
                        firstMatch = memberInfo;
                    }
                }
            }

            // It is invalid for the extension data property to bind with a constructor argument.
            if (DataExtensionProperty != null &&
                parameterCache.ContainsKey(DataExtensionProperty.NameAsString))
            {
                ThrowHelper.ThrowInvalidOperationException_ExtensionDataCannotBindToCtorParam(DataExtensionProperty.MemberInfo!, Type, constructorInfo);
            }

            ParameterCache = parameterCache;
            ParameterCount = parameters.Length;

            PropertyCache = propertyCache;
        }

        private static bool PropertyIsOverridenAndIgnored(MemberInfo currentMember, Dictionary<string, MemberInfo>? ignoredMembers)
        {
            if (ignoredMembers == null || !ignoredMembers.TryGetValue(currentMember.Name, out MemberInfo? ignoredProperty))
            {
                return false;
            }

            Debug.Assert(currentMember is PropertyInfo || currentMember is FieldInfo);
            PropertyInfo? currentPropertyInfo = currentMember as PropertyInfo;
            Type currentMemberType = currentPropertyInfo == null
                ? Unsafe.As<FieldInfo>(currentMember).FieldType
                : currentPropertyInfo.PropertyType;

            Debug.Assert(ignoredProperty is PropertyInfo || ignoredProperty is FieldInfo);
            PropertyInfo? ignoredPropertyInfo = ignoredProperty as PropertyInfo;
            Type ignoredPropertyType = ignoredPropertyInfo == null
                ? Unsafe.As<FieldInfo>(ignoredProperty).FieldType
                : ignoredPropertyInfo.PropertyType;

            return currentMemberType == ignoredPropertyType &&
                PropertyIsVirtual(currentPropertyInfo) &&
                PropertyIsVirtual(ignoredPropertyInfo);
        }

        private static bool PropertyIsVirtual(PropertyInfo? propertyInfo)
        {
            return propertyInfo != null && (propertyInfo.GetMethod?.IsVirtual == true || propertyInfo.SetMethod?.IsVirtual == true);
        }

        public bool DetermineExtensionDataProperty(Dictionary<string, JsonPropertyInfo> cache)
        {
            JsonPropertyInfo? jsonPropertyInfo = GetPropertyWithUniqueAttribute(Type, typeof(JsonExtensionDataAttribute), cache);
            if (jsonPropertyInfo != null)
            {
                Type declaredPropertyType = jsonPropertyInfo.DeclaredPropertyType;
                if (typeof(IDictionary<string, object>).IsAssignableFrom(declaredPropertyType) ||
                    typeof(IDictionary<string, JsonElement>).IsAssignableFrom(declaredPropertyType))
                {
                    JsonConverter converter = Options.GetConverter(declaredPropertyType);
                    Debug.Assert(converter != null);
                }
                else
                {
                    ThrowHelper.ThrowInvalidOperationException_SerializationDataExtensionPropertyInvalid(Type, jsonPropertyInfo);
                }

                DataExtensionProperty = jsonPropertyInfo;
                return true;
            }

            return false;
        }

        private static JsonPropertyInfo? GetPropertyWithUniqueAttribute(Type classType, Type attributeType, Dictionary<string, JsonPropertyInfo> cache)
        {
            JsonPropertyInfo? property = null;

            foreach (JsonPropertyInfo jsonPropertyInfo in cache.Values)
            {
                Debug.Assert(jsonPropertyInfo.MemberInfo != null);
                Attribute? attribute = jsonPropertyInfo.MemberInfo.GetCustomAttribute(attributeType);
                if (attribute != null)
                {
                    if (property != null)
                    {
                        ThrowHelper.ThrowInvalidOperationException_SerializationDuplicateTypeAttribute(classType, attributeType);
                    }

                    property = jsonPropertyInfo;
                }
            }

            return property;
        }

        private static JsonParameterInfo AddConstructorParameter(
            ParameterInfo parameterInfo,
            JsonPropertyInfo jsonPropertyInfo,
            JsonSerializerOptions options)
        {
            if (jsonPropertyInfo.IsIgnored)
            {
                return JsonParameterInfo.CreateIgnoredParameterPlaceholder(jsonPropertyInfo);
            }

            JsonConverter converter = jsonPropertyInfo.ConverterBase;
            JsonParameterInfo jsonParameterInfo = converter.CreateJsonParameterInfo();

            jsonParameterInfo.Initialize(
                jsonPropertyInfo.RuntimePropertyType!,
                parameterInfo,
                jsonPropertyInfo,
                options);

            return jsonParameterInfo;
        }

        // This method gets the runtime information for a given type or property.
        // The runtime information consists of the following:
        // - class type,
        // - runtime type,
        // - element type (if the type is a collection),
        // - the converter (either native or custom), if one exists.
        public static JsonConverter GetConverter(
            Type type,
            Type? parentClassType,
            MemberInfo? memberInfo,
            out Type runtimeType,
            JsonSerializerOptions options)
        {
            Debug.Assert(type != null);
            ValidateType(type, parentClassType, memberInfo, options);

            JsonConverter converter = options.DetermineConverter(parentClassType, type, memberInfo);

            // The runtimeType is the actual value being assigned to the property.
            // There are three types to consider for the runtimeType:
            // 1) The declared type (the actual property type).
            // 2) The converter.TypeToConvert (the T value that the converter supports).
            // 3) The converter.RuntimeType (used with interfaces such as IList).

            Type converterRuntimeType = converter.RuntimeType;
            if (type == converterRuntimeType)
            {
                runtimeType = type;
            }
            else
            {
                if (type.IsInterface)
                {
                    runtimeType = converterRuntimeType;
                }
                else if (converterRuntimeType.IsInterface)
                {
                    runtimeType = type;
                }
                else
                {
                    // Use the most derived version from the converter.RuntimeType or converter.TypeToConvert.
                    if (type.IsAssignableFrom(converterRuntimeType))
                    {
                        runtimeType = converterRuntimeType;
                    }
                    else if (converterRuntimeType.IsAssignableFrom(type) || converter.TypeToConvert.IsAssignableFrom(type))
                    {
                        runtimeType = type;
                    }
                    else
                    {
                        runtimeType = default!;
                        ThrowHelper.ThrowNotSupportedException_SerializationNotSupported(type);
                    }
                }
            }

            Debug.Assert(!IsInvalidForSerialization(runtimeType));

            return converter;
        }

        private static void ValidateType(Type type, Type? parentClassType, MemberInfo? memberInfo, JsonSerializerOptions options)
        {
            if (!options.TypeIsCached(type) && IsInvalidForSerialization(type))
            {
                ThrowHelper.ThrowInvalidOperationException_CannotSerializeInvalidType(type, parentClassType, memberInfo);
            }
        }

        private static bool IsInvalidForSerialization(Type type)
        {
            return type.IsPointer || IsByRefLike(type) || type.ContainsGenericParameters;
        }

        private static bool IsByRefLike(Type type)
        {
#if BUILDING_INBOX_LIBRARY
            return type.IsByRefLike;
#else
            if (!type.IsValueType)
            {
                return false;
            }

            object[] attributes = type.GetCustomAttributes(inherit: false);

            for (int i = 0; i < attributes.Length; i++)
            {
                if (attributes[i].GetType().FullName == "System.Runtime.CompilerServices.IsByRefLikeAttribute")
                {
                    return true;
                }
            }

            return false;
#endif
        }
    }
}
