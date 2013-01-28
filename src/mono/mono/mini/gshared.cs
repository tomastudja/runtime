using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

struct Foo {
	public int i, j;
}

struct GFoo<T> {
	public T dummy;
	public T t;
	public int i;
	public Foo f;
	public static T static_dummy;
	public static T static_t;
	public static Foo static_f;
}

struct GFoo2<T> {
	public T t, t2;
}

class GFoo3<T> {
	public T t, t2;

	public GFoo3 () {
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	public GFoo3 (T i1, T i2) {
		t = i1;
		t2 = i2;
	}
}

//
// Tests for generic sharing of vtypes.
// The tests use arrays to pass/receive values to keep the calling convention of the methods stable, which is a current limitation of the runtime support for gsharedvt.
//

// FIXME: Add mixed ref/noref tests, i.e. Dictionary<string, int>

public class Tests
{
	public static int Main (String[] args) {
		return TestDriver.RunTests (typeof (Tests), args);
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void gshared<T> (T [] array, int i, int j) {
		T tmp = array [i];
		array [i] = array [j];
		array [j] = tmp;
	}

	// Test that the gshared and gsharedvt versions don't mix
	public static int test_0_vt_gshared () {
		string[] sarr = new string [2] { "A", "B" };

		gshared<string> (sarr, 0, 1);

		Foo[] arr = new Foo [2];
		arr [0] = new Foo () { i = 1, j = 2 };
		arr [1] = new Foo () { i = 3, j = 4 };

		gshared<Foo> (arr, 0, 1);
		if (arr [0].i != 3 || arr [0].j != 4)
			return 1;
		if (arr [1].i != 1 || arr [1].j != 2)
			return 2;

		return 0;
	}

	static void ldelem_stelem<T> (T [] array, int i, int j) {
		T tmp = array [i];
		array [i] = array [j];
		array [j] = tmp;
	}

	public static int test_0_vt_ldelem_stelem () {
		Foo[] arr = new Foo [2];
		arr [0] = new Foo () { i = 1, j = 2 };
		arr [1] = new Foo () { i = 3, j = 4 };

		ldelem_stelem<Foo> (arr, 0, 1);
		if (arr [0].i != 3 || arr [0].j != 4)
			return 1;
		if (arr [1].i != 1 || arr [1].j != 2)
			return 2;

		int[] arr2 = new int [2] { 1, 2 };
		ldelem_stelem<int> (arr2, 0, 1);
		if (arr2 [0] !=2 || arr2 [1] != 1)
			return 3;

		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	private static void initobj<T> (T [] array, int i, int j) {
		T x = default(T);
		array [i] = x;
	}

	public static int test_0_vt_initobj () {
		Foo[] arr = new Foo [2];
		arr [0] = new Foo () { i = 1, j = 2 };
		arr [1] = new Foo () { i = 3, j = 4 };

		initobj<Foo> (arr, 0, 1);
		if (arr [0].i != 0 || arr [0].j != 0)
			return 1;
		if (arr [1].i != 3 || arr [1].j != 4)
			return 2;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T ldobj_stobj<T> (ref T t1, ref T t2) {
		t1 = t2;
		T t = t2;
		t2 = default(T);
		return t;
	}

	public static int test_0_vt_ldobj_stobj () {
		int i = 5;
		int j = 6;
 		if (ldobj_stobj (ref i, ref j) != 6)
			return 1;
		if (i != 6 || j != 0)
			return 2;
		double d1 = 1.0;
		double d2 = 2.0;
 		if (ldobj_stobj (ref d1, ref d2) != 2.0)
			return 3;
		if (d1 != 2.0 || d2 != 0.0)
			return 4;		
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	private static void box<T> (T [] array, object[] arr) {
		object x = array [0];
		arr [0] = x;
	}

	public static int test_0_vt_box () {
		Foo[] arr = new Foo [2];
		arr [0] = new Foo () { i = 1, j = 2 };

		object[] arr2 = new object [16];
		box<Foo> (arr, arr2);
		if (arr2 [0].GetType () != typeof (Foo))
			return 1;
		Foo f = (Foo)arr2 [0];
		if (f.i != 1 || f.j != 2)
			return 2;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	private static void unbox_any<T> (T [] array, object[] arr) {
		T t = (T)arr [0];
		array [0] = t;
	}

	public static int test_0_vt_unbox_any () {
		Foo[] arr = new Foo [2];

		object[] arr2 = new object [16];
		arr2 [0] = new Foo () { i = 1, j = 2 };
		unbox_any<Foo> (arr, arr2);
		if (arr [0].i != 1 || arr [0].j != 2)
			return 2;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void ldfld_nongeneric<T> (GFoo<T>[] foo, int[] arr) {
		arr [0] = foo [0].i;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void ldfld<T> (GFoo<T>[] foo, T[] arr) {
		arr [0] = foo [0].t;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void stfld_nongeneric<T> (GFoo<T>[] foo, int[] arr) {
		foo [0].i = arr [0];
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void stfld<T> (GFoo<T>[] foo, T[] arr) {
		foo [0].t = arr [0];
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void ldflda<T> (GFoo<T>[] foo, int[] arr) {
		arr [0] = foo [0].f.i;
	}

	public static int test_0_vt_ldfld_stfld () {
		var foo = new GFoo<Foo> () { t = new Foo () { i = 1, j = 2 }, i = 5, f = new Foo () { i = 5, j = 6 } };
		var farr = new GFoo<Foo>[] { foo };

		/* Normal fields with a variable offset */
		var iarr = new int [10];
		ldfld_nongeneric<Foo> (farr, iarr);
		if (iarr [0] != 5)
			return 1;
		iarr [0] = 16;
		stfld_nongeneric<Foo> (farr, iarr);
		if (farr [0].i != 16)
			return 2;

		/* Variable type field with a variable offset */
		var arr = new Foo [10];
		ldfld<Foo> (farr, arr);
		if (arr [0].i != 1 || arr [0].j != 2)
			return 3;
		arr [0] = new Foo () { i = 3, j = 4 };
		stfld<Foo> (farr, arr);
		if (farr [0].t.i != 3 || farr [0].t.j != 4)
			return 4;

		ldflda<Foo> (farr, iarr);
		if (iarr [0] != 5)
			return 5;

		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void stsfld<T> (T[] arr) {
		GFoo<T>.static_t = arr [0];
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void ldsfld<T> (T[] arr) {
		arr [0] = GFoo<T>.static_t;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void ldsflda<T> (int[] iarr) {
		iarr [0] = GFoo<T>.static_f.i;
	}
	
	public static int test_0_stsfld () {
		Foo[] farr = new Foo [] { new Foo () { i = 1, j = 2 } };
		stsfld<Foo> (farr);

		if (GFoo<Foo>.static_t.i != 1 || GFoo<Foo>.static_t.j != 2)
			return 1;

		Foo[] farr2 = new Foo [1];
		ldsfld<Foo> (farr2);
		if (farr2 [0].i != 1 || farr2 [0].j != 2)
			return 2;

		var iarr = new int [10];
		GFoo<Foo>.static_f = new Foo () { i = 5, j = 6 };
		ldsflda<Foo> (iarr);
		if (iarr [0] != 5)
			return 3;

		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static object newarr<T> () {
		object o = new T[10];
		return o;
	}

	public static int test_0_vt_newarr () {
		object o = newarr<Foo> ();
		if (!(o is Foo[]))
			return 1;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static Type ldtoken<T> () {
		return typeof (GFoo<T>);
	}

	public static int test_0_vt_ldtoken () {
		Type t = ldtoken<Foo> ();
		if (t != typeof (GFoo<Foo>))
			return 1;
		t = ldtoken<int> ();
		if (t != typeof (GFoo<int>))
			return 2;

		return 0;
	}

	public static int test_0_vtype_list () {
		List<int> l = new List<int> ();

		l.Add (5);
		if (l.Count != 1)
			return 1;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static int args_simple<T> (T t, int i) {
		return i;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static int args_simple<T> (T t, int i, T t2) {
		return i;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static Type args_rgctx<T> (T t, int i) {
		return typeof (T);
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static Type eh_in<T> (T t, int i) {
		throw new OverflowException ();
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T return_t<T> (T t) {
		return t;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	T return_this_t<T> (T t) {
		return t;
	}

	public static int test_0_gsharedvt_in () {
		// Check that the non-generic argument is passed at the correct stack position
		int r = args_simple<bool> (true, 42);
		if (r != 42)
			return 1;
		r = args_simple<Foo> (new Foo (), 43);
		if (r != 43)
			return 2;
		// Check that the proper rgctx is passed to the method
		Type t = args_rgctx<int> (5, 42);
		if (t != typeof (int))
			return 3;
		var v = args_simple<GFoo2<int>> (new GFoo2<int> () { t = 11, t2 = 12 }, 44, new GFoo2<int> () { t = 11, t2 = 12 });
		if (v != 44)
			return 4;
		// Check that EH works properly
		try {
			eh_in<int> (1, 2);
		} catch (OverflowException) {
		}
		return 0;
	}

	public static int test_0_gsharedvt_in_ret () {
		int i = return_t<int> (42);
		if (i != 42)
			return 1;
		long l = return_t<long> (Int64.MaxValue);
		if (l != Int64.MaxValue)
			return 2;
		double d = return_t<double> (3.0);
		if (d != 3.0)
			return 3;
		float f = return_t<float> (3.0f);
		if (f != 3.0f)
			return 4;
		short s = return_t<short> (16);
		if (s != 16)
			return 5;
		var v = new GFoo2<int> () { t = 55, t2 = 32 };
		var v2 = return_t<GFoo2<int>> (v);
		if (v2.t != 55 || v2.t2 != 32)
			return 6;
		i = new Tests ().return_this_t<int> (42);
		if (i != 42)
			return 7;
		return 0;
	}

	public static int test_0_gsharedvt_in_delegates () {
		Func<int, int> f = new Func<int, int> (return_t<int>);
		if (f (42) != 42)
			return 1;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T return2_t<T> (T t) {
		return return_t (t);
	}

	public static int test_0_gsharedvt_calls () {
		if (return2_t (2) != 2)
			return 1;
		if (return2_t ("A") != "A")
			return 2;
		if (return2_t (2.0) != 2.0)
			return 3;
		return 0;
	}

	static GFoo3<T> newobj<T> (T t1, T t2) {
		return new GFoo3<T> (t1, t2);
	}
	
	public static int test_0_gshared_new () {
		var g1 = newobj (1, 2);
		if (g1.t != 1 || g1.t2 != 2)
			return 1;
		var g2 = newobj (1.0, 2.0);
		if (g1.t != 1.0 || g1.t2 != 2.0)
			return 2;

		return 0;
	}

	//
	// Tests for transitioning out of gsharedvt code
	//

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T return_t_nogshared<T> (T t) {
		// This is not currently supported by gsharedvt
		object o = t;
		T t2 = (T)o;
		//Console.WriteLine ("X: " + t);
		return t;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static int return_int_nogshared<T> (T t) {
		// This is not currently supported by gsharedvt
		object o = t;
		T t2 = (T)o;
		return 2;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static A return_vtype_nogshared<T> (T t) {
		// This is not currently supported by gsharedvt
		object o = t;
		T t2 = (T)o;
		return new A () { a = 1, b = 2, c = 3 };
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T return2_t_out<T> (T t) {
		return return_t_nogshared (t);
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static int return2_int_out<T> (T t) {
		return return_int_nogshared (t);
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static A return2_vtype_out<T> (T t) {
		return return_vtype_nogshared (t);
	}

	struct A {
		public int a, b, c;
	}

	public static int test_0_gsharedvt_out () {
		if (return2_t_out (2) != 2)
			return 1;
		if (return2_t_out ("A") != "A")
			return 2;
		if (return2_t_out (2.0) != 2.0)
			return 3;
		if (return2_t_out (2.0f) != 2.0f)
			return 4;
		A a = new A () { a = 1, b = 2, c = 3 };
		A a2 = return2_t_out (a);
		if (a2.a != 1 || a2.b != 2 || a2.c != 3)
			return 5;
		// Calls with non gsharedvt return types
		if (return2_int_out (1) != 2)
			return 6;
		A c = return2_vtype_out (a);
		if (a2.a != 1 || a2.b != 2 || a2.c != 3)
			return 7;
		return 0;
	}

	public class GenericClass<T> {
		public static T Z (IList<T> x, int index)
		{
			return x [index];
		}
	}

	public static int test_0_generic_array_helpers () {
		int[] x = new int[] {100, 200};

		// Generic array helpers should be treated as gsharedvt-out
		if (GenericClass<int>.Z (x, 0) != 100)
			return 1;

		return 0;
	}

	internal class IntComparer : IComparer<int>
	{
		public int Compare (int ix, int iy)
		{
			if (ix == iy)
				return 0;

			if (((uint) ix) < ((uint) iy))
				return -1;
			return 1;
		}
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static int gshared_out_iface<T> (T t1, T t2, IComparer<T> comp) {
		return comp.Compare (t1, t2);
	}

	public static int test_0_gshared_out_iface () {
		// Call out from gshared to a nongeneric method through a generic interface method
		if (gshared_out_iface (2, 2, new IntComparer ()) != 0)
			return 1;
		return 0;
	}

	struct Foo1 {
		public int i1, i2, i3;
	}

	struct Foo2<T> {
		int i1, i2, i3, i4, i5;
		public T foo;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]	
	public static void locals<T> (T t) {
		Foo2<T> t2 = new Foo2<T> ();
		object o = t2;
	}

	public static int test_0_locals () {
		// Test that instantiations of type parameters are allocated the proper local type
		int i = 1;
		for (int j = 0; j < 10; ++j)
			i ++;
		locals<Foo1> (new Foo1 () { i1 = 1, i2 = 2, i3 = 3 });
		return 0;
	}

	public interface IFace<T> {
		T return_t_iface (T t);
	}

	public class Parent<T> {
		public virtual T return_t_vcall (T t) {
			throw new Exception ();
			return t;
		}
	}

	public class Child<T> : Parent<T>, IFace<T> {
		public override T return_t_vcall (T t) {
			return t;
		}
		public T return_t_iface (T t) {
			return t;
		}
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T return_t_vcall<T> (Parent<T> r, T t) {
		return r.return_t_vcall (t);
	}

	public static int test_0_vcalls () {
		if (return_t_vcall (new Child<int> (), 2) != 2)
			return 1;
		// Patching
		for (int i = 0; i < 10; ++i) {
			if (return_t_vcall (new Child<int> (), 2) != 2)
				return 2;
		}
		if (return_t_vcall (new Child<double> (), 2.0) != 2.0)
			return 3;
		return 0;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T return_t_iface<T> (IFace<T> r, T t) {
		return r.return_t_iface (t);
	}

	public static int test_0_iface_calls () {
		if (return_t_iface (new Child<int> (), 2) != 2)
			return 1;
		if (return_t_iface (new Child<double> (), 2.0) != 2.0)
			return 3;
		return 0;
	}

	static KeyValuePair<T1, T2> make_kvp<T1, T2> (T1 t1, T2 t2) {
		return new KeyValuePair<T1, T2> (t1, t2);
	}

	static T2 use_kvp<T1, T2> (KeyValuePair<T1, T2> kvp) {
		return kvp.Value;
	}
		
	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T do_kvp<T> (T a) {
		var t = make_kvp (a, a);
		// argument is an instance of a vtype instantiated with gsharedvt type arguments
		return use_kvp (t);
	}

	public static int test_0_gsharedvt_ginstvt_constructed_arg () {
		if (do_kvp<long> (1) != 1)
			return 1;
		return 0;
	}

	interface IGetter
	{
		T Get<T>();
	}

	class Getter : IGetter
	{
		public T Get<T>() { return default(T); }
	}

	abstract class Session
	{
		public abstract IGetter Getter { get; }
	}

	class IosSession : Session
	{
		private IGetter getter = new Getter();
		public override IGetter Getter { get { return getter; } }
	}

	enum ENUM_TYPE {
	}

	public static int test_0_regress_5156 () {
		new IosSession().Getter.Get<ENUM_TYPE>();
		return 0;
	}

	public struct VT
	{
		public Action a;
	}

	public class D
	{
	}

	public class A3
	{
		public void OuterMethod<TArg1>(TArg1 value)
		{
			this.InnerMethod<TArg1, long>(value, 0);
		}

		private void InnerMethod<TArg1, TArg2>(TArg1 v1, TArg2 v2)
		{
			//Console.WriteLine("{0} {1}",v1,v2);
		}
	}

	public static int test_0_regress_2096 () {
		var a = new A3();

		// The following work:
		a.OuterMethod<int>(1);
		a.OuterMethod<DateTime>(DateTime.Now);

		var v = new VT();
		a.OuterMethod<VT>(v);

		var x = new D();
		// Next line will crash with Attempting to JIT compile method on device
		//  Attempting to JIT compile method
		a.OuterMethod<D>(x);
		return 0;
	}

	public class B
	{
		public void Test<T>()
		{
			//System.Console.WriteLine(typeof(T));
		}
	}

	public class A<T>
	{
		public void Test()
		{
			new B().Test<System.Collections.Generic.KeyValuePair<T, T>>();
		}
	}

    public static int test_0_regress_6040 () {
        //new B().Test<System.Collections.Generic.KeyValuePair<string, string>>();
        new A<int>().Test();
        new A<object>().Test();
        new A<string>().Test();
		return 0;
    }

	class ArrayContainer<T> {
		private T[,] container = new T[1,1];

		public T Prop {
			[MethodImplAttribute (MethodImplOptions.NoInlining)]
			get {
				return container [0, 0];
			}
			[MethodImplAttribute (MethodImplOptions.NoInlining)]
			set {
				container [0, 0] = value;
			}
		}
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	public static int test_0_multi_dim_arrays () {
		var c = new ArrayContainer<int> ();
		c.Prop = 5;
		return c.Prop == 5 ? 0 : 1;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static T2 rgctx_in_call_innner_inner<T1, T2> (T1 t1, T2 t2) {
		return t2;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static GFoo3<T> rgctx_in_call_inner<T> (T t) {
		return rgctx_in_call_innner_inner (1, new GFoo3<T> ());
	}

    public static int test_0_rgctx_in_call () {
		// The call is made through the rgctx call, and it needs an IN trampoline
		var t = rgctx_in_call_inner (1);
		if (t is GFoo3<int>)
			return 0;
		return 1;
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void arm_params1<T> (T t1, T t2, T t3, T t4, T t5, T t6) {
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	static void arm_params2<T> (T t1, T t2, T t3, long t4, T t5, T t6) {
	}

	public static int test_0_arm_param_passing () {
		arm_params1<int> (1, 2, 3, 4, 5, 6);
		arm_params1<int> (1, 2, 3, 4, 5, 6);
		return 0;
	}

	sealed class ScheduledItem<TAbsolute, TValue> {
		private readonly object _scheduler;
		private readonly TValue _state;
		private readonly object _action;

		public ScheduledItem(object o, TValue state, object action, TAbsolute dueTime) {
			_state = state;
		}
	}

    abstract class VirtualTimeSchedulerBase<TAbsolute, TRelative> {
        public abstract void ScheduleAbsolute<TState>(TState state, TAbsolute dueTime);
	}

	class VirtualTimeScheduler<TAbsolute, TRelative> : VirtualTimeSchedulerBase<TAbsolute, TRelative> {
		public override void ScheduleAbsolute<TState>(TState state, TAbsolute dueTime) {
			var si = new ScheduledItem<TAbsolute, TState>(this, state, null, dueTime);
		}
	}

	public static int test_0_rx_mixed_regress () {
		var v = new VirtualTimeScheduler<long, long> ();
		v.ScheduleAbsolute<Action> (null, 22);
		return 0;
	}

	public class Base {
		public virtual T foo<T> (T t) {
			return t;
		}
	}

	class Class1 : Base {
		public object o;

		public override T foo<T> (T t) {
			o = t;
			return t;
		}
	}

	class Class2 : Base {
		public object o;

		public override T foo<T> (T t) {
			o = t;
			return t;
		}
	}

	[MethodImplAttribute (MethodImplOptions.NoInlining)]
	public static void bar<T> (Base b, T t) {
		b.foo (t);
	}

	public static int test_0_virtual_generic () {
		Class1 c1 = new Class1 ();
		Class2 c2 = new Class2 ();
		bar (c1, 5);
		if (!(c1.o is int) || ((int)c1.o != 5))
			return 1;
		bar (c1, 6.0);
		if (!(c1.o is double) || ((double)c1.o != 6.0))
			return 2;
		bar (c1, 7.0f);
		if (!(c1.o is float) || ((float)c1.o != 7.0f))
			return 3;
		bar (c2, 5);
		if (!(c2.o is int) || ((int)c2.o != 5))
			return 4;
		bar (c2, 6.0);
		bar (c2, 7.0f);
		return 0;
	}

}