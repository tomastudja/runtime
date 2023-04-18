// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.IO;
using System.Text;
using Xunit;

namespace Common.Tests
{
    public class procfsTests : FileCleanupTestBase
    {
        [Theory]
        [InlineData("1 (systemd) S 0 1 1 0 -1 4194560 11536 2160404 55 593 70 169 4213 1622 20 0 1 0 4 189767680 1491 18446744073709551615 1 1 0 0 0 0 671173123 4096 1260 0 0 0 17 4 0 0 25 0 0 0 0 0 0 0 0 0 0", 1, "systemd", 'S', 1, 70, 169, 0, 4, 189767680, 1491, 18446744073709551615)]
        [InlineData("5955 (dotnet with space) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "dotnet with space", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (bash) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "bash", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("22831 (dotnet) S 22708 22613 1811 34822 22613 4194304 215237 0 0 0 6491 4634 0 0 20 0 18 0 6054850 3672928256 23593 18446744073709551615 4194304 4294944 140729667338512 140729667328848 139709719439277 0 4 4096 17630 0 0 0 17 4 0 0 0 0 0 6392096 6393592 15552512 140729667342465 140729667342553 140729667342553 140729667346408 0", 22831, "dotnet", 'S', 1811, 6491, 4634, 0, 6054850, 3672928256, 23593, 18446744073709551615)]
        [InlineData("22719 (fsnotifier64) S 22671 22613 1811 34822 22613 4194304 135 0 0 0 28 541 0 0 20 0 1 0 6052788 4882432 498 18446744073709551615 4194304 4212092 140731745592928 140731745592312 139627376925939 0 4 4096 0 1 0 0 17 3 0 0 0 0 0 6311936 6312884 25800704 140731745595967 140731745596016 140731745596016 140731745599431 0", 22719, "fsnotifier64", 'S', 1811, 28, 541, 0, 6052788, 4882432, 498, 18446744073709551615)]
        [InlineData("1512 (at-spi-bus-laun) S 1163 1295 1295 0 -1 4194304 377 0 1 0 0 0 0 0 20 0 4 0 967 345894912 1453 18446744073709551615 4194304 4210132 140732353904128 140732353903600 139935649062541 0 0 4096 81920 0 0 0 17 5 0 0 0 0 0 6310768 6311952 8675328 140732353906996 140732353907038 140732353907038 140732353908686 0", 1512, "at-spi-bus-laun", 'S', 1295, 0, 0, 0, 967, 345894912, 1453, 18446744073709551615)]
        [InlineData("15 (kworker/1:0H) S 2 0 0 0 -1 69238880 0 0 0 0 0 0 0 0 0 -20 1 0 4 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 1 0 0 0 0 0 0 0 0 0 0 0 0 0", 15, "kworker/1:0H", 'S', 0, 0, 0, -20, 4, 0, 0, 18446744073709551615)]
        [InlineData("1369 (sh) S 1163 1369 1369 0 -1 4194304 240 1291 0 0 0 0 0 0 20 0 1 0 902 4616192 419 18446744073709551615 93922724966400 93922725110300 140735239002352 140735239000952 140586773169914 0 0 0 65538 1 0 0 17 4 0 0 0 0 0 93922727210856 93922727215648 93922759745536 140735239006661 140735239006720 140735239006720 140735239008240 0", 1369, "sh", 'S', 1369, 0, 0, 0, 902, 4616192, 419, 18446744073709551615)]
        [InlineData("1295 (dbus-daemon) S 1163 1295 1295 0 -1 4194368 1909 460 0 0 21 35 0 0 20 0 1 0 794 44421120 987 18446744073709551615 94692716101632 94692716315676 140720853604656 140720853602536 140124915441971 0 0 0 16385 1 0 0 17 5 0 0 0 0 0 94692718415176 94692718420816 94692735217664 140720853613214 140720853613288 140720853613288 140720853614563 0", 1295, "dbus-daemon", 'S', 1295, 21, 35, 0, 794, 44421120, 987, 18446744073709551615)]
        [InlineData("1291 (upstart-udev-br) S 1163 1290 1290 0 -1 4194368 59 0 0 0 0 0 0 0 20 0 1 0 792 41316352 71 18446744073709551615 94876928040960 94876928122852 140734109368976 140734109368152 140299085954291 0 0 1 81920 1 0 0 17 4 0 0 0 0 0 94876930220584 94876930224400 94876938022912 140734109371165 140734109371201 140734109371201 140734109372382 0", 1291, "upstart-udev-br", 'S', 1290, 0, 0, 0, 792, 41316352, 71, 18446744073709551615)]
        [InlineData("123 (deferwq) S 2 0 0 0 -1 69238880 0 0 0 0 0 0 0 0 0 -20 1 0 90 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 5 0 0 0 0 0 0 0 0 0 0 0 0 0", 123, "deferwq", 'S', 0, 0, 0, -20, 90, 0, 0, 18446744073709551615)]
        [InlineData("1160 ((sd-pam)) S 1158 1158 1158 0 -1 1077936448 28 0 0 0 0 0 0 0 20 0 1 0 758 64921600 510 18446744073709551615 1 1 0 0 0 0 0 4096 0 0 0 0 17 1 0 0 0 0 0 0 0 0 0 0 0 0 0", 1160, "(sd-pam)", 'S', 1158, 0, 0, 0, 758, 64921600, 510, 18446744073709551615)]
        [InlineData("11 (watchdog/1) S 2 0 0 0 -1 69239104 0 0 0 0 0 324 0 0 -100 0 1 0 4 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 1 99 1 0 0 0 0 0 0 0 0 0 0 0", 11, "watchdog/1", 'S', 0, 0, 324, 0, 4, 0, 0, 18446744073709551615)]
        [InlineData("109 (ipv6_addrconf) S 2 0 0 0 -1 69238880 0 0 0 0 0 0 0 0 0 -20 1 0 89 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 1 0 0 0 0 0 0 0 0 0 0 0 0 0", 109, "ipv6_addrconf", 'S', 0, 0, 0, -20, 89, 0, 0, 18446744073709551615)]
        [InlineData("102 (scsi_tmf_0) S 2 0 0 0 -1 69238880 0 0 0 0 0 0 0 0 0 -20 1 0 83 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 1 0 0 0 0 0 0 0 0 0 0 0 0 0", 102, "scsi_tmf_0", 'S', 0, 0, 0, -20, 83, 0, 0, 18446744073709551615)]
        [InlineData("10 (watchdog/0) S 2 0 0 0 -1 69239104 0 0 0 0 0 243 0 0 -100 0 1 0 4 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 0 99 1 0 0 0 0 0 0 0 0 0 0 0", 10, "watchdog/0", 'S', 0, 0, 243, 0, 4, 0, 0, 18446744073709551615)]
        [InlineData("5955 (dotnet (nested) space) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "dotnet (nested) space", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (dotnet (with (parens)) sp(a)ce) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "dotnet (with (parens)) sp(a)ce", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (executable-(name)) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "executable-(name)", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (a)))b) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "a)))b", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (a(((b) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "a(((b", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (a)( ) b (() () S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "a)( ) b (() (", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        [InlineData("5955 (has\\backslash) S 1806 5955 5955 34823 5955 4194304 1426 5872 0 3 16 3 16 4 20 0 1 0 674762 32677888 1447 18446744073709551615 4194304 5192652 140725672538992 140725672534152 140236068968880 0 0 3670020 1266777851 1 0 0 17 4 0 0 0 0 0 7290352 7326856 21204992 140725672540419 140725672540424 140725672540424 140725672542190 0", 5955, "has\\backslash", 'S', 5955, 16, 3, 0, 674762, 32677888, 1447, 18446744073709551615)]
        public void ParseValidStatFiles_Success(
            string statFileText,
            int expectedPid, string expectedComm, char expectedState, int expectedSession,
            ulong expectedUtime, ulong expectedStime, long expectedNice, ulong expectedStarttime,
            ulong expectedVsize, long expectedRss, ulong expectedRsslim)
        {
            string path = GetTestFilePath();
            File.WriteAllText(path, statFileText);

            Interop.procfs.ParsedStat result;
            Assert.True(Interop.procfs.TryParseStatFile(path, out result));

            Assert.Equal(expectedPid, result.pid);
            Assert.Equal(expectedComm, result.comm);
            Assert.Equal(expectedState, result.state);
            Assert.Equal(expectedSession, result.session);
            Assert.Equal(expectedUtime, result.utime);
            Assert.Equal(expectedStime, result.stime);
            Assert.Equal(expectedNice, result.nice);
            Assert.Equal(expectedStarttime, result.starttime);
            Assert.Equal(expectedVsize, result.vsize);
            Assert.Equal(expectedRss, result.rss);
            Assert.Equal(expectedRsslim, result.rsslim);
        }
    }
}
