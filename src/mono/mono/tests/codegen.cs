using System;
using System.Reflection;
using System.Reflection.Emit;

class CGen {

	public static int Main() {
		AssemblyBuilder abuilder;
		ModuleBuilder mbuilder;
		TypeBuilder tbuilder;
		FieldBuilder fbuilder;
		PropertyBuilder pbuilder;
		AssemblyName an;
		String name = "tcgen.exe";
		MethodBuilder method, get_method;
		TypeAttributes attrs = TypeAttributes.Public | TypeAttributes.Class;
		MethodAttributes mattrs = MethodAttributes.Public | MethodAttributes.Static;
		byte[] body = {0x16, 0x2a}; // ldc.i4.0 ret

		an = new AssemblyName ();
		an.Name = name;
		abuilder = AppDomain.CurrentDomain.DefineDynamicAssembly (an, AssemblyBuilderAccess.Save);

		mbuilder = abuilder.DefineDynamicModule (name, name);

		tbuilder = mbuilder.DefineType ("Test.CodeGen", attrs);
		Type result = typeof(int);
		Type[] param = new Type[] {typeof (String[])};
		method = tbuilder.DefineMethod("Main", mattrs, result, param);
		method.CreateMethodBody (body, body.Length);

		fbuilder = tbuilder.DefineField ("int_field", typeof(int), FieldAttributes.Private);
		fbuilder = tbuilder.DefineField ("string_field", typeof(string), FieldAttributes.Public);
		/*pbuilder = tbuilder.DefineProperty ("FieldI", PropertyAttributes.None, typeof(int), null);
		get_method = tbuilder.DefineMethod("get_FieldI", MethodAttributes.Public, result, null);
		get_method.CreateMethodBody (body, body.Length);
		pbuilder.SetGetMethod (get_method);*/

		Type t = tbuilder.CreateType ();
		abuilder.SetEntryPoint (method);
		abuilder.Save (name);
		Console.WriteLine ("abuilder == module.assembly: {0}", abuilder == mbuilder.Assembly);
		Console.WriteLine ("abuilder == type.assembly: {0}", abuilder == t.Assembly);
		Console.WriteLine ("abuilder == tbuilder.assembly: {0}", abuilder == tbuilder.Assembly);
		Console.WriteLine ("mbuilder == type.Module: {0}", mbuilder == t.Module);
		Console.WriteLine ("mbuilder == tbuilder.Module: {0}", mbuilder == tbuilder.Module);
		return 0;
	}
}
