﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;
using Mono.Linker.Steps;

using BindingFlags = System.Reflection.BindingFlags;

namespace Mono.Linker.Dataflow
{
	class ReflectionMethodBodyScanner : MethodBodyScanner
	{
		readonly LinkContext _context;
		readonly MarkStep _markStep;
		readonly FlowAnnotations _flowAnnotations;

		public static bool RequiresReflectionMethodBodyScannerForCallSite (LinkContext context, FlowAnnotations flowAnnotations, MethodReference calledMethod)
		{
			MethodDefinition methodDefinition = calledMethod.Resolve ();
			if (methodDefinition != null) {
				return
					GetIntrinsicIdForMethod (methodDefinition) > IntrinsicId.RequiresReflectionBodyScanner_Sentinel ||
					flowAnnotations.RequiresDataFlowAnalysis (methodDefinition) ||
					context.Annotations.HasLinkerAttribute<RequiresUnreferencedCodeAttribute> (methodDefinition);
			}

			return false;
		}

		public static bool RequiresReflectionMethodBodyScannerForMethodBody (FlowAnnotations flowAnnotations, MethodReference method)
		{
			MethodDefinition methodDefinition = method.Resolve ();
			if (methodDefinition != null) {
				return
					GetIntrinsicIdForMethod (methodDefinition) > IntrinsicId.RequiresReflectionBodyScanner_Sentinel ||
					flowAnnotations.RequiresDataFlowAnalysis (methodDefinition);
			}

			return false;
		}

		public static bool RequiresReflectionMethodBodyScannerForAccess (FlowAnnotations flowAnnotations, FieldReference field)
		{
			FieldDefinition fieldDefinition = field.Resolve ();
			if (fieldDefinition != null)
				return flowAnnotations.RequiresDataFlowAnalysis (fieldDefinition);

			return false;
		}

		public static bool AutomaticallySuppressReflectionMethodBodyScannerForMethod (LinkContext context, MethodReference method)
		{
			MethodDefinition methodDefinition = method.Resolve ();
			if (methodDefinition != null) {
				return context.Annotations.HasLinkerAttribute<RequiresUnreferencedCodeAttribute> (methodDefinition);
			}

			return false;
		}

		public ReflectionMethodBodyScanner (LinkContext context, MarkStep parent, FlowAnnotations flowAnnotations)
		{
			_context = context;
			_markStep = parent;
			_flowAnnotations = flowAnnotations;
		}

		public void ScanAndProcessReturnValue (MethodBody methodBody)
		{
			Scan (methodBody);

			if (MethodReturnValue != null) {
				var requiredMemberKinds = _flowAnnotations.GetReturnParameterAnnotation (methodBody.Method);
				if (requiredMemberKinds != 0) {
					var reflectionContext = new ReflectionPatternContext (_context, methodBody.Method, methodBody.Method.MethodReturnType);
					reflectionContext.AnalyzingPattern ();
					RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, MethodReturnValue, methodBody.Method.MethodReturnType);
				}
			}
		}

		public void ProcessAttributeDataflow (MethodDefinition source, MethodDefinition method, IList<CustomAttributeArgument> arguments)
		{
			int paramOffset = method.HasImplicitThis () ? 1 : 0;

			for (int i = 0; i < method.Parameters.Count; i++) {
				var annotation = _flowAnnotations.GetParameterAnnotation (method, i + paramOffset);
				if (annotation != 0) {
					ValueNode valueNode = GetValueNodeForCustomAttributeArgument (arguments[i]);
					if (valueNode != null) {
						ReflectionPatternContext context = new ReflectionPatternContext (_context, source, method.Parameters[i]);
						context.AnalyzingPattern ();
						RequireDynamicallyAccessedMembers (ref context, annotation, valueNode, method);
					}
				}
			}
		}

		public void ProcessAttributeDataflow (FieldDefinition field, CustomAttributeArgument value)
		{
			var annotation = _flowAnnotations.GetFieldAnnotation (field);
			Debug.Assert (annotation != 0);

			ValueNode valueNode = GetValueNodeForCustomAttributeArgument (value);
			if (valueNode != null) {
				var reflectionContext = new ReflectionPatternContext (_context, field.DeclaringType.Methods[0], value.Value as IMemberDefinition);
				reflectionContext.AnalyzingPattern ();
				RequireDynamicallyAccessedMembers (ref reflectionContext, annotation, valueNode, field);
			}
		}

		ValueNode GetValueNodeForCustomAttributeArgument (CustomAttributeArgument argument)
		{
			ValueNode valueNode;
			if (argument.Type.Name == "Type") {
				TypeDefinition referencedType = ((TypeReference) argument.Value).Resolve ();
				valueNode = referencedType == null ? null : new SystemTypeValue (referencedType);
			} else if (argument.Type.MetadataType == MetadataType.String) {
				valueNode = new KnownStringValue ((string) argument.Value);
			} else {
				// We shouldn't have gotten a non-null annotation for this from GetParameterAnnotation
				throw new InvalidOperationException ();
			}

			return valueNode;
		}

		protected override void WarnAboutInvalidILInMethod (MethodBody method, int ilOffset)
		{
			// TODO: remove once we're ready to scan actual invalid IL
			// Serves as a debug helper for now to make sure valid IL is not considered invalid.
			throw new Exception ();
		}

		protected override ValueNode GetMethodParameterValue (MethodDefinition method, int parameterIndex)
		{
			DynamicallyAccessedMemberTypes memberKinds = _flowAnnotations.GetParameterAnnotation (method, parameterIndex);
			return new MethodParameterValue (parameterIndex, memberKinds) {
				SourceContext = method
			};
		}

		protected override ValueNode GetFieldValue (MethodDefinition method, FieldDefinition field)
		{
			DynamicallyAccessedMemberTypes memberKinds = _flowAnnotations.GetFieldAnnotation (field);
			return new LoadFieldValue (field, memberKinds) {
				SourceContext = method
			};
		}

		protected override void HandleStoreField (MethodDefinition method, FieldDefinition field, Instruction operation, ValueNode valueToStore)
		{
			var requiredMemberKinds = _flowAnnotations.GetFieldAnnotation (field);
			if (requiredMemberKinds != 0) {
				var reflectionContext = new ReflectionPatternContext (_context, method, field, operation);
				reflectionContext.AnalyzingPattern ();
				RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, valueToStore, field);
			}
		}

		protected override void HandleStoreParameter (MethodDefinition method, int index, Instruction operation, ValueNode valueToStore)
		{
			var requiredMemberKinds = _flowAnnotations.GetParameterAnnotation (method, index);
			if (requiredMemberKinds != 0) {
				ParameterDefinition parameter = method.Parameters[index - (method.HasImplicitThis () ? 1 : 0)];
				var reflectionContext = new ReflectionPatternContext (_context, method, parameter, operation);
				reflectionContext.AnalyzingPattern ();
				RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, valueToStore, parameter);
			}
		}

		enum IntrinsicId
		{
			None = 0,
			IntrospectionExtensions_GetTypeInfo,
			Type_GetTypeFromHandle,
			Type_get_TypeHandle,
			Object_GetType,
			TypeDelegator_Ctor,

			// Anything above this marker will require the method to be run through
			// the reflection body scanner.
			RequiresReflectionBodyScanner_Sentinel = 1000,
			Type_MakeGenericType,
			Type_GetType,
			Type_GetConstructor,
			Type_GetMethod,
			Type_GetField,
			Type_GetProperty,
			Type_GetEvent,
			Type_GetNestedType,
			Expression_Call,
			Expression_Field,
			Expression_Property,
			Expression_New,
			Activator_CreateInstance_Type,
			Activator_CreateInstance_AssemblyName_TypeName,
			Activator_CreateInstanceFrom,
			Activator_CreateInstanceOfT,
			AppDomain_CreateInstance,
			AppDomain_CreateInstanceAndUnwrap,
			AppDomain_CreateInstanceFrom,
			AppDomain_CreateInstanceFromAndUnwrap,
			Assembly_CreateInstance,
			RuntimeReflectionExtensions_GetRuntimeEvent,
			RuntimeReflectionExtensions_GetRuntimeField,
			RuntimeReflectionExtensions_GetRuntimeMethod,
			RuntimeReflectionExtensions_GetRuntimeProperty,
			RuntimeHelpers_RunClassConstructor
		}

		static IntrinsicId GetIntrinsicIdForMethod (MethodDefinition calledMethod)
		{
			return calledMethod.Name switch
			{
				// static System.Reflection.IntrospectionExtensions.GetTypeInfo (Type type)
				"GetTypeInfo" when calledMethod.IsDeclaredOnType ("System.Reflection", "IntrospectionExtensions") => IntrinsicId.IntrospectionExtensions_GetTypeInfo,

				// System.Type.GetTypeInfo (Type type)
				"GetTypeFromHandle" when calledMethod.IsDeclaredOnType ("System", "Type") => IntrinsicId.Type_GetTypeFromHandle,

				// System.Type.GetTypeHandle (Type type)
				"get_TypeHandle" when calledMethod.IsDeclaredOnType ("System", "Type") => IntrinsicId.Type_get_TypeHandle,

				// static System.Type.MakeGenericType (Type [] typeArguments)
				"MakeGenericType" when calledMethod.IsDeclaredOnType ("System", "Type") => IntrinsicId.Type_MakeGenericType,

				// static System.Reflection.RuntimeReflectionExtensions.GetRuntimeEvent (this Type type, string name)
				"GetRuntimeEvent" when calledMethod.IsDeclaredOnType ("System.Reflection", "RuntimeReflectionExtensions")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.RuntimeReflectionExtensions_GetRuntimeEvent,

				// static System.Reflection.RuntimeReflectionExtensions.GetRuntimeField (this Type type, string name)
				"GetRuntimeField" when calledMethod.IsDeclaredOnType ("System.Reflection", "RuntimeReflectionExtensions")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.RuntimeReflectionExtensions_GetRuntimeField,

				// static System.Reflection.RuntimeReflectionExtensions.GetRuntimeMethod (this Type type, string name, Type[] parameters)
				"GetRuntimeMethod" when calledMethod.IsDeclaredOnType ("System.Reflection", "RuntimeReflectionExtensions")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.RuntimeReflectionExtensions_GetRuntimeMethod,

				// static System.Reflection.RuntimeReflectionExtensions.GetRuntimeProperty (this Type type, string name)
				"GetRuntimeProperty" when calledMethod.IsDeclaredOnType ("System.Reflection", "RuntimeReflectionExtensions")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.RuntimeReflectionExtensions_GetRuntimeProperty,

				// static System.Linq.Expressions.Expression.Call (Type, String, Type[], Expression[])
				"Call" when calledMethod.IsDeclaredOnType ("System.Linq.Expressions", "Expression")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					&& calledMethod.Parameters.Count == 4
					=> IntrinsicId.Expression_Call,

				// static System.Linq.Expressions.Expression.Field (Expression, Type, String)
				"Field" when calledMethod.IsDeclaredOnType ("System.Linq.Expressions", "Expression")
					&& calledMethod.HasParameterOfType (1, "System", "Type")
					&& calledMethod.Parameters.Count == 3
					=> IntrinsicId.Expression_Field,

				// static System.Linq.Expressions.Expression.Property (Expression, Type, String)
				"Property" when calledMethod.IsDeclaredOnType ("System.Linq.Expressions", "Expression")
					&& calledMethod.HasParameterOfType (1, "System", "Type")
					&& calledMethod.Parameters.Count == 3
					=> IntrinsicId.Expression_Property,

				// static System.Linq.Expressions.Expression.New (Type)
				"New" when calledMethod.IsDeclaredOnType ("System.Linq.Expressions", "Expression")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					&& calledMethod.Parameters.Count == 1
					=> IntrinsicId.Expression_New,

				// static System.Type.GetType (string)
				// static System.Type.GetType (string, Boolean)
				// static System.Type.GetType (string, Boolean, Boolean)
				// static System.Type.GetType (string, Func<AssemblyName, Assembly>, Func<Assembly, String, Boolean, Type>)
				// static System.Type.GetType (string, Func<AssemblyName, Assembly>, Func<Assembly, String, Boolean, Type>, Boolean)
				// static System.Type.GetType (string, Func<AssemblyName, Assembly>, Func<Assembly, String, Boolean, Type>, Boolean, Boolean)
				"GetType" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					=> IntrinsicId.Type_GetType,

				// System.Type.GetConstructor (Type[])
				// System.Type.GetConstructor (BindingFlags, Binder, Type[], ParameterModifier [])
				// System.Type.GetConstructor (BindingFlags, Binder, CallingConventions, Type[], ParameterModifier [])
				"GetConstructor" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasThis
					=> IntrinsicId.Type_GetConstructor,

				// System.Type.GetMethod (string)
				// System.Type.GetMethod (string, BindingFlags)
				// System.Type.GetMethod (string, Type[])
				// System.Type.GetMethod (string, Type[], ParameterModifier[])
				// System.Type.GetMethod (string, BindingFlags, Binder, Type[], ParameterModifier[]) 6
				// System.Type.GetMethod (string, BindingFlags, Binder, CallingConventions, Type[], ParameterModifier[]) 7
				// System.Type.GetMethod (string, int, Type[])
				// System.Type.GetMethod (string, int, Type[], ParameterModifier[]?)
				// System.Type.GetMethod (string, int, BindingFlags, Binder?, Type[], ParameterModifier[]?)
				// System.Type.GetMethod (string, int, BindingFlags, Binder?, CallingConventions, Type[], ParameterModifier[]?)
				"GetMethod" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasThis
					=> IntrinsicId.Type_GetMethod,

				// System.Type.GetField (string)
				// System.Type.GetField (string, BindingFlags)
				"GetField" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasThis
					=> IntrinsicId.Type_GetField,

				// System.Type.GetEvent (string)
				// System.Type.GetEvent (string, BindingFlags)
				"GetEvent" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasThis
					=> IntrinsicId.Type_GetEvent,

				// System.Type.GetNestedType (string)
				// System.Type.GetNestedType (string, BindingFlags)
				"GetNestedType" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasThis
					=> IntrinsicId.Type_GetNestedType,

				// System.Type.GetProperty (string)
				// System.Type.GetProperty (string, BindingFlags)
				// System.Type.GetProperty (string, Type)
				// System.Type.GetProperty (string, Type[])
				// System.Type.GetProperty (string, Type, Type[])
				// System.Type.GetProperty (string, Type, Type[], ParameterModifier[])
				// System.Type.GetProperty (string, BindingFlags, Binder, Type, Type[], ParameterModifier[])
				"GetProperty" when calledMethod.IsDeclaredOnType ("System", "Type")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasThis
					=> IntrinsicId.Type_GetProperty,

				// static System.Object.GetType ()
				"GetType" when calledMethod.IsDeclaredOnType ("System", "Object")
					=> IntrinsicId.Object_GetType,

				".ctor" when calledMethod.IsDeclaredOnType ("System.Reflection", "TypeDelegator")
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					=> IntrinsicId.TypeDelegator_Ctor,

				// static System.Activator.CreateInstance (System.Type type)
				// static System.Activator.CreateInstance (System.Type type, bool nonPublic)
				// static System.Activator.CreateInstance (System.Type type, params object?[]? args)
				// static System.Activator.CreateInstance (System.Type type, object?[]? args, object?[]? activationAttributes)
				// static System.Activator.CreateInstance (System.Type type, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object?[]? args, System.Globalization.CultureInfo? culture)
				// static System.Activator.CreateInstance (System.Type type, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object?[]? args, System.Globalization.CultureInfo? culture, object?[]? activationAttributes) { throw null; }
				"CreateInstance" when calledMethod.IsDeclaredOnType ("System", "Activator")
					&& !calledMethod.ContainsGenericParameter
					&& calledMethod.HasParameterOfType (0, "System", "Type")
					=> IntrinsicId.Activator_CreateInstance_Type,

				// static System.Activator.CreateInstance (string assemblyName, string typeName)
				// static System.Activator.CreateInstance (string assemblyName, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object?[]? args, System.Globalization.CultureInfo? culture, object?[]? activationAttributes)
				// static System.Activator.CreateInstance (string assemblyName, string typeName, object?[]? activationAttributes)
				"CreateInstance" when calledMethod.IsDeclaredOnType ("System", "Activator")
					&& !calledMethod.ContainsGenericParameter
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.Activator_CreateInstance_AssemblyName_TypeName,

				// static System.Activator.CreateInstanceFrom (string assemblyFile, string typeName)
				// static System.Activator.CreateInstanceFrom (string assemblyFile, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// static System.Activator.CreateInstanceFrom (string assemblyFile, string typeName, object? []? activationAttributes)
				"CreateInstanceFrom" when calledMethod.IsDeclaredOnType ("System", "Activator")
					&& !calledMethod.ContainsGenericParameter
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.Activator_CreateInstanceFrom,

				// static T System.Activator.CreateInstance<T> ()
				"CreateInstance" when calledMethod.IsDeclaredOnType ("System", "Activator")
					&& calledMethod.ContainsGenericParameter
					&& calledMethod.Parameters.Count == 0
					=> IntrinsicId.Activator_CreateInstanceOfT,

				// System.AppDomain.CreateInstance (string assemblyName, string typeName)
				// System.AppDomain.CreateInstance (string assemblyName, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// System.AppDomain.CreateInstance (string assemblyName, string typeName, object? []? activationAttributes)
				"CreateInstance" when calledMethod.IsDeclaredOnType ("System", "AppDomain")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.AppDomain_CreateInstance,

				// System.AppDomain.CreateInstanceAndUnwrap (string assemblyName, string typeName)
				// System.AppDomain.CreateInstanceAndUnwrap (string assemblyName, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// System.AppDomain.CreateInstanceAndUnwrap (string assemblyName, string typeName, object? []? activationAttributes)
				"CreateInstanceAndUnwrap" when calledMethod.IsDeclaredOnType ("System", "AppDomain")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.AppDomain_CreateInstanceAndUnwrap,

				// System.AppDomain.CreateInstanceFrom (string assemblyFile, string typeName)
				// System.AppDomain.CreateInstanceFrom (string assemblyFile, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// System.AppDomain.CreateInstanceFrom (string assemblyFile, string typeName, object? []? activationAttributes)
				"CreateInstanceFrom" when calledMethod.IsDeclaredOnType ("System", "AppDomain")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.AppDomain_CreateInstanceFrom,

				// System.AppDomain.CreateInstanceFromAndUnwrap (string assemblyFile, string typeName)
				// System.AppDomain.CreateInstanceFromAndUnwrap (string assemblyFile, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// System.AppDomain.CreateInstanceFromAndUnwrap (string assemblyFile, string typeName, object? []? activationAttributes)
				"CreateInstanceFromAndUnwrap" when calledMethod.IsDeclaredOnType ("System", "AppDomain")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					&& calledMethod.HasParameterOfType (1, "System", "String")
					=> IntrinsicId.AppDomain_CreateInstanceFromAndUnwrap,

				// System.Reflection.Assembly.CreateInstance (string typeName)
				// System.Reflection.Assembly.CreateInstance (string typeName, bool ignoreCase)
				// System.Reflection.Assembly.CreateInstance (string typeName, bool ignoreCase, BindingFlags bindingAttr, Binder? binder, object []? args, CultureInfo? culture, object []? activationAttributes)
				"CreateInstance" when calledMethod.IsDeclaredOnType ("System.Reflection", "Assembly")
					&& calledMethod.HasParameterOfType (0, "System", "String")
					=> IntrinsicId.Assembly_CreateInstance,

				// System.Runtime.CompilerServices.RuntimeHelpers.RunClassConstructor (RuntimeTypeHandle type)
				"RunClassConstructor" when calledMethod.IsDeclaredOnType ("System.Runtime.CompilerServices", "RuntimeHelpers")
					&& calledMethod.HasParameterOfType (0, "System", "RuntimeTypeHandle")
					=> IntrinsicId.RuntimeHelpers_RunClassConstructor,

				_ => IntrinsicId.None,
			};
		}

		public override bool HandleCall (MethodBody callingMethodBody, MethodReference calledMethod, Instruction operation, ValueNodeList methodParams, out ValueNode methodReturnValue)
		{
			var reflectionContext = new ReflectionPatternContext (_context, callingMethodBody.Method, calledMethod.Resolve (), operation);

			DynamicallyAccessedMemberTypes returnValueDynamicallyAccessedMemberKinds = 0;

			methodReturnValue = null;

			var calledMethodDefinition = calledMethod.Resolve ();
			if (calledMethodDefinition == null)
				return false;

			try {

				bool requiresDataFlowAnalysis = _flowAnnotations.RequiresDataFlowAnalysis (calledMethodDefinition);
				returnValueDynamicallyAccessedMemberKinds = requiresDataFlowAnalysis ?
					_flowAnnotations.GetReturnParameterAnnotation (calledMethodDefinition) : 0;

				switch (GetIntrinsicIdForMethod (calledMethodDefinition)) {
				case IntrinsicId.IntrospectionExtensions_GetTypeInfo: {
						// typeof(Foo).GetTypeInfo()... will be commonly present in code targeting
						// the dead-end reflection refactoring. The call doesn't do anything and we
						// don't want to lose the annotation.
						methodReturnValue = methodParams[0];
					}
					break;

				case IntrinsicId.TypeDelegator_Ctor: {
						// This is an identity function for analysis purposes
						methodReturnValue = methodParams[1];
					}
					break;

				case IntrinsicId.Type_GetTypeFromHandle: {
						// Infrastructure piece to support "typeof(Foo)"
						if (methodParams[0] is RuntimeTypeHandleValue typeHandle)
							methodReturnValue = new SystemTypeValue (typeHandle.TypeRepresented);
					}
					break;

				case IntrinsicId.Type_get_TypeHandle: {
						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue typeValue)
								methodReturnValue = MergePointValue.MergeValues (methodReturnValue, new RuntimeTypeHandleValue (typeValue.TypeRepresented));
							else if (value == NullValue.Instance)
								methodReturnValue = MergePointValue.MergeValues (methodReturnValue, value);
							else
								methodReturnValue = MergePointValue.MergeValues (methodReturnValue, UnknownValue.Instance);
						}
					}
					break;

				case IntrinsicId.Type_MakeGenericType: {
						// Don't care about the actual arguments, but we don't want to lose track of the type
						// in case this is e.g. Activator.CreateInstance(typeof(Foo<>).MakeGenericType(...));
						methodReturnValue = methodParams[0];
					}
					break;

				//
				// System.Reflection.RuntimeReflectionExtensions
				//
				// static GetRuntimeEvent (this Type type, string name)
				// static GetRuntimeField (this Type type, string name)
				// static GetRuntimeMethod (this Type type, string name, Type[] parameters)
				// static GetRuntimeProperty (this Type type, string name)
				//
				case var getRuntimeMember when getRuntimeMember == IntrinsicId.RuntimeReflectionExtensions_GetRuntimeEvent
					|| getRuntimeMember == IntrinsicId.RuntimeReflectionExtensions_GetRuntimeField
					|| getRuntimeMember == IntrinsicId.RuntimeReflectionExtensions_GetRuntimeMethod
					|| getRuntimeMember == IntrinsicId.RuntimeReflectionExtensions_GetRuntimeProperty: {

						reflectionContext.AnalyzingPattern ();
						BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public;
						DynamicallyAccessedMemberTypes requiredMemberTypes = getRuntimeMember switch
						{
							IntrinsicId.RuntimeReflectionExtensions_GetRuntimeEvent => DynamicallyAccessedMemberTypes.PublicEvents,
							IntrinsicId.RuntimeReflectionExtensions_GetRuntimeField => DynamicallyAccessedMemberTypes.PublicFields,
							IntrinsicId.RuntimeReflectionExtensions_GetRuntimeMethod => DynamicallyAccessedMemberTypes.PublicMethods,
							IntrinsicId.RuntimeReflectionExtensions_GetRuntimeProperty => DynamicallyAccessedMemberTypes.PublicProperties,
							_ => throw new InternalErrorException ($"Reflection call '{calledMethod.FullName}' inside '{callingMethodBody.Method.FullName}' is of unexpected member type."),
						};

						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								foreach (var stringParam in methodParams[1].UniqueValues ()) {
									if (stringParam is KnownStringValue stringValue) {
										switch (getRuntimeMember) {
										case IntrinsicId.RuntimeReflectionExtensions_GetRuntimeEvent:
											MarkEventsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, e => e.Name == stringValue.Contents, bindingFlags);
											reflectionContext.RecordHandledPattern ();
											break;
										case IntrinsicId.RuntimeReflectionExtensions_GetRuntimeField:
											MarkFieldsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, f => f.Name == stringValue.Contents, bindingFlags);
											reflectionContext.RecordHandledPattern ();
											break;
										case IntrinsicId.RuntimeReflectionExtensions_GetRuntimeMethod:
											MarkMethodsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, m => m.Name == stringValue.Contents, bindingFlags);
											reflectionContext.RecordHandledPattern ();
											break;
										case IntrinsicId.RuntimeReflectionExtensions_GetRuntimeProperty:
											MarkPropertiesOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, p => p.Name == stringValue.Contents, bindingFlags);
											reflectionContext.RecordHandledPattern ();
											break;
										default:
											throw new InternalErrorException ($"Error processing reflection call '{calledMethod.FullName}' inside {callingMethodBody.Method.FullName}. Unexpected member kind.");
										}
									} else {
										RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberTypes, value, calledMethod.Parameters[0]);
									}
								}
							} else {
								RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberTypes, value, calledMethod.Parameters[0]);
							}
						}
					}
					break;
				//
				// System.Linq.Expressions.Expression
				// 
				// static Call (Type, String, Type[], Expression[])
				//
				case IntrinsicId.Expression_Call: {
						reflectionContext.AnalyzingPattern ();
						BindingFlags bindingFlags = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy;

						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								foreach (var stringParam in methodParams[1].UniqueValues ()) {
									// TODO: Change this as needed after deciding whether or not we are to keep
									// all methods on a type that was accessed via reflection.
									if (stringParam is KnownStringValue stringValue) {
										MarkMethodsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, m => m.Name == stringValue.Contents, bindingFlags);
										reflectionContext.RecordHandledPattern ();
									} else {
										RequireDynamicallyAccessedMembers (
											ref reflectionContext,
											GetDynamicallyAccessedMemberTypesFromBindingFlagsForMethods (bindingFlags),
											value,
											calledMethod.Parameters[0]);
									}
								}
							} else {
								RequireDynamicallyAccessedMembers (
									ref reflectionContext,
									GetDynamicallyAccessedMemberTypesFromBindingFlagsForMethods (bindingFlags),
									value,
									calledMethod.Parameters[0]);
							}
						}
					}
					break;

				//
				// System.Linq.Expressions.Expression
				// 
				// static Field (Expression, Type, String)
				// static Property (Expression, Type, String)
				//
				case var fieldOrPropertyInstrinsic when fieldOrPropertyInstrinsic == IntrinsicId.Expression_Field || fieldOrPropertyInstrinsic == IntrinsicId.Expression_Property: {
						reflectionContext.AnalyzingPattern ();
						DynamicallyAccessedMemberTypes memberKind = fieldOrPropertyInstrinsic == IntrinsicId.Expression_Property
							? DynamicallyAccessedMemberTypes.PublicProperties | DynamicallyAccessedMemberTypes.NonPublicProperties
							: DynamicallyAccessedMemberTypes.PublicFields | DynamicallyAccessedMemberTypes.NonPublicFields;

						foreach (var value in methodParams[1].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								foreach (var stringParam in methodParams[2].UniqueValues ()) {
									if (stringParam is KnownStringValue stringValue) {
										BindingFlags bindingFlags = methodParams[0].Kind == ValueNodeKind.Null ? BindingFlags.Static : BindingFlags.Default;
										// TODO: Change this as needed after deciding if we are to keep all fields/properties on a type
										// that is accessed via reflection. For now, let's only keep the field/property that is retrieved.
										if (fieldOrPropertyInstrinsic == IntrinsicId.Expression_Property) {
											MarkPropertiesOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, filter: p => p.Name == stringValue.Contents, bindingFlags);
										} else {
											MarkFieldsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, filter: f => f.Name == stringValue.Contents, bindingFlags);
										}

										reflectionContext.RecordHandledPattern ();
									} else {
										RequireDynamicallyAccessedMembers (ref reflectionContext, memberKind, value, calledMethod.Parameters[2]);
									}
								}
							} else {
								RequireDynamicallyAccessedMembers (ref reflectionContext, memberKind, value, calledMethod.Parameters[1]);
							}
						}
					}
					break;

				//
				// System.Linq.Expressions.Expression
				// 
				// static New (Type)
				//
				case IntrinsicId.Expression_New: {
						reflectionContext.AnalyzingPattern ();

						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								MarkConstructorsOnType (ref reflectionContext, systemTypeValue.TypeRepresented, null, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
								reflectionContext.RecordHandledPattern ();
							} else {
								RequireDynamicallyAccessedMembers (ref reflectionContext, DynamicallyAccessedMemberTypes.DefaultConstructor, value, calledMethod.Parameters[0]);
							}
						}
					}
					break;

				//
				// System.Object
				// 
				// GetType()
				//
				case IntrinsicId.Object_GetType: {
						// We could do better here if we start tracking the static types of values within the method body.
						// Right now, this can only analyze a couple cases for which we have static information for.
						TypeDefinition staticType = null;
						if (methodParams[0] is MethodParameterValue methodParam) {
							if (callingMethodBody.Method.HasThis) {
								if (methodParam.ParameterIndex == 0) {
									staticType = callingMethodBody.Method.DeclaringType;
								} else {
									staticType = callingMethodBody.Method.Parameters[methodParam.ParameterIndex - 1].ParameterType.Resolve ();
								}
							} else {
								staticType = callingMethodBody.Method.Parameters[methodParam.ParameterIndex].ParameterType.Resolve ();
							}
						} else if (methodParams[0] is LoadFieldValue loadedField) {
							staticType = loadedField.Field.FieldType.Resolve ();
						}

						if (staticType != null) {
							// We can only analyze the Object.GetType call with the precise type if the type is sealed.
							// The type could be a descendant of the type in question, making us miss reflection.
							bool canUse = staticType.IsSealed;

							if (!canUse) {
								// We can allow Object.GetType to be modeled as System.Delegate because we keep all methods
								// on delegates anyway so reflection on something this approximation would miss is actually safe.
								canUse = staticType.IsTypeOf ("System", "Delegate");
							}

							if (canUse) {
								methodReturnValue = new SystemTypeValue (staticType);
							}
						}
					}
					break;

				//
				// System.Type
				//
				// GetType (string)
				// GetType (string, Boolean)
				// GetType (string, Boolean, Boolean)
				// GetType (string, Func<AssemblyName, Assembly>, Func<Assembly, String, Boolean, Type>)
				// GetType (string, Func<AssemblyName, Assembly>, Func<Assembly, String, Boolean, Type>, Boolean)
				// GetType (string, Func<AssemblyName, Assembly>, Func<Assembly, String, Boolean, Type>, Boolean, Boolean)
				//
				case IntrinsicId.Type_GetType: {
						reflectionContext.AnalyzingPattern ();

						foreach (var typeNameValue in methodParams[0].UniqueValues ()) {
							if (typeNameValue is KnownStringValue knownStringValue) {
								TypeDefinition foundType = AssemblyUtilities.ResolveFullyQualifiedTypeName (_context, knownStringValue.Contents);
								if (foundType == null) {
									// Intentionally ignore - it's not wrong for code to call Type.GetType on non-existing name, the code might expect null/exception back.
									reflectionContext.RecordHandledPattern ();
								} else {
									var methodCalling = reflectionContext.SourceMethod;
									reflectionContext.RecordRecognizedPattern (foundType, () => _markStep.MarkType (foundType, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling)));
									methodReturnValue = MergePointValue.MergeValues (methodReturnValue, new SystemTypeValue (foundType));
								}
							} else if (typeNameValue == NullValue.Instance) {
								reflectionContext.RecordHandledPattern ();
							} else if (typeNameValue is LeafValueWithDynamicallyAccessedMemberNode valueWithDynamicallyAccessedMember && valueWithDynamicallyAccessedMember.DynamicallyAccessedMemberKinds != 0) {
								// Propagate the annotation from the type name to the return value. Annotation on a string value will be fullfilled whenever a value is assigned to the string with annotation.
								// So while we don't know which type it is, we can guarantee that it will fullfill the annotation.
								reflectionContext.RecordHandledPattern ();
								methodReturnValue = MergePointValue.MergeValues (methodReturnValue, new MethodReturnValue (valueWithDynamicallyAccessedMember.DynamicallyAccessedMemberKinds) {
									SourceContext = calledMethod.Parameters[0]
								});
							} else {
								reflectionContext.RecordUnrecognizedPattern ($"Reflection call '{calledMethod.FullName}' inside '{reflectionContext.SourceMethod}' was detected with unknown value for the type name.");
							}
						}

					}
					break;

				//
				// GetConstructor (Type[])
				// GetConstructor (BindingFlags, Binder, Type[], ParameterModifier [])
				// GetConstructor (BindingFlags, Binder, CallingConventions, Type[], ParameterModifier [])
				//
				case IntrinsicId.Type_GetConstructor: {
						reflectionContext.AnalyzingPattern ();

						var parameters = calledMethod.Parameters;
						BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Instance;
						if (parameters.Count > 1) {
							if (methodParams[1].AsConstInt () != null)
								bindingFlags |= (BindingFlags) methodParams[1].AsConstInt ();
						}
						// Go over all types we've seen
						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								MarkConstructorsOnType (ref reflectionContext, systemTypeValue.TypeRepresented, (Func<MethodDefinition, bool>) null, bindingFlags);
								reflectionContext.RecordHandledPattern ();
							} else {
								// Otherwise fall back to the bitfield requirements
								var requiredMemberKinds = bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicConstructors : DynamicallyAccessedMemberTypes.None;
								requiredMemberKinds |= bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicConstructors : DynamicallyAccessedMemberTypes.None;
								RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, value, calledMethodDefinition);
							}
						}
					}
					break;

				//
				// GetMethod (string)
				// GetMethod (string, BindingFlags)
				// GetMethod (string, Type[])
				// GetMethod (string, Type[], ParameterModifier[])
				// GetMethod (string, BindingFlags, Binder, Type[], ParameterModifier[]) 6
				// GetMethod (string, BindingFlags, Binder, CallingConventions, Type[], ParameterModifier[]) 7
				// GetMethod (string, int, Type[])
				// GetMethod (string, int, Type[], ParameterModifier[]?)
				// GetMethod (string, int, BindingFlags, Binder?, Type[], ParameterModifier[]?)
				// GetMethod (string, int, BindingFlags, Binder?, CallingConventions, Type[], ParameterModifier[]?)
				//
				case IntrinsicId.Type_GetMethod: {
						reflectionContext.AnalyzingPattern ();

						BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public;
						if (calledMethod.Parameters.Count > 1 && calledMethod.Parameters[1].ParameterType.Name == "BindingFlags" && methodParams[2].AsConstInt () != null) {
							bindingFlags = (BindingFlags) methodParams[2].AsConstInt ();
						} else if (calledMethod.Parameters.Count > 2 && calledMethod.Parameters[2].ParameterType.Name == "BindingFlags" && methodParams[3].AsConstInt () != null) {
							bindingFlags = (BindingFlags) methodParams[3].AsConstInt ();
						}

						var requiredMemberKinds = GetDynamicallyAccessedMemberTypesFromBindingFlagsForMethods (bindingFlags);
						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								foreach (var stringParam in methodParams[1].UniqueValues ()) {
									if (stringParam is KnownStringValue stringValue) {
										MarkMethodsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, m => m.Name == stringValue.Contents, bindingFlags);
										reflectionContext.RecordHandledPattern ();
									} else {
										// Otherwise fall back to the bitfield requirements
										RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, value, calledMethodDefinition);
									}
								}
							} else {
								// Otherwise fall back to the bitfield requirements
								RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, value, calledMethodDefinition);
							}
						}
					}
					break;

				//
				// GetNestedType (string)
				// GetNestedType (string, BindingFlags)
				//
				case IntrinsicId.Type_GetNestedType: {
						reflectionContext.AnalyzingPattern ();

						BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public;
						if (calledMethod.Parameters.Count > 1 && calledMethod.Parameters[1].ParameterType.Name == "BindingFlags" && methodParams[2].AsConstInt () != null) {
							bindingFlags = (BindingFlags) methodParams[2].AsConstInt ();
						}

						var requiredMemberKinds = GetDynamicallyAccessedMemberTypesFromBindingFlagsForNestedTypes (bindingFlags);
						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								foreach (var stringParam in methodParams[1].UniqueValues ()) {
									if (stringParam is KnownStringValue stringValue) {
										TypeDefinition[] matchingNestedTypes = MarkNestedTypesOnType (ref reflectionContext, systemTypeValue.TypeRepresented, m => m.Name == stringValue.Contents, bindingFlags);

										if (matchingNestedTypes != null) {
											for (int i = 0; i < matchingNestedTypes.Length; i++)
												methodReturnValue = MergePointValue.MergeValues (methodReturnValue, new SystemTypeValue (matchingNestedTypes[i]));
										}

										reflectionContext.RecordHandledPattern ();
									} else {
										// Otherwise fall back to the bitfield requirements
										RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, value, calledMethodDefinition);
									}
								}
							} else {
								// Otherwise fall back to the bitfield requirements
								RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, value, calledMethodDefinition);
							}
						}
					}
					break;

				//
				// GetField (string)
				// GetField (string, BindingFlags)
				// GetEvent (string)
				// GetEvent (string, BindingFlags)
				// GetProperty (string)
				// GetProperty (string, BindingFlags)
				// GetProperty (string, Type)
				// GetProperty (string, Type[])
				// GetProperty (string, Type, Type[])
				// GetProperty (string, Type, Type[], ParameterModifier[])
				// GetProperty (string, BindingFlags, Binder, Type, Type[], ParameterModifier[])
				//
				case var fieldPropertyOrEvent when ((fieldPropertyOrEvent == IntrinsicId.Type_GetField || fieldPropertyOrEvent == IntrinsicId.Type_GetProperty || fieldPropertyOrEvent == IntrinsicId.Type_GetEvent)
					&& calledMethod.DeclaringType.Namespace == "System"
					&& calledMethod.DeclaringType.Name == "Type"
					&& calledMethod.Parameters[0].ParameterType.FullName == "System.String")
					&& calledMethod.HasThis: {

						reflectionContext.AnalyzingPattern ();
						BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public;
						if (calledMethod.Parameters.Count > 1 && calledMethod.Parameters[1].ParameterType.Name == "BindingFlags" && methodParams[2].AsConstInt () != null) {
							bindingFlags = (BindingFlags) methodParams[2].AsConstInt ();
						}

						DynamicallyAccessedMemberTypes memberKind = fieldPropertyOrEvent switch
						{
							IntrinsicId.Type_GetEvent => GetDynamicallyAccessedMemberTypesFromBindingFlagsForEvents (bindingFlags),
							IntrinsicId.Type_GetField => GetDynamicallyAccessedMemberTypesFromBindingFlagsForFields (bindingFlags),
							IntrinsicId.Type_GetProperty => GetDynamicallyAccessedMemberTypesFromBindingFlagsForProperties (bindingFlags),
							_ => throw new ArgumentException ($"Reflection call '{calledMethod.FullName}' inside '{callingMethodBody.Method.FullName}' is of unexpected member type."),
						};

						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								foreach (var stringParam in methodParams[1].UniqueValues ()) {
									if (stringParam is KnownStringValue stringValue) {
										switch (fieldPropertyOrEvent) {
										case IntrinsicId.Type_GetEvent:
											MarkEventsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, filter: e => e.Name == stringValue.Contents, bindingFlags);
											break;
										case IntrinsicId.Type_GetField:
											MarkFieldsOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, filter: f => f.Name == stringValue.Contents, bindingFlags);
											break;
										case IntrinsicId.Type_GetProperty:
											MarkPropertiesOnTypeHierarchy (ref reflectionContext, systemTypeValue.TypeRepresented, filter: p => p.Name == stringValue.Contents, bindingFlags);
											break;
										default:
											Debug.Fail ("Unreachable.");
											break;
										}
										reflectionContext.RecordHandledPattern ();
									} else {
										RequireDynamicallyAccessedMembers (ref reflectionContext, memberKind, value, calledMethodDefinition);
									}
								}
							} else {
								RequireDynamicallyAccessedMembers (ref reflectionContext, memberKind, value, calledMethodDefinition);
							}
						}
					}
					break;

				//
				// System.Activator
				// 
				// static CreateInstance (System.Type type)
				// static CreateInstance (System.Type type, bool nonPublic)
				// static CreateInstance (System.Type type, params object?[]? args)
				// static CreateInstance (System.Type type, object?[]? args, object?[]? activationAttributes)
				// static CreateInstance (System.Type type, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object?[]? args, System.Globalization.CultureInfo? culture)
				// static CreateInstance (System.Type type, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object?[]? args, System.Globalization.CultureInfo? culture, object?[]? activationAttributes) { throw null; }
				//
				case IntrinsicId.Activator_CreateInstance_Type: {
						var parameters = calledMethod.Parameters;

						reflectionContext.AnalyzingPattern ();

						int? ctorParameterCount = null;
						BindingFlags bindingFlags = BindingFlags.Instance;
						if (parameters.Count > 1) {
							if (parameters[1].ParameterType.MetadataType == MetadataType.Boolean) {
								// The overload that takes a "nonPublic" bool
								bool nonPublic = true;
								if (methodParams[1] is ConstIntValue constInt) {
									nonPublic = constInt.Value != 0;
								}

								if (nonPublic)
									bindingFlags |= BindingFlags.NonPublic | BindingFlags.Public;
								else
									bindingFlags |= BindingFlags.Public;
								ctorParameterCount = 0;
							} else {
								// Overload that has the parameters as the second or fourth argument
								int argsParam = parameters.Count == 2 || parameters.Count == 3 ? 1 : 3;

								if (methodParams.Count > argsParam &&
									methodParams[argsParam] is ArrayValue arrayValue &&
									arrayValue.Size.AsConstInt () != null) {
									ctorParameterCount = arrayValue.Size.AsConstInt ();
								}

								if (parameters.Count > 3) {
									if (methodParams[1].AsConstInt () != null)
										bindingFlags |= (BindingFlags) methodParams[1].AsConstInt ();
									else
										bindingFlags |= BindingFlags.NonPublic | BindingFlags.Public;
								} else {
									bindingFlags |= BindingFlags.Public;
								}
							}
						} else {
							// The overload with a single System.Type argument
							ctorParameterCount = 0;
							bindingFlags |= BindingFlags.Public;
						}

						// Go over all types we've seen
						foreach (var value in methodParams[0].UniqueValues ()) {
							if (value is SystemTypeValue systemTypeValue) {
								// Special case known type values as we can do better by applying exact binding flags and parameter count.
								MarkConstructorsOnType (ref reflectionContext, systemTypeValue.TypeRepresented,
									ctorParameterCount == null ? (Func<MethodDefinition, bool>) null : m => m.Parameters.Count == ctorParameterCount, bindingFlags);
								reflectionContext.RecordHandledPattern ();
							} else {
								// Otherwise fall back to the bitfield requirements
								var requiredMemberKinds = ctorParameterCount == 0
									? DynamicallyAccessedMemberTypes.DefaultConstructor
									: GetDynamicallyAccessedMemberTypesFromBindingFlagsForConstructors (bindingFlags);
								RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, value, calledMethod.Parameters[0]);
							}
						}
					}
					break;

				//
				// System.Activator
				// 
				// static CreateInstance (string assemblyName, string typeName)
				// static CreateInstance (string assemblyName, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object?[]? args, System.Globalization.CultureInfo? culture, object?[]? activationAttributes)
				// static CreateInstance (string assemblyName, string typeName, object?[]? activationAttributes)
				//
				case IntrinsicId.Activator_CreateInstance_AssemblyName_TypeName:
					ProcessCreateInstanceByName (ref reflectionContext, calledMethodDefinition, methodParams);
					break;

				//
				// System.Activator
				// 
				// static CreateInstanceFrom (string assemblyFile, string typeName)
				// static CreateInstanceFrom (string assemblyFile, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// static CreateInstanceFrom (string assemblyFile, string typeName, object? []? activationAttributes)
				//
				case IntrinsicId.Activator_CreateInstanceFrom:
					ProcessCreateInstanceByName (ref reflectionContext, calledMethodDefinition, methodParams);
					break;

				//
				// System.Activator
				// 
				// static T CreateInstance<T> ()
				//
				case IntrinsicId.Activator_CreateInstanceOfT: {
						reflectionContext.AnalyzingPattern ();

						if (calledMethod is GenericInstanceMethod genericCalledMethod && genericCalledMethod.GenericArguments.Count == 1
							&& genericCalledMethod.GenericArguments[0] is GenericParameter genericParameter) {
							if (genericParameter.HasDefaultConstructorConstraint) {
								// This is safe, the linker would have marked the default .ctor already
								reflectionContext.RecordHandledPattern ();
								break;
							}
						}

						// Not yet supported in any combination
						reflectionContext.RecordUnrecognizedPattern ($"Activator call '{reflectionContext.MemberWithRequirements}' inside '{reflectionContext.SourceMethod.FullName}' is not recognized, the type to instantiate may be missing the necessary constructor.");
					}
					break;

				//
				// System.AppDomain
				//
				// CreateInstance (string assemblyName, string typeName)
				// CreateInstance (string assemblyName, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// CreateInstance (string assemblyName, string typeName, object? []? activationAttributes)
				//
				// CreateInstanceAndUnwrap (string assemblyName, string typeName)
				// CreateInstanceAndUnwrap (string assemblyName, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// CreateInstanceAndUnwrap (string assemblyName, string typeName, object? []? activationAttributes)
				//
				// CreateInstanceFrom (string assemblyFile, string typeName)
				// CreateInstanceFrom (string assemblyFile, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// CreateInstanceFrom (string assemblyFile, string typeName, object? []? activationAttributes)
				//
				// CreateInstanceFromAndUnwrap (string assemblyFile, string typeName)
				// CreateInstanceFromAndUnwrap (string assemblyFile, string typeName, bool ignoreCase, System.Reflection.BindingFlags bindingAttr, System.Reflection.Binder? binder, object? []? args, System.Globalization.CultureInfo? culture, object? []? activationAttributes)
				// CreateInstanceFromAndUnwrap (string assemblyFile, string typeName, object? []? activationAttributes)
				//
				case var appDomainCreateInstance when appDomainCreateInstance == IntrinsicId.AppDomain_CreateInstance
						|| appDomainCreateInstance == IntrinsicId.AppDomain_CreateInstanceAndUnwrap
						|| appDomainCreateInstance == IntrinsicId.AppDomain_CreateInstanceFrom
						|| appDomainCreateInstance == IntrinsicId.AppDomain_CreateInstanceFromAndUnwrap:
					ProcessCreateInstanceByName (ref reflectionContext, calledMethodDefinition, methodParams);
					break;

				//
				// System.Reflection.Assembly
				//
				// CreateInstance (string typeName)
				// CreateInstance (string typeName, bool ignoreCase)
				// CreateInstance (string typeName, bool ignoreCase, BindingFlags bindingAttr, Binder? binder, object []? args, CultureInfo? culture, object []? activationAttributes)
				//
				case IntrinsicId.Assembly_CreateInstance:
					//
					// TODO: This could be supported for "this" only calls
					//
					reflectionContext.AnalyzingPattern ();
					reflectionContext.RecordUnrecognizedPattern ($"Activator call '{reflectionContext.MemberWithRequirements}' inside '{reflectionContext.SourceMethod.FullName}' is not yet supported");
					break;

				//
				// System.Runtime.CompilerServices.RuntimeHelpers
				//
				// RunClassConstructor (RuntimeTypeHandle type)
				//
				case IntrinsicId.RuntimeHelpers_RunClassConstructor: {
						reflectionContext.AnalyzingPattern ();
						foreach (var typeHandleValue in methodParams[0].UniqueValues ()) {
							if (typeHandleValue is RuntimeTypeHandleValue runtimeTypeHandleValue) {
								_markStep.MarkStaticConstructor (runtimeTypeHandleValue.TypeRepresented, new DependencyInfo (DependencyKind.AccessedViaReflection, reflectionContext.SourceMethod));
								reflectionContext.RecordHandledPattern ();
							} else if (typeHandleValue == NullValue.Instance)
								reflectionContext.RecordHandledPattern ();
							else {
								reflectionContext.RecordUnrecognizedPattern ($"A {GetValueDescriptionForErrorMessage (typeHandleValue)} " +
									$"is passed into the {GetMetadataTokenDescriptionForErrorMessage ((reflectionContext.MemberWithRequirements as MethodDefinition).Parameters[0])}. " +
									$"It's not possible to guarantee availability of the target static constructor.");
							}
						}
					}
					break;

				default:
					if (requiresDataFlowAnalysis) {
						reflectionContext.AnalyzingPattern ();
						for (int parameterIndex = 0; parameterIndex < methodParams.Count; parameterIndex++) {
							var requiredMemberKinds = _flowAnnotations.GetParameterAnnotation (calledMethodDefinition, parameterIndex);
							if (requiredMemberKinds != 0) {
								IMetadataTokenProvider targetContext;
								if (calledMethodDefinition.HasImplicitThis ()) {
									if (parameterIndex == 0)
										targetContext = calledMethodDefinition;
									else
										targetContext = calledMethodDefinition.Parameters[parameterIndex - 1];
								} else {
									targetContext = calledMethodDefinition.Parameters[parameterIndex];
								}

								RequireDynamicallyAccessedMembers (ref reflectionContext, requiredMemberKinds, methodParams[parameterIndex], targetContext);
							}
						}

						reflectionContext.RecordHandledPattern ();
					}

					if (_context.Annotations.TryGetLinkerAttribute (calledMethodDefinition, out RequiresUnreferencedCodeAttribute requiresUnreferencedCode)) {
						string message =
							$"Calling '{calledMethodDefinition}' which has `RequiresUnreferencedCodeAttribute` can break functionality when trimming application code. " +
							$"{requiresUnreferencedCode.Message}.";

						if (requiresUnreferencedCode.Url != null) {
							message += " " + requiresUnreferencedCode.Url;
						}

						_context.LogMessage (MessageContainer.CreateWarningMessage (_context, message, 2026, origin: MessageOrigin.TryGetOrigin (callingMethodBody.Method, operation.Offset)));
					}

					// To get good reporting of errors we need to track the origin of the value for all method calls
					// but except Newobj as those are special.
					if (calledMethodDefinition.ReturnType.MetadataType != MetadataType.Void) {
						methodReturnValue = new MethodReturnValue (returnValueDynamicallyAccessedMemberKinds) {
							SourceContext = calledMethodDefinition
						};

						return true;
					}

					return false;
				}
			} finally {
				reflectionContext.Dispose ();
			}

			// If we get here, we handled this as an intrinsic.  As a convenience, if the code above
			// didn't set the return value (and the method has a return value), we will set it to be an
			// unknown value with the return type of the method.
			if (methodReturnValue == null) {
				if (calledMethod.ReturnType.MetadataType != MetadataType.Void) {
					methodReturnValue = new MethodReturnValue (returnValueDynamicallyAccessedMemberKinds) {
						SourceContext = calledMethodDefinition
					};
				}
			}

			// Validate that the return value has the correct annotations as per the method return value annotations
			if (returnValueDynamicallyAccessedMemberKinds != 0 && methodReturnValue != null) {
				if (methodReturnValue is LeafValueWithDynamicallyAccessedMemberNode methodReturnValueWithMemberKinds) {
					if (!methodReturnValueWithMemberKinds.DynamicallyAccessedMemberKinds.HasFlag (returnValueDynamicallyAccessedMemberKinds))
						throw new InvalidOperationException ($"Internal linker error: processing of call from {callingMethodBody.Method} to {calledMethod} returned value which is not correctly annotated with the expected dynamic member access kinds.");
				} else if (methodReturnValue is SystemTypeValue) {
					// SystemTypeValue can fullfill any requirement, so it's always valid
					// The requirements will be applied at the point where it's consumed (passed as a method parameter, set as field value, returned from the method)
				} else {
					throw new InvalidOperationException ($"Internal linker error: processing of call from {callingMethodBody.Method} to {calledMethod} returned value which is not correctly annotated with the expected dynamic member access kinds.");
				}
			}

			return true;
		}

		void ProcessCreateInstanceByName (ref ReflectionPatternContext reflectionContext, MethodDefinition calledMethod, ValueNodeList methodParams)
		{
			reflectionContext.AnalyzingPattern ();

			BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public;
			bool parameterlessConstructor = true;
			if (calledMethod.Parameters.Count == 8 && calledMethod.Parameters[2].ParameterType.MetadataType == MetadataType.Boolean &&
				methodParams[3].AsConstInt () != null) {
				parameterlessConstructor = false;
				bindingFlags = BindingFlags.Instance | (BindingFlags) methodParams[3].AsConstInt ();
			}

			int methodParamsOffset = calledMethod.HasImplicitThis () ? 1 : 0;

			foreach (var assemblyNameValue in methodParams[methodParamsOffset].UniqueValues ()) {
				if (assemblyNameValue is KnownStringValue assemblyNameStringValue) {
					foreach (var typeNameValue in methodParams[methodParamsOffset + 1].UniqueValues ()) {
						if (typeNameValue is KnownStringValue typeNameStringValue) {
							var resolvedAssembly = _context.GetLoadedAssembly (assemblyNameStringValue.Contents);
							if (resolvedAssembly == null) {
								reflectionContext.RecordUnrecognizedPattern ($"Activator call '{reflectionContext.MemberWithRequirements}' inside '{reflectionContext.SourceMethod.FullName}' references assembly '{assemblyNameStringValue.Contents}' which could not be found");
								continue;
							}

							var resolvedType = resolvedAssembly.FindType (typeNameStringValue.Contents);
							if (resolvedType == null) {
								reflectionContext.RecordUnrecognizedPattern ($"Activator call '{reflectionContext.MemberWithRequirements}' inside '{reflectionContext.SourceMethod.FullName}' references type '{typeNameStringValue}' in assembly '{resolvedAssembly.FullName}' which could not be found");
								continue;
							}

							MarkConstructorsOnType (ref reflectionContext, resolvedType,
								parameterlessConstructor ? m => m.Parameters.Count == 0 : (Func<MethodDefinition, bool>) null, bindingFlags);
						} else {
							reflectionContext.RecordUnrecognizedPattern ($"Activator call '{reflectionContext.MemberWithRequirements}' inside '{reflectionContext.SourceMethod.FullName}' has unrecognized value for the 'typeName' parameter.");
						}
					}
				} else {
					reflectionContext.RecordUnrecognizedPattern ($"Activator call '{reflectionContext.MemberWithRequirements}' inside '{reflectionContext.SourceMethod.FullName}' has unrecognized value for the 'assemblyName' parameter.");
				}
			}
		}

		void RequireDynamicallyAccessedMembers (ref ReflectionPatternContext reflectionContext, DynamicallyAccessedMemberTypes requiredMemberKinds, ValueNode value, IMetadataTokenProvider targetContext)
		{
			foreach (var uniqueValue in value.UniqueValues ()) {
				if (uniqueValue is LeafValueWithDynamicallyAccessedMemberNode valueWithDynamicallyAccessedMember) {
					if (!valueWithDynamicallyAccessedMember.DynamicallyAccessedMemberKinds.HasFlag (requiredMemberKinds)) {
						reflectionContext.RecordUnrecognizedPattern ($"The {GetValueDescriptionForErrorMessage (valueWithDynamicallyAccessedMember)} " +
							$"with dynamically accessed member kinds '{GetDynamicallyAccessedMemberKindsDescription (valueWithDynamicallyAccessedMember.DynamicallyAccessedMemberKinds)}' " +
							$"is passed into the {GetMetadataTokenDescriptionForErrorMessage (targetContext)} " +
							$"which requires dynamically accessed member kinds '{GetDynamicallyAccessedMemberKindsDescription (requiredMemberKinds)}'. " +
							$"To fix this add DynamicallyAccessedMembersAttribute to it and specify at least these member kinds '{GetDynamicallyAccessedMemberKindsDescription (requiredMemberKinds)}'.");
					} else {
						reflectionContext.RecordHandledPattern ();
					}
				} else if (uniqueValue is SystemTypeValue systemTypeValue) {
					MarkTypeForDynamicallyAccessedMembers (ref reflectionContext, systemTypeValue.TypeRepresented, requiredMemberKinds);
				} else if (uniqueValue is KnownStringValue knownStringValue) {
					TypeDefinition foundType = AssemblyUtilities.ResolveFullyQualifiedTypeName (_context, knownStringValue.Contents);
					if (foundType == null) {
						// Intentionally ignore - it's not wrong for code to call Type.GetType on non-existing name, the code might expect null/exception back.
						reflectionContext.RecordHandledPattern ();
					} else {
						MarkTypeForDynamicallyAccessedMembers (ref reflectionContext, foundType, requiredMemberKinds);
					}
				} else if (uniqueValue == NullValue.Instance) {
					// Ignore - probably unreachable path as it would fail at runtime anyway.
				} else {
					reflectionContext.RecordUnrecognizedPattern ($"A {GetValueDescriptionForErrorMessage (uniqueValue)} " +
						$"is passed into the {GetMetadataTokenDescriptionForErrorMessage (targetContext)} " +
						$"which requires dynamically accessed member kinds '{GetDynamicallyAccessedMemberKindsDescription (requiredMemberKinds)}'. " +
						$"It's not possible to guarantee that these requirements are met by the application.");
				}
			}

			reflectionContext.RecordHandledPattern ();
		}

		void MarkTypeForDynamicallyAccessedMembers (ref ReflectionPatternContext reflectionContext, TypeDefinition typeDefinition, DynamicallyAccessedMemberTypes requiredMemberKinds)
		{
			if (requiredMemberKinds == DynamicallyAccessedMemberTypes.All) {
				var sourceMethod = reflectionContext.SourceMethod;
				reflectionContext.RecordRecognizedPattern (typeDefinition, () => _markStep.MarkEntireType (typeDefinition, includeBaseTypes: true, new DependencyInfo (DependencyKind.AccessedViaReflection, sourceMethod)));
				return;
			}

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicConstructors))
				MarkConstructorsOnType (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.NonPublic);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicConstructors))
				MarkConstructorsOnType (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.Public);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.DefaultConstructor))
				MarkConstructorsOnType (ref reflectionContext, typeDefinition, filter: m => m.IsPublic && m.Parameters.Count == 0);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicMethods))
				MarkMethodsOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.NonPublic);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicMethods))
				MarkMethodsOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.Public);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicFields))
				MarkFieldsOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.NonPublic);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicFields))
				MarkFieldsOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.Public);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicNestedTypes))
				MarkNestedTypesOnType (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.NonPublic);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicNestedTypes))
				MarkNestedTypesOnType (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.Public);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicProperties))
				MarkPropertiesOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.NonPublic);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicProperties))
				MarkPropertiesOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.Public);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicEvents))
				MarkEventsOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.NonPublic);

			if (requiredMemberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicEvents))
				MarkEventsOnTypeHierarchy (ref reflectionContext, typeDefinition, filter: null, bindingFlags: BindingFlags.Public);
		}

		void MarkConstructorsOnType (ref ReflectionPatternContext reflectionContext, TypeDefinition type, Func<MethodDefinition, bool> filter, BindingFlags? bindingFlags = null)
		{
			foreach (var method in type.Methods) {
				if (!method.IsConstructor)
					continue;

				if (filter != null && !filter (method))
					continue;

				if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Static && !method.IsStatic)
					continue;

				if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Instance && method.IsStatic)
					continue;

				if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.Public && !method.IsPublic)
					continue;

				if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.NonPublic && method.IsPublic)
					continue;

				var methodCalling = reflectionContext.SourceMethod;
				reflectionContext.RecordRecognizedPattern (method, () => _markStep.MarkIndirectlyCalledMethod (method, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling)));
			}
		}

		void MarkMethodsOnTypeHierarchy (ref ReflectionPatternContext reflectionContext, TypeDefinition type, Func<MethodDefinition, bool> filter, BindingFlags? bindingFlags = null)
		{
			bool onBaseType = false;
			while (type != null) {
				foreach (var method in type.Methods) {
					// Ignore constructors as those are not considered methods from a reflection's point of view
					if (method.IsConstructor)
						continue;

					// Ignore private methods on a base type - those are completely ignored by reflection
					// (anything private on the base type is not visible via the derived type)
					if (onBaseType && method.IsPrivate)
						continue;

					// Note that special methods like property getter/setter, event adder/remover will still get through and will be marked.
					// This is intentional as reflection treats these as methods as well.

					if (filter != null && !filter (method))
						continue;

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Static && !method.IsStatic)
						continue;

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Instance && method.IsStatic)
						continue;

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.Public && !method.IsPublic)
						continue;

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.NonPublic && method.IsPublic)
						continue;

					var methodCalling = reflectionContext.SourceMethod;
					reflectionContext.RecordRecognizedPattern (method, () => _markStep.MarkIndirectlyCalledMethod (method, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling)));
				}

				type = type.BaseType?.Resolve ();
				onBaseType = true;
			}
		}

		void MarkFieldsOnTypeHierarchy (ref ReflectionPatternContext reflectionContext, TypeDefinition type, Func<FieldDefinition, bool> filter, BindingFlags bindingFlags = BindingFlags.Default)
		{
			bool onBaseType = false;
			while (type != null) {
				foreach (var field in type.Fields) {
					// Ignore private fields on a base type - those are completely ignored by reflection
					// (anything private on the base type is not visible via the derived type)
					if (onBaseType && field.IsPrivate)
						continue;

					// Note that compiler generated fields backing some properties and events will get through here.
					// This is intentional as reflection treats these as fields as well.

					if (filter != null && !filter (field))
						continue;

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Static && !field.IsStatic)
						continue;

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Instance && field.IsStatic)
						continue;

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.Public && !field.IsPublic)
						continue;

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.NonPublic && field.IsPublic)
						continue;

					var methodCalling = reflectionContext.SourceMethod;
					reflectionContext.RecordRecognizedPattern (field, () => _markStep.MarkField (field, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling)));
				}

				type = type.BaseType?.Resolve ();
				onBaseType = true;
			}
		}

		TypeDefinition[] MarkNestedTypesOnType (ref ReflectionPatternContext reflectionContext, TypeDefinition type, Func<TypeDefinition, bool> filter, BindingFlags bindingFlags = BindingFlags.Default)
		{
			var result = new ArrayBuilder<TypeDefinition> ();

			foreach (var nestedType in type.NestedTypes) {
				if (filter != null && !filter (nestedType))
					continue;

				if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.Public) {
					if (!nestedType.IsNestedPublic)
						continue;
				}

				if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.NonPublic) {
					if (nestedType.IsNestedPublic)
						continue;
				}

				result.Add (nestedType);

				var methodCalling = reflectionContext.SourceMethod;
				reflectionContext.RecordRecognizedPattern (nestedType, () => _markStep.MarkType (nestedType, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling)));
			}

			return result.ToArray ();
		}

		void MarkPropertiesOnTypeHierarchy (ref ReflectionPatternContext reflectionContext, TypeDefinition type, Func<PropertyDefinition, bool> filter, BindingFlags bindingFlags = BindingFlags.Default)
		{
			bool onBaseType = false;
			while (type != null) {
				foreach (var property in type.Properties) {
					// Ignore private properties on a base type - those are completely ignored by reflection
					// (anything private on the base type is not visible via the derived type)
					// Note that properties themselves are not actually private, their accessors are
					if (onBaseType &&
						(property.GetMethod == null || property.GetMethod.IsPrivate) &&
						(property.SetMethod == null || property.SetMethod.IsPrivate))
						continue;

					if (filter != null && !filter (property))
						continue;

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Static) {
						if ((property.GetMethod != null) && !property.GetMethod.IsStatic) continue;
						if ((property.SetMethod != null) && !property.SetMethod.IsStatic) continue;
					}

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Instance) {
						if ((property.GetMethod != null) && property.GetMethod.IsStatic) continue;
						if ((property.SetMethod != null) && property.SetMethod.IsStatic) continue;
					}

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.Public) {
						if ((property.GetMethod == null || !property.GetMethod.IsPublic)
							&& (property.SetMethod == null || !property.SetMethod.IsPublic))
							continue;
					}

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.NonPublic) {
						if ((property.GetMethod != null) && property.GetMethod.IsPublic) continue;
						if ((property.SetMethod != null) && property.SetMethod.IsPublic) continue;
					}

					var methodCalling = reflectionContext.SourceMethod;
					reflectionContext.RecordRecognizedPattern (property, () => {
						// Marking the property itself actually doesn't keep it (it only marks its attributes and records the dependency), we have to mark the methods on it
						_markStep.MarkProperty (property, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling));
						// TODO - this is sort of questionable - when somebody asks for a property they probably want to call either get or set
						// but linker tracks those separately, and so accessing the getter/setter will raise a warning as it's potentially trimmed.
						// So including them here doesn't actually remove the warning even if the code is written correctly.
						_markStep.MarkMethodIfNotNull (property.GetMethod, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling));
						_markStep.MarkMethodIfNotNull (property.SetMethod, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling));
						_markStep.MarkMethodsIf (property.OtherMethods, m => true, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling));
					});
				}

				type = type.BaseType?.Resolve ();
				onBaseType = true;
			}
		}

		void MarkEventsOnTypeHierarchy (ref ReflectionPatternContext reflectionContext, TypeDefinition type, Func<EventDefinition, bool> filter, BindingFlags bindingFlags = BindingFlags.Default)
		{
			bool onBaseType = false;
			while (type != null) {
				foreach (var @event in type.Events) {
					// Ignore private properties on a base type - those are completely ignored by reflection
					// (anything private on the base type is not visible via the derived type)
					// Note that properties themselves are not actually private, their accessors are
					if (onBaseType &&
						(@event.AddMethod == null || @event.AddMethod.IsPrivate) &&
						(@event.RemoveMethod == null || @event.RemoveMethod.IsPrivate))
						continue;

					if (filter != null && !filter (@event))
						continue;

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Static) {
						if ((@event.AddMethod != null) && !@event.AddMethod.IsStatic) continue;
						if ((@event.RemoveMethod != null) && !@event.RemoveMethod.IsStatic) continue;
					}

					if ((bindingFlags & (BindingFlags.Instance | BindingFlags.Static)) == BindingFlags.Instance) {
						if ((@event.AddMethod != null) && @event.AddMethod.IsStatic) continue;
						if ((@event.RemoveMethod != null) && @event.RemoveMethod.IsStatic) continue;
					}

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.Public) {
						if ((@event.AddMethod == null || !@event.AddMethod.IsPublic)
							&& (@event.RemoveMethod == null || !@event.RemoveMethod.IsPublic))
							continue;
					}

					if ((bindingFlags & (BindingFlags.Public | BindingFlags.NonPublic)) == BindingFlags.NonPublic) {
						if ((@event.AddMethod != null) && @event.AddMethod.IsPublic) continue;
						if ((@event.RemoveMethod != null) && @event.RemoveMethod.IsPublic) continue;
					}

					var methodCalling = reflectionContext.SourceMethod;
					reflectionContext.RecordRecognizedPattern (@event, () => {
						// MarkEvent actually marks the add/remove/invoke methods as well, so no need to mark those explicitly
						_markStep.MarkEvent (@event, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling));
						_markStep.MarkMethodsIf (@event.OtherMethods, m => true, new DependencyInfo (DependencyKind.AccessedViaReflection, methodCalling));
					});
				}

				type = type.BaseType?.Resolve ();
				onBaseType = true;
			}
		}

		string GetValueDescriptionForErrorMessage (ValueNode value)
		{
			switch (value) {
			case MethodParameterValue methodParameterValue: {
					if (methodParameterValue.SourceContext is MethodDefinition method) {
						int declaredParameterIndex;
						if (method.HasImplicitThis ()) {
							if (methodParameterValue.ParameterIndex == 0)
								return GetMetadataTokenDescriptionForErrorMessage (method);

							declaredParameterIndex = methodParameterValue.ParameterIndex - 1;
						} else
							declaredParameterIndex = methodParameterValue.ParameterIndex;

						if (declaredParameterIndex >= 0 && declaredParameterIndex < method.Parameters.Count)
							return GetParameterDescriptionForErrorMessage (method.Parameters[declaredParameterIndex]);
					}

					return $"parameter #{methodParameterValue.ParameterIndex} of method '{methodParameterValue.SourceContext}'";
				}

			case MethodReturnValue methodReturnValue: {
					if (methodReturnValue.SourceContext is MethodDefinition method) {
						return GetMetadataTokenDescriptionForErrorMessage (method.MethodReturnType);
					}

					return "method return value";
				}

			case LoadFieldValue loadFieldValue:
				return GetMetadataTokenDescriptionForErrorMessage (loadFieldValue.Field);

			default:
				return $"value from unknown source";
			}
		}

		string GetMetadataTokenDescriptionForErrorMessage (IMetadataTokenProvider targetContext)
		{
			return targetContext switch
			{
				ParameterDefinition parameterDefinition => GetParameterDescriptionForErrorMessage (parameterDefinition),
				MethodReturnType methodReturnType => $"return value of method '{methodReturnType.Method}'",
				FieldDefinition fieldDefinition => $"field '{fieldDefinition}'",
				// MethodDefinition is used to represent the "this" parameter as we don't support annotations on the method itself.
				MethodDefinition methodDefinition => $"implicit 'this' parameter of method '{methodDefinition}'",
				_ => $"'{targetContext}'",
			};
			;
		}

		string GetParameterDescriptionForErrorMessage (ParameterDefinition parameterDefinition)
		{
			if (string.IsNullOrEmpty (parameterDefinition.Name))
				return $"parameter #{parameterDefinition.Index} of method '{parameterDefinition.Method}'";

			return $"parameter '{parameterDefinition.Name}' of method '{parameterDefinition.Method}'";
		}

		string GetDynamicallyAccessedMemberKindsDescription (DynamicallyAccessedMemberTypes memberKinds)
		{
			if (memberKinds == DynamicallyAccessedMemberTypes.All)
				return DynamicallyAccessedMemberTypes.All.ToString ();

			var results = new List<DynamicallyAccessedMemberTypes> ();
			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicConstructors))
				results.Add (DynamicallyAccessedMemberTypes.NonPublicConstructors);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicConstructors))
				results.Add (DynamicallyAccessedMemberTypes.PublicConstructors);
			else if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.DefaultConstructor))
				results.Add (DynamicallyAccessedMemberTypes.DefaultConstructor);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicMethods))
				results.Add (DynamicallyAccessedMemberTypes.NonPublicMethods);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicMethods))
				results.Add (DynamicallyAccessedMemberTypes.PublicMethods);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicProperties))
				results.Add (DynamicallyAccessedMemberTypes.NonPublicProperties);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicProperties))
				results.Add (DynamicallyAccessedMemberTypes.PublicProperties);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicFields))
				results.Add (DynamicallyAccessedMemberTypes.NonPublicFields);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicFields))
				results.Add (DynamicallyAccessedMemberTypes.PublicFields);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicEvents))
				results.Add (DynamicallyAccessedMemberTypes.NonPublicEvents);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicEvents))
				results.Add (DynamicallyAccessedMemberTypes.PublicEvents);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.NonPublicNestedTypes))
				results.Add (DynamicallyAccessedMemberTypes.NonPublicNestedTypes);

			if (memberKinds.HasFlag (DynamicallyAccessedMemberTypes.PublicNestedTypes))
				results.Add (DynamicallyAccessedMemberTypes.PublicNestedTypes);

			if (results.Count == 0)
				return DynamicallyAccessedMemberTypes.None.ToString ();

			return string.Join (" | ", results.Select (r => r.ToString ()));
		}

		static DynamicallyAccessedMemberTypes GetDynamicallyAccessedMemberTypesFromBindingFlagsForNestedTypes (BindingFlags bindingFlags) =>
			(bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicNestedTypes : DynamicallyAccessedMemberTypes.None) |
			(bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicNestedTypes : DynamicallyAccessedMemberTypes.None);

		static DynamicallyAccessedMemberTypes GetDynamicallyAccessedMemberTypesFromBindingFlagsForConstructors (BindingFlags bindingFlags) =>
			(bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicConstructors : DynamicallyAccessedMemberTypes.None) |
			(bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicConstructors : DynamicallyAccessedMemberTypes.None);

		static DynamicallyAccessedMemberTypes GetDynamicallyAccessedMemberTypesFromBindingFlagsForMethods (BindingFlags bindingFlags) =>
			(bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicMethods : DynamicallyAccessedMemberTypes.None) |
			(bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicMethods : DynamicallyAccessedMemberTypes.None);

		static DynamicallyAccessedMemberTypes GetDynamicallyAccessedMemberTypesFromBindingFlagsForFields (BindingFlags bindingFlags) =>
			(bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicFields : DynamicallyAccessedMemberTypes.None) |
			(bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicFields : DynamicallyAccessedMemberTypes.None);

		static DynamicallyAccessedMemberTypes GetDynamicallyAccessedMemberTypesFromBindingFlagsForProperties (BindingFlags bindingFlags) =>
			(bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicProperties : DynamicallyAccessedMemberTypes.None) |
			(bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicProperties : DynamicallyAccessedMemberTypes.None);

		static DynamicallyAccessedMemberTypes GetDynamicallyAccessedMemberTypesFromBindingFlagsForEvents (BindingFlags bindingFlags) =>
			(bindingFlags.HasFlag (BindingFlags.Public) ? DynamicallyAccessedMemberTypes.PublicEvents : DynamicallyAccessedMemberTypes.None) |
			(bindingFlags.HasFlag (BindingFlags.NonPublic) ? DynamicallyAccessedMemberTypes.NonPublicEvents : DynamicallyAccessedMemberTypes.None);
	}
}