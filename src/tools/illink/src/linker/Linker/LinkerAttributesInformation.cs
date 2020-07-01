﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using Mono.Cecil;

namespace Mono.Linker
{
	readonly struct LinkerAttributesInformation
	{
		readonly Dictionary<Type, List<Attribute>> _linkerAttributes;

		public LinkerAttributesInformation (LinkContext context, ICustomAttributeProvider provider, bool removable = false)
		{
			_linkerAttributes = null;
			if (context.CustomAttributes.HasCustomAttributes (provider)) {
				foreach (var customAttribute in context.CustomAttributes.GetCustomAttributes (provider)) {
					var attributeType = customAttribute.AttributeType;
					Attribute attributeValue = null;
					if (attributeType.IsTypeOf<RequiresUnreferencedCodeAttribute> ())
						attributeValue = ProcessRequiresUnreferencedCodeAttribute (context, provider, customAttribute);
					else if (attributeType.IsTypeOf<DynamicDependencyAttribute> ())
						attributeValue = DynamicDependency.ProcessAttribute (context, provider, customAttribute);
					AddAttribute (ref _linkerAttributes, attributeValue);
				}
			}
			if (removable)
				AddAttribute (ref _linkerAttributes, new LinkerRemovableAttribute ());
		}

		static void AddAttribute (ref Dictionary<Type, List<Attribute>> attributes, Attribute attributeValue)
		{
			if (attributeValue == null)
				return;

			if (attributes == null)
				attributes = new Dictionary<Type, List<Attribute>> ();

			Type attributeValueType = attributeValue.GetType ();
			if (!attributes.TryGetValue (attributeValueType, out var attributeList)) {
				attributeList = new List<Attribute> ();
				attributes.Add (attributeValueType, attributeList);
			}

			attributeList.Add (attributeValue);
		}

		public bool HasAttribute<T> () where T : Attribute
		{
			return _linkerAttributes != null && _linkerAttributes.ContainsKey (typeof (T));
		}

		public IEnumerable<T> GetAttributes<T> () where T : Attribute
		{
			if (_linkerAttributes == null || !_linkerAttributes.TryGetValue (typeof (T), out var attributeList))
				return Enumerable.Empty<T> ();

			if (attributeList == null || attributeList.Count == 0) {
				throw new LinkerFatalErrorException ("Unexpected list of attributes.");
			}

			return attributeList.Cast<T> ();
		}

		static Attribute ProcessRequiresUnreferencedCodeAttribute (LinkContext context, ICustomAttributeProvider provider, CustomAttribute customAttribute)
		{
			if (!(provider is MethodDefinition method))
				return null;

			if (customAttribute.HasConstructorArguments) {
				string message = (string) customAttribute.ConstructorArguments[0].Value;
				string url = null;
				foreach (var prop in customAttribute.Properties) {
					if (prop.Name == "Url") {
						url = (string) prop.Argument.Value;
					}
				}

				return new RequiresUnreferencedCodeAttribute (message) { Url = url };
			}

			context.LogWarning ($"Attribute '{typeof (RequiresUnreferencedCodeAttribute).FullName}' on '{method}' doesn't have a required constructor argument.", 2028, method);
			return null;
		}
	}
}
