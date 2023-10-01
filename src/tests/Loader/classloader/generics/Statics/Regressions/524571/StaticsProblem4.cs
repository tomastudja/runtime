// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// the subtypes here that contain a Canonical type are Node <NodeStruct<NodeSys<a>>> and Node <NodeClass<NodeSys<a>>>

using System;

using Xunit;

public class Node<a> 
{
    public static Node<a> leaf;

        static Node() 
    {
            leaf = new Node<a>();
            Console.WriteLine("Node<A>'s .cctor ran, where A was {0}.", typeof(a));
        Console.WriteLine("Leaf: {0}", leaf);
        }
}


public struct NodeStruct<a> { }
public class NodeClass<a> { }
public class NodeSys<a> { }

public class SystemMap<a>
{
    public Node <NodeStruct<NodeSys<a>>> rootStruct;
    public Node <NodeClass<NodeSys<a>>> rootClass;

        public SystemMap(a x)
        {
            Console.WriteLine("Accessing a static from Node<NodeStruct<NodeSys<a>>>...");
            this.rootStruct = Node<NodeStruct<NodeSys<a>>>.leaf;
            
            Console.WriteLine("\nAccessing a static from Node<NodeClass<NodeSys<a>>>...");
            this.rootClass = Node<NodeClass<NodeSys<a>>>.leaf;
    }

    public bool Eval()
    {
        Console.WriteLine("Read a static from Node<NodeStruct<NodeSys<a>>>.  Got: {0}", (rootStruct == null) ? "<null>" : rootStruct.ToString());
        Console.WriteLine("Read a static from Node<NodeClass<NodeSys<a>>>.  Got: {0}", (rootClass == null) ? "<null>" : rootClass.ToString());

        if (rootStruct == null || rootClass == null)
            return false;
        else
            return true;

    }
}


public class Test
{
    [Fact]
    public static void TestEntryPoint() 
    { 
        Console.WriteLine("-------------------------------------------------------------------");
        SystemMap<Int32>  y1 = new SystemMap<Int32> (5);
        Console.WriteLine("-------------------------------------------------------------------");
        SystemMap<String> y2 = new SystemMap<String> ("S");
        Console.WriteLine("-------------------------------------------------------------------");
        SystemMap<Object> y3 = new SystemMap<Object> ("S");
        Console.WriteLine("-------------------------------------------------------------------");

        Assert.True(y1.Eval());
        Assert.True(y2.Eval());
        Assert.True(y3.Eval());
    }
}
