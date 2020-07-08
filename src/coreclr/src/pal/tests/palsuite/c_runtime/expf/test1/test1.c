// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*=====================================================================
**
** Source:  test1.c
**
** Purpose: Tests expf with a normal set of values.
**
**===================================================================*/

#include <palsuite.h>

// binary32 (float) has a machine epsilon of 2^-23 (approx. 1.19e-07). However, this 
// is slightly too accurate when writing tests meant to run against libm implementations
// for various platforms. 2^-21 (approx. 4.76e-07) seems to be as accurate as we can get.
//
// The tests themselves will take PAL_EPSILON and adjust it according to the expected result
// so that the delta used for comparison will compare the most significant digits and ignore
// any digits that are outside the double precision range (6-9 digits).

// For example, a test with an expect result in the format of 0.xxxxxxxxx will use PAL_EPSILON
// for the variance, while an expected result in the format of 0.0xxxxxxxxx will use
// PAL_EPSILON / 10 and and expected result in the format of x.xxxxxx will use PAL_EPSILON * 10.
#define PAL_EPSILON 4.76837158e-07

#define PAL_NAN     sqrtf(-1.0f)
#define PAL_POSINF -logf(0.0f)
#define PAL_NEGINF  logf(0.0f)

/**
 * Helper test structure
 */
struct test
{
    float value;     /* value to test the function with */
    float expected;  /* expected result */
    float variance;  /* maximum delta between the expected and actual result */
};

/**
 * validate
 *
 * test validation function
 */
void __cdecl validate(float value, float expected, float variance)
{
    float result = expf(value);

    /*
     * The test is valid when the difference between result
     * and expected is less than or equal to variance
     */
    float delta = fabsf(result - expected);

    if (delta > variance)
    {
        Fail("expf(%g) returned %10.9g when it should have returned %10.9g",
             value, result, expected);
    }
}

/**
 * validate
 *
 * test validation function for values returning NaN
 */
void __cdecl validate_isnan(float value)
{
    float result = expf(value);

    if (!_isnanf(result))
    {
        Fail("expf(%g) returned %10.9g when it should have returned %10.9g",
             value, result, PAL_NAN);
    }
}

/**
 * main
 * 
 * executable entry point
 */
int __cdecl main(int argc, char **argv)
{
    struct test tests[] = 
    {
        /* value            expected          variance */
        { PAL_NEGINF,       0,                PAL_EPSILON },
        { -3.14159265f,     0.0432139183f,    PAL_EPSILON / 10 },   // value: -(pi)
        { -2.71828183f,     0.0659880358f,    PAL_EPSILON / 10 },   // value: -(e)
        { -2.30258509f,     0.1f,             PAL_EPSILON },        // value: -(ln(10))
        { -1.57079633f,     0.207879576f,     PAL_EPSILON },        // value: -(pi / 2)
        { -1.44269504f,     0.236290088f,     PAL_EPSILON },        // value: -(logf2(e))
        { -1.41421356f,     0.243116734f,     PAL_EPSILON },        // value: -(sqrtf(2))
        { -1.12837917f,     0.323557264f,     PAL_EPSILON },        // value: -(2 / sqrtf(pi))
        { -1,               0.367879441f,     PAL_EPSILON },        // value: -(1)
        { -0.785398163f,    0.455938128f,     PAL_EPSILON },        // value: -(pi / 4)
        { -0.707106781f,    0.493068691f,     PAL_EPSILON },        // value: -(1 / sqrtf(2))
        { -0.693147181f,    0.5f,             PAL_EPSILON },        // value: -(ln(2))
        { -0.636619772f,    0.529077808f,     PAL_EPSILON },        // value: -(2 / pi)
        { -0.434294482f,    0.647721485f,     PAL_EPSILON },        // value: -(log10f(e))
        { -0.318309886f,    0.727377349f,     PAL_EPSILON },        // value: -(1 / pi)
        {  0,               1,                PAL_EPSILON * 10 },
        {  0.318309886f,    1.37480223f,      PAL_EPSILON * 10 },   // value:  1 / pi
        {  0.434294482f,    1.54387344f,      PAL_EPSILON * 10 },   // value:  log10f(e)
        {  0.636619772f,    1.89008116f,      PAL_EPSILON * 10 },   // value:  2 / pi
        {  0.693147181f,    2,                PAL_EPSILON * 10 },   // value:  ln(2)
        {  0.707106781f,    2.02811498f,      PAL_EPSILON * 10 },   // value:  1 / sqrtf(2)
        {  0.785398163f,    2.19328005f,      PAL_EPSILON * 10 },   // value:  pi / 4
        {  1,               2.71828183f,      PAL_EPSILON * 10 },   //                           expected: e
        {  1.12837917f,     3.09064302f,      PAL_EPSILON * 10 },   // value:  2 / sqrtf(pi)
        {  1.41421356f,     4.11325038f,      PAL_EPSILON * 10 },   // value:  sqrtf(2)
        {  1.44269504f,     4.23208611f,      PAL_EPSILON * 10 },   // value:  logf2(e)
        {  1.57079633f,     4.81047738f,      PAL_EPSILON * 10 },   // value:  pi / 2
        {  2.30258509f,     10,               PAL_EPSILON * 100 },  // value:  ln(10)
        {  2.71828183f,     15.1542622f,      PAL_EPSILON * 100 },  // value:  e
        {  3.14159265f,     23.1406926f,      PAL_EPSILON * 100 },  // value:  pi
        {  PAL_POSINF,      PAL_POSINF,       0 },
    };

    if (PAL_Initialize(argc, argv) != 0)
    {
        return FAIL;
    }

    for (int i = 0; i < (sizeof(tests) / sizeof(struct test)); i++)
    {
        validate(tests[i].value, tests[i].expected, tests[i].variance);
    }

    validate_isnan(PAL_NAN);

    PAL_Terminate();
    return PASS;
}
