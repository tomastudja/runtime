// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//{{NO_DEPENDENCIES}}
// Used by mscorrc.rc
//


// For (failing) hresults of facility FACILITY_URT, we store
// unparameterized description strings in the range
// 0x6000..0x9000.
#define MSG_FOR_URT_HR(hr) (0x6000 + (HRESULT_CODE(hr)))
#define MAX_URT_HRESULT_CODE 0x3000

#define HR_FOR_URT_MSG(code) (((code) >=0x6000 && (code) <= 0x6000+MAX_URT_HRESULT_CODE) ? \
                                 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_URT, (code) - 0x6000) : \
                                 (code))

#ifndef HRESULT_CODE
#define HRESULT_CODE(hr)    ((hr) & 0xFFFF)
#endif // HRESULT_CODE


//-----------------------------------------------------------------------------
// Resource strings for MDA descriptions.
//-----------------------------------------------------------------------------

#define IDS_RTL                                 0x01F5

#define IDS_DS_ACTIVESESSIONS                   0x1701
#define IDS_DS_DATASOURCENAME                   0x1702
#define IDS_DS_DATASOURCEREADONLY               0x1703
#define IDS_DS_DBMSNAME                         0x1704
#define IDS_DS_DBMSVER                          0x1705
#define IDS_DS_IDENTIFIERCASE                   0x1706
#define IDS_DS_DSOTHREADMODEL                   0x1707

#define IDS_EE_NDIRECT_UNSUPPORTED_SIG          0x1708
#define IDS_EE_NDIRECT_BADNATL                  0x170a
#define IDS_EE_NDIRECT_LOADLIB_WIN              0x170b
#define IDS_EE_NDIRECT_GETPROCADDRESS_WIN       0x170c
#define IDS_EE_COM_UNSUPPORTED_SIG              0x170d
#define IDS_EE_NOSYNCHRONIZED                   0x170f
#define IDS_EE_NDIRECT_BADNATL_THISCALL         0x1710
#define IDS_EE_MULTIPLE_CALLCONV_UNSUPPORTED    0x1711

#define IDS_EE_LOAD_BAD_MAIN_SIG                0x1712
#define IDS_EE_COM_UNSUPPORTED_TYPE             0x1713

#define IDS_EE_RETHROW_NOT_ALLOWED              0x171d
#define IDS_EE_INVALID_OLE_VARIANT              0x171e

#define IDS_EE_FILE_NOT_FOUND                   0x80070002
#define IDS_EE_PATH_TOO_LONG                    0x8007006F
#define IDS_EE_PROC_NOT_FOUND                   0x8007007F
#define IDS_EE_ALREADY_EXISTS                   0x800700B7
#define IDS_EE_BAD_USER_PROFILE                 0x800704E5
#define IDS_INET_E_CANNOT_CONNECT               0x1799 // 0x800C0004
#define IDS_INET_E_RESOURCE_NOT_FOUND           0x1a60 // 0x800C0005
#define IDS_INET_E_CONNECTION_TIMEOUT           0x1a1e // 0x800C000B
#define IDS_INET_E_SECURITY_PROBLEM             0x800C000E

#define IDS_EE_TO_MANY_ARGUMENTS_IN_MAIN        0x1721
#define IDS_EE_FAILED_TO_FIND_MAIN              0x1722
#define IDS_EE_ILLEGAL_TOKEN_FOR_MAIN           0x1723
#define IDS_EE_MAIN_METHOD_MUST_BE_STATIC       0x1724
#define IDS_EE_MAIN_METHOD_HAS_INVALID_RTN      0x1725
#define IDS_EE_VTABLECALLSNOTSUPPORTED          0x1726

#define IDS_EE_BADMARSHALFIELD_STRING           0x1727
#define IDS_EE_BADMARSHALFIELD_NOCUSTOMMARSH    0x1728
#define IDS_EE_BADMARSHALFIELD_FIXEDARRAY_ZEROSIZE 0x172a
#define IDS_EE_BADMARSHALFIELD_LAYOUTCLASS      0x172b
#define IDS_EE_BADMARSHALFIELD_ARRAY            0x172c

#define IDS_EE_BADMARSHALPARAM_NO_LPTSTR        0x172d

#define IDS_EE_SAFEARRAYTYPEMISMATCH            0x1738
#define IDS_EE_SAFEARRAYRANKMISMATCH            0x1739
#define IDS_EE_BADMARSHAL_GENERIC               0x173a
#define IDS_EE_BADMARSHAL_CHAR                  0x173b
#define IDS_EE_BADMARSHAL_BOOLEAN               0x173c
#define IDS_EE_BADMARSHAL_I1                    0x173d
#define IDS_EE_BADMARSHAL_I2                    0x173e
#define IDS_EE_BADMARSHAL_I4                    0x173f
#define IDS_EE_BADMARSHAL_I8                    0x1740
#define IDS_EE_BADMARSHAL_I                     0x1741
#define IDS_EE_BADMARSHAL_R4                    0x1742
#define IDS_EE_BADMARSHAL_R8                    0x1743
#define IDS_EE_BADMARSHAL_PTR                   0x1745
#define IDS_EE_BADMARSHAL_NOLAYOUT              0x1746
#define IDS_EE_BADMARSHALPARAM_STRING           0x1747
#define IDS_EE_BADMARSHALPARAM_STRINGBUILDER    0x1748
#define IDS_EE_BADMARSHAL_DELEGATE              0x1749
#define IDS_EE_BADMARSHAL_FNPTR                 0x174a
#define IDS_EE_BADMARSHAL_INTERFACE             0x174b
#define IDS_EE_BADMARSHAL_CLASS                 0x174c
#define IDS_EE_BADMARSHAL_VALUETYPE             0x174d
#define IDS_EE_BADMARSHAL_OBJECT                0x174e
#define IDS_EE_BADMARSHALFIELD_OBJECT           0x174f
#define IDS_EE_BADMARSHALPARAM_DECIMAL          0x1750
#define IDS_EE_BADMARSHAL_GUID                  0x1751
#define IDS_EE_BADMARSHAL_DATETIME              0x1753
#define IDS_EE_BADMARSHAL_ARRAY                 0x1754
#define IDS_EE_BADMARSHAL_BADMANAGED            0x1756
#define IDS_EE_SRC_OBJ_NOT_COMOBJECT            0x1757
#define IDS_EE_CANNOT_COERCE_COMOBJECT          0x1759
#define IDS_EE_BADMARSHAL_AUTOLAYOUT            0x175a
#define IDS_EE_BADMARSHAL_RESTRICTION           0x175d
#define IDS_EE_BADMARSHAL_ASANYRESTRICTION      0x175f
#define IDS_EE_BADMARSHAL_VBBYVALSTRRESTRICTION 0x1760
#define IDS_EE_BADMARSHAL_AWORESTRICTION        0x1761
#define IDS_EE_BADMARSHAL_ARGITERATORRESTRICTION 0x1765
#define IDS_EE_BADMARSHAL_HANDLEREFRESTRICTION  0x1766

#define IDS_EE_ADUNLOAD_NOT_ALLOWED             0x1767

#define IDS_CANNOT_MARSHAL                      0x1770
#define IDS_CANNOT_MARSHAL_RECURSIVE_DEF        0x1771
#define IDS_EE_HASH_VAL_FAILED                  0x1772


#define IDS_CLASSLOAD_GENERAL                   0x80131522
#define IDS_CLASSLOAD_BADFORMAT                 0x1774
#define IDS_CLASSLOAD_BYREFARRAY                0x1775
#define IDS_CLASSLOAD_BYREFLIKEARRAY            0x1776
#define IDS_CLASSLOAD_STATICVIRTUAL             0x1778
#define IDS_CLASSLOAD_REDUCEACCESS              0x1779
#define IDS_CLASSLOAD_BADPINVOKE                0x177a
#define IDS_CLASSLOAD_VALUECLASSTOOLARGE        0x177b
#define IDS_CLASSLOAD_NOTIMPLEMENTED            0x177c
#define IDS_CLASSLOAD_PARENTNULL                0x177d
#define IDS_CLASSLOAD_PARENTINTERFACE           0x177e
#define IDS_CLASSLOAD_INTERFACEOBJECT           0x177f
#define IDS_CLASSLOAD_INTERFACENULL             0x1780
#define IDS_CLASSLOAD_NOTINTERFACE              0x1781
#define IDS_CLASSLOAD_VALUEINSTANCEFIELD        0x1782
#define IDS_CLASSLOAD_EXPLICIT_GENERIC          0x1783
#define IDS_CLASSLOAD_RANK_TOOLARGE             0x1785
#define IDS_CLASSLOAD_BAD_UNMANAGED_RVA         0x1787
#define IDS_CLASSLOAD_ENCLOSING                 0x1789
#define IDS_CLASSLOAD_EXPLICIT_LAYOUT           0x178a
#define IDS_CLASSLOAD_SEALEDPARENT              0x178b
#define IDS_CLASSLOAD_NOMETHOD_NAME             0x178c
#define IDS_CLASSLOAD_BADSPECIALMETHOD          0x178e
#define IDS_CLASSLOAD_MI_DECLARATIONNOTFOUND    0x178f
#define IDS_CLASSLOAD_MI_MULTIPLEOVERRIDES      0x1790
#define IDS_CLASSLOAD_MI_ACCESS_FAILURE         0x1791
#define IDS_CLASSLOAD_MI_BADSIGNATURE           0x1793
#define IDS_CLASSLOAD_MI_NOTIMPLEMENTED         0x1794
#define IDS_CLASSLOAD_MI_MUSTBEVIRTUAL          0x1796
#define IDS_CLASSLOAD_MISSINGMETHODRVA          0x1797
#define IDS_CLASSLOAD_FIELDTOOLARGE             0x1798
#define IDS_CLASSLOAD_CANTEXTEND                0x179a
#define IDS_CLASSLOAD_ZEROSIZE                  0x179b
#define IDS_CLASSLOAD_TYPESPEC                  0x179c
#define IDS_CLASSLOAD_BAD_FIELD                 0x179d
#define IDS_CLASSLOAD_MI_ILLEGAL_BODY           0x179e
#define IDS_CLASSLOAD_MI_ILLEGAL_TOKEN_BODY     0x17a0
#define IDS_CLASSLOAD_MI_ILLEGAL_TOKEN_DECL     0x17a1
#define IDS_CLASSLOAD_MI_SEALED_DECL            0x17a2
#define IDS_CLASSLOAD_MI_FINAL_DECL             0x17a3
#define IDS_CLASSLOAD_MI_NONVIRTUAL_DECL        0x17a4
#define IDS_CLASSLOAD_MI_BODY_DECL_MISMATCH     0x17a5
#define IDS_CLASSLOAD_MI_MISSING_SIG_BODY       0x17a6
#define IDS_CLASSLOAD_MI_MISSING_SIG_DECL       0x17a7
#define IDS_CLASSLOAD_MI_BADRETURNTYPE          0x17a8
#define IDS_CLASSLOAD_STATICVIRTUAL_NOTIMPL     0x17a9

#define IDS_INVALID_RECURSIVE_GENERIC_FIELD_LOAD 0x17aa
#define IDS_CLASSLOAD_TOOMANYGENERICARGS        0x17ab

#define IDS_CLASSLOAD_INLINE_ARRAY_FIELD_COUNT  0x17ac
#define IDS_CLASSLOAD_INLINE_ARRAY_LENGTH       0x17ad
#define IDS_CLASSLOAD_INLINE_ARRAY_EXPLICIT     0x17ae

#define IDS_DEBUG_USERBREAKPOINT                0x17b6

#define IDS_PERFORMANCEMON_FUNCNOTFOUND         0x17bb
#define IDS_PERFORMANCEMON_FUNCNOTFOUND_TITLE   0x17bc
#define IDS_PERFORMANCEMON_PSAPINOTFOUND        0x17bd
#define IDS_PERFORMANCEMON_PSAPINOTFOUND_TITLE  0x17be

#define IDS_INVALID_REDIM                       0x17c3
#define IDS_INVALID_PINVOKE_CALLCONV            0x17c4
#define IDS_CLASSLOAD_NSTRUCT_EXPLICIT_OFFSET   0x17c7
#define IDS_EE_BADPINVOKEFIELD_NOTMARSHALABLE   0x17c9
#define IDS_WRONGSIZEARRAY_IN_NSTRUCT           0x17ca

#define IDS_EE_INVALIDLCIDPARAM                 0x17cd
#define IDS_EE_BADMARSHAL_NESTEDARRAY           0x17ce
#define IDS_EE_INVALIDCOMSOURCEITF              0x17d1
#define IDS_EE_CANNOT_COERCE_BYREF_VARIANT      0x17d2
#define IDS_EE_WRAPPER_MUST_HAVE_DEF_CONS       0x17d3
#define IDS_EE_INVALID_STD_DISPID_NAME          0x17d4
#define IDS_EE_NO_IDISPATCH_ON_TARGET           0x17d5
#define IDS_EE_NON_STD_NAME_WITH_STD_DISPID     0x17d6
#define IDS_EE_INVOKE_NEW_ENUM_INVALID_RETURN   0x17d7
#define IDS_EE_COM_OBJECT_RELEASE_RACE          0x17d8
#define IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER 0x17d9
#define IDS_EE_NDIRECT_BADNATL_CALLCONV         0x17df
#define IDS_EE_CANNOTCAST                       0x17e0

#define IDS_EE_NOCUSTOMMARSHALER                0x17e7
#define IDS_EE_SIZECONTROLOUTOFRANGE            0x17e8
#define IDS_EE_SIZECONTROLBADTYPE               0x17e9
#define IDS_EE_SAFEARRAYSZARRAYMISMATCH         0x17eb
#define IDS_EE_INVALID_VT_FOR_CUSTOM_MARHALER   0x17ec
#define IDS_EE_BAD_COMEXTENDS_CLASS             0x17ed

#define IDS_EE_LOCAL_COGETCLASSOBJECT_FAILED    0x17f5

#define IDS_EE_MISSING_FIELD                    0x17f7
#define IDS_EE_MISSING_METHOD                   0x17f8

#define IDS_EE_INTERFACE_NOT_DISPATCH_BASED     0x17f9

#define IDS_EE_UNHANDLED_EXCEPTION              0x17fc
#define IDS_EE_EXCEPTION_TOSTRING_FAILED        0x17fd

#define IDS_CLASSLOAD_EQUIVALENTSTRUCTMETHODS   0x17fe
#define IDS_CLASSLOAD_EQUIVALENTSTRUCTFIELDS    0x17ff

#define IDS_EE_SIGTOOCOMPLEX                    0x1a03
#define IDS_EE_STRUCTTOOCOMPLEX                 0x1a04
#define IDS_EE_STRUCTARRAYTOOLARGE              0x1a05
#define IDS_EE_BADMARSHALFIELD_NOSTRINGBUILDER  0x1a06

#define IDS_EE_NO_BACKING_CLASS_FACTORY         0x1a0b
#define IDS_EE_STRING_TOOLONG                   0x1a0d
#define IDS_EE_VARARG_NOT_SUPPORTED             0x1a0f

#define IDS_EE_INVALID_CA                       0x1a10

#define IDS_EE_THREAD_CANNOT_GET                0x1a15
#define IDS_EE_THREAD_BAD_STATE                 0x1a1b
#define IDS_EE_THREAD_ABORT_WHILE_SUSPEND       0x1a1c

#define IDS_EE_NOVARIANTRETURN                  0x1a1d

#define IDS_EE_METHOD_NOT_FOUND_ON_EV_PROV      0x1a24
#define IDS_EE_BAD_COMEVENTITF_CLASS            0x1a25

#define IDS_EE_COREXEMAIN2_FAILED_TITLE         0x1a2b
#define IDS_EE_COREXEMAIN2_FAILED_TEXT          0x1a2c

#define IDS_EE_ICUSTOMMARSHALERNOTIMPL          0x1a2e
#define IDS_EE_GETINSTANCENOTIMPL               0x1a2f

#define IDS_EE_BADMARSHAL_CUSTOMMARSHALER       0x1a30

#define IDS_CLASSLOAD_COMIMPCANNOTHAVELAYOUT    0x1a31
#define IDS_EE_INVALIDCOMDEFITF                 0x1a32
#define IDS_EE_COMDEFITFNOTSUPPORTED            0x1a33

#define IDS_EE_CLASS_TO_VARIANT_TLB_NOT_REG     0x1a35
#define IDS_EE_CANNOT_MAP_TO_MANAGED_VC         0x1a36

#define IDS_EE_MARSHAL_UNMAPPABLE_CHAR          0x1a37

#define IDS_EE_BADMARSHAL_SAFEHANDLENATIVETOCOM 0x1a3a
#define IDS_EE_BADMARSHAL_ABSTRACTOUTSAFEHANDLE 0x1a3b
#define IDS_EE_BADMARSHAL_RETURNSHCOMTONATIVE   0x1a3c
#define IDS_EE_BADMARSHAL_SAFEHANDLE            0x1a3d

#define IDS_EE_SAFEHANDLECLOSED                 0x1a3f
#define IDS_EE_SAFEHANDLECANNOTSETHANDLE        0x1a40

#define IDS_EE_BADMARSHAL_ABSTRACTRETSAFEHANDLE 0x1a44
#define IDS_EE_SH_IN_VARIANT_NOT_SUPPORTED      0x1a47

#define IDS_EE_BADMARSHAL_SYSARRAY              0x1a48
#define IDS_EE_VAR_WRAP_IN_VAR_NOT_SUPPORTED    0x1a49
#define IDS_EE_RECORD_NON_SUPPORTED_FIELDS      0x1a4a

#define IDS_CLASSLOAD_TYPEWRONGNUMGENERICARGS   0x1a4b
#define IDS_CLASSLOAD_NSTRUCT_NEGATIVE_OFFSET   0x1a4d

#define IDS_CLASSLOAD_INVALIDINSTANTIATION      0x1a59

#define IDS_EE_CLASSLOAD_INVALIDINSTANTIATION      0x1a59
#define IDS_EE_BADMARSHALFIELD_ZEROLENGTHFIXEDSTRING 0x1a5a

#define IDS_EE_BADMARSHAL_CRITICALHANDLENATIVETOCOM 0x1a62
#define IDS_EE_BADMARSHAL_ABSTRACTOUTCRITICALHANDLE 0x1a63
#define IDS_EE_BADMARSHAL_RETURNCHCOMTONATIVE       0x1a64
#define IDS_EE_BADMARSHAL_CRITICALHANDLE            0x1a65
#define IDS_EE_BADMARSHAL_INT128_RESTRICTION        0x1a66

#define IDS_EE_BADMARSHAL_ABSTRACTRETCRITICALHANDLE 0x1a6a
#define IDS_EE_CH_IN_VARIANT_NOT_SUPPORTED          0x1a6b

#define IDS_CLASSLOAD_CONSTRAINT_MISMATCH_ON_IMPLICIT_OVERRIDE 0x1a6f
#define IDS_CLASSLOAD_CONSTRAINT_MISMATCH_ON_IMPLICIT_IMPLEMENTATION 0x1a70
#define IDS_CLASSLOAD_CONSTRAINT_MISMATCH_ON_LOCAL_METHOD_IMPL 0x1a71
#define IDS_CLASSLOAD_CONSTRAINT_MISMATCH_ON_PARENT_METHOD_IMPL 0x1a72
#define IDS_CLASSLOAD_CONSTRAINT_MISMATCH_ON_INTERFACE_METHOD_IMPL 0x1a73

#define IDS_EE_NDIRECT_BADNATL_VARARGS_CALLCONV     0x1a75

#define IDS_CLASSLOAD_VARIANCE_IN_METHOD_ARG    0x1a79
#define IDS_CLASSLOAD_VARIANCE_IN_METHOD_RESULT 0x1a7a
#define IDS_CLASSLOAD_VARIANCE_IN_BASE          0x1a7b
#define IDS_CLASSLOAD_VARIANCE_IN_INTERFACE     0x1a7c
#define IDS_CLASSLOAD_VARIANCE_IN_CONSTRAINT    0x1a7d
#define IDS_CLASSLOAD_VARIANCE_CLASS            0x1a7e
#define IDS_CLASSLOAD_BADVARIANCE               0x1a7f

#define IDS_CLASSLOAD_OVERLAPPING_INTERFACES 0x1a80
#define IDS_CLASSLOAD_32BITCLRLOADING64BITASSEMBLY 0x1a81

#define IDS_EE_NEEDS_ASSEMBLY_SPEC              0x1a87

#define IDS_EE_FILELOAD_ERROR_GENERIC           0x1a88

#define IDS_EE_BADMARSHAL_UNSUPPORTED_SIG       0x1a89
#define IDS_EE_BADMARSHAL_STRINGARRAY           0x1a8a
#define IDS_EE_BADMARSHAL_OBJECTARRAY           0x1a8b
#define IDS_EE_BADMARSHAL_DATETIMEARRAY         0x1a8c
#define IDS_EE_BADMARSHAL_DECIMALARRAY          0x1a8d
#define IDS_EE_BADMARSHAL_SAFEHANDLEARRAY       0x1a8f
#define IDS_EE_BADMARSHAL_CRITICALHANDLEARRAY   0x1a90
#define IDS_EE_BADMARSHALFIELD_ERROR_MSG        0x1a92
#define IDS_EE_BADMARSHAL_ERROR_MSG             0x1a93
#define IDS_EE_COM_INVISIBLE_PARENT             0x1a97

#define IDS_EE_REMOTE_COGETCLASSOBJECT_FAILED   0x1a98
#define IDS_EE_CREATEINSTANCE_FAILED            0x1a99
#define IDS_EE_CREATEINSTANCE_LIC_FAILED        0x1a9a

#define IDS_EE_RCW_INVALIDCAST_ITF              0x1a9b
#define IDS_EE_RCW_INVALIDCAST_EVENTITF         0x1a9c
#define IDS_EE_RCW_INVALIDCAST_IENUMERABLE      0x1a9d
#define IDS_EE_RCW_INVALIDCAST_MNGSTDITF        0x1a9e
#define IDS_EE_RCW_INVALIDCAST_COMOBJ_TO_MD     0x1a9f
#define IDS_EE_RCW_INVALIDCAST_TO_NON_COMOBJTYPE 0x1aa0
#define IDS_EE_RCW_INVALIDCAST_MD_TO_MD         0x1aa1

#define IDS_EE_GENERIC                          0x1aa2
#define IDS_EE_BADMARSHAL_GENERICS_RESTRICTION  0x1aa3

#define IDS_EE_THREAD_ABORT                     0x1aa4
#define IDS_EE_THREAD_INTERRUPTED               0x1aa5
#define IDS_EE_OUT_OF_MEMORY                    0x1aa6

#define IDS_EE_ATTEMPT_TO_CREATE_GENERIC_CCW    0x1aa9
#define IDS_EE_ATTEMPT_TO_CREATE_NON_ABSTRACT_CCW    0x1aaa
#define IDS_EE_COMIMPORT_METHOD_NO_INTERFACE    0x1aab
#define IDS_EE_OUT_OF_MEMORY_WITHIN_RANGE       0x1aac
#define IDS_EE_ARRAY_DIMENSIONS_EXCEEDED        0x1aad
#define IDS_EE_OUT_OF_SYNCBLOCKS                0x1aae

#define IDS_CLASSLOAD_MI_CANNOT_OVERRIDE        0x1ab3
#define IDS_CLASSLOAD_COLLECTIBLEFIXEDVTATTR    0x1ab6
#define IDS_CLASSLOAD_EQUIVALENTBADTYPE         0x1ab7
#define IDS_EE_CODEEXECUTION_CONTAINSGENERICVAR 0x1abb
#define IDS_CLASSLOAD_WRONGCPU                  0x1abc

#define IDS_CLASSLOAD_MI_FINAL_IMPL             0x1ac8
#define IDS_CLASSLOAD_AMBIGUOUS_OVERRIDE        0x1ac9
#define IDS_CLASSLOAD_UNSUPPORTED_DISPATCH      0x1aca
#define IDS_CLASSLOAD_METHOD_NOT_IMPLEMENTED    0x1acb

#define BFA_INVALID_TOKEN_TYPE                  0x2001
#define BFA_INVALID_TOKEN                       0x2003
#define BFA_UNABLE_TO_GET_NESTED_PROPS          0x2005
#define BFA_METHOD_TOKEN_OUT_OF_RANGE           0x2006
#define BFA_METHOD_NAME_TOO_LONG                0x2007
#define BFA_METHOD_IN_A_ENUM                    0x2009
#define BFA_METHOD_WITH_NONZERO_RVA             0x200a
#define BFA_ABSTRACT_METHOD_WITH_RVA            0x200b
#define BFA_RUNTIME_METHOD_WITH_RVA             0x200c
#define BFA_INTERNAL_METHOD_WITH_RVA            0x200d
#define BFA_AB_METHOD_IN_AB_CLASS               0x200e
#define BFA_NONVIRT_AB_METHOD                   0x200f
#define BFA_NONAB_NONCCTOR_METHOD_ON_INT        0x2010
#define BFA_VIRTUAL_PINVOKE_METHOD              0x2011
#define BFA_VIRTUAL_STATIC_METHOD               0x2012
#define BFA_VIRTUAL_INSTANCE_CTOR               0x2013
#define BFA_VIRTUAL_NONAB_INT_METHOD            0x2014
#define BFA_NONVIRT_INST_INT_METHOD             0x2015
#define BFA_SYNC_METHOD_IN_VT                   0x2016
#define BFA_NONSTATIC_GLOBAL_METHOD             0x2017
#define BFA_GLOBAL_INST_CTOR                    0x2018
#define BFA_BAD_PLACE_FOR_GENERIC_METHOD        0x2019
#define BFA_GENERIC_METHOD_RUNTIME_IMPL         0x201a
#define BFA_BAD_RUNTIME_IMPL                    0x201b
#define BFA_BAD_FLAGS_ON_DELEGATE               0x201c
#define BFA_UNKNOWN_DELEGATE_METHOD             0x201d
#define BFA_GENERIC_METHODS_INST                0x201e
#define BFA_BAD_FIELD_TOKEN                     0x201f
#define BFA_INVALID_FIELD_ACC_FLAGS             0x2020
#define BFA_FIELD_LITERAL_AND_INIT              0x2021
#define BFA_NONSTATIC_GLOBAL_FIELD              0x2022
#define BFA_INSTANCE_FIELD_IN_INT               0x2023
#define BFA_INSTANCE_FIELD_IN_ENUM              0x2024
#define BFA_NONVIRT_NO_SEARCH                   0x2025
#define BFA_MANAGED_NATIVE_NYI                  0x2027
#define BFA_BAD_IMPL_FLAGS                      0x2028
#define BFA_BAD_UNMANAGED_ENTRY_POINT           0x2029
#define BFA_GENCODE_NOT_BE_VARARG               0x202b
#define BFA_CANNOT_INHERIT_FROM_DELEGATE        0x202c
#define BFA_DELEGATE_CLASS_NOTSEALED            0x202d
#define BFA_ENCLOSING_TYPE_NOT_FOUND            0x202e
#define BFA_ILLEGAL_DELEGATE_METHOD             0x202f
#define BFA_MISSING_DELEGATE_METHOD             0x2030
#define BFA_MULT_TYPE_SAME_NAME                 0x2031
#define BFA_INVALID_METHOD_TOKEN                0x2032
#define BFA_ECALLS_MUST_BE_IN_SYS_MOD           0x2034
#define BFA_CANT_GET_CLASSLAYOUT                0x2035
#define BFA_CALLCONV_NOT_LOCAL_SIG              0x2036
#define BFA_BAD_CLASS_TOKEN                     0x2037
#define BFA_BAD_IL_RANGE                        0x2038
#define BFA_METHODDEF_WO_TYPEDEF_PARENT         0x2039
#define BFA_METHODDEF_PARENT_NO_MEMBERS         0x203a
#define BFA_INVALID_TOKEN_IN_MANIFESTRES        0x203c
#define BFA_EMPTY_ASSEMDEF_NAME                 0x203d
#define BFA_BAD_IL                              0x203e
#define BFA_CLASSLOAD_VALUETYPEMISMATCH         0x203f
#define BFA_METHODDECL_NOT_A_METHODDEF          0x2040
#define BFA_DUPLICATE_DELEGATE_METHOD           0x2041
#define BFA_ECALLS_MUST_HAVE_ZERO_RVA           0x2042
#define BFA_METADATA_CORRUPT                    0x2043
#define BFA_BAD_SIGNATURE                       0x2044
#define BFA_TYPEREG_NAME_TOO_LONG               0x2045
#define BFA_BAD_TYPEREF_TOKEN                   0x2046
#define BFA_BAD_CLASS_INT_CA_FORMAT             0x2048
#define BFA_BAD_COMPLUS_SIG                     0x2049
#define BFA_BAD_ELEM_IN_SIZEOF                  0x204b
#define BFA_IJW_IN_COLLECTIBLE_ALC              0x204c
#define BFA_INVALID_UNSAFEACCESSOR              0x204d

#define IDS_CLASSLOAD_INTERFACE_NO_ACCESS       0x204f

#define BFA_BAD_CA_HEADER                       0x2050
#define BFA_BAD_STRING_TOKEN_RANGE              0x2053
#define BFA_FIXUP_WRONG_PLATFORM                0x2054
#define BFA_UNEXPECTED_GENERIC_TOKENTYPE        0x2055
#define BFA_MDARRAY_BADRANK                     0x2056
#define BFA_SDARRAY_BADRANK                     0x2057
#define BFA_BAD_PACKING_SIZE                    0x2058
#define BFA_UNEXPECTED_ARRAY_TYPE               0x2059
#define BFA_BAD_VISIBILITY                      0x205a
#define BFA_FAMILY_ON_GLOBAL                    0x205b
#define BFA_NOT_AN_ARRAY                        0x205d
#define BFA_EXPECTED_METHODDEF_OR_MEMBERREF     0x205e

#define IDS_CLASSLOAD_BAD_METHOD_COUNT          0x2062
#define IDS_CLASSLOAD_BAD_FIELD_COUNT           0x2063
#define IDS_CLASSLOAD_MUST_BE_BYVAL             0x2064
#define IDS_CLASSLOAD_BAD_VARIANCE_SIG          0x2065
#define IDS_CLASSLOAD_VARIANCE_IN_DELEGATE      0x2066

#define BFA_UNEXPECTED_FIELD_SIGNATURE          0x2068
#define BFA_UNEXPECTED_TOKEN_AFTER_CLASSVALTYPE 0x2069
#define BFA_FNPTR_CANNOT_BE_A_FIELD             0x206a
#define BFA_FNPTR_CANNOT_BE_GENERIC             0x206b
#define BFA_UNEXPECTED_TOKEN_AFTER_GENINST      0x206c
#define BFA_TYPEDBYREFCANNOTHAVEBYREF           0x206e

#define IDS_CLASSLOAD_MI_BAD_SIG                0x2070

#define IDS_EE_TOOMANYFIELDS                    0x2072

#define IDS_EE_NDIRECT_GETPROCADDRESS_NONAME    0x2073
#define IDS_EE_CLASS_CONSTRAINTS_VIOLATION      0x2076
#define IDS_EE_METHOD_CONSTRAINTS_VIOLATION     0x2077
#define IDS_CLASSLOAD_TOO_MANY_METHODS          0x2078
#define IDS_CLASSLOAD_ENUM_EXTRA_GENERIC_TYPE_PARAM 0x2079

#define IDS_CLASSLOAD_GENERICTYPE_RECURSIVE     0x207D
#define IDS_EE_JIT_COMPILER_ERROR               0x207F

#define IDS_ER_APPLICATION                      0x2082
#define IDS_ER_UNKNOWN                          0x2083
#define IDS_ER_FRAMEWORK_VERSION                0x2084
#define IDS_ER_UNHANDLEDEXCEPTION               0x2085
#define IDS_ER_UNHANDLEDEXCEPTIONMSG            0x2086
#define IDS_ER_MANAGEDFAILFAST                  0x2087
#define IDS_ER_MANAGEDFAILFASTMSG               0x2088
#define IDS_ER_UNMANAGEDFAILFAST                0x2089
#define IDS_ER_STACK_OVERFLOW                   0x208a
#define IDS_ER_STACK                            0x208b
#define IDS_ER_WORDAT                           0x208c


#define IDS_ER_MESSAGE_TRUNCATE                 0x208f

#define IDS_EE_OBJECT_TO_VARIANT_NOT_SUPPORTED  0x2090
#define IDS_EE_OBJECT_TO_ITF_NOT_SUPPORTED      0x2091

#define IDS_EE_BADMARSHALFIELD_DECIMAL          0x2099

#define IDS_EE_CANNOTCASTSAME                   0x209a

// For ForwardInteropStubAttribute
#ifdef FEATURE_COMINTEROP
#define IDS_EE_INTEROP_STUB_CA_MUST_BE_WITHIN_SAME_ASSEMBLY         0x2107
#define IDS_EE_INTEROP_STUB_CA_STUB_CLASS_MUST_NOT_BE_GENERIC       0x2108
#define IDS_EE_INTEROP_STUB_CA_STUB_CLASS_MUST_NOT_BE_INTERFACE     0x2109
#define IDS_EE_INTEROP_STUB_CA_STUB_METHOD_MISSING                  0x2110
#define IDS_EE_INTEROP_STUB_CA_NO_ACCESS_TO_STUB_METHOD             0x2111
#endif

#define BFA_REFERENCE_ASSEMBLY                  0x2113

#define IDS_E_FIELDACCESS                       0x2114
#define IDS_E_METHODACCESS                      0x2115
#define IDS_E_TYPEACCESS                        0x2116

// Profiler error messages for event log
#define IDS_E_PROF_NO_CLSID                     0x2500
#define IDS_E_PROF_INTERNAL_INIT                0x2501
#define IDS_E_PROF_BAD_CLSID                    0x2502
#define IDS_E_PROF_NO_CALLBACK_IFACE            0x2503
#define IDS_E_PROF_CCI_FAILED                   0x2504
#define IDS_E_PROF_INIT_CALLBACK_FAILED         0x2505
#define IDS_PROF_SUPPLEMENTARY_INFO             0x2506
#define IDS_PROF_LOAD_COMPLETE                  0x2507
#define IDS_E_PROF_BAD_PATH                     0x2508
#define IDS_E_PROF_NOTIFICATION_DISABLED        0x2509
#define IDS_PROF_ALREADY_LOADED                 0x250A
#define IDS_E_PROF_NOTIFICATION_LIMIT_EXCEEDED  0x250B
#define IDS_E_PROF_NOT_ATTACHABLE               0x250E
#define IDS_E_PROF_UNHANDLED_EXCEPTION_ON_LOAD  0x250F
#define IDS_PROF_ATTACH_REQUEST_RECEIVED        0x2512
#define IDS_PROF_DETACH_INITIATED               0x2513
#define IDS_PROF_DETACH_COMPLETE                0x2514
#define IDS_PROF_DETACH_THREAD_ERROR            0x2515
#define IDS_PROF_CANCEL_ACTIVATION              0x2516
#define IDS_PROF_V2PROFILER_DISABLED            0x2517
#define IDS_PROF_V2PROFILER_ENABLED             0x2518
#define IDS_PROF_PROFILER_DISABLED              0x251A

#define IDS_ER_CODECONTRACT_FAILED              0x251B
#define IDS_ER_CODECONTRACT_DETAILMSG           0x251C

#define IDS_E_PROF_TIMEOUT_WAITING_FOR_CONCURRENT_GC    0x251D

#define IDS_EE_CANNOTCAST_NOMARSHAL             0x2629
#if defined(FEATURE_COMINTEROP) || defined(FEATURE_COMWRAPPERS)
#define IDS_EE_NATIVE_COM_WEAKREF_BAD_TYPE           0x262e
#endif // FEATURE_COMINTEROP || FEATURE_COMWRAPPERS

#define IDS_HOST_ASSEMBLY_RESOLVER_ASSEMBLY_ALREADY_LOADED_IN_CONTEXT                  0x2636
#define IDS_HOST_ASSEMBLY_RESOLVER_DYNAMICALLY_EMITTED_ASSEMBLIES_UNSUPPORTED          0x2637

#define IDS_NATIVE_IMAGE_CANNOT_BE_LOADED_MULTIPLE_TIMES                               0x263a

#define IDS_CLASSLOAD_BYREF_OR_BYREFLIKE_STATICFIELD    0x263b
#define IDS_CLASSLOAD_BYREF_OR_BYREFLIKE_INSTANCEFIELD  0x263c
#define IDS_EE_NDIRECT_LOADLIB_LINUX               0x263e
#define IDS_EE_NDIRECT_LOADLIB_MAC                 0x263f
#define IDS_EE_NDIRECT_GETPROCADDRESS_UNIX         0x2640
#define IDS_EE_ERROR_COM                           0x2641

#define IDS_EE_CANNOT_SET_INITONLY_STATIC_FIELD    0x2643

#define IDS_EE_NDIRECT_GETPROCADDR_WIN_DLL         0x2644
#define IDS_EE_NDIRECT_GETPROCADDR_UNIX_SO         0x2645
#define IDS_EE_BADMARSHAL_STRING_OUT               0x2646
#define IDS_EE_BADMARSHAL_COPYCTORRESTRICTION      0x2647
#define IDS_EE_BADMARSHAL_DELEGATE_TLB_INTERFACE   0x2649
#define IDS_EE_THREAD_APARTMENT_NOT_SUPPORTED      0x264A
#define IDS_EE_NO_IINSPECTABLE                     0x264B
#define IDS_EE_BADMARSHAL_MARSHAL_DISABLED         0x264C
#define IDS_EE_NDIRECT_DISABLEDMARSHAL_SETLASTERROR 0x264D
#define IDS_EE_NDIRECT_DISABLEDMARSHAL_LCID        0x264E
#define IDS_EE_NDIRECT_DISABLEDMARSHAL_PRESERVESIG 0x264F
#define IDS_EE_NDIRECT_DISABLEDMARSHAL_VARARGS     0x2650
