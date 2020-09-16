﻿using Mono.Cecil;

namespace Mono.Linker
{
	public static class AssemblyDefinitionExtensions
	{
		public static EmbeddedResource FindEmbeddedResource (this AssemblyDefinition assembly, string name)
		{
			foreach (var resource in assembly.MainModule.Resources) {
				if (resource is EmbeddedResource embeddedResource && embeddedResource.Name == name)
					return embeddedResource;
			}
			return null;
		}
	}
}
