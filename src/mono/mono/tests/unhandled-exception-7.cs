using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.Remoting.Messaging;

class CustomException : Exception
{
}

class CrossDomain : MarshalByRefObject
{
	public Action NewDelegateWithTarget ()
	{
		return new Action (Bar);
	}

	public Action NewDelegateWithoutTarget ()
	{
		return () => { throw new CustomException (); };
	}

	public void Bar ()
	{
		throw new CustomException ();
	}
}

class Driver
{
	/* expected exit code: 0 */
	static void Main (string[] args)
	{
		ManualResetEvent mre = new ManualResetEvent (false);

		var cd = (CrossDomain) AppDomain.CreateDomain ("ad").CreateInstanceAndUnwrap (typeof(CrossDomain).Assembly.FullName, "CrossDomain");

		var action = cd.NewDelegateWithoutTarget ();
		var ares = action.BeginInvoke (Callback, null);

		Thread.Sleep (5000);

		Environment.Exit (1);
	}

	static void Callback (IAsyncResult iares)
	{
		((Action) ((AsyncResult) iares).AsyncDelegate).EndInvoke (iares);
	}
}
