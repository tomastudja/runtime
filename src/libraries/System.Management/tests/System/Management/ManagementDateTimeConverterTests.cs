// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

namespace System.Management.Tests
{
    public class ManagementDateTimeConverterTests
    {
        [ConditionalFact(typeof(WmiTestHelper), nameof(WmiTestHelper.IsWmiSupported))]
        public void DateTime_RoundTrip()
        {
            var date = new DateTime(2002, 4, 8, 14, 18, 35, 978, DateTimeKind.Utc).AddMinutes(150);
            var dmtfDate = "20020408141835.978000-150";
            var dmtfDateExpected = "20020408164835.978000+000";

            DateTime convertedDate = ManagementDateTimeConverter.ToDateTime(dmtfDate).ToUniversalTime();
            Assert.Equal(date, convertedDate);

            // Converting System.DateTime to DMTF datetime
            string convertedDmtfDate = ManagementDateTimeConverter.ToDmtfDateTime(date);
            Assert.Equal(dmtfDateExpected, convertedDmtfDate);
        }

        [ConditionalFact(typeof(WmiTestHelper), nameof(WmiTestHelper.IsWmiSupported))]
        public void DateTime_MinValue_RoundTrip()
        {
            string dmtfFromDateTimeMinValue = ManagementDateTimeConverter.ToDmtfDateTime(DateTime.MinValue);
            DateTime convertedDate = ManagementDateTimeConverter.ToDateTime(dmtfFromDateTimeMinValue);
            if (PlatformDetection.IsNetFramework)
            {
                Assert.Equal(DateTimeKind.Unspecified, convertedDate.Kind);
            }
            else
            {
                Assert.Equal(DateTimeKind.Local, convertedDate.Kind);
            }
            Assert.Equal(DateTime.MinValue, convertedDate);
        }

        [ConditionalFact(typeof(WmiTestHelper), nameof(WmiTestHelper.IsWmiSupported))]
        public void TimeSpan_RoundTrip()
        {
            var timeSpan = new TimeSpan(10, 12, 25, 32, 123);
            var dmtfTimeInterval = "00000010122532.123000:000";

            // Converting DMTF timeinterval to System.TimeSpan
            TimeSpan convertedTimeSpan = ManagementDateTimeConverter.ToTimeSpan(dmtfTimeInterval);
            Assert.Equal(timeSpan, convertedTimeSpan);

            // Converting System.TimeSpan to DMTF time interval format
            string convertedDmtfTimeInterval = ManagementDateTimeConverter.ToDmtfTimeInterval(timeSpan);
            Assert.Equal(dmtfTimeInterval, convertedDmtfTimeInterval);
        }
    }
}
