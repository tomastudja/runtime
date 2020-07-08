// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using OLEDB.Test.ModuleCore;

namespace System.Xml.Tests
{
    internal class XmlEmbeddedNullCharConvertTests1 : XmlEmbeddedNullCharConvertTests
    {
        #region Constructors and Destructors

        public XmlEmbeddedNullCharConvertTests1()
        {
            for (int i = 0; i < _byte_EmbeddedNull.Length; i = i + 2)
            {
                AddVariation(new CVariation(this, "EncodeName-EncodeLocalName : " + _Expbyte_EmbeddedNull[i / 2], XmlEncodeName1));
            }
        }

        #endregion

        #region Public Methods and Operators

        public int XmlEncodeName1()
        {
            int i = ((CurVariation.id) - 1) * 2;
            string strEnVal = string.Empty;

            strEnVal = XmlConvert.EncodeName((BitConverter.ToChar(_byte_EmbeddedNull, i)).ToString());
            CError.Compare(strEnVal, _Expbyte_EmbeddedNull[i / 2], "Comparison failed at " + i);
            return TEST_PASS;
        }
        #endregion
    }
}
