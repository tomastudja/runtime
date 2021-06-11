// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include "../../AnyOS/entrypoints.h"

// Include System.Security.Cryptography.Native headers
#include "openssl.h"
#include "pal_asn1.h"
#include "pal_bignum.h"
#include "pal_bio.h"
#include "pal_dsa.h"
#include "pal_ecc_import_export.h"
#include "pal_ecdsa.h"
#include "pal_eckey.h"
#include "pal_err.h"
#include "pal_evp.h"
#include "pal_evp_cipher.h"
#include "pal_evp_pkey.h"
#include "pal_evp_pkey_dsa.h"
#include "pal_evp_pkey_ecdh.h"
#include "pal_evp_pkey_eckey.h"
#include "pal_evp_pkey_rsa.h"
#include "pal_hmac.h"
#include "pal_ocsp.h"
#include "pal_pkcs7.h"
#include "pal_rsa.h"
#include "pal_ssl.h"
#include "pal_x509.h"
#include "pal_x509ext.h"
#include "pal_x509_name.h"
#include "pal_x509_root.h"


static const Entry s_cryptoNative[] =
{
    DllImportEntry(CryptoNative_Asn1BitStringFree)
    DllImportEntry(CryptoNative_Asn1ObjectFree)
    DllImportEntry(CryptoNative_Asn1OctetStringFree)
    DllImportEntry(CryptoNative_Asn1OctetStringNew)
    DllImportEntry(CryptoNative_Asn1OctetStringSet)
    DllImportEntry(CryptoNative_BigNumDestroy)
    DllImportEntry(CryptoNative_BigNumFromBinary)
    DllImportEntry(CryptoNative_BigNumToBinary)
    DllImportEntry(CryptoNative_BioCtrlPending)
    DllImportEntry(CryptoNative_BioDestroy)
    DllImportEntry(CryptoNative_BioGets)
    DllImportEntry(CryptoNative_BioNewFile)
    DllImportEntry(CryptoNative_BioRead)
    DllImportEntry(CryptoNative_BioSeek)
    DllImportEntry(CryptoNative_BioTell)
    DllImportEntry(CryptoNative_BioWrite)
    DllImportEntry(CryptoNative_CheckX509Hostname)
    DllImportEntry(CryptoNative_CheckX509IpAddress)
    DllImportEntry(CryptoNative_CreateMemoryBio)
    DllImportEntry(CryptoNative_D2IPkcs7Bio)
    DllImportEntry(CryptoNative_DecodeAsn1BitString)
    DllImportEntry(CryptoNative_DecodeExtendedKeyUsage)
    DllImportEntry(CryptoNative_DecodeOcspResponse)
    DllImportEntry(CryptoNative_DecodePkcs7)
    DllImportEntry(CryptoNative_DecodeRsaPublicKey)
    DllImportEntry(CryptoNative_DecodeX509)
    DllImportEntry(CryptoNative_DecodeX509BasicConstraints2Extension)
    DllImportEntry(CryptoNative_DecodeX509Crl)
    DllImportEntry(CryptoNative_DsaDestroy)
    DllImportEntry(CryptoNative_DsaGenerateKey)
    DllImportEntry(CryptoNative_DsaKeyCreateByExplicitParameters)
    DllImportEntry(CryptoNative_DsaSign)
    DllImportEntry(CryptoNative_DsaSizeP)
    DllImportEntry(CryptoNative_DsaSizeQ)
    DllImportEntry(CryptoNative_DsaSizeSignature)
    DllImportEntry(CryptoNative_DsaUpRef)
    DllImportEntry(CryptoNative_DsaVerify)
    DllImportEntry(CryptoNative_EcDsaSign)
    DllImportEntry(CryptoNative_EcDsaSize)
    DllImportEntry(CryptoNative_EcDsaVerify)
    DllImportEntry(CryptoNative_EcKeyCreateByExplicitParameters)
    DllImportEntry(CryptoNative_EcKeyCreateByKeyParameters)
    DllImportEntry(CryptoNative_EcKeyCreateByOid)
    DllImportEntry(CryptoNative_EcKeyDestroy)
    DllImportEntry(CryptoNative_EcKeyGenerateKey)
    DllImportEntry(CryptoNative_EcKeyGetCurveName2)
    DllImportEntry(CryptoNative_EcKeyGetSize)
    DllImportEntry(CryptoNative_EcKeyUpRef)
    DllImportEntry(CryptoNative_EncodeAsn1Integer)
    DllImportEntry(CryptoNative_EncodeOcspRequest)
    DllImportEntry(CryptoNative_EncodePkcs7)
    DllImportEntry(CryptoNative_EncodeX509)
    DllImportEntry(CryptoNative_EncodeX509SubjectPublicKeyInfo)
    DllImportEntry(CryptoNative_ErrClearError)
    DllImportEntry(CryptoNative_ErrErrorStringN)
    DllImportEntry(CryptoNative_ErrGetErrorAlloc)
    DllImportEntry(CryptoNative_ErrPeekError)
    DllImportEntry(CryptoNative_ErrPeekLastError)
    DllImportEntry(CryptoNative_ErrReasonErrorString)
    DllImportEntry(CryptoNative_EvpAes128Cbc)
    DllImportEntry(CryptoNative_EvpAes128Ccm)
    DllImportEntry(CryptoNative_EvpAes128Cfb128)
    DllImportEntry(CryptoNative_EvpAes128Cfb8)
    DllImportEntry(CryptoNative_EvpAes128Ecb)
    DllImportEntry(CryptoNative_EvpAes128Gcm)
    DllImportEntry(CryptoNative_EvpAes192Cbc)
    DllImportEntry(CryptoNative_EvpAes192Ccm)
    DllImportEntry(CryptoNative_EvpAes192Cfb128)
    DllImportEntry(CryptoNative_EvpAes192Cfb8)
    DllImportEntry(CryptoNative_EvpAes192Ecb)
    DllImportEntry(CryptoNative_EvpAes192Gcm)
    DllImportEntry(CryptoNative_EvpAes256Cbc)
    DllImportEntry(CryptoNative_EvpAes256Ccm)
    DllImportEntry(CryptoNative_EvpAes256Cfb128)
    DllImportEntry(CryptoNative_EvpAes256Cfb8)
    DllImportEntry(CryptoNative_EvpAes256Ecb)
    DllImportEntry(CryptoNative_EvpAes256Gcm)
    DllImportEntry(CryptoNative_EvpChaCha20Poly1305)
    DllImportEntry(CryptoNative_EvpCipherCreate2)
    DllImportEntry(CryptoNative_EvpCipherCreatePartial)
    DllImportEntry(CryptoNative_EvpCipherCtxSetPadding)
    DllImportEntry(CryptoNative_EvpCipherDestroy)
    DllImportEntry(CryptoNative_EvpCipherFinalEx)
    DllImportEntry(CryptoNative_EvpCipherGetCcmTag)
    DllImportEntry(CryptoNative_EvpCipherGetGcmTag)
    DllImportEntry(CryptoNative_EvpCipherGetAeadTag)
    DllImportEntry(CryptoNative_EvpCipherSetAeadTag)
    DllImportEntry(CryptoNative_EvpCipherReset)
    DllImportEntry(CryptoNative_EvpCipherSetCcmNonceLength)
    DllImportEntry(CryptoNative_EvpCipherSetCcmTag)
    DllImportEntry(CryptoNative_EvpCipherSetGcmNonceLength)
    DllImportEntry(CryptoNative_EvpCipherSetGcmTag)
    DllImportEntry(CryptoNative_EvpCipherSetKeyAndIV)
    DllImportEntry(CryptoNative_EvpCipherUpdate)
    DllImportEntry(CryptoNative_EvpDes3Cbc)
    DllImportEntry(CryptoNative_EvpDes3Cfb64)
    DllImportEntry(CryptoNative_EvpDes3Cfb8)
    DllImportEntry(CryptoNative_EvpDes3Ecb)
    DllImportEntry(CryptoNative_EvpDesCbc)
    DllImportEntry(CryptoNative_EvpDesCfb8)
    DllImportEntry(CryptoNative_EvpDesEcb)
    DllImportEntry(CryptoNative_EvpDigestCurrent)
    DllImportEntry(CryptoNative_EvpDigestFinalEx)
    DllImportEntry(CryptoNative_EvpDigestOneShot)
    DllImportEntry(CryptoNative_EvpDigestReset)
    DllImportEntry(CryptoNative_EvpDigestUpdate)
    DllImportEntry(CryptoNative_EvpMd5)
    DllImportEntry(CryptoNative_EvpMdCtxCreate)
    DllImportEntry(CryptoNative_EvpMdCtxDestroy)
    DllImportEntry(CryptoNative_EvpMdSize)
    DllImportEntry(CryptoNative_EvpPkeyCreate)
    DllImportEntry(CryptoNative_EvpPKeyCtxCreate)
    DllImportEntry(CryptoNative_EvpPKeyCtxDestroy)
    DllImportEntry(CryptoNative_EvpPKeyDeriveSecretAgreement)
    DllImportEntry(CryptoNative_EvpPkeyDestroy)
    DllImportEntry(CryptoNative_EvpPkeyGetDsa)
    DllImportEntry(CryptoNative_EvpPkeyGetEcKey)
    DllImportEntry(CryptoNative_EvpPkeyGetRsa)
    DllImportEntry(CryptoNative_EvpPkeySetDsa)
    DllImportEntry(CryptoNative_EvpPkeySetEcKey)
    DllImportEntry(CryptoNative_EvpPkeySetRsa)
    DllImportEntry(CryptoNative_EvpPKeySize)
    DllImportEntry(CryptoNative_EvpRC2Cbc)
    DllImportEntry(CryptoNative_EvpRC2Ecb)
    DllImportEntry(CryptoNative_EvpSha1)
    DllImportEntry(CryptoNative_EvpSha256)
    DllImportEntry(CryptoNative_EvpSha384)
    DllImportEntry(CryptoNative_EvpSha512)
    DllImportEntry(CryptoNative_ExtendedKeyUsageDestory)
    DllImportEntry(CryptoNative_GetAsn1IntegerDerSize)
    DllImportEntry(CryptoNative_GetAsn1StringBytes)
    DllImportEntry(CryptoNative_GetBigNumBytes)
    DllImportEntry(CryptoNative_GetDsaParameters)
    DllImportEntry(CryptoNative_GetECCurveParameters)
    DllImportEntry(CryptoNative_GetECKeyParameters)
    DllImportEntry(CryptoNative_GetMaxMdSize)
    DllImportEntry(CryptoNative_GetMemoryBioSize)
    DllImportEntry(CryptoNative_GetObjectDefinitionByName)
    DllImportEntry(CryptoNative_GetOcspRequestDerSize)
    DllImportEntry(CryptoNative_GetPkcs7Certificates)
    DllImportEntry(CryptoNative_GetPkcs7DerSize)
    DllImportEntry(CryptoNative_GetRandomBytes)
    DllImportEntry(CryptoNative_GetRsaParameters)
    DllImportEntry(CryptoNative_GetX509CrlNextUpdate)
    DllImportEntry(CryptoNative_GetX509DerSize)
    DllImportEntry(CryptoNative_GetX509EkuField)
    DllImportEntry(CryptoNative_GetX509EkuFieldCount)
    DllImportEntry(CryptoNative_GetX509EvpPublicKey)
    DllImportEntry(CryptoNative_GetX509NameInfo)
    DllImportEntry(CryptoNative_GetX509NameRawBytes)
    DllImportEntry(CryptoNative_GetX509NameStackField)
    DllImportEntry(CryptoNative_GetX509NameStackFieldCount)
    DllImportEntry(CryptoNative_GetX509NotAfter)
    DllImportEntry(CryptoNative_GetX509NotBefore)
    DllImportEntry(CryptoNative_GetX509PublicKeyAlgorithm)
    DllImportEntry(CryptoNative_GetX509PublicKeyBytes)
    DllImportEntry(CryptoNative_GetX509PublicKeyParameterBytes)
    DllImportEntry(CryptoNative_GetX509RootStoreFile)
    DllImportEntry(CryptoNative_GetX509RootStorePath)
    DllImportEntry(CryptoNative_GetX509SignatureAlgorithm)
    DllImportEntry(CryptoNative_GetX509StackField)
    DllImportEntry(CryptoNative_GetX509StackFieldCount)
    DllImportEntry(CryptoNative_GetX509SubjectPublicKeyInfoDerSize)
    DllImportEntry(CryptoNative_GetX509Thumbprint)
    DllImportEntry(CryptoNative_GetX509Version)
    DllImportEntry(CryptoNative_HmacCreate)
    DllImportEntry(CryptoNative_HmacCurrent)
    DllImportEntry(CryptoNative_HmacDestroy)
    DllImportEntry(CryptoNative_HmacFinal)
    DllImportEntry(CryptoNative_HmacOneShot)
    DllImportEntry(CryptoNative_HmacReset)
    DllImportEntry(CryptoNative_HmacUpdate)
    DllImportEntry(CryptoNative_LookupFriendlyNameByOid)
    DllImportEntry(CryptoNative_NewX509Stack)
    DllImportEntry(CryptoNative_ObjNid2Obj)
    DllImportEntry(CryptoNative_ObjObj2Txt)
    DllImportEntry(CryptoNative_ObjSn2Nid)
    DllImportEntry(CryptoNative_ObjTxt2Nid)
    DllImportEntry(CryptoNative_ObjTxt2Obj)
    DllImportEntry(CryptoNative_OcspRequestDestroy)
    DllImportEntry(CryptoNative_OcspResponseDestroy)
    DllImportEntry(CryptoNative_OpenSslAvailable)
    DllImportEntry(CryptoNative_Pbkdf2)
    DllImportEntry(CryptoNative_PemReadBioPkcs7)
    DllImportEntry(CryptoNative_PemReadBioX509Crl)
    DllImportEntry(CryptoNative_PemReadX509FromBio)
    DllImportEntry(CryptoNative_PemReadX509FromBioAux)
    DllImportEntry(CryptoNative_PemWriteBioX509Crl)
    DllImportEntry(CryptoNative_Pkcs7CreateCertificateCollection)
    DllImportEntry(CryptoNative_Pkcs7Destroy)
    DllImportEntry(CryptoNative_PushX509StackField)
    DllImportEntry(CryptoNative_ReadX509AsDerFromBio)
    DllImportEntry(CryptoNative_RecursiveFreeX509Stack)
    DllImportEntry(CryptoNative_RegisterLegacyAlgorithms)
    DllImportEntry(CryptoNative_RsaCreate)
    DllImportEntry(CryptoNative_RsaDecrypt)
    DllImportEntry(CryptoNative_RsaDestroy)
    DllImportEntry(CryptoNative_RsaEncrypt)
    DllImportEntry(CryptoNative_RsaGenerateKey)
    DllImportEntry(CryptoNative_RsaSignHash)
    DllImportEntry(CryptoNative_RsaSize)
    DllImportEntry(CryptoNative_RsaUpRef)
    DllImportEntry(CryptoNative_RsaVerifyHash)
    DllImportEntry(CryptoNative_SetRsaParameters)
    DllImportEntry(CryptoNative_UpRefEvpPkey)
    DllImportEntry(CryptoNative_X509ChainBuildOcspRequest)
    DllImportEntry(CryptoNative_X509ChainGetCachedOcspStatus)
    DllImportEntry(CryptoNative_X509ChainNew)
    DllImportEntry(CryptoNative_X509ChainVerifyOcsp)
    DllImportEntry(CryptoNative_X509CheckPurpose)
    DllImportEntry(CryptoNative_X509CrlDestroy)
    DllImportEntry(CryptoNative_X509Destroy)
    DllImportEntry(CryptoNative_X509ExtensionCreateByObj)
    DllImportEntry(CryptoNative_X509ExtensionDestroy)
    DllImportEntry(CryptoNative_X509ExtensionGetCritical)
    DllImportEntry(CryptoNative_X509ExtensionGetData)
    DllImportEntry(CryptoNative_X509ExtensionGetOid)
    DllImportEntry(CryptoNative_X509FindExtensionData)
    DllImportEntry(CryptoNative_X509GetExt)
    DllImportEntry(CryptoNative_X509GetExtCount)
    DllImportEntry(CryptoNative_X509GetIssuerName)
    DllImportEntry(CryptoNative_X509GetSerialNumber)
    DllImportEntry(CryptoNative_X509GetSubjectName)
    DllImportEntry(CryptoNative_X509IssuerNameHash)
    DllImportEntry(CryptoNative_X509StackAddDirectoryStore)
    DllImportEntry(CryptoNative_X509StackAddMultiple)
    DllImportEntry(CryptoNative_X509StoreAddCrl)
    DllImportEntry(CryptoNative_X509StoreCtxCommitToChain)
    DllImportEntry(CryptoNative_X509StoreCtxCreate)
    DllImportEntry(CryptoNative_X509StoreCtxDestroy)
    DllImportEntry(CryptoNative_X509StoreCtxGetChain)
    DllImportEntry(CryptoNative_X509StoreCtxGetCurrentCert)
    DllImportEntry(CryptoNative_X509StoreCtxGetError)
    DllImportEntry(CryptoNative_X509StoreCtxGetErrorDepth)
    DllImportEntry(CryptoNative_X509StoreCtxGetSharedUntrusted)
    DllImportEntry(CryptoNative_X509StoreCtxInit)
    DllImportEntry(CryptoNative_X509StoreCtxRebuildChain)
    DllImportEntry(CryptoNative_X509StoreCtxReset)
    DllImportEntry(CryptoNative_X509StoreCtxResetForSignatureError)
    DllImportEntry(CryptoNative_X509StoreCtxSetVerifyCallback)
    DllImportEntry(CryptoNative_X509StoreDestory)
    DllImportEntry(CryptoNative_X509StoreSetRevocationFlag)
    DllImportEntry(CryptoNative_X509StoreSetVerifyTime)
    DllImportEntry(CryptoNative_X509UpRef)
    DllImportEntry(CryptoNative_X509V3ExtPrint)
    DllImportEntry(CryptoNative_X509VerifyCert)
    DllImportEntry(CryptoNative_X509VerifyCertErrorString)
    DllImportEntry(CryptoNative_EnsureOpenSslInitialized)
    DllImportEntry(CryptoNative_OpenSslGetProtocolSupport)
    DllImportEntry(CryptoNative_OpenSslVersionNumber)
    DllImportEntry(CryptoNative_BioWrite)
    DllImportEntry(CryptoNative_EnsureLibSslInitialized)
    DllImportEntry(CryptoNative_GetOpenSslCipherSuiteName)
    DllImportEntry(CryptoNative_IsSslRenegotiatePending)
    DllImportEntry(CryptoNative_IsSslStateOK)
    DllImportEntry(CryptoNative_SetCiphers)
    DllImportEntry(CryptoNative_SetEncryptionPolicy)
    DllImportEntry(CryptoNative_SetProtocolOptions)
    DllImportEntry(CryptoNative_SslAddExtraChainCert)
    DllImportEntry(CryptoNative_SslCreate)
    DllImportEntry(CryptoNative_SslCtxCheckPrivateKey)
    DllImportEntry(CryptoNative_SslCtxCreate)
    DllImportEntry(CryptoNative_SslCtxDestroy)
    DllImportEntry(CryptoNative_SslCtxSetAlpnProtos)
    DllImportEntry(CryptoNative_SslCtxSetAlpnSelectCb)
    DllImportEntry(CryptoNative_SslCtxSetQuietShutdown)
    DllImportEntry(CryptoNative_SslCtxSetVerify)
    DllImportEntry(CryptoNative_SslCtxUseCertificate)
    DllImportEntry(CryptoNative_SslCtxUsePrivateKey)
    DllImportEntry(CryptoNative_SslDestroy)
    DllImportEntry(CryptoNative_SslDoHandshake)
    DllImportEntry(CryptoNative_SslGetClientCAList)
    DllImportEntry(CryptoNative_SslGetCurrentCipherId)
    DllImportEntry(CryptoNative_SslGetError)
    DllImportEntry(CryptoNative_SslGetFinished)
    DllImportEntry(CryptoNative_SslGetPeerCertChain)
    DllImportEntry(CryptoNative_SslGetPeerCertificate)
    DllImportEntry(CryptoNative_SslGetPeerFinished)
    DllImportEntry(CryptoNative_SslGetVersion)
    DllImportEntry(CryptoNative_SslRead)
    DllImportEntry(CryptoNative_SslSessionReused)
    DllImportEntry(CryptoNative_SslSetAcceptState)
    DllImportEntry(CryptoNative_SslSetBio)
    DllImportEntry(CryptoNative_SslSetConnectState)
    DllImportEntry(CryptoNative_SslSetQuietShutdown)
    DllImportEntry(CryptoNative_SslSetTlsExtHostName)
    DllImportEntry(CryptoNative_SslShutdown)
    DllImportEntry(CryptoNative_SslV2_3Method)
    DllImportEntry(CryptoNative_SslWrite)
    DllImportEntry(CryptoNative_X509StoreCtxGetTargetCert)
    DllImportEntry(CryptoNative_Tls13Supported)
    DllImportEntry(CryptoNative_X509Duplicate)
    DllImportEntry(CryptoNative_SslGet0AlpnSelected)
    DllImportEntry(CryptoNative_Asn1StringFree)
};

EXTERN_C const void* CryptoResolveDllImport(const char* name);

EXTERN_C const void* CryptoResolveDllImport(const char* name)
{
    return ResolveDllImport(s_cryptoNative, lengthof(s_cryptoNative), name);
}
