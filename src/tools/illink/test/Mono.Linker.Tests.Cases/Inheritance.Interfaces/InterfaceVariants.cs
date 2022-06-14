﻿// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

using System;
using System.Collections;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Helpers;
using Mono.Linker.Tests.Cases.Expectations.Metadata;
using Mono.Linker.Tests.Cases.Inheritance.Interfaces.Dependencies;

namespace Mono.Linker.Tests.Cases.Inheritance.Interfaces
{
	[SetupCompileBefore ("copylibrary.dll", new[] { "Dependencies/CopyLibrary.cs" })]
	[SetupLinkerAction ("copy", "copylibrary")]
	public class InterfaceVariants
	{
		public static void Main ()
		{
			Type t = typeof (UninstantiatedPublicClassWithInterface);
			t = typeof (UninstantiatedClassWithImplicitlyImplementedInterface);
			t = typeof (UninstantiatedPublicClassWithPrivateInterface);
			t = typeof (ImplementsUsedStaticInterface.InterfaceMethodUnused);

			ImplementsUnusedStaticInterface.Test (); ;
			GenericMethodThatCallsInternalStaticInterfaceMethod
				<ImplementsUsedStaticInterface.InterfaceMethodUsedThroughInterface> ();
			// Use all public interfaces - they're marked as public only to denote them as "used"
			typeof (IPublicInterface).RequiresPublicMethods ();
			typeof (IPublicStaticInterface).RequiresPublicMethods ();
			var ___ = new InstantiatedClassWithInterfaces ();
		}

		[Kept]
		internal static void GenericMethodThatCallsInternalStaticInterfaceMethod<T> () where T : IStaticInterfaceUsed
		{
			T.StaticMethodUsedThroughInterface ();
		}

		[Kept]
		class ImplementsUsedStaticInterface
		{

			[Kept]
			[KeptInterface (typeof (IStaticInterfaceUsed))]
			internal class InterfaceMethodUsedThroughInterface : IStaticInterfaceUsed
			{
				[Kept]
				public static void StaticMethodUsedThroughInterface ()
				{
				}
				public static void UnusedMethod () { }
			}

			[Kept]
			[KeptInterface (typeof (IStaticInterfaceUsed))]
			internal class InterfaceMethodUnused : IStaticInterfaceUsed
			{
				[Kept]
				public static void StaticMethodUsedThroughInterface ()
				{
				}
				public static void UnusedMethod () { }
			}
		}


		[Kept]
		internal class ImplementsUnusedStaticInterface
		{
			[Kept]
			// The interface methods themselves are not used, but the implementation of these methods is
			internal interface IStaticInterfaceMethodUnused
			{
				static abstract void InterfaceUsedMethodNot ();
			}

			internal interface IStaticInterfaceUnused
			{
				static abstract void InterfaceAndMethodNoUsed ();
			}

			[Kept]
			internal class InterfaceMethodUsedThroughImplementation : IStaticInterfaceMethodUnused, IStaticInterfaceUnused
			{
				[Kept]
				[RemovedOverride (typeof (IStaticInterfaceMethodUnused))]
				public static void InterfaceUsedMethodNot () { }

				[Kept]
				[RemovedOverride (typeof (IStaticInterfaceUnused))]
				public static void InterfaceAndMethodNoUsed () { }
			}

			[Kept]
			internal class InterfaceMethodUnused : IStaticInterfaceMethodUnused, IStaticInterfaceUnused
			{
				public static void InterfaceUsedMethodNot () { }

				public static void InterfaceAndMethodNoUsed () { }
			}

			[Kept]
			// This method keeps InterfaceMethodUnused without making it 'relevant to variant casting' like
			//	doing a typeof or type argument would do. If the type is relevant to variant casting,
			//	we will keep all interface implementations for interfaces that are kept
			internal static void KeepInterfaceMethodUnused (InterfaceMethodUnused x) { }

			[Kept]
			public static void Test ()
			{
				InterfaceMethodUsedThroughImplementation.InterfaceUsedMethodNot ();
				InterfaceMethodUsedThroughImplementation.InterfaceAndMethodNoUsed ();

				// The interface has to be kept this way, because if both the type and the interface may
				//	appear on the stack then they would be marked as relevant to variant casting and the
				//	interface implementation would be kept.
				Type t = typeof (IStaticInterfaceMethodUnused);
				KeepInterfaceMethodUnused (null);
			}
		}

		[Kept]
		[KeptInterface (typeof (IEnumerator))]
		[KeptInterface (typeof (IPublicInterface))]
		[KeptInterface (typeof (IPublicStaticInterface))]
		[KeptInterface (typeof (ICopyLibraryInterface))]
		[KeptInterface (typeof (ICopyLibraryStaticInterface))]
		public class UninstantiatedPublicClassWithInterface :
			IPublicInterface,
			IPublicStaticInterface,
			IInternalInterface,
			IInternalStaticInterface,
			IEnumerator,
			ICopyLibraryInterface,
			ICopyLibraryStaticInterface
		{
			internal UninstantiatedPublicClassWithInterface () { }

			[Kept]
			public void PublicInterfaceMethod () { }

			[Kept]
			void IPublicInterface.ExplicitImplementationPublicInterfaceMethod () { }

			[Kept]
			public static void PublicStaticInterfaceMethod () { }

			[Kept]
			static void IPublicStaticInterface.ExplicitImplementationPublicStaticInterfaceMethod () { }

			public void InternalInterfaceMethod () { }

			void IInternalInterface.ExplicitImplementationInternalInterfaceMethod () { }

			public static void InternalStaticInterfaceMethod () { }

			static void IInternalStaticInterface.ExplicitImplementationInternalStaticInterfaceMethod () { }


			[Kept]
			[ExpectBodyModified]
			bool IEnumerator.MoveNext () { throw new PlatformNotSupportedException (); }

			[Kept]
			object IEnumerator.Current {
				[Kept]
				[ExpectBodyModified]
				get { throw new PlatformNotSupportedException (); }
			}

			[Kept]
			void IEnumerator.Reset () { }

			[Kept]
			public void CopyLibraryInterfaceMethod () { }

			[Kept]
			void ICopyLibraryInterface.CopyLibraryExplicitImplementationInterfaceMethod () { }

			[Kept]
			public static void CopyLibraryStaticInterfaceMethod () { }

			[Kept]
			static void ICopyLibraryStaticInterface.CopyLibraryExplicitImplementationStaticInterfaceMethod () { }
		}

		[Kept]
		[KeptInterface (typeof (IFormattable))]
		public class UninstantiatedClassWithImplicitlyImplementedInterface : IInternalInterface, IFormattable
		{
			internal UninstantiatedClassWithImplicitlyImplementedInterface () { }

			public void InternalInterfaceMethod () { }

			void IInternalInterface.ExplicitImplementationInternalInterfaceMethod () { }

			[Kept]
			[ExpectBodyModified]
			[ExpectLocalsModified]
			public string ToString (string format, IFormatProvider formatProvider)
			{
				return "formatted string";
			}
		}

		[Kept]
		[KeptInterface (typeof (IEnumerator))]
		[KeptInterface (typeof (IPublicInterface))]
		[KeptInterface (typeof (IPublicStaticInterface))]
		[KeptInterface (typeof (ICopyLibraryInterface))]
		[KeptInterface (typeof (ICopyLibraryStaticInterface))]
		public class InstantiatedClassWithInterfaces :
			IPublicInterface,
			IPublicStaticInterface,
			IInternalInterface,
			IInternalStaticInterface,
			IEnumerator,
			ICopyLibraryInterface,
			ICopyLibraryStaticInterface
		{
			[Kept]
			public InstantiatedClassWithInterfaces () { }

			[Kept]
			public void PublicInterfaceMethod () { }

			[Kept]
			void IPublicInterface.ExplicitImplementationPublicInterfaceMethod () { }

			[Kept]
			public static void PublicStaticInterfaceMethod () { }

			[Kept]
			static void IPublicStaticInterface.ExplicitImplementationPublicStaticInterfaceMethod () { }

			public void InternalInterfaceMethod () { }

			void IInternalInterface.ExplicitImplementationInternalInterfaceMethod () { }

			public static void InternalStaticInterfaceMethod () { }

			static void IInternalStaticInterface.ExplicitImplementationInternalStaticInterfaceMethod () { }

			[Kept]
			bool IEnumerator.MoveNext () { throw new PlatformNotSupportedException (); }

			[Kept]
			object IEnumerator.Current { [Kept] get { throw new PlatformNotSupportedException (); } }

			[Kept]
			void IEnumerator.Reset () { }

			[Kept]
			public void CopyLibraryInterfaceMethod () { }

			[Kept]
			void ICopyLibraryInterface.CopyLibraryExplicitImplementationInterfaceMethod () { }

			[Kept]
			public static void CopyLibraryStaticInterfaceMethod () { }

			[Kept]
			static void ICopyLibraryStaticInterface.CopyLibraryExplicitImplementationStaticInterfaceMethod () { }
		}

		[Kept]
		public class UninstantiatedPublicClassWithPrivateInterface : IPrivateInterface
		{
			internal UninstantiatedPublicClassWithPrivateInterface () { }

			void IPrivateInterface.PrivateInterfaceMethod () { }
		}

		[Kept]
		public interface IPublicInterface
		{
			[Kept]
			void PublicInterfaceMethod ();

			[Kept]
			void ExplicitImplementationPublicInterfaceMethod ();
		}

		[Kept]
		public interface IPublicStaticInterface
		{
			[Kept]
			static abstract void PublicStaticInterfaceMethod ();

			[Kept]
			static abstract void ExplicitImplementationPublicStaticInterfaceMethod ();
		}

		internal interface IInternalInterface
		{
			void InternalInterfaceMethod ();

			void ExplicitImplementationInternalInterfaceMethod ();
		}

		internal interface IInternalStaticInterface
		{
			static abstract void InternalStaticInterfaceMethod ();

			static abstract void ExplicitImplementationInternalStaticInterfaceMethod ();
		}

		// The interface methods themselves are used through the interface
		[Kept]
		internal interface IStaticInterfaceUsed
		{
			[Kept]
			static abstract void StaticMethodUsedThroughInterface ();

			static abstract void UnusedMethod ();
		}

		private interface IPrivateInterface
		{
			void PrivateInterfaceMethod ();
		}
	}
}
