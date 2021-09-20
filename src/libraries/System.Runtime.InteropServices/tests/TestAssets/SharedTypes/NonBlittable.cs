using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace SharedTypes
{
    [NativeMarshalling(typeof(StringContainerNative))]
    public struct StringContainer
    {
        public string str1;
        public string str2;
    }

    [BlittableType]
    public struct StringContainerNative
    {
        public IntPtr str1;
        public IntPtr str2;

        public StringContainerNative(StringContainer managed)
        {
            str1 = Marshal.StringToCoTaskMemUTF8(managed.str1);
            str2 = Marshal.StringToCoTaskMemUTF8(managed.str2);
        }

        public StringContainer ToManaged()
        {
            return new StringContainer
            {
                str1 = Marshal.PtrToStringUTF8(str1),
                str2 = Marshal.PtrToStringUTF8(str2)
            };
        }

        public void FreeNative()
        {
            Marshal.FreeCoTaskMem(str1);
            Marshal.FreeCoTaskMem(str2);
        }
    }

    public struct DoubleToLongMarshaler
    {
        public long l;

        public DoubleToLongMarshaler(double d)
        {
            l = MemoryMarshal.Cast<double, long>(MemoryMarshal.CreateSpan(ref d, 1))[0];
        }

        public double ToManaged() => MemoryMarshal.Cast<long, double>(MemoryMarshal.CreateSpan(ref l, 1))[0];

        public long Value
        {
            get => l;
            set => l = value;
        }
    }

    [NativeMarshalling(typeof(BoolStructNative))]
    public struct BoolStruct
    {
        public bool b1;
        public bool b2;
        public bool b3;
    }

    [BlittableType]
    public struct BoolStructNative
    {
        public byte b1;
        public byte b2;
        public byte b3;
        public BoolStructNative(BoolStruct bs)
        {
            b1 = (byte)(bs.b1 ? 1 : 0);
            b2 = (byte)(bs.b2 ? 1 : 0);
            b3 = (byte)(bs.b3 ? 1 : 0);
        }

        public BoolStruct ToManaged()
        {
            return new BoolStruct
            {
                b1 = b1 != 0,
                b2 = b2 != 0,
                b3 = b3 != 0
            };
        }
    }

    [NativeMarshalling(typeof(IntWrapperMarshaler))]
    public class IntWrapper
    {
        public int i;

        public ref int GetPinnableReference() => ref i;
    }

    public unsafe struct IntWrapperMarshaler
    {
        public IntWrapperMarshaler(IntWrapper managed)
        {
            Value = (int*)Marshal.AllocCoTaskMem(sizeof(int));
            *Value = managed.i;
        }

        public int* Value { get; set; }

        public IntWrapper ToManaged() => new IntWrapper { i = *Value };

        public void FreeNative()
        {
            Marshal.FreeCoTaskMem((IntPtr)Value);
        }
    }

    public unsafe ref struct Utf16StringMarshaler
    {
        private ushort* ptr;
        private Span<ushort> span;

        public Utf16StringMarshaler(string str)
        {
            ptr = str is null ? null : (ushort*)Marshal.StringToCoTaskMemUni(str);
            span = default;
        }

        public Utf16StringMarshaler(string str, Span<byte> buffer)
        {
            if (str is null)
            {
                ptr = null;
                span = default;
            }
            else if ((str.Length + 1) < StackBufferSize)
            {
                span = MemoryMarshal.Cast<byte, ushort>(buffer);
                str.AsSpan().CopyTo(MemoryMarshal.Cast<byte, char>(buffer));
                // Supplied memory is in an undefined state so ensure
                // there is a trailing null in the buffer.
                span[str.Length] = '\0';
                ptr = null;
            }
            else
            {
                span = default;
                ptr = (ushort*)Marshal.StringToCoTaskMemUni(str);
            }
        }

        public ref ushort GetPinnableReference()
        {
            if (ptr != null)
            {
                return ref *ptr;
            }
            return ref span.GetPinnableReference();
        }

        public ushort* Value
        {
            get
            {
                if (ptr == null && span != default)
                {
                    throw new InvalidOperationException();
                }
                return ptr;
            }
            set
            {
                ptr = value;
                span = default;
            }
        }

        public string ToManaged()
        {
            if (ptr == null && span == default)
            {
                return null;
            }
            else if (ptr != null)
            {
                return Marshal.PtrToStringUni((IntPtr)ptr);
            }
            else
            {
                return MemoryMarshal.Cast<ushort, char>(span).ToString();
            }
        }

        public void FreeNative()
        {
            if (ptr != null)
            {
                Marshal.FreeCoTaskMem((IntPtr)ptr);
            }
        }

        public const int StackBufferSize = 0x100;
    }

    [NativeMarshalling(typeof(IntStructWrapperNative))]
    public struct IntStructWrapper
    {
        public int Value;
    }

    public struct IntStructWrapperNative
    {
        public int value;
        public IntStructWrapperNative(IntStructWrapper managed)
        {
            value = managed.Value;
        }

        public IntStructWrapper ToManaged() => new IntStructWrapper { Value = value };
    }

    [GenericContiguousCollectionMarshaller]
    public unsafe ref struct ListMarshaller<T>
    {
        private List<T> managedList;
        private readonly int sizeOfNativeElement;
        private IntPtr allocatedMemory;

        public ListMarshaller(int sizeOfNativeElement)
            : this()
        {
            this.sizeOfNativeElement = sizeOfNativeElement;
        }

        public ListMarshaller(List<T> managed, int sizeOfNativeElement)
        {
            allocatedMemory = default;
            this.sizeOfNativeElement = sizeOfNativeElement;
            if (managed is null)
            {
                managedList = null;
                NativeValueStorage = default;
                return;
            }
            managedList = managed;
            this.sizeOfNativeElement = sizeOfNativeElement;
            // Always allocate at least one byte when the array is zero-length.
            int spaceToAllocate = Math.Max(managed.Count * sizeOfNativeElement, 1);
            allocatedMemory = Marshal.AllocCoTaskMem(spaceToAllocate);
            NativeValueStorage = new Span<byte>((void*)allocatedMemory, spaceToAllocate);
        }

        public ListMarshaller(List<T> managed, Span<byte> stackSpace, int sizeOfNativeElement)
        {
            allocatedMemory = default;
            this.sizeOfNativeElement = sizeOfNativeElement;
            if (managed is null)
            {
                managedList = null;
                NativeValueStorage = default;
                return;
            }
            managedList = managed;
            // Always allocate at least one byte when the array is zero-length.
            int spaceToAllocate = Math.Max(managed.Count * sizeOfNativeElement, 1);
            if (spaceToAllocate <= stackSpace.Length)
            {
                NativeValueStorage = stackSpace[0..spaceToAllocate];
            }
            else
            {
                allocatedMemory = Marshal.AllocCoTaskMem(spaceToAllocate);
                NativeValueStorage = new Span<byte>((void*)allocatedMemory, spaceToAllocate);
            }
        }

        /// <summary>
        /// Stack-alloc threshold set to 256 bytes to enable small arrays to be passed on the stack.
        /// Number kept small to ensure that P/Invokes with a lot of array parameters doesn't
        /// blow the stack since this is a new optimization in the code-generated interop.
        /// </summary>
        public const int StackBufferSize = 0x200;

        public Span<T> ManagedValues => CollectionsMarshal.AsSpan(managedList);

        public Span<byte> NativeValueStorage { get; private set; }

        public ref byte GetPinnableReference() => ref NativeValueStorage.GetPinnableReference();

        public void SetUnmarshalledCollectionLength(int length)
        {
            managedList = new List<T>(length);
            for (int i = 0; i < length; i++)
            {
                managedList.Add(default);
            }
        }

        public byte* Value
        {
            get
            {
                Debug.Assert(managedList is null || allocatedMemory != IntPtr.Zero);
                return (byte*)allocatedMemory;
            }
            set
            {
                if (value == null)
                {
                    managedList = null;
                    NativeValueStorage = default;
                }
                else
                {
                    allocatedMemory = (IntPtr)value;
                    NativeValueStorage = new Span<byte>(value, (managedList?.Count ?? 0) * sizeOfNativeElement);
                }
            }
        }

        public List<T> ToManaged() => managedList;

        public void FreeNative()
        {
            if (allocatedMemory != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(allocatedMemory);
            }
        }
    }
}
