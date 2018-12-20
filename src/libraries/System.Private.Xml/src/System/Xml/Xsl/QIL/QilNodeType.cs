// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System.Xml.Xsl.Qil
{
    /// <summary>An enumeration of all the possible QilExpression node types.</summary>
    /// <remarks>See <see href="http://dynamo/qil/qil.xml">the QIL functional specification</see> for documentation.</remarks>
    internal enum QilNodeType
    {
        // Do not edit this region
        // It is auto-generated
        #region AUTOGENERATED
        #region meta
        //-----------------------------------------------
        // meta
        //-----------------------------------------------
        QilExpression,
        FunctionList,
        GlobalVariableList,
        GlobalParameterList,
        ActualParameterList,
        FormalParameterList,
        SortKeyList,
        BranchList,
        OptimizeBarrier,
        Unknown,
        #endregion // meta

        #region specials
        //-----------------------------------------------
        // specials
        //-----------------------------------------------
        DataSource,
        Nop,
        Error,
        Warning,
        #endregion // specials

        #region variables
        //-----------------------------------------------
        // variables
        //-----------------------------------------------
        For,
        Let,
        Parameter,
        PositionOf,
        #endregion // variables

        #region literals
        //-----------------------------------------------
        // literals
        //-----------------------------------------------
        True,
        False,
        LiteralString,
        LiteralInt32,
        LiteralInt64,
        LiteralDouble,
        LiteralDecimal,
        LiteralQName,
        LiteralType,
        LiteralObject,
        #endregion // literals

        #region boolean operators
        //-----------------------------------------------
        // boolean operators
        //-----------------------------------------------
        And,
        Or,
        Not,
        #endregion // boolean operators

        #region choice
        //-----------------------------------------------
        // choice
        //-----------------------------------------------
        Conditional,
        Choice,
        #endregion // choice

        #region collection operators
        //-----------------------------------------------
        // collection operators
        //-----------------------------------------------
        Length,
        Sequence,
        Union,
        Intersection,
        Difference,
        Average,
        Sum,
        Minimum,
        Maximum,
        #endregion // collection operators

        #region arithmetic operators
        //-----------------------------------------------
        // arithmetic operators
        //-----------------------------------------------
        Negate,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,
        #endregion // arithmetic operators

        #region string operators
        //-----------------------------------------------
        // string operators
        //-----------------------------------------------
        StrLength,
        StrConcat,
        StrParseQName,
        #endregion // string operators

        #region value comparison operators
        //-----------------------------------------------
        // value comparison operators
        //-----------------------------------------------
        Ne,
        Eq,
        Gt,
        Ge,
        Lt,
        Le,
        #endregion // value comparison operators

        #region node comparison operators
        //-----------------------------------------------
        // node comparison operators
        //-----------------------------------------------
        Is,
        After,
        Before,
        #endregion // node comparison operators

        #region loops
        //-----------------------------------------------
        // loops
        //-----------------------------------------------
        Loop,
        Filter,
        #endregion // loops

        #region sorting
        //-----------------------------------------------
        // sorting
        //-----------------------------------------------
        Sort,
        SortKey,
        DocOrderDistinct,
        #endregion // sorting

        #region function definition and invocation
        //-----------------------------------------------
        // function definition and invocation
        //-----------------------------------------------
        Function,
        Invoke,
        #endregion // function definition and invocation

        #region XML navigation
        //-----------------------------------------------
        // XML navigation
        //-----------------------------------------------
        Content,
        Attribute,
        Parent,
        Root,
        XmlContext,
        Descendant,
        DescendantOrSelf,
        Ancestor,
        AncestorOrSelf,
        Preceding,
        FollowingSibling,
        PrecedingSibling,
        NodeRange,
        Deref,
        #endregion // XML navigation

        #region XML construction
        //-----------------------------------------------
        // XML construction
        //-----------------------------------------------
        ElementCtor,
        AttributeCtor,
        CommentCtor,
        PICtor,
        TextCtor,
        RawTextCtor,
        DocumentCtor,
        NamespaceDecl,
        RtfCtor,
        #endregion // XML construction

        #region Node properties
        //-----------------------------------------------
        // Node properties
        //-----------------------------------------------
        NameOf,
        LocalNameOf,
        NamespaceUriOf,
        PrefixOf,
        #endregion // Node properties

        #region Type operators
        //-----------------------------------------------
        // Type operators
        //-----------------------------------------------
        TypeAssert,
        IsType,
        IsEmpty,
        #endregion // Type operators

        #region XPath operators
        //-----------------------------------------------
        // XPath operators
        //-----------------------------------------------
        XPathNodeValue,
        XPathFollowing,
        XPathPreceding,
        XPathNamespace,
        #endregion // XPath operators

        #region XSLT
        //-----------------------------------------------
        // XSLT
        //-----------------------------------------------
        XsltGenerateId,
        XsltInvokeLateBound,
        XsltInvokeEarlyBound,
        XsltCopy,
        XsltCopyOf,
        XsltConvert,
        #endregion // XSLT

        #endregion // AUTOGENERATED
    }
}
