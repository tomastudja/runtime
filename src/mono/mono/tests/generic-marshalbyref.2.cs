using System;

static class Program
{
    static int Main()
    {
	    DocumentForm<object> browseForm = new DocumentForm<object> ();
	    if (browseForm.DoInit () != 248)
		    return 1;
	    return 0;
    }
}

public abstract class EntityBase
{
}

public class GenEntity<T> : EntityBase
{
}

class DocumentForm<T>
{
	internal int DoInit()
	{
		var g = new Grid1<GenEntity<T>>(123);
		var g2 = new Grid2<GenEntity<T>>(123);
		return g.num + g2.num;
	}
}

public class Grid1<TEntity> : MarshalByRefObject
{
	public int num;

	public Grid1 (int i)
	{
		num = i + 1;
	}
}

public class Grid2<TEntity> : MarshalByRefObject where TEntity : EntityBase, new()
{
	public int num;

	public Grid2 (int i)
	{
		num = i + 1;
	}
}
