// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/******************************************************************************
 * This file is auto-generated from a template file by the GenerateTests.csx  *
 * script in tests\src\JIT\HardwareIntrinsics.Arm\Shared. In order to make    *
 * changes, please update the corresponding template and run according to the *
 * directions listed in the file.                                             *
 ******************************************************************************/

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Intrinsics.Arm;

namespace JIT.HardwareIntrinsics.Arm
{
    public static partial class Program
    {
        private static void LoadAndInsertScalar_Vector64_Byte_7()
        {
            var test = new LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7();

            if (test.IsSupported)
            {
                // Validates basic functionality works, using Unsafe.Read
                test.RunBasicScenario_UnsafeRead();

                if (AdvSimd.IsSupported)
                {
                    // Validates basic functionality works, using Load
                    test.RunBasicScenario_Load();
                }

                // Validates calling via reflection works, using Unsafe.Read
                test.RunReflectionScenario_UnsafeRead();

                if (AdvSimd.IsSupported)
                {
                    // Validates calling via reflection works, using Load
                    test.RunReflectionScenario_Load();
                }

                // Validates passing a static member works
                test.RunClsVarScenario();

                if (AdvSimd.IsSupported)
                {
                    // Validates passing a static member works, using pinning and Load
                    test.RunClsVarScenario_Load();
                }

                // Validates passing a local works, using Unsafe.Read
                test.RunLclVarScenario_UnsafeRead();

                if (AdvSimd.IsSupported)
                {
                    // Validates passing a local works, using Load
                    test.RunLclVarScenario_Load();
                }

                // Validates passing the field of a local class works
                test.RunClassLclFldScenario();

                if (AdvSimd.IsSupported)
                {
                    // Validates passing the field of a local class works, using pinning and Load
                    test.RunClassLclFldScenario_Load();
                }

                // Validates passing an instance member of a class works
                test.RunClassFldScenario();

                if (AdvSimd.IsSupported)
                {
                    // Validates passing an instance member of a class works, using pinning and Load
                    test.RunClassFldScenario_Load();
                }

                // Validates passing the field of a local struct works
                test.RunStructLclFldScenario();

                if (AdvSimd.IsSupported)
                {
                    // Validates passing the field of a local struct works, using pinning and Load
                    test.RunStructLclFldScenario_Load();
                }

                // Validates passing an instance member of a struct works
                test.RunStructFldScenario();

                if (AdvSimd.IsSupported)
                {
                    // Validates passing an instance member of a struct works, using pinning and Load
                    test.RunStructFldScenario_Load();
                }
            }
            else
            {
                // Validates we throw on unsupported hardware
                test.RunUnsupportedScenario();
            }

            if (!test.Succeeded)
            {
                throw new Exception("One or more scenarios did not complete as expected.");
            }
        }
    }

    public sealed unsafe class LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7
    {
        private struct DataTable
        {
            private byte[] inArray1;
            private byte[] outArray;

            private GCHandle inHandle1;
            private GCHandle outHandle;

            private ulong alignment;

            public DataTable(Byte[] inArray1, Byte[] outArray, int alignment)
            {
                int sizeOfinArray1 = inArray1.Length * Unsafe.SizeOf<Byte>();
                int sizeOfoutArray = outArray.Length * Unsafe.SizeOf<Byte>();
                if ((alignment != 16 && alignment != 8) || (alignment * 2) < sizeOfinArray1 || (alignment * 2) < sizeOfoutArray)
                {
                    throw new ArgumentException("Invalid value of alignment");
                }

                this.inArray1 = new byte[alignment * 2];
                this.outArray = new byte[alignment * 2];

                this.inHandle1 = GCHandle.Alloc(this.inArray1, GCHandleType.Pinned);
                this.outHandle = GCHandle.Alloc(this.outArray, GCHandleType.Pinned);

                this.alignment = (ulong)alignment;

                Unsafe.CopyBlockUnaligned(ref Unsafe.AsRef<byte>(inArray1Ptr), ref Unsafe.As<Byte, byte>(ref inArray1[0]), (uint)sizeOfinArray1);
            }

            public void* inArray1Ptr => Align((byte*)(inHandle1.AddrOfPinnedObject().ToPointer()), alignment);
            public void* outArrayPtr => Align((byte*)(outHandle.AddrOfPinnedObject().ToPointer()), alignment);

            public void Dispose()
            {
                inHandle1.Free();
                outHandle.Free();
            }

            private static unsafe void* Align(byte* buffer, ulong expectedAlignment)
            {
                return (void*)(((ulong)buffer + expectedAlignment - 1) & ~(expectedAlignment - 1));
            }
        }

        private struct TestStruct
        {
            public Vector64<Byte> _fld1;
            public Byte _fld3;

            public static TestStruct Create()
            {
                var testStruct = new TestStruct();

                for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetByte(); }
                Unsafe.CopyBlockUnaligned(ref Unsafe.As<Vector64<Byte>, byte>(ref testStruct._fld1), ref Unsafe.As<Byte, byte>(ref _data1[0]), (uint)Unsafe.SizeOf<Vector64<Byte>>());

                testStruct._fld3 = TestLibrary.Generator.GetByte();

                return testStruct;
            }

            public void RunStructFldScenario(LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7 testClass)
            {
                fixed (Byte* pFld3 = &_fld3)
                {
                    var result = AdvSimd.LoadAndInsertScalar(_fld1, 7, pFld3);

                    Unsafe.Write(testClass._dataTable.outArrayPtr, result);
                }

                testClass.ValidateResult(_fld1, _fld3, testClass._dataTable.outArrayPtr);
            }

            public void RunStructFldScenario_Load(LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7 testClass)
            {
                fixed (Vector64<Byte>* pFld1 = &_fld1)
                fixed (Byte* pFld3 = &_fld3)
                {
                    var result = AdvSimd.LoadAndInsertScalar(
                        AdvSimd.LoadVector64((Byte*)pFld1),
                        7,
                        pFld3
                    );

                    Unsafe.Write(testClass._dataTable.outArrayPtr, result);
                    testClass.ValidateResult(_fld1, _fld3, testClass._dataTable.outArrayPtr);
                }
            }
        }

        private static readonly int LargestVectorSize = 8;

        private static readonly int Op1ElementCount = Unsafe.SizeOf<Vector64<Byte>>() / sizeof(Byte);
        private static readonly int RetElementCount = Unsafe.SizeOf<Vector64<Byte>>() / sizeof(Byte);
        private static readonly byte ElementIndex = 7;

        private static Byte[] _data1 = new Byte[Op1ElementCount];

        private static Vector64<Byte> _clsVar1;
        private static Byte _clsVar3;

        private Vector64<Byte> _fld1;
        private Byte _fld3;

        private DataTable _dataTable;

        static LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7()
        {
            for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetByte(); }
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Vector64<Byte>, byte>(ref _clsVar1), ref Unsafe.As<Byte, byte>(ref _data1[0]), (uint)Unsafe.SizeOf<Vector64<Byte>>());

            _clsVar3 = TestLibrary.Generator.GetByte();
        }

        public LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7()
        {
            Succeeded = true;

            for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetByte(); }
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Vector64<Byte>, byte>(ref _fld1), ref Unsafe.As<Byte, byte>(ref _data1[0]), (uint)Unsafe.SizeOf<Vector64<Byte>>());

            _fld3 = TestLibrary.Generator.GetByte();

            for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetByte(); }
            _dataTable = new DataTable(_data1, new Byte[RetElementCount], LargestVectorSize);
        }

        public bool IsSupported => AdvSimd.IsSupported;

        public bool Succeeded { get; set; }

        public void RunBasicScenario_UnsafeRead()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunBasicScenario_UnsafeRead));

            Byte op3 = TestLibrary.Generator.GetByte();

            var result = AdvSimd.LoadAndInsertScalar(
                Unsafe.Read<Vector64<Byte>>(_dataTable.inArray1Ptr),
                7,
                &op3
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(_dataTable.inArray1Ptr, op3, _dataTable.outArrayPtr);
        }

        public void RunBasicScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunBasicScenario_Load));

            Byte op3 = TestLibrary.Generator.GetByte();

            var result = AdvSimd.LoadAndInsertScalar(
                AdvSimd.LoadVector64((Byte*)(_dataTable.inArray1Ptr)),
                7,
                &op3
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(_dataTable.inArray1Ptr, op3, _dataTable.outArrayPtr);
        }

        public void RunReflectionScenario_UnsafeRead()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunReflectionScenario_UnsafeRead));

            Byte op3 = TestLibrary.Generator.GetByte();

            var result = typeof(AdvSimd).GetMethod(nameof(AdvSimd.LoadAndInsertScalar), new Type[] { typeof(Vector64<Byte>), typeof(byte), typeof(Byte*) })
                                     .Invoke(null, new object[] {
                                        Unsafe.Read<Vector64<Byte>>(_dataTable.inArray1Ptr),
                                        ElementIndex,
                                        Pointer.Box(&op3, typeof(Byte*))
                                     });

            Unsafe.Write(_dataTable.outArrayPtr, (Vector64<Byte>)(result));
            ValidateResult(_dataTable.inArray1Ptr, op3, _dataTable.outArrayPtr);
        }

        public void RunReflectionScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunReflectionScenario_Load));

            Byte op3 = TestLibrary.Generator.GetByte();

            var result = typeof(AdvSimd).GetMethod(nameof(AdvSimd.LoadAndInsertScalar), new Type[] { typeof(Vector64<Byte>), typeof(byte), typeof(Byte*) })
                                     .Invoke(null, new object[] {
                                        AdvSimd.LoadVector64((Byte*)(_dataTable.inArray1Ptr)),
                                        ElementIndex,
                                        Pointer.Box(&op3, typeof(Byte*))
                                     });

            Unsafe.Write(_dataTable.outArrayPtr, (Vector64<Byte>)(result));
            ValidateResult(_dataTable.inArray1Ptr, op3, _dataTable.outArrayPtr);
        }

        public void RunClsVarScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClsVarScenario));

            fixed (Byte* pClsVar3 = &_clsVar3)
            {
                var result = AdvSimd.LoadAndInsertScalar(
                    _clsVar1,
                    7,
                    pClsVar3
                );
                Unsafe.Write(_dataTable.outArrayPtr, result);
            }
            ValidateResult(_clsVar1, _clsVar3, _dataTable.outArrayPtr);
        }

        public void RunClsVarScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClsVarScenario_Load));

            fixed (Vector64<Byte>* pClsVar1 = &_clsVar1)
            fixed (Byte* pClsVar3 = &_clsVar3)
            {
                var result = AdvSimd.LoadAndInsertScalar(
                    AdvSimd.LoadVector64((Byte*)pClsVar1),
                    7,
                    pClsVar3
                );
                Unsafe.Write(_dataTable.outArrayPtr, result);
            }
            ValidateResult(_clsVar1, _clsVar3, _dataTable.outArrayPtr);
        }

        public void RunLclVarScenario_UnsafeRead()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunLclVarScenario_UnsafeRead));

            var op1 = Unsafe.Read<Vector64<Byte>>(_dataTable.inArray1Ptr);
            var op3 = TestLibrary.Generator.GetByte();

            var result = AdvSimd.LoadAndInsertScalar(op1, 7, &op3);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(op1, op3, _dataTable.outArrayPtr);
        }

        public void RunLclVarScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunLclVarScenario_Load));

            var op1 = AdvSimd.LoadVector64((Byte*)(_dataTable.inArray1Ptr));
            var op3 = TestLibrary.Generator.GetByte();

            var result = AdvSimd.LoadAndInsertScalar(op1, 7, &op3);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(op1, op3, _dataTable.outArrayPtr);
        }

        public void RunClassLclFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassLclFldScenario));

            var test = new LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7();
            fixed (Byte* pFld3 = &test._fld3)
            {
                var result = AdvSimd.LoadAndInsertScalar(test._fld1, 7, pFld3);

                Unsafe.Write(_dataTable.outArrayPtr, result);
                ValidateResult(test._fld1, test._fld3, _dataTable.outArrayPtr);
            }
        }

        public void RunClassLclFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassLclFldScenario_Load));

            var test = new LoadAndInsertTest__LoadAndInsertScalar_Vector64_Byte_7();

            fixed (Vector64<Byte>* pFld1 = &test._fld1)
            fixed (Byte* pFld3 = &test._fld3)
            {
                var result = AdvSimd.LoadAndInsertScalar(
                    AdvSimd.LoadVector64((Byte*)pFld1),
                    7,
                    pFld3
                );

                Unsafe.Write(_dataTable.outArrayPtr, result);
                ValidateResult(test._fld1, test._fld3, _dataTable.outArrayPtr);
            }
        }

        public void RunClassFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassFldScenario));

            fixed (Byte* pFld3 = &_fld3)
            {
                var result = AdvSimd.LoadAndInsertScalar(_fld1, 7, pFld3);

                Unsafe.Write(_dataTable.outArrayPtr, result);
            }

            ValidateResult(_fld1, _fld3, _dataTable.outArrayPtr);
        }

        public void RunClassFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassFldScenario_Load));

            fixed (Vector64<Byte>* pFld1 = &_fld1)
            fixed (Byte* pFld3 = &_fld3)
            {
                var result = AdvSimd.LoadAndInsertScalar(
                    AdvSimd.LoadVector64((Byte*)pFld1),
                    7,
                    pFld3
                );

                Unsafe.Write(_dataTable.outArrayPtr, result);
                ValidateResult(_fld1, _fld3, _dataTable.outArrayPtr);
            }
        }

        public void RunStructLclFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunStructLclFldScenario));

            var test = TestStruct.Create();
            var result = AdvSimd.LoadAndInsertScalar(test._fld1, 7, &test._fld3);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(test._fld1, test._fld3, _dataTable.outArrayPtr);
        }

        public void RunStructLclFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunStructLclFldScenario_Load));

            var test = TestStruct.Create();
            var result = AdvSimd.LoadAndInsertScalar(
                AdvSimd.LoadVector64((Byte*)(&test._fld1)),
                7,
                &test._fld3
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(test._fld1, test._fld3, _dataTable.outArrayPtr);
        }

        public void RunStructFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunStructFldScenario));

            var test = TestStruct.Create();
            test.RunStructFldScenario(this);
        }

        public void RunStructFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunStructFldScenario_Load));

            var test = TestStruct.Create();
            test.RunStructFldScenario_Load(this);
        }

        public void RunUnsupportedScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunUnsupportedScenario));

            bool succeeded = false;

            try
            {
                RunBasicScenario_UnsafeRead();
            }
            catch (PlatformNotSupportedException)
            {
                succeeded = true;
            }

            if (!succeeded)
            {
                Succeeded = false;
            }
        }

        private void ValidateResult(Vector64<Byte> op1, Byte op3, void* result, [CallerMemberName] string method = "")
        {
            Byte[] inArray1 = new Byte[Op1ElementCount];
            Byte[] outArray = new Byte[RetElementCount];

            Unsafe.WriteUnaligned(ref Unsafe.As<Byte, byte>(ref inArray1[0]), op1);
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Byte, byte>(ref outArray[0]), ref Unsafe.AsRef<byte>(result), (uint)Unsafe.SizeOf<Vector64<Byte>>());

            ValidateResult(inArray1, op3, outArray, method);
        }

        private void ValidateResult(void* op1, Byte op3, void* result, [CallerMemberName] string method = "")
        {
            Byte[] inArray1 = new Byte[Op1ElementCount];
            Byte[] outArray = new Byte[RetElementCount];

            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Byte, byte>(ref inArray1[0]), ref Unsafe.AsRef<byte>(op1), (uint)Unsafe.SizeOf<Vector64<Byte>>());
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Byte, byte>(ref outArray[0]), ref Unsafe.AsRef<byte>(result), (uint)Unsafe.SizeOf<Vector64<Byte>>());

            ValidateResult(inArray1, op3, outArray, method);
        }

        private void ValidateResult(Byte[] firstOp, Byte thirdOp, Byte[] result, [CallerMemberName] string method = "")
        {
            bool succeeded = true;

            for (var i = 0; i < RetElementCount; i++)
            {
                if (Helpers.Insert(firstOp, ElementIndex, thirdOp, i) != result[i])
                {
                    succeeded = false;
                    break;
                }
            }

            if (!succeeded)
            {
                TestLibrary.TestFramework.LogInformation($"{nameof(AdvSimd)}.{nameof(AdvSimd.LoadAndInsertScalar)}<Byte>(Vector64<Byte>, 7, Byte*): {method} failed:");
                TestLibrary.TestFramework.LogInformation($" firstOp: ({string.Join(", ", firstOp)})");
                TestLibrary.TestFramework.LogInformation($" thirdOp: {thirdOp}");
                TestLibrary.TestFramework.LogInformation($"  result: ({string.Join(", ", result)})");
                TestLibrary.TestFramework.LogInformation(string.Empty);

                Succeeded = false;
            }
        }
    }
}
