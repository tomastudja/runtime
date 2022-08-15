﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;

namespace System.Xml
{
    // This file is a decompiled "snapshot" based on actual file in System.Private.DataContractSerialization
    // All members are assigned constants so that accidental changes to original will not be reflected here
    internal enum XmlBinaryNodeType : byte
    {
        EndElement = 1,
        Comment = 2,
        Array = 3,
        MinAttribute = 4,
        ShortAttribute = 4,
        Attribute = 5,
        ShortDictionaryAttribute = 6,
        DictionaryAttribute = 7,
        ShortXmlnsAttribute = 8,
        XmlnsAttribute = 9,
        ShortDictionaryXmlnsAttribute = 10,
        DictionaryXmlnsAttribute = 11,
        PrefixDictionaryAttributeA = 12,
        PrefixDictionaryAttributeB = 13,
        PrefixDictionaryAttributeC = 14,
        PrefixDictionaryAttributeD = 15,
        PrefixDictionaryAttributeE = 16,
        PrefixDictionaryAttributeF = 17,
        PrefixDictionaryAttributeG = 18,
        PrefixDictionaryAttributeH = 19,
        PrefixDictionaryAttributeI = 20,
        PrefixDictionaryAttributeJ = 21,
        PrefixDictionaryAttributeK = 22,
        PrefixDictionaryAttributeL = 23,
        PrefixDictionaryAttributeM = 24,
        PrefixDictionaryAttributeN = 25,
        PrefixDictionaryAttributeO = 26,
        PrefixDictionaryAttributeP = 27,
        PrefixDictionaryAttributeQ = 28,
        PrefixDictionaryAttributeR = 29,
        PrefixDictionaryAttributeS = 30,
        PrefixDictionaryAttributeT = 31,
        PrefixDictionaryAttributeU = 32,
        PrefixDictionaryAttributeV = 33,
        PrefixDictionaryAttributeW = 34,
        PrefixDictionaryAttributeX = 35,
        PrefixDictionaryAttributeY = 36,
        PrefixDictionaryAttributeZ = 37,
        PrefixAttributeA = 38,
        PrefixAttributeB = 39,
        PrefixAttributeC = 40,
        PrefixAttributeD = 41,
        PrefixAttributeE = 42,
        PrefixAttributeF = 43,
        PrefixAttributeG = 44,
        PrefixAttributeH = 45,
        PrefixAttributeI = 46,
        PrefixAttributeJ = 47,
        PrefixAttributeK = 48,
        PrefixAttributeL = 49,
        PrefixAttributeM = 50,
        PrefixAttributeN = 51,
        PrefixAttributeO = 52,
        PrefixAttributeP = 53,
        PrefixAttributeQ = 54,
        PrefixAttributeR = 55,
        PrefixAttributeS = 56,
        PrefixAttributeT = 57,
        PrefixAttributeU = 58,
        PrefixAttributeV = 59,
        PrefixAttributeW = 60,
        PrefixAttributeX = 61,
        PrefixAttributeY = 62,
        MaxAttribute = 63,
        PrefixAttributeZ = 63,
        MinElement = 64,
        ShortElement = 64,
        Element = 65,
        ShortDictionaryElement = 66,
        DictionaryElement = 67,
        PrefixDictionaryElementA = 68,
        PrefixDictionaryElementB = 69,
        PrefixDictionaryElementC = 70,
        PrefixDictionaryElementD = 71,
        PrefixDictionaryElementE = 72,
        PrefixDictionaryElementF = 73,
        PrefixDictionaryElementG = 74,
        PrefixDictionaryElementH = 75,
        PrefixDictionaryElementI = 76,
        PrefixDictionaryElementJ = 77,
        PrefixDictionaryElementK = 78,
        PrefixDictionaryElementL = 79,
        PrefixDictionaryElementM = 80,
        PrefixDictionaryElementN = 81,
        PrefixDictionaryElementO = 82,
        PrefixDictionaryElementP = 83,
        PrefixDictionaryElementQ = 84,
        PrefixDictionaryElementR = 85,
        PrefixDictionaryElementS = 86,
        PrefixDictionaryElementT = 87,
        PrefixDictionaryElementU = 88,
        PrefixDictionaryElementV = 89,
        PrefixDictionaryElementW = 90,
        PrefixDictionaryElementX = 91,
        PrefixDictionaryElementY = 92,
        PrefixDictionaryElementZ = 93,
        PrefixElementA = 94,
        PrefixElementB = 95,
        PrefixElementC = 96,
        PrefixElementD = 97,
        PrefixElementE = 98,
        PrefixElementF = 99,
        PrefixElementG = 100,
        PrefixElementH = 101,
        PrefixElementI = 102,
        PrefixElementJ = 103,
        PrefixElementK = 104,
        PrefixElementL = 105,
        PrefixElementM = 106,
        PrefixElementN = 107,
        PrefixElementO = 108,
        PrefixElementP = 109,
        PrefixElementQ = 110,
        PrefixElementR = 111,
        PrefixElementS = 112,
        PrefixElementT = 113,
        PrefixElementU = 114,
        PrefixElementV = 115,
        PrefixElementW = 116,
        PrefixElementX = 117,
        PrefixElementY = 118,
        MaxElement = 119,
        PrefixElementZ = 119,
        MinText = 128,
        ZeroText = 128,
        ZeroTextWithEndElement = 129,
        OneText = 130,
        OneTextWithEndElement = 131,
        FalseText = 132,
        FalseTextWithEndElement = 133,
        TrueText = 134,
        TrueTextWithEndElement = 135,
        Int8Text = 136,
        Int8TextWithEndElement = 137,
        Int16Text = 138,
        Int16TextWithEndElement = 139,
        Int32Text = 140,
        Int32TextWithEndElement = 141,
        Int64Text = 142,
        Int64TextWithEndElement = 143,
        FloatText = 144,
        FloatTextWithEndElement = 145,
        DoubleText = 146,
        DoubleTextWithEndElement = 147,
        DecimalText = 148,
        DecimalTextWithEndElement = 149,
        DateTimeText = 150,
        DateTimeTextWithEndElement = 151,
        Chars8Text = 152,
        Chars8TextWithEndElement = 153,
        Chars16Text = 154,
        Chars16TextWithEndElement = 155,
        Chars32Text = 156,
        Chars32TextWithEndElement = 157,
        Bytes8Text = 158,
        Bytes8TextWithEndElement = 159,
        Bytes16Text = 160,
        Bytes16TextWithEndElement = 161,
        Bytes32Text = 162,
        Bytes32TextWithEndElement = 163,
        StartListText = 164,
        StartListTextWithEndElement = 165,
        EndListText = 166,
        EndListTextWithEndElement = 167,
        EmptyText = 168,
        EmptyTextWithEndElement = 169,
        DictionaryText = 170,
        DictionaryTextWithEndElement = 171,
        UniqueIdText = 172,
        UniqueIdTextWithEndElement = 173,
        TimeSpanText = 174,
        TimeSpanTextWithEndElement = 175,
        GuidText = 176,
        GuidTextWithEndElement = 177,
        UInt64Text = 178,
        UInt64TextWithEndElement = 179,
        BoolText = 180,
        BoolTextWithEndElement = 181,
        UnicodeChars8Text = 182,
        UnicodeChars8TextWithEndElement = 183,
        UnicodeChars16Text = 184,
        UnicodeChars16TextWithEndElement = 185,
        UnicodeChars32Text = 186,
        UnicodeChars32TextWithEndElement = 187,
        QNameDictionaryText = 188,
        MaxText = 189,
        QNameDictionaryTextWithEndElement = 189
    }
}
