// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

//
// Authors:
//    Alexandre Pigolkine (pigolkine@gmx.de)
//    Jordi Mas i Hernandez (jordi@ximian.com)
//    Sanjay Gupta (gsanjay@novell.com)
//    Ravindra (rkumar@novell.com)
//    Peter Dennis Bartok (pbartok@novell.com)
//    Sebastien Pouliot <sebastien@ximian.com>
//
// Copyright (C) 2004 - 2007 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System.Runtime.InteropServices;

namespace System.Drawing
{
    internal static unsafe class MarshallingHelpers
    {
        // Copies a Ptr to an array of Points and releases the memory
        public static void FromUnManagedMemoryToPointI(IntPtr ptr, Point[] pts)
        {
            var sourceSpan = new Span<Point>((void*)ptr, pts.Length);
            sourceSpan.CopyTo(new Span<Point>(pts));
            Marshal.FreeHGlobal(ptr);
        }

        // Copies a Ptr to an array of Points and releases the memory
        public static void FromUnManagedMemoryToPoint(IntPtr ptr, PointF[] pts)
        {
            var sourceSpan = new Span<PointF>((void*)ptr, pts.Length);
            sourceSpan.CopyTo(new Span<PointF>(pts));
            Marshal.FreeHGlobal(ptr);
        }

        // Copies an array of Points to unmanaged memory
        public static IntPtr FromPointToUnManagedMemoryI(Point[] pts)
        {
            IntPtr dest = Marshal.AllocHGlobal(sizeof(Point) * pts.Length);
            var destinationSpan = new Span<Point>((void*)dest, pts.Length);
            pts.CopyTo(destinationSpan);
            return dest;
        }

        // Copies an array of Points to unmanaged memory
        public static IntPtr FromPointToUnManagedMemory(PointF[] pts)
        {
            IntPtr dest = Marshal.AllocHGlobal(sizeof(PointF) * pts.Length);
            var destinationSpan = new Span<PointF>((void*)dest, pts.Length);
            pts.CopyTo(destinationSpan);
            return dest;
        }
    }
}
