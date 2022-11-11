// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics;

namespace System.Security.Cryptography
{
    public sealed partial class CngKey : IDisposable
    {
        /// <summary>
        /// Does the key represent a named curve (Win10+)
        /// </summary>
        /// <returns></returns>
        internal bool IsECNamedCurve()
        {
            return IsECNamedCurve(Algorithm.Algorithm);
        }

        internal static bool IsECNamedCurve(string algorithm)
        {
            return (algorithm == CngAlgorithm.ECDiffieHellman.Algorithm ||
                algorithm == CngAlgorithm.ECDsa.Algorithm);
        }

        internal string? GetCurveName(out string? oidValue)
        {
            if (IsECNamedCurve())
            {
                string? curveName = _keyHandle.GetPropertyAsString(KeyPropertyName.ECCCurveName, CngPropertyOptions.None);
                oidValue = curveName is null ? null : OidLookup.ToOid(curveName, OidGroup.PublicKeyAlgorithm, fallBackToAllGroups: false);
                return curveName;
            }

            // Use hard-coded values (for use with pre-Win10 APIs)
            return GetECSpecificCurveName(out oidValue);
        }

        private string GetECSpecificCurveName(out string oidValue)
        {
            string algorithm = Algorithm.Algorithm;

            if (algorithm == CngAlgorithm.ECDiffieHellmanP256.Algorithm ||
                algorithm == CngAlgorithm.ECDsaP256.Algorithm)
            {
                oidValue = Oids.secp256r1;
                return "nistP256";
            }

            if (algorithm == CngAlgorithm.ECDiffieHellmanP384.Algorithm ||
                algorithm == CngAlgorithm.ECDsaP384.Algorithm)
            {
                oidValue = Oids.secp384r1;
                return "nistP384";
            }

            if (algorithm == CngAlgorithm.ECDiffieHellmanP521.Algorithm ||
                algorithm == CngAlgorithm.ECDsaP521.Algorithm)
            {
                oidValue = Oids.secp521r1;
                return "nistP521";
            }

            Debug.Fail($"Unknown curve {algorithm}");
            throw new PlatformNotSupportedException(SR.Format(SR.Cryptography_CurveNotSupported, algorithm));
        }

        /// <summary>
        ///     Return a CngProperty representing a named curve.
        /// </summary>
        internal static CngProperty GetPropertyFromNamedCurve(ECCurve curve)
        {
            string curveName = curve.Oid.FriendlyName!;
            unsafe
            {
                byte[] curveNameBytes = new byte[(curveName.Length + 1) * sizeof(char)]; // +1 to add trailing null
                System.Text.Encoding.Unicode.GetBytes(curveName, 0, curveName.Length, curveNameBytes, 0);
                return new CngProperty(KeyPropertyName.ECCCurveName, curveNameBytes, CngPropertyOptions.None);
            }
        }

        /// <summary>
        /// Map a curve name to algorithm. This enables curves that worked pre-Win10
        /// to work with newer APIs for import and export.
        /// </summary>
        internal static CngAlgorithm EcdsaCurveNameToAlgorithm(ReadOnlySpan<char> name)
        {
            const int MaxCurveNameLength = 16;
            Span<char> nameLower = stackalloc char[MaxCurveNameLength];
            int written = name.ToLowerInvariant(nameLower);

            // Either it is empty or too big for the buffer, and the buffer is large enough to hold all mapped
            // curve names, so return the generic algorithm.
            if (written < 1)
            {
                return CngAlgorithm.ECDsa;
            }

            return nameLower.Slice(0, written) switch
            {
                "nistp256" or "ecdsa_p256" => CngAlgorithm.ECDsaP256,
                "nistp384" or "ecdsa_p384" => CngAlgorithm.ECDsaP384,
                "nistp521" or "ecdsa_p521" => CngAlgorithm.ECDsaP521,
                _ => CngAlgorithm.ECDsa, // All other curves are new in Win10 so use generic algorithm
            };
        }

        /// <summary>
        /// Map a curve name to algorithm. This enables curves that worked pre-Win10
        /// to work with newer APIs for import and export.
        /// </summary>
        internal static CngAlgorithm EcdhCurveNameToAlgorithm(ReadOnlySpan<char> name)
        {
            const int MaxCurveNameLength = 16;
            Span<char> nameLower = stackalloc char[MaxCurveNameLength];
            int written = name.ToLowerInvariant(nameLower);

            // Either it is empty or too big for the buffer, and the buffer is large enough to hold all mapped
            // curve names, so return the generic algorithm.
            if (written < 1)
            {
                return CngAlgorithm.ECDiffieHellman;
            }

            return nameLower.Slice(0, written) switch
            {
                "nistp256" or "ecdsa_p256" or "ecdh_p256" => CngAlgorithm.ECDiffieHellmanP256,
                "nistp384" or "ecdsa_p384" or "ecdh_p384" => CngAlgorithm.ECDiffieHellmanP384,
                "nistp521" or "ecdsa_p521" or "ecdh_p521" => CngAlgorithm.ECDiffieHellmanP521,
                _ => CngAlgorithm.ECDiffieHellman, // All other curves are new in Win10 so use generic algorithm
            };
        }

        internal static CngKey Create(ECCurve curve, Func<string?, CngAlgorithm> algorithmResolver)
        {
            System.Diagnostics.Debug.Assert(algorithmResolver != null);

            curve.Validate();

            CngKeyCreationParameters creationParameters = new CngKeyCreationParameters
            {
                ExportPolicy = CngExportPolicies.AllowPlaintextExport,
            };

            CngAlgorithm alg;

            if (curve.IsNamed)
            {
                if (string.IsNullOrEmpty(curve.Oid.FriendlyName))
                    throw new PlatformNotSupportedException(SR.Format(SR.Cryptography_InvalidCurveOid, curve.Oid.Value));

                // Map curve name to algorithm to support pre-Win10 curves
                alg = algorithmResolver(curve.Oid.FriendlyName);

                if (CngKey.IsECNamedCurve(alg.Algorithm))
                {
                    creationParameters.Parameters.Add(GetPropertyFromNamedCurve(curve));
                }
                else
                {
                    if (alg == CngAlgorithm.ECDsaP256 || alg == CngAlgorithm.ECDiffieHellmanP256 ||
                        alg == CngAlgorithm.ECDsaP384 || alg == CngAlgorithm.ECDiffieHellmanP384 ||
                        alg == CngAlgorithm.ECDsaP521 || alg == CngAlgorithm.ECDiffieHellmanP521)
                    {
                        // No parameters required, the algorithm ID has everything built-in.
                    }
                    else
                    {
                        Debug.Fail($"Unknown algorithm {alg}");
                        throw new ArgumentException(SR.Cryptography_InvalidKeySize);
                    }
                }
            }
            else if (curve.IsPrime)
            {
                byte[] parametersBlob = ECCng.GetPrimeCurveParameterBlob(ref curve);

                CngProperty prop = new CngProperty(
                    KeyPropertyName.ECCParameters,
                    parametersBlob,
                    CngPropertyOptions.None);

                creationParameters.Parameters.Add(prop);
                alg = algorithmResolver(null);
            }
            else
            {
                throw new PlatformNotSupportedException(SR.Format(SR.Cryptography_CurveNotSupported, curve.CurveType.ToString()));
            }

            try
            {
                return Create(alg, null, creationParameters);
            }
            catch (CryptographicException e)
            {
                Interop.NCrypt.ErrorCode errorCode = (Interop.NCrypt.ErrorCode)e.HResult;

                if (errorCode == Interop.NCrypt.ErrorCode.NTE_INVALID_PARAMETER ||
                    errorCode == Interop.NCrypt.ErrorCode.NTE_NOT_SUPPORTED)
                {
                    string? target = curve.IsNamed ? curve.Oid.FriendlyName : curve.CurveType.ToString();
                    throw new PlatformNotSupportedException(SR.Format(SR.Cryptography_CurveNotSupported, target), e);
                }

                throw;
            }
        }

    }
}
