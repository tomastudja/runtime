// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices.JavaScript;
using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace System.Runtime.InteropServices.JavaScript.Tests
{
    public static class HelperMarshal
    {
        internal const string INTEROP_CLASS = "[System.Runtime.InteropServices.JavaScript.Tests]System.Runtime.InteropServices.JavaScript.Tests.HelperMarshal:";
        internal static int _i32Value;
        private static void InvokeI32(int a, int b)
        {
            _i32Value = a + b;
        }

        internal static float _f32Value;
        private static void InvokeFloat(float f)
        {
            _f32Value = f;
        }

        internal static double _f64Value;
        private static void InvokeDouble(double d)
        {
            _f64Value = d;
        }

        internal static long _i64Value;
        private static void InvokeLong(long l)
        {
            _i64Value = l;
        }

        internal static byte[] _byteBuffer;
        private static void MarshalArrayBuffer(ArrayBuffer buffer)
        {
            using (var bytes = new Uint8Array(buffer))
                _byteBuffer = bytes.ToArray();
        }

        private static void MarshalByteBuffer(Uint8Array buffer)
        {
            _byteBuffer = buffer.ToArray();
        }

        internal static int[] _intBuffer;
        private static void MarshalArrayBufferToInt32Array(ArrayBuffer buffer)
        {
            using (var ints = new Int32Array(buffer))
                _intBuffer = ints.ToArray();
        }

        internal static string _stringResource;
        private static void InvokeString(string s)
        {
            _stringResource = s;
        }

        internal static string _marshalledString;
        private static string InvokeMarshalString()
        {
            _marshalledString = "Hic Sunt Dracones";
            return _marshalledString;
        }

        internal static object _object1;
        private static object InvokeObj1(object obj)
        {
            _object1 = obj;
            return obj;
        }

        internal static object _object2;
        private static object InvokeObj2(object obj)
        {
            _object2 = obj;
            return obj;
        }

        internal static object _marshalledObject;
        private static object InvokeMarshalObj()
        {
            _marshalledObject = new object();
            return _marshalledObject;
        }

        internal static int _valOne, _valTwo;
        private static void ManipulateObject(JSObject obj)
        {
            _valOne = (int)obj.Invoke("inc");
            _valTwo = (int)obj.Invoke("add", 20);
        }

        internal static object[] _jsObjects;
        private static void MinipulateObjTypes(JSObject obj)
        {
            _jsObjects = new object[4];
            _jsObjects[0] = obj.Invoke("return_int");
            _jsObjects[1] = obj.Invoke("return_double");
            _jsObjects[2] = obj.Invoke("return_string");
            _jsObjects[3] = obj.Invoke("return_bool");
        }

        internal static int _jsAddFunctionResult;
        private static void UseFunction(JSObject obj)
        {
            _jsAddFunctionResult = (int)obj.Invoke("call", null, 10, 20);
        }

        internal static int _jsAddAsFunctionResult;
        private static void UseAsFunction(Function func)
        {
            _jsAddAsFunctionResult = (int)func.Call(null, 20, 30);
        }

        internal static int _functionResultValue;
        private static Func<int, int, int> CreateFunctionDelegate()
        {
            return (a, b) =>
            {
                _functionResultValue = a + b;
                return _functionResultValue;
            };
        }

        internal static int _intValue;
        private static void InvokeInt(int value)
        {
            _intValue = value;
        }

        internal static IntPtr _intPtrValue;
        private static void InvokeIntPtr(IntPtr i)
        {
            _intPtrValue = i;
        }

        internal static IntPtr _marshaledIntPtrValue;
        private static IntPtr InvokeMarshalIntPtr()
        {
            _marshaledIntPtrValue = (IntPtr)42;
            return _marshaledIntPtrValue;
        }

        internal static object[] _jsProperties;
        private static void RetrieveObjectProperties(JSObject obj)
        {
            _jsProperties = new object[4];
            _jsProperties[0] = obj.GetObjectProperty("myInt");
            _jsProperties[1] = obj.GetObjectProperty("myDouble");
            _jsProperties[2] = obj.GetObjectProperty("myString");
            _jsProperties[3] = obj.GetObjectProperty("myBoolean");
        }

        private static void PopulateObjectProperties(JSObject obj, bool createIfNotExist)
        {
            _jsProperties = new object[4];
            obj.SetObjectProperty("myInt", 100, createIfNotExist);
            obj.SetObjectProperty("myDouble", 4.5, createIfNotExist);
            obj.SetObjectProperty("myString", "qwerty", createIfNotExist);
            obj.SetObjectProperty("myBoolean", true, createIfNotExist);
        }

        private static void MarshalByteBufferToInts(ArrayBuffer buffer)
        {
            using (var bytes = new Uint8Array(buffer))
            {
                var byteBuffer = bytes.ToArray();
                _intBuffer = new int[bytes.Length / sizeof(int)];
                for (int i = 0; i < bytes.Length; i += sizeof(int))
                    _intBuffer[i / sizeof(int)] = BitConverter.ToInt32(byteBuffer, i);
            }
        }

        private static void MarshalInt32Array(Int32Array buffer)
        {
            _intBuffer = buffer.ToArray();
        }

        internal static float[] _floatBuffer;
        private static void MarshalFloat32Array(Float32Array buffer)
        {
            _floatBuffer = buffer.ToArray();
        }
        private static void MarshalArrayBufferToFloat32Array(ArrayBuffer buffer)
        {
            using (var floats = new Float32Array(buffer))
                _floatBuffer = floats.ToArray();
        }

        internal static double[] _doubleBuffer;
        private static void MarshalFloat64Array(Float64Array buffer)
        {
            _doubleBuffer = buffer.ToArray();
        }

        private static void MarshalArrayBufferToFloat64Array(ArrayBuffer buffer)
        {
            using (var doubles = new Float64Array(buffer))
                _doubleBuffer = doubles.ToArray();
        }

        private static void MarshalByteBufferToDoubles(ArrayBuffer buffer)
        {
            using (var doubles = new Float64Array(buffer))
                _doubleBuffer = doubles.ToArray();
        }

        private static void SetTypedArraySByte(JSObject obj)
        {
            sbyte[] buffer = Enumerable.Repeat((sbyte)0x20, 11).ToArray();
            obj.SetObjectProperty("typedArray", Int8Array.From(buffer));
        }

        internal static sbyte[] _taSByte;
        private static void GetTypedArraySByte(JSObject obj)
        {
            _taSByte = ((Int8Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        private static void SetTypedArrayByte(JSObject obj)
        {
            var dragons = "hic sunt dracones";
            byte[] buffer = System.Text.Encoding.ASCII.GetBytes(dragons);
            obj.SetObjectProperty("dracones", Uint8Array.From(buffer));
        }

        internal static byte[] _taByte;
        private static void GetTypedArrayByte(JSObject obj)
        {
            _taByte = ((Uint8Array)obj.GetObjectProperty("dracones")).ToArray();
        }

        private static void SetTypedArrayShort(JSObject obj)
        {
            short[] buffer = Enumerable.Repeat((short)0x20, 13).ToArray();
            obj.SetObjectProperty("typedArray", Int16Array.From(buffer));
        }

        internal static short[] _taShort;
        private static void GetTypedArrayShort(JSObject obj)
        {
            _taShort = ((Int16Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        private static void SetTypedArrayUShort(JSObject obj)
        {
            ushort[] buffer = Enumerable.Repeat((ushort)0x20, 14).ToArray();
            obj.SetObjectProperty("typedArray", Uint16Array.From(buffer));
        }

        internal static ushort[] _taUShort;
        private static void GetTypedArrayUShort(JSObject obj)
        {
            _taUShort = ((Uint16Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        private static void SetTypedArrayInt(JSObject obj)
        {
            int[] buffer = Enumerable.Repeat((int)0x20, 15).ToArray();
            obj.SetObjectProperty("typedArray", Int32Array.From(buffer));
        }

        internal static int[] _taInt;
        private static void GetTypedArrayInt(JSObject obj)
        {
            _taInt = ((Int32Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        public static void SetTypedArrayUInt(JSObject obj)
        {
            uint[] buffer = Enumerable.Repeat((uint)0x20, 16).ToArray();
            obj.SetObjectProperty("typedArray", Uint32Array.From(buffer));
        }

        internal static uint[] _taUInt;
        private static void GetTypedArrayUInt(JSObject obj)
        {
            _taUInt = ((Uint32Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        private static void SetTypedArrayFloat(JSObject obj)
        {
            float[] buffer = Enumerable.Repeat(3.14f, 17).ToArray();
            obj.SetObjectProperty("typedArray", Float32Array.From(buffer));
        }

        internal static float[] _taFloat;
        private static void GetTypedArrayFloat(JSObject obj)
        {
            _taFloat = ((Float32Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        private static void SetTypedArrayDouble(JSObject obj)
        {
            double[] buffer = Enumerable.Repeat(3.14d, 18).ToArray();
            obj.SetObjectProperty("typedArray", Float64Array.From(buffer));
        }

        internal static double[] _taDouble;
        private static void GetTypedArrayDouble(JSObject obj)
        {
            _taDouble = ((Float64Array)obj.GetObjectProperty("typedArray")).ToArray();
        }

        private static Function _sumFunction;
        private static void CreateFunctionSum()
        {
            _sumFunction = new Function("a", "b", "return a + b");
        }

        internal static int _sumValue = 0;
        private static void CallFunctionSum()
        {
            _sumValue = (int)_sumFunction.Call(null, 3, 5);
        }

        private static Function _mathMinFunction;
        private static void CreateFunctionApply()
        {
            var math = (JSObject)Runtime.GetGlobalObject("Math");
            _mathMinFunction = (Function)math.GetObjectProperty("min");

        }

        internal static int _minValue = 0;
        private static void CallFunctionApply()
        {
            _minValue = (int)_mathMinFunction.Apply(null, new object[] { 5, 6, 2, 3, 7 });
        }
    }
}
