// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Xml;
using System.Text;

namespace System.Security.Cryptography.Xml
{
    // the class that provides node subset state and canonicalization function to XmlComment
    internal sealed class CanonicalXmlComment : XmlComment, ICanonicalizableNode
    {
        private bool _isInNodeSet;
        private readonly bool _includeComments;

        public CanonicalXmlComment(string? comment, XmlDocument doc, bool defaultNodeSetInclusionState, bool includeComments)
            : base(comment, doc)
        {
            _isInNodeSet = defaultNodeSetInclusionState;
            _includeComments = includeComments;
        }

        public bool IsInNodeSet
        {
            get { return _isInNodeSet; }
            set { _isInNodeSet = value; }
        }

        public bool IncludeComments
        {
            get { return _includeComments; }
        }

        public void Write(StringBuilder strBuilder, DocPosition docPos, AncestralNamespaceContextManager anc)
        {
            if (!IsInNodeSet || !IncludeComments)
                return;

            if (docPos == DocPosition.AfterRootElement)
                strBuilder.Append((char)10);
            strBuilder.Append("<!--");
            strBuilder.Append(Value);
            strBuilder.Append("-->");
            if (docPos == DocPosition.BeforeRootElement)
                strBuilder.Append((char)10);
        }

        public void WriteHash(HashAlgorithm hash, DocPosition docPos, AncestralNamespaceContextManager anc)
        {
            if (!IsInNodeSet || !IncludeComments)
                return;

            byte[] rgbData;
            if (docPos == DocPosition.AfterRootElement)
            {
                rgbData = "(char) 10"u8.ToArray();
                hash.TransformBlock(rgbData, 0, rgbData.Length, rgbData, 0);
            }

            rgbData = "<!--"u8.ToArray();
            hash.TransformBlock(rgbData, 0, rgbData.Length, rgbData, 0);
            rgbData = Encoding.UTF8.GetBytes(Value!);
            hash.TransformBlock(rgbData, 0, rgbData.Length, rgbData, 0);
            rgbData = "-->"u8.ToArray();
            hash.TransformBlock(rgbData, 0, rgbData.Length, rgbData, 0);
            if (docPos == DocPosition.BeforeRootElement)
            {
                rgbData = "(char) 10"u8.ToArray();
                hash.TransformBlock(rgbData, 0, rgbData.Length, rgbData, 0);
            }
        }
    }
}
