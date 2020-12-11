using System;
using System.Runtime.Serialization;
using Mono.Linker.Tests.Cases.Expectations.Assertions;

namespace Mono.Linker.Tests.Cases.BCLFeatures
{
	public class SerializationCtors
	{
		public static void Main ()
		{
			new C (2);
		}
	}

	[Kept]

	[Serializable]
	class C
	{
#if !NETCOREAPP
		[Kept]
#endif
		public C ()
		{
		}

		[Kept]
		public C (int i)
		{
		}

#if !NETCOREAPP
		[Kept]
#endif
		protected C (SerializationInfo info, StreamingContext context)
		{
		}
	}
}
