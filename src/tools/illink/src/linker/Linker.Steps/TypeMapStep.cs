﻿//
// TypeMapStep.cs
//
// Author:
//   Jb Evain (jbevain@novell.com)
//
// (C) 2009 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System.Collections.Generic;
using Mono.Cecil;

namespace Mono.Linker.Steps
{

	public class TypeMapStep : BaseStep
	{

		protected override void ProcessAssembly (AssemblyDefinition assembly)
		{
			foreach (TypeDefinition type in assembly.MainModule.Types)
				MapType (type);
		}

		protected virtual void MapType (TypeDefinition type)
		{
			MapVirtualMethods (type);
			MapInterfaceMethodsInTypeHierarchy (type);

			if (!type.HasNestedTypes)
				return;

			foreach (var nested in type.NestedTypes)
				MapType (nested);
		}

		void MapInterfaceMethodsInTypeHierarchy (TypeDefinition type)
		{
			if (!type.HasInterfaces)
				return;

			// Foreach interface and for each newslot virtual method on the interface, try
			// to find the method implementation and record it.
			foreach (var interfaceImpl in type.GetInflatedInterfaces ()) {
				foreach (MethodReference interfaceMethod in interfaceImpl.InflatedInterface.GetMethods ()) {
					MethodDefinition resolvedInterfaceMethod = interfaceMethod.Resolve ();
					if (resolvedInterfaceMethod == null)
						continue;

					// TODO-NICE: if the interface method is implemented explicitly (with an override),
					// we shouldn't need to run the below logic. This results in linker potentially
					// keeping more methods than needed.

					if (!resolvedInterfaceMethod.IsVirtual
						|| resolvedInterfaceMethod.IsFinal
						|| !resolvedInterfaceMethod.IsNewSlot)
						continue;

					// Try to find an implementation with a name/sig match on the current type
					MethodDefinition exactMatchOnType = TryMatchMethod (type, interfaceMethod);
					if (exactMatchOnType != null) {
						AnnotateMethods (resolvedInterfaceMethod, exactMatchOnType);
						continue;
					}

					// Next try to find an implementation with a name/sig match in the base hierarchy
					var @base = GetBaseMethodInTypeHierarchy (type, interfaceMethod);
					if (@base != null) {
						AnnotateMethods (resolvedInterfaceMethod, @base, interfaceImpl.OriginalImpl);
						continue;
					}

					// Look for a default implementation last.
					foreach (var defaultImpl in GetDefaultInterfaceImplementations (type, resolvedInterfaceMethod)) {
						Annotations.AddDefaultInterfaceImplementation (resolvedInterfaceMethod, type, defaultImpl);
					}
				}
			}
		}

		void MapVirtualMethods (TypeDefinition type)
		{
			if (!type.HasMethods)
				return;

			foreach (MethodDefinition method in type.Methods) {
				if (!method.IsVirtual)
					continue;

				MapVirtualMethod (method);

				if (method.HasOverrides)
					MapOverrides (method);
			}
		}

		void MapVirtualMethod (MethodDefinition method)
		{
			MethodDefinition @base = GetBaseMethodInTypeHierarchy (method);
			if (@base == null)
				return;

			AnnotateMethods (@base, method);
		}

		void MapOverrides (MethodDefinition method)
		{
			foreach (MethodReference override_ref in method.Overrides) {
				MethodDefinition @override = override_ref.Resolve ();
				if (@override == null)
					continue;

				AnnotateMethods (@override, method);
			}
		}

		void AnnotateMethods (MethodDefinition @base, MethodDefinition @override, InterfaceImplementation matchingInterfaceImplementation = null)
		{
			Annotations.AddBaseMethod (@override, @base);
			Annotations.AddOverride (@base, @override, matchingInterfaceImplementation);
		}

		static MethodDefinition GetBaseMethodInTypeHierarchy (MethodDefinition method)
		{
			return GetBaseMethodInTypeHierarchy (method.DeclaringType, method);
		}

		static MethodDefinition GetBaseMethodInTypeHierarchy (TypeDefinition type, MethodReference method)
		{
			TypeReference @base = type.GetInflatedBaseType ();
			while (@base != null) {
				MethodDefinition base_method = TryMatchMethod (@base, method);
				if (base_method != null)
					return base_method;

				@base = @base.GetInflatedBaseType ();
			}

			return null;
		}

		// Returns a list of default implementations of the given interface method on this type.
		// Note that this returns a list to potentially cover the diamond case (more than one
		// most specific implementation of the given interface methods). Linker needs to preserve
		// all the implementations so that the proper exception can be thrown at runtime.
		static IEnumerable<InterfaceImplementation> GetDefaultInterfaceImplementations (TypeDefinition type, MethodDefinition interfaceMethod)
		{
			// Go over all interfaces, trying to find a method that is an explicit MethodImpl of the
			// interface method in question.
			foreach (var interfaceImpl in type.Interfaces) {
				var potentialImplInterface = interfaceImpl.InterfaceType.Resolve ();
				if (potentialImplInterface == null)
					continue;

				bool foundImpl = false;

				foreach (var potentialImplMethod in potentialImplInterface.Methods) {
					if (potentialImplMethod == interfaceMethod &&
						!potentialImplMethod.IsAbstract) {
						yield return interfaceImpl;
					}

					if (!potentialImplMethod.HasOverrides)
						continue;

					// This method is an override of something. Let's see if it's the method we are looking for.
					foreach (var @override in potentialImplMethod.Overrides) {
						if (@override.Resolve () == interfaceMethod) {
							yield return interfaceImpl;
							foundImpl = true;
							break;
						}
					}

					if (foundImpl) {
						break;
					}
				}

				// We haven't found a MethodImpl on the current interface, but one of the interfaces
				// this interface requires could still provide it.
				if (!foundImpl) {
					foreach (var impl in GetDefaultInterfaceImplementations (potentialImplInterface, interfaceMethod))
						yield return impl;
				}
			}
		}

		static MethodDefinition TryMatchMethod (TypeReference type, MethodReference method)
		{
			foreach (var candidate in type.GetMethods ()) {
				if (MethodMatch (candidate, method))
					return candidate.Resolve ();
			}

			return null;
		}

		static bool MethodMatch (MethodReference candidate, MethodReference method)
		{
			var candidateDef = candidate.Resolve ();

			if (!candidateDef.IsVirtual)
				return false;

			if (candidate.HasParameters != method.HasParameters)
				return false;

			if (candidate.Name != method.Name)
				return false;

			if (candidate.HasGenericParameters != method.HasGenericParameters)
				return false;

			// we need to track what the generic parameter represent - as we cannot allow it to
			// differ between the return type or any parameter
			if (!TypeMatch (candidate.GetReturnType (), method.GetReturnType ()))
				return false;

			if (!candidate.HasParameters)
				return true;

			var cp = candidate.Parameters;
			var mp = method.Parameters;
			if (cp.Count != mp.Count)
				return false;

			if (candidate.GenericParameters.Count != method.GenericParameters.Count)
				return false;

			for (int i = 0; i < cp.Count; i++) {
				if (!TypeMatch (candidate.GetParameterType (i), method.GetParameterType (i)))
					return false;
			}

			return true;
		}

		static bool TypeMatch (IModifierType a, IModifierType b)
		{
			if (!TypeMatch (a.ModifierType, b.ModifierType))
				return false;

			return TypeMatch (a.ElementType, b.ElementType);
		}

		static bool TypeMatch (TypeSpecification a, TypeSpecification b)
		{
			if (a is GenericInstanceType gita)
				return TypeMatch (gita, (GenericInstanceType) b);

			if (a is IModifierType mta)
				return TypeMatch (mta, (IModifierType) b);

			return TypeMatch (a.ElementType, b.ElementType);
		}

		static bool TypeMatch (GenericInstanceType a, GenericInstanceType b)
		{
			if (!TypeMatch (a.ElementType, b.ElementType))
				return false;

			if (a.HasGenericArguments != b.HasGenericArguments)
				return false;

			if (!a.HasGenericArguments)
				return true;

			var gaa = a.GenericArguments;
			var gab = b.GenericArguments;
			if (gaa.Count != gab.Count)
				return false;

			for (int i = 0; i < gaa.Count; i++) {
				if (!TypeMatch (gaa[i], gab[i]))
					return false;
			}

			return true;
		}

		static bool TypeMatch (GenericParameter a, GenericParameter b)
		{
			if (a.Position != b.Position)
				return false;

			if (a.Type != b.Type)
				return false;

			return true;
		}

		static bool TypeMatch (TypeReference a, TypeReference b)
		{
			if (a is TypeSpecification || b is TypeSpecification) {
				if (a.GetType () != b.GetType ())
					return false;

				return TypeMatch ((TypeSpecification) a, (TypeSpecification) b);
			}

			if (a is GenericParameter genericParameterA && b is GenericParameter genericParameterB)
				return TypeMatch (genericParameterA, genericParameterB);

			return a.FullName == b.FullName;
		}
	}
}
