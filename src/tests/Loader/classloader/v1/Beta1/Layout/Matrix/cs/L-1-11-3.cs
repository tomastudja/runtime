// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

//////////////////////////////////////////////////////////
// L-1-11-1.cs - Beta1 Layout Test - RDawson
//
// Tests layout of classes two unrelated classes in
// the same assembly and module
//
// See ReadMe.txt in the same project as this source for
// further details about these tests.
//

using System;

class Test{
public static int Main(){
  int mi_RetCode = 100;
  A a = new A();
  B b = new B();

//  if(a.Test(b) != 100)
//    mi_RetCode = 0;

  if(b.Test(a) != 100)
    mi_RetCode = 0;

  if(mi_RetCode == 100)
    Console.WriteLine("Pass");
  else
    Console.WriteLine("FAIL");

  return mi_RetCode;
}
}


class B{
public int Test(A a){
  int mi_RetCode = 100;

  /////////////////////////////////
  // Test instance field access
  a.FldPubInst = 100;
  if(a.FldPubInst != 100)
    mi_RetCode = 0;

  //@csharp - Note that C# will not compile an illegal access of a.FldPrivInst
  //So there is no negative test here, it should be covered elsewhere and
  //should throw a FieldAccessException within the runtime.  (IL sources is
  //the most logical, only?, choice)

  //@csharp - C# Won't compile illegial family access from non-family members

  /////////////////////////////////
  // Test static field access
  A.FldPubStat = 100;
  if(A.FldPubStat != 100)
    mi_RetCode = 0;

  //@csharp - Again, note C# won't do private field access

  //@csharp - C# Won't compile illegial family access from non-family members

  /////////////////////////////////
  // Test instance a.Method access
  if(a.MethPubInst() != 100)
    mi_RetCode = 0;

  //@csharp - C# won't do private a.Method access

  //@csharp - C# Won't compile illegial family access from non-family members

  /////////////////////////////////
  // Test static a.Method access
  if(A.MethPubStat() != 100)
    mi_RetCode = 0;

  //@csharp - C# won't do private a.Method access

  //@csharp - C# Won't compile illegial family access from non-family members

  /////////////////////////////////
  // Test virtual a.Method access
  if(a.MethPubVirt() != 100)
    mi_RetCode = 0;

  //@csharp - C# won't do private a.Method access

  //@csharp - C# Won't compile illegial family access from non-family members

  return mi_RetCode;
}


  //////////////////////////////
  // Instance Fields
public int FldPubInst;
private int FldPrivInst;
protected int FldFamInst;          //Translates to "family"
internal int FldAsmInst;           //Translates to "assembly"
protected internal int FldFoaInst; //Translates to "famorassem"

  //////////////////////////////
  // Static Fields
public static int FldPubStat;
private static int FldPrivStat;
protected static int FldFamStat;   //family
internal static int FldAsmStat;    //assembly
protected internal static int FldFoaStat; //famorassem

  //////////////////////////////
  // Instance Methods
public int MethPubInst(){
  Console.WriteLine("B::MethPubInst()");
  return 100;
}

private int MethPrivInst(){
  Console.WriteLine("B::MethPrivInst()");
  return 100;
}

protected int MethFamInst(){
  Console.WriteLine("B::MethFamInst()");
  return 100;
}

internal int MethAsmInst(){
  Console.WriteLine("B::MethAsmInst()");
  return 100;
}

protected internal int MethFoaInst(){
  Console.WriteLine("B::MethFoaInst()");
  return 100;
}

  //////////////////////////////
  // Static Methods
public static int MethPubStat(){
  Console.WriteLine("B::MethPubStat()");
  return 100;
}

private static int MethPrivStat(){
  Console.WriteLine("B::MethPrivStat()");
  return 100;
}

protected static int MethFamStat(){
  Console.WriteLine("B::MethFamStat()");
  return 100;
}

internal static int MethAsmStat(){
  Console.WriteLine("B::MethAsmStat()");
  return 100;
}

protected internal static int MethFoaStat(){
  Console.WriteLine("B::MethFoaStat()");
  return 100;
}

  //////////////////////////////
  // Virtual Instance Methods
public virtual int MethPubVirt(){
  Console.WriteLine("B::MethPubVirt()");
  return 100;
}

  //@csharp - Note that C# won't compile an illegal private virtual function
  //So there is no negative testing MethPrivVirt() here.

protected virtual int MethFamVirt(){
  Console.WriteLine("B::MethFamVirt()");
  return 100;
}

internal virtual int MethAsmVirt(){
  Console.WriteLine("B::MethAsmVirt()");
  return 100;
}

protected internal virtual int MethFoaVirt(){
  Console.WriteLine("B::MethFoaVirt()");
  return 100;
}

}




