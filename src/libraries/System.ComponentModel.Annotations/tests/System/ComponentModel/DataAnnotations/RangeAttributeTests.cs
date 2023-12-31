// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Tests;
using Xunit;

namespace System.ComponentModel.DataAnnotations.Tests
{
    public class RangeAttributeTests : ValidationAttributeTestBase
    {
        protected override IEnumerable<TestCase> ValidValues()
        {
            RangeAttribute intRange = new RangeAttribute(1, 3);
            yield return new TestCase(intRange, null);
            yield return new TestCase(intRange, string.Empty);
            yield return new TestCase(intRange, 1);
            yield return new TestCase(intRange, 2);
            yield return new TestCase(intRange, 3);
            yield return new TestCase(new RangeAttribute(1, 1), 1);

            intRange = new RangeAttribute(0, 10) { MinimumIsExclusive = true };
            yield return new TestCase(intRange, 1);
            yield return new TestCase(intRange, 2);
            yield return new TestCase(intRange, 9);
            yield return new TestCase(intRange, 10);

            intRange = new RangeAttribute(0, 10) { MaximumIsExclusive = true };
            yield return new TestCase(intRange, 0);
            yield return new TestCase(intRange, 1);
            yield return new TestCase(intRange, 9);

            intRange = new RangeAttribute(0, 10) { MinimumIsExclusive = true, MaximumIsExclusive = true };
            yield return new TestCase(intRange, 1);
            yield return new TestCase(intRange, 2);
            yield return new TestCase(intRange, 8);
            yield return new TestCase(intRange, 9);

            RangeAttribute doubleRange = new RangeAttribute(1.0, 3.0);
            yield return new TestCase(doubleRange, null);
            yield return new TestCase(doubleRange, string.Empty);
            yield return new TestCase(doubleRange, 1.0);
            yield return new TestCase(doubleRange, 2.0);
            yield return new TestCase(doubleRange, 3.0);
            yield return new TestCase(new RangeAttribute(1.0, 1.0), 1);

            doubleRange = new RangeAttribute(0d, 1d) { MinimumIsExclusive = true };
            yield return new TestCase(doubleRange, double.Epsilon);
            yield return new TestCase(doubleRange, 1e-100);
            yield return new TestCase(doubleRange, 0.00000001);
            yield return new TestCase(doubleRange, 0.99999999);
            yield return new TestCase(doubleRange, 1d);

            doubleRange = new RangeAttribute(0d, 1d) { MaximumIsExclusive = true };
            yield return new TestCase(doubleRange, -0d);
            yield return new TestCase(doubleRange, 0d);
            yield return new TestCase(doubleRange, double.Epsilon);
            yield return new TestCase(doubleRange, 1e-100);
            yield return new TestCase(doubleRange, 0.00000001);
            yield return new TestCase(doubleRange, 0.99999999);

            doubleRange = new RangeAttribute(0d, 1d) { MinimumIsExclusive = true, MaximumIsExclusive = true };
            yield return new TestCase(doubleRange, double.Epsilon);
            yield return new TestCase(doubleRange, 1e-100);
            yield return new TestCase(doubleRange, 0.00000001);
            yield return new TestCase(doubleRange, 0.99999999);

            RangeAttribute stringIntRange = new RangeAttribute(typeof(int), "1", "3");
            yield return new TestCase(stringIntRange, null);
            yield return new TestCase(stringIntRange, string.Empty);
            yield return new TestCase(stringIntRange, 1);
            yield return new TestCase(stringIntRange, "1");
            yield return new TestCase(stringIntRange, 2);
            yield return new TestCase(stringIntRange, "2");
            yield return new TestCase(stringIntRange, 3);
            yield return new TestCase(stringIntRange, "3");

            RangeAttribute stringDoubleRange = new RangeAttribute(typeof(double), (1.0).ToString("F1"), (3.0).ToString("F1"));
            yield return new TestCase(stringDoubleRange, null);
            yield return new TestCase(stringDoubleRange, string.Empty);
            yield return new TestCase(stringDoubleRange, 1.0);
            yield return new TestCase(stringDoubleRange, (1.0).ToString("F1"));
            yield return new TestCase(stringDoubleRange, 2.0);
            yield return new TestCase(stringDoubleRange, (2.0).ToString("F1"));
            yield return new TestCase(stringDoubleRange, 3.0);
            yield return new TestCase(stringDoubleRange, (3.0).ToString("F1"));
        }

        protected override IEnumerable<TestCase> InvalidValues()
        {
            RangeAttribute intRange = new RangeAttribute(1, 3);
            yield return new TestCase(intRange, 0);
            yield return new TestCase(intRange, 4);
            yield return new TestCase(intRange, "abc");
            yield return new TestCase(intRange, new object());
            // Implements IConvertible (throws NotSupportedException - is caught)
            yield return new TestCase(intRange, new IConvertibleImplementor() { IntThrow = new NotSupportedException() });

            intRange = new RangeAttribute(0, 10) { MinimumIsExclusive = true };
            yield return new TestCase(intRange, -1);
            yield return new TestCase(intRange, 0);
            yield return new TestCase(intRange, 11);

            intRange = new RangeAttribute(0, 10) { MaximumIsExclusive = true };
            yield return new TestCase(intRange, -1);
            yield return new TestCase(intRange, 10);
            yield return new TestCase(intRange, 11);

            intRange = new RangeAttribute(0, 10) { MinimumIsExclusive = true, MaximumIsExclusive = true };
            yield return new TestCase(intRange, -1);
            yield return new TestCase(intRange, 0);
            yield return new TestCase(intRange, 10);
            yield return new TestCase(intRange, 11);

            RangeAttribute doubleRange = new RangeAttribute(1.0, 3.0);
            yield return new TestCase(doubleRange, 0.9999999);
            yield return new TestCase(doubleRange, 3.0000001);
            yield return new TestCase(doubleRange, "abc");
            yield return new TestCase(doubleRange, new object());
            // Implements IConvertible (throws NotSupportedException - is caught)
            yield return new TestCase(doubleRange, new IConvertibleImplementor() { DoubleThrow = new NotSupportedException() });

            doubleRange = new RangeAttribute(0d, 1d) { MinimumIsExclusive = true };
            yield return new TestCase(doubleRange, -0.1);
            yield return new TestCase(doubleRange, -0d);
            yield return new TestCase(doubleRange, 0d);
            yield return new TestCase(doubleRange, 1.00000001);

            doubleRange = new RangeAttribute(0d, 1d) { MaximumIsExclusive = true };
            yield return new TestCase(doubleRange, -0.1);
            yield return new TestCase(doubleRange, 1d);
            yield return new TestCase(doubleRange, 1.00000001);

            doubleRange = new RangeAttribute(0d, 1d) { MinimumIsExclusive = true, MaximumIsExclusive = true };
            yield return new TestCase(doubleRange, -0.1);
            yield return new TestCase(doubleRange, -0d);
            yield return new TestCase(doubleRange, 0d);
            yield return new TestCase(doubleRange, 1d);
            yield return new TestCase(doubleRange, 1.00000001);

            RangeAttribute stringIntRange = new RangeAttribute(typeof(int), "1", "3");
            yield return new TestCase(stringIntRange, 0);
            yield return new TestCase(stringIntRange, "0");
            yield return new TestCase(stringIntRange, 4);
            yield return new TestCase(stringIntRange, "4");
            yield return new TestCase(stringIntRange, new object());
            // Implements IConvertible (throws NotSupportedException - is caught)
            yield return new TestCase(stringIntRange, new IConvertibleImplementor() { IntThrow = new NotSupportedException() });

            RangeAttribute stringDoubleRange = new RangeAttribute(typeof(double), (1.0).ToString("F1"), (3.0).ToString("F1"));
            yield return new TestCase(stringDoubleRange, 0.9999999);
            yield return new TestCase(stringDoubleRange, (0.9999999).ToString());
            yield return new TestCase(stringDoubleRange, 3.0000001);
            yield return new TestCase(stringDoubleRange, (3.0000001).ToString());
            yield return new TestCase(stringDoubleRange, new object());
            // Implements IConvertible (throws NotSupportedException - is caught)
            yield return new TestCase(stringDoubleRange, new IConvertibleImplementor() { DoubleThrow = new NotSupportedException() });
        }

        public static IEnumerable<object[]> DotDecimalRanges()
        {
            yield return new object[] {typeof(decimal), "1.0", "3.0"};
            yield return new object[] {typeof(double), "1.0", "3.0"};
        }

        public static IEnumerable<object[]> CommaDecimalRanges()
        {
            yield return new object[] { typeof(decimal), "1,0", "3,0" };
            yield return new object[] { typeof(double), "1,0", "3,0" };
        }

        public static IEnumerable<object[]> DotDecimalValidValues()
        {
            yield return new object[] { typeof(decimal), "1.0", "3.0", "1.0" };
            yield return new object[] { typeof(decimal), "1.0", "3.0", "3.0" };
            yield return new object[] { typeof(decimal), "1.0", "3.0", "2.9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(decimal), "1.0", "3.0", "2.9999999999999999999999999999" };
            yield return new object[] { typeof(double), "1.0", "3.0", "1.0" };
            yield return new object[] { typeof(double), "1.0", "3.0", "3.0" };
            yield return new object[] { typeof(double), "1.0", "3.0", "2.9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(double), "1.0", "3.0", "2.99999999999999" };
        }

        public static IEnumerable<object[]> CommaDecimalValidValues()
        {
            yield return new object[] { typeof(decimal), "1,0", "3,0", "1,0" };
            yield return new object[] { typeof(decimal), "1,0", "3,0", "3,0" };
            yield return new object[] { typeof(decimal), "1,0", "3,0", "2,9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(decimal), "1,0", "3,0", "2,9999999999999999999999999999" };
            yield return new object[] { typeof(double), "1,0", "3,0", "1,0" };
            yield return new object[] { typeof(double), "1,0", "3,0", "3,0" };
            yield return new object[] { typeof(double), "1,0", "3,0", "2,99999999999999" };
        }

        public static IEnumerable<object[]> DotDecimalInvalidValues()
        {
            yield return new object[] { typeof(decimal), "1.0", "3.0", "9.0" };
            yield return new object[] { typeof(decimal), "1.0", "3.0", "0.1" };
            yield return new object[] { typeof(decimal), "1.0", "3.0", "3.9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(decimal), "1.0", "3.0", "3.9999999999999999999999999999" };
            yield return new object[] { typeof(double), "1.0", "3.0", "9.0" };
            yield return new object[] { typeof(double), "1.0", "3.0", "0.1" };
            yield return new object[] { typeof(double), "1.0", "3.0", "3.9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(double), "1.0", "3.0", "3.99999999999999" };
        }

        public static IEnumerable<object[]> CommaDecimalInvalidValues()
        {
            yield return new object[] { typeof(decimal), "1,0", "3,0", "9,0" };
            yield return new object[] { typeof(decimal), "1,0", "3,0", "0,1" };
            yield return new object[] { typeof(decimal), "1,0", "3,0", "3,9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(decimal), "1,0", "3,0", "3,9999999999999999999999999999" };
            yield return new object[] { typeof(double), "1,0", "3,0", "9,0" };
            yield return new object[] { typeof(double), "1,0", "3,0", "0,1" };
            yield return new object[] { typeof(double), "1,0", "3,0", "3,9999999999999999999999999999999999999999999" };
            yield return new object[] { typeof(double), "1,0", "3,0", "3,99999999999999" };
        }

        public static IEnumerable<object[]> DotDecimalNonStringValidValues()
        {
            yield return new object[] { typeof(decimal), "1.0", "3.0", 1.0m };
            yield return new object[] { typeof(decimal), "1.0", "3.0", 3.0m };
            yield return new object[] { typeof(decimal), "1.0", "3.0", 2.9999999999999999999999999999m };
            yield return new object[] { typeof(double), "1.0", "3.0", 1.0 };
            yield return new object[] { typeof(double), "1.0", "3.0", 3.0 };
            yield return new object[] { typeof(double), "1.0", "3.0", 2.99999999999999 };
        }

        public static IEnumerable<object[]> CommaDecimalNonStringValidValues()
        {
            yield return new object[] { typeof(decimal), "1,0", "3,0", 1.0m };
            yield return new object[] { typeof(decimal), "1,0", "3,0", 3.0m };
            yield return new object[] { typeof(decimal), "1,0", "3,0", 2.9999999999999999999999999999m };
            yield return new object[] { typeof(double), "1,0", "3,0", 1.0 };
            yield return new object[] { typeof(double), "1,0", "3,0", 3.0 };
            yield return new object[] { typeof(double), "1,0", "3,0", 2.99999999999999 };
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalRanges))]
        public static void ParseDotSeparatorExtremaInCommaSeparatorCultures(Type type, string min, string max)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(null));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(null));
            }
        }

        [Theory]
        [MemberData(nameof(DotDecimalRanges))]
        public static void ParseDotSeparatorInvariantExtremaInCommaSeparatorCultures(Type type, string min, string max)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(null));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(null));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(CommaDecimalRanges))]
        public static void ParseCommaSeparatorExtremaInCommaSeparatorCultures(Type type, string min, string max)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(null));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(null));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(CommaDecimalRanges))]
        public static void ParseCommaSeparatorInvariantExtremaInCommaSeparatorCultures(Type type, string min, string max)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(null));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(null));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalValidValues))]
        public static void DotDecimalExtremaAndValues(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalValidValues))]
        public static void DotDecimalExtremaAndValuesInvariantParse(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalValidValues))]
        public static void DotDecimalExtremaAndValuesInvariantConvert(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(DotDecimalValidValues))]
        public static void DotDecimalExtremaAndValuesInvariantBoth(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true,
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true,
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalNonStringValidValues))]
        public static void DotDecimalExtremaAndNonStringValues(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(DotDecimalNonStringValidValues))]
        public static void DotDecimalExtremaAndNonStringValuesInvariantParse(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalNonStringValidValues))]
        public static void DotDecimalExtremaAndNonStringValuesInvariantConvert(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(DotDecimalNonStringValidValues))]
        public static void DotDecimalExtremaAndNonStringValuesInvariantBoth(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true,
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true,
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(CommaDecimalNonStringValidValues))]
        public static void CommaDecimalExtremaAndNonStringValues(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalNonStringValidValues))]
        public static void CommaDecimalExtremaAndNonStringValuesInvariantParse(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(CommaDecimalNonStringValidValues))]
        public static void CommaDecimalExtremaAndNonStringValuesInvariantConvert(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true
                    }.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalNonStringValidValues))]
        public static void CommaDecimalExtremaAndNonStringValuesInvariantBoth(Type type, string min, string max, object value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true,
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true,
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }


        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalInvalidValues))]
        public static void DotDecimalExtremaAndInvalidValues(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.False(new RangeAttribute(type, min, max).IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalInvalidValues))]
        public static void DotDecimalExtremaAndInvalidValuesInvariantParse(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.False(
                    new RangeAttribute(type, min, max)
                    {
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(DotDecimalInvalidValues))]
        public static void DotDecimalExtremaAndInvalidValuesInvariantConvert(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.False(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(DotDecimalInvalidValues))]
        public static void DotDecimalExtremaAndInvalidValuesInvariantBoth(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                Assert.False(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true,
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.False(
                    new RangeAttribute(type, min, max)
                    {
                        ConvertValueInInvariantCulture = true,
                        ParseLimitsInInvariantCulture = true
                    }.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(CommaDecimalValidValues))]
        public static void CommaDecimalExtremaAndValues(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.True(new RangeAttribute(type, min, max).IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalValidValues))]
        public static void CommaDecimalExtremaAndValuesInvariantParse(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalValidValues))]
        public static void CommaDecimalExtremaAndValuesInvariantConvert(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalValidValues))]
        public static void CommaDecimalExtremaAndValuesInvariantBoth(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true,
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true,
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [ConditionalTheory(typeof(PlatformDetection), nameof(PlatformDetection.IsNotInvariantGlobalization))]
        [MemberData(nameof(CommaDecimalInvalidValues))]
        public static void CommaDecimalExtremaAndInvalidValues(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max);
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                Assert.False(new RangeAttribute(type, min, max).IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalInvalidValues))]
        public static void CommaDecimalExtremaAndInvalidValuesInvariantParse(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalInvalidValues))]
        public static void CommaDecimalExtremaAndInvalidValuesInvariantConvert(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [MemberData(nameof(CommaDecimalInvalidValues))]
        public static void CommaDecimalExtremaAndInvalidValuesInvariantBoth(Type type, string min, string max, string value)
        {
            using (new ThreadCultureChange("en-US"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true,
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }

            using (new ThreadCultureChange("fr-FR"))
            {
                var range = new RangeAttribute(type, min, max)
                {
                    ConvertValueInInvariantCulture = true,
                    ParseLimitsInInvariantCulture = true
                };
                AssertExtensions.Throws<ArgumentException>("value", () => range.IsValid(value));
            }
        }

        [Theory]
        [InlineData(typeof(int), "1", "3")]
        [InlineData(typeof(double), "1", "3")]
        public static void Validate_CantConvertValueToTargetType_ThrowsException(Type type, string minimum, string maximum)
        {
            var attribute = new RangeAttribute(type, minimum, maximum);
            AssertExtensions.Throws<ArgumentException, Exception>(() => attribute.Validate("abc", new ValidationContext(new object())));
            AssertExtensions.Throws<ArgumentException, Exception>(() => attribute.IsValid("abc"));
        }

        [Fact]
        public static void Ctor_Int_Int()
        {
            var attribute = new RangeAttribute(1, 3);
            Assert.Equal(1, attribute.Minimum);
            Assert.Equal(3, attribute.Maximum);
            Assert.Equal(typeof(int), attribute.OperandType);
        }

        [Fact]
        public static void Ctor_Double_Double()
        {
            var attribute = new RangeAttribute(1.0, 3.0);
            Assert.Equal(1.0, attribute.Minimum);
            Assert.Equal(3.0, attribute.Maximum);
            Assert.Equal(typeof(double), attribute.OperandType);
        }

        [Theory]
        [InlineData(null)]
        [InlineData(typeof(object))]
        public static void Ctor_Type_String_String(Type type)
        {
            var attribute = new RangeAttribute(type, "SomeMinimum", "SomeMaximum");
            Assert.Equal("SomeMinimum", attribute.Minimum);
            Assert.Equal("SomeMaximum", attribute.Maximum);
            Assert.Equal(type, attribute.OperandType);
        }

        [Theory]
        [MemberData(nameof(GetRangeAttributeConstructorResults))]
        public static void ExclusiveBoundProperties_DefaultToFalse(RangeAttribute attribute)
        {
            Assert.False(attribute.MinimumIsExclusive);
            Assert.False(attribute.MaximumIsExclusive);
        }

        [Theory]
        [MemberData(nameof(GetRangeAttributeConstructorResults))]
        public static void ExclusiveBoundProperties_CanBeSet(RangeAttribute attribute)
        {
            attribute.MinimumIsExclusive = true;
            Assert.True(attribute.MinimumIsExclusive);

            attribute.MaximumIsExclusive = true;
            Assert.True(attribute.MaximumIsExclusive);
        }

        public static IEnumerable<object[]> GetRangeAttributeConstructorResults()
        {
            yield return new[] { new RangeAttribute(0, 1) };
            yield return new[] { new RangeAttribute(0d, 1d) };
            yield return new[] { new RangeAttribute(typeof(double), "0.0", "0.1") };
        }

        [Theory]
        [InlineData(null)]
        [InlineData(typeof(object))]
        public static void Validate_InvalidOperandType_ThrowsInvalidOperationException(Type type)
        {
            var attribute = new RangeAttribute(type, "someMinimum", "someMaximum");
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));
        }

        [Fact]
        public static void Validate_MinimumGreaterThanMaximum_ThrowsInvalidOperationException()
        {
            var attribute = new RangeAttribute(3, 1);
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));

            attribute = new RangeAttribute(3.0, 1.0);
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));

            attribute = new RangeAttribute(typeof(int), "3", "1");
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));

            attribute = new RangeAttribute(typeof(double), (3.0).ToString("F1"), (1.0).ToString("F1"));
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));

            attribute = new RangeAttribute(typeof(string), "z", "a");
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));
        }


        [Theory]
        [MemberData(nameof(GetRangeAttributesWithExclusiveEqualBounds))]
        public static void Validate_ExclusiveEqualBounds_ThrowsInvalidOperationException(RangeAttribute attribute)
        {
            // sanity check
            Assert.Equal(attribute.Minimum, attribute.Maximum);
            Assert.True(attribute.MinimumIsExclusive || attribute.MaximumIsExclusive);
            // Validate SUT
            Assert.Throws<InvalidOperationException>(() => attribute.Validate(attribute.Minimum, new ValidationContext(new object())));
        }

        public static IEnumerable<object[]> GetRangeAttributesWithExclusiveEqualBounds()
        {
            yield return new[] { new RangeAttribute(0, 0) { MinimumIsExclusive = true } };
            yield return new[] { new RangeAttribute(0, 0) { MaximumIsExclusive = true } };
            yield return new[] { new RangeAttribute(0, 0) { MinimumIsExclusive = true, MaximumIsExclusive = true } };
            yield return new[] { new RangeAttribute(1.1, 1.1) { MinimumIsExclusive = true } };
            yield return new[] { new RangeAttribute(1.1, 1.1) { MaximumIsExclusive = true } };
            yield return new[] { new RangeAttribute(1.1, 1.1) { MinimumIsExclusive = true, MaximumIsExclusive = true } };
            yield return new[] { new RangeAttribute(typeof(double), "0.0", "0.0") { MinimumIsExclusive = true, ParseLimitsInInvariantCulture = true } };
            yield return new[] { new RangeAttribute(typeof(double), "0.0", "0.0") { MaximumIsExclusive = true, ParseLimitsInInvariantCulture = true } };
            yield return new[] { new RangeAttribute(typeof(double), "0.0", "0.0") { MinimumIsExclusive = true, MaximumIsExclusive = true, ParseLimitsInInvariantCulture = true, } };
        }

        [Theory]
        [InlineData(null, "3")]
        [InlineData("3", null)]
        public static void Validate_MinimumOrMaximumNull_ThrowsInvalidOperationException(string minimum, string maximum)
        {
            RangeAttribute attribute = new RangeAttribute(typeof(int), minimum, maximum);
            Assert.Throws<InvalidOperationException>(() => attribute.Validate("Any", new ValidationContext(new object())));
        }

        [Theory]
        [InlineData(typeof(int), "Cannot Convert", "3")]
        [InlineData(typeof(int), "1", "Cannot Convert")]
        [InlineData(typeof(double), "Cannot Convert", "3")]
        [InlineData(typeof(double), "1", "Cannot Convert")]
        public static void Validate_MinimumOrMaximumCantBeConvertedToIntegralType_ThrowsException(Type type, string minimum, string maximum)
        {
            RangeAttribute attribute = new RangeAttribute(type, minimum, maximum);
            AssertExtensions.Throws<ArgumentException, Exception>(() => attribute.Validate("Any", new ValidationContext(new object())));
        }

        [Theory]
        [InlineData(typeof(DateTime), "Cannot Convert", "2014-03-19")]
        [InlineData(typeof(DateTime), "2014-03-19", "Cannot Convert")]
        public static void Validate_MinimumOrMaximumCantBeConvertedToDateTime_ThrowsFormatException(Type type, string minimum, string maximum)
        {
            RangeAttribute attribute = new RangeAttribute(type, minimum, maximum);
            Assert.Throws<FormatException>(() => attribute.Validate("Any", new ValidationContext(new object())));
        }

        [Theory]
        [InlineData(1, 2, "2147483648")]
        [InlineData(1, 2, "-2147483649")]
        public static void Validate_IntConversionOverflows_ThrowsOverflowException(int minimum, int maximum, object value)
        {
            RangeAttribute attribute = new RangeAttribute(minimum, maximum);
            Assert.Throws<OverflowException>(() => attribute.Validate(value, new ValidationContext(new object())));
        }

        [Fact]
        public static void Validate_IConvertibleThrowsCustomException_IsNotCaught()
        {
            RangeAttribute attribute = new RangeAttribute(typeof(int), "1", "1");
            Assert.Throws<ValidationException>(() => attribute.Validate(new IConvertibleImplementor() { IntThrow = new ArithmeticException() }, new ValidationContext(new object())));
        }
    }
}
