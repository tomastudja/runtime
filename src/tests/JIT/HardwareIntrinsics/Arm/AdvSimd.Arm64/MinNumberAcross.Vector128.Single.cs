// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/******************************************************************************
 * This file is auto-generated from a template file by the GenerateTests.csx  *
 * script in tests\src\JIT\HardwareIntrinsics.Arm\Shared. In order to make    *
 * changes, please update the corresponding template and run according to the *
 * directions listed in the file.                                             *
 ******************************************************************************/

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Intrinsics.Arm;
using Xunit;

namespace JIT.HardwareIntrinsics.Arm._AdvSimd.Arm64
{
    public static partial class Program
    {
        [Fact]
        public static void MinNumberAcross_Vector128_Single()
        {
            var test = new SimpleUnaryOpTest__MinNumberAcross_Vector128_Single();

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

    public sealed unsafe class SimpleUnaryOpTest__MinNumberAcross_Vector128_Single
    {
        private struct DataTable
        {
            private byte[] inArray1;
            private byte[] outArray;

            private GCHandle inHandle1;
            private GCHandle outHandle;

            private ulong alignment;

            public DataTable(Single[] inArray1, Single[] outArray, int alignment)
            {
                int sizeOfinArray1 = inArray1.Length * Unsafe.SizeOf<Single>();
                int sizeOfoutArray = outArray.Length * Unsafe.SizeOf<Single>();
                if ((alignment != 16 && alignment != 8) || (alignment * 2) < sizeOfinArray1 || (alignment * 2) < sizeOfoutArray)
                {
                    throw new ArgumentException("Invalid value of alignment");
                }

                this.inArray1 = new byte[alignment * 2];
                this.outArray = new byte[alignment * 2];

                this.inHandle1 = GCHandle.Alloc(this.inArray1, GCHandleType.Pinned);
                this.outHandle = GCHandle.Alloc(this.outArray, GCHandleType.Pinned);

                this.alignment = (ulong)alignment;

                Unsafe.CopyBlockUnaligned(ref Unsafe.AsRef<byte>(inArray1Ptr), ref Unsafe.As<Single, byte>(ref inArray1[0]), (uint)sizeOfinArray1);
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
            public Vector128<Single> _fld1;

            public static TestStruct Create()
            {
                var testStruct = new TestStruct();

                for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetSingle(); }
                Unsafe.CopyBlockUnaligned(ref Unsafe.As<Vector128<Single>, byte>(ref testStruct._fld1), ref Unsafe.As<Single, byte>(ref _data1[0]), (uint)Unsafe.SizeOf<Vector128<Single>>());

                return testStruct;
            }

            public void RunStructFldScenario(SimpleUnaryOpTest__MinNumberAcross_Vector128_Single testClass)
            {
                var result = AdvSimd.Arm64.MinNumberAcross(_fld1);

                Unsafe.Write(testClass._dataTable.outArrayPtr, result);
                testClass.ValidateResult(_fld1, testClass._dataTable.outArrayPtr);
            }

            public void RunStructFldScenario_Load(SimpleUnaryOpTest__MinNumberAcross_Vector128_Single testClass)
            {
                fixed (Vector128<Single>* pFld1 = &_fld1)
                {
                    var result = AdvSimd.Arm64.MinNumberAcross(
                        AdvSimd.LoadVector128((Single*)(pFld1))
                    );

                    Unsafe.Write(testClass._dataTable.outArrayPtr, result);
                    testClass.ValidateResult(_fld1, testClass._dataTable.outArrayPtr);
                }
            }
        }

        private static readonly int LargestVectorSize = 16;

        private static readonly int Op1ElementCount = Unsafe.SizeOf<Vector128<Single>>() / sizeof(Single);
        private static readonly int RetElementCount = Unsafe.SizeOf<Vector64<Single>>() / sizeof(Single);

        private static Single[] _data1 = new Single[Op1ElementCount];

        private static Vector128<Single> _clsVar1;

        private Vector128<Single> _fld1;

        private DataTable _dataTable;

        static SimpleUnaryOpTest__MinNumberAcross_Vector128_Single()
        {
            for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetSingle(); }
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Vector128<Single>, byte>(ref _clsVar1), ref Unsafe.As<Single, byte>(ref _data1[0]), (uint)Unsafe.SizeOf<Vector128<Single>>());
        }

        public SimpleUnaryOpTest__MinNumberAcross_Vector128_Single()
        {
            Succeeded = true;

            for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetSingle(); }
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Vector128<Single>, byte>(ref _fld1), ref Unsafe.As<Single, byte>(ref _data1[0]), (uint)Unsafe.SizeOf<Vector128<Single>>());

            for (var i = 0; i < Op1ElementCount; i++) { _data1[i] = TestLibrary.Generator.GetSingle(); }
            _dataTable = new DataTable(_data1, new Single[RetElementCount], LargestVectorSize);
        }

        public bool IsSupported => AdvSimd.Arm64.IsSupported;

        public bool Succeeded { get; set; }

        public void RunBasicScenario_UnsafeRead()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunBasicScenario_UnsafeRead));

            var result = AdvSimd.Arm64.MinNumberAcross(
                Unsafe.Read<Vector128<Single>>(_dataTable.inArray1Ptr)
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(_dataTable.inArray1Ptr, _dataTable.outArrayPtr);
        }

        public void RunBasicScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunBasicScenario_Load));

            var result = AdvSimd.Arm64.MinNumberAcross(
                AdvSimd.LoadVector128((Single*)(_dataTable.inArray1Ptr))
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(_dataTable.inArray1Ptr, _dataTable.outArrayPtr);
        }

        public void RunReflectionScenario_UnsafeRead()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunReflectionScenario_UnsafeRead));

            var result = typeof(AdvSimd.Arm64).GetMethod(nameof(AdvSimd.Arm64.MinNumberAcross), new Type[] { typeof(Vector128<Single>) })
                                     .Invoke(null, new object[] {
                                        Unsafe.Read<Vector128<Single>>(_dataTable.inArray1Ptr)
                                     });

            Unsafe.Write(_dataTable.outArrayPtr, (Vector64<Single>)(result));
            ValidateResult(_dataTable.inArray1Ptr, _dataTable.outArrayPtr);
        }

        public void RunReflectionScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunReflectionScenario_Load));

            var result = typeof(AdvSimd.Arm64).GetMethod(nameof(AdvSimd.Arm64.MinNumberAcross), new Type[] { typeof(Vector128<Single>) })
                                     .Invoke(null, new object[] {
                                        AdvSimd.LoadVector128((Single*)(_dataTable.inArray1Ptr))
                                     });

            Unsafe.Write(_dataTable.outArrayPtr, (Vector64<Single>)(result));
            ValidateResult(_dataTable.inArray1Ptr, _dataTable.outArrayPtr);
        }

        public void RunClsVarScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClsVarScenario));

            var result = AdvSimd.Arm64.MinNumberAcross(
                _clsVar1
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(_clsVar1, _dataTable.outArrayPtr);
        }

        public void RunClsVarScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClsVarScenario_Load));

            fixed (Vector128<Single>* pClsVar1 = &_clsVar1)
            {
                var result = AdvSimd.Arm64.MinNumberAcross(
                    AdvSimd.LoadVector128((Single*)(pClsVar1))
                );

                Unsafe.Write(_dataTable.outArrayPtr, result);
                ValidateResult(_clsVar1, _dataTable.outArrayPtr);
            }
        }

        public void RunLclVarScenario_UnsafeRead()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunLclVarScenario_UnsafeRead));

            var op1 = Unsafe.Read<Vector128<Single>>(_dataTable.inArray1Ptr);
            var result = AdvSimd.Arm64.MinNumberAcross(op1);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(op1, _dataTable.outArrayPtr);
        }

        public void RunLclVarScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunLclVarScenario_Load));

            var op1 = AdvSimd.LoadVector128((Single*)(_dataTable.inArray1Ptr));
            var result = AdvSimd.Arm64.MinNumberAcross(op1);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(op1, _dataTable.outArrayPtr);
        }

        public void RunClassLclFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassLclFldScenario));

            var test = new SimpleUnaryOpTest__MinNumberAcross_Vector128_Single();
            var result = AdvSimd.Arm64.MinNumberAcross(test._fld1);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(test._fld1, _dataTable.outArrayPtr);
        }

        public void RunClassLclFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassLclFldScenario_Load));

            var test = new SimpleUnaryOpTest__MinNumberAcross_Vector128_Single();

            fixed (Vector128<Single>* pFld1 = &test._fld1)
            {
                var result = AdvSimd.Arm64.MinNumberAcross(
                    AdvSimd.LoadVector128((Single*)(pFld1))
                );

                Unsafe.Write(_dataTable.outArrayPtr, result);
                ValidateResult(test._fld1, _dataTable.outArrayPtr);
            }
        }

        public void RunClassFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassFldScenario));

            var result = AdvSimd.Arm64.MinNumberAcross(_fld1);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(_fld1, _dataTable.outArrayPtr);
        }

        public void RunClassFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunClassFldScenario_Load));

            fixed (Vector128<Single>* pFld1 = &_fld1)
            {
                var result = AdvSimd.Arm64.MinNumberAcross(
                    AdvSimd.LoadVector128((Single*)(pFld1))
                );

                Unsafe.Write(_dataTable.outArrayPtr, result);
                ValidateResult(_fld1, _dataTable.outArrayPtr);
            }
        }

        public void RunStructLclFldScenario()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunStructLclFldScenario));

            var test = TestStruct.Create();
            var result = AdvSimd.Arm64.MinNumberAcross(test._fld1);

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(test._fld1, _dataTable.outArrayPtr);
        }

        public void RunStructLclFldScenario_Load()
        {
            TestLibrary.TestFramework.BeginScenario(nameof(RunStructLclFldScenario_Load));

            var test = TestStruct.Create();
            var result = AdvSimd.Arm64.MinNumberAcross(
                AdvSimd.LoadVector128((Single*)(&test._fld1))
            );

            Unsafe.Write(_dataTable.outArrayPtr, result);
            ValidateResult(test._fld1, _dataTable.outArrayPtr);
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

        private void ValidateResult(Vector128<Single> op1, void* result, [CallerMemberName] string method = "")
        {
            Single[] inArray1 = new Single[Op1ElementCount];
            Single[] outArray = new Single[RetElementCount];

            Unsafe.WriteUnaligned(ref Unsafe.As<Single, byte>(ref inArray1[0]), op1);
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Single, byte>(ref outArray[0]), ref Unsafe.AsRef<byte>(result), (uint)Unsafe.SizeOf<Vector64<Single>>());

            ValidateResult(inArray1, outArray, method);
        }

        private void ValidateResult(void* op1, void* result, [CallerMemberName] string method = "")
        {
            Single[] inArray1 = new Single[Op1ElementCount];
            Single[] outArray = new Single[RetElementCount];

            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Single, byte>(ref inArray1[0]), ref Unsafe.AsRef<byte>(op1), (uint)Unsafe.SizeOf<Vector128<Single>>());
            Unsafe.CopyBlockUnaligned(ref Unsafe.As<Single, byte>(ref outArray[0]), ref Unsafe.AsRef<byte>(result), (uint)Unsafe.SizeOf<Vector64<Single>>());

            ValidateResult(inArray1, outArray, method);
        }

        private void ValidateResult(Single[] firstOp, Single[] result, [CallerMemberName] string method = "")
        {
            bool succeeded = true;

            if (BitConverter.SingleToInt32Bits(Helpers.MinNumberAcross(firstOp)) != BitConverter.SingleToInt32Bits(result[0]))
            {
                succeeded = false;
            }
            else
            {
                for (int i = 1; i < RetElementCount; i++)
                {
                    if (BitConverter.SingleToInt32Bits(result[i]) != 0)
                    {
                        succeeded = false;
                        break;
                    }
                }
            }

            if (!succeeded)
            {
                TestLibrary.TestFramework.LogInformation($"{nameof(AdvSimd.Arm64)}.{nameof(AdvSimd.Arm64.MinNumberAcross)}<Single>(Vector128<Single>): {method} failed:");
                TestLibrary.TestFramework.LogInformation($" firstOp: ({string.Join(", ", firstOp)})");
                TestLibrary.TestFramework.LogInformation($"  result: ({string.Join(", ", result)})");
                TestLibrary.TestFramework.LogInformation(string.Empty);

                Succeeded = false;
            }
        }
    }
}
