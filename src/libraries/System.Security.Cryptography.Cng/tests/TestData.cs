// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Test.Cryptography;

namespace System.Security.Cryptography.Cng.Tests
{
    // Note to contributors:
    //   Keys contained in this file should be randomly generated for the purpose of inclusion here,
    //   or obtained from some fixed set of test data. (Please) DO NOT use any key that has ever been
    //   used for any real purpose.
    //
    // Note to readers:
    //   The keys contained in this file should all be treated as compromised. That means that you
    //   absolutely SHOULD NOT use these keys on anything that you actually want to be protected.
    internal static class TestData
    {
        public static RSA CreateRsaCng(this RSAParameters rsaParameters)
        {
            RSA rsa = new RSACng();
            rsa.ImportParameters(rsaParameters);
            return rsa;
        }

        public static RSAParameters TestRsaKeyPair;

        // AllowExport|AllowPlainTextExport,  CngKeyCreationOptions.None, UIPolicy(CngUIProtectionLevels.None), CngKeyUsages.Decryption
        public static byte[] Key_ECDiffieHellmanP256 =
           ("45434b3120000000d679ed064a01dacd012d24495795d4a3272fb6f6bd3d9baf8b40c0db26a81dfb8b4919d5477a07ae5c4b"
          + "4b577f2221be085963abc7515bbbf6998919a34baefe").HexToByteArray();

        internal static readonly byte[] s_ECDsa256KeyBlob =
                ("454353322000000096e476f7473cb17c5b38684daae437277ae1efadceb380fad3d7072be2ffe5f0b54a94c2d6951f073bfc"
              + "25e7b81ac2a4c41317904929d167c3dfc99122175a9438e5fb3e7625493138d4149c9438f91a2fecc7f48f804a92b6363776"
              + "892ee134").HexToByteArray();

        public static readonly CngKey s_ECDsa256Key =
            CngKey.Import(s_ECDsa256KeyBlob, CngKeyBlobFormat.GenericPrivateBlob);

        internal static readonly byte[] s_ECDsa384KeyBlob =
                ("45435334300000009dc6bb9cdc8dac31e3db6e6b5f58f8e3a304e5c08e632705ca9a236f1134646dca526b89f7ea98653962"
               + "f4a781f2fc9bf479a2d627561b1269548050e6d2c388018b837f4ceba8ee7fe2eefea67c8418ad1e84f60c1309385e573ea5"
               + "183e9ae8b6d5308a78da207c6e556af2053983321a5f8ac057b787089ee783c99093b9f2afb2f9a1e9a560ad3095b9667aa6"
               + "99fa").HexToByteArray();

        public static readonly CngKey s_ECDsa384Key =
            CngKey.Import(s_ECDsa384KeyBlob, CngKeyBlobFormat.GenericPrivateBlob);

        internal static readonly byte[] s_ECDsa521KeyBlob =
             ("454353364200000001f9f06ea4e00fd3fecc1753af7983b43cb9b692941ee6364616c9c4168845fce804beca7aa23d0a5049"
            + "910db45dfb61112f4cb02e93ff62af1be203ad248dd70952015ddc31d1ad7411ca5996b8b76a40ea65f286c665225114bec8"
            + "557365aa4bc79358f8c68b873cb76a1c86a5a394185d8eeb9602b8b968db1e4ac49b7cc51f83c7170055ad9b0b2d0d5d2306"
            + "a66bf87a256a3739696121eb131e64ae61991ea23db99b397c32df95efb0cb284147a929c65e9f671073ca3c7a084cb9211d"
            + "ceb06c987277").HexToByteArray();

        public static readonly CngKey s_ECDsa521Key =
            CngKey.Import(s_ECDsa521KeyBlob, CngKeyBlobFormat.GenericPrivateBlob);

        static TestData()
        {
            RSAParameters rp = new RSAParameters();
            rp.D = ("2806880f41dfba6ea9cb91f141c07e09cc0def786030162e1947c50d427d21dc5c0779ded52c50e570665884ba0ba32977c6"
                  + "3019da0d255de458c9f421f0a17cd70bc21ea1e97152d3ded5ef1f17927bf2c03f83a72534033baacc670443d4e9c80e2d87"
                  + "e206a3c3094ee5b20c3a1edf99c275f8f63cd4de7cdea326050cb151").HexToByteArray();

            rp.DP = ("0aa6fc0436a24aa03c7a4d0b4cb84b75b9475eb0410ffaaa2a2c6d4dd8d4c3a5ac815bdeb93245babef613f983e4770d63d0"
                   + "d931e33f0509019a1e431e6b5911").HexToByteArray();

            rp.DQ = ("b7944d4d4846708c33adb0ad964623ad0e55d7c5bbd6475d25b12fbb39ab8c75794fdc977d67f54833ba59acbec8f3d91ddb"
                   + "f29d0e780d52f8c656cad787fad5").HexToByteArray();

            rp.Exponent = ("010001").HexToByteArray();

            rp.InverseQ = ("8fdd8821b7fcc6e907436bc33d7311f9344ee18a3af36429c550f34f83c4c93fd0429f63bdc502db9cc03d3d857a6354e98b"
                         + "db7c76b3ab54c32cdae75c539f2c").HexToByteArray();

            rp.Modulus = ("c7b5012552672f812a015bf3356abdfe4964cfe2ae35b8aba819120c58ffa2f1fc0f512e76fd22e6d32646ceea78829a9cbb"
                        + "2dbe5c66d14390e1bcef05afbababfe1f5ca07983b1f688a01b2beef8886b05df9e9420e65a1c0dc605ccfa2e27d84b39433"
                        + "ffcd07441ef5be8ab80497bc553fce022c7620922d1d624b6e3babe1").HexToByteArray();

            rp.P = ("c7eb601fdd49b22eda5b9a5ccb2fcfc35a660bb3bd2872857c864432e32916c2231e3b3da8afddc3efa38d04f9b1a08a08ab"
                  + "08b4603ff28345ba32d24de3cfa5").HexToByteArray();

            rp.Q = ("ffba608710355472b48b41e57eadd19a3f1a5d2fc1baa3d6210520c95694f11a065a16354827abdb06a59c3616f5ff2c5ca3"
                  + "be835f1278e9a9e9f0373027b68d").HexToByteArray();

            TestRsaKeyPair = rp;
        }

        public static byte[] Key_DSA1024Key = (
            "4453505680000000000000AEE5A1D69A3F46E5225E5AFF69C5065D6AB1D6E2D9AB9515365DB8CAF4181C26C97F65BFB1931F" +
            "EE7D865A61C79F41BE6ED77DF2E9BC7E1FF471FFDCFBEEA506234878C9871F5F85E650CDDE73F0B445BE5DD6A4A42A67F1F8" +
            "9DB4C8CE799DD874AF06D8B8819210FF056A8DD76957C6B192AC590B13FC39000EF00AE2E77CD3AF869871BC758B8ABC2F97" +
            "7FAADC9B3116C7F0A7B2970B621066464E6A5AA2AD528CB88DC0A2C5B8B92007C7C3A4C7472B47B0BB61F4F9A32A7F0F08B6" +
            "8DF2C153B08AFD127C4483C6123C3375171C1040B8F0CFAAFDC58668AA819BB466DED1CF1A88026F8BFC73BA0D9B60218749" +
            "88C90DBB2552B122D7A693F5183E7C455C7D984488D14DE99998DD14CC1B0420EBCC4E858762F2D61CEFDC4C46F72948AA24" +
            "C7AF42CE544FE3E6446E408B6CFB1F81C212AE398BB6D2E69BD5B0AF35D2A8115A8609F3ADF101AE2288496CE05F9E49785D" +
            "A01416787FBF270424874147995EC298854129E1469A265330EF4143DF02A20A8FB49C28D743EC9A06978BF7BA6AEE5D5FA4" +
            "E177110CF26AF2791EBA2CEC63FB3B186A3E77ADC18F2070924C83BAED880DEC593AE1B9A6E41B4A33BC89A635F033C93B9C" +
            "BA0FA7749E17").HexToByteArray();

        public static byte[] Key_DSA1024PublicKey = (
            "445350428000000000000003E9C7ECDB129947A0B6E1051311AF4410F3615518AE45325323742A6F08D3E69C9564C3909FC6" +
            "A04DA3F643E048B96FF73CADAB870EA2F2A5E183691AEECC9694889E3BE02A81DCBBDB4DA810E38AB8072192AB26DDAD2F95" +
            "4B5F0DF8DECFF1E72E56017F50E476BEE3702D89FAACEDB3FF6457C5545FA1D838299EDCF432785188313EF361322BF78AD3" +
            "B491606BD6EBCA227571F2E14EFA9379AD9A78329C89FBB56835F6728513614A0037D042D82C486878A153C71F8C4890A9C9" +
            "8CAB84FAF7D97C30081A2BE599CDD59E42BEC299BEFAFCBA4369653C81FF64F75976D8258E6CBD45A28C5774F90C29ED46BD" +
            "178FEB031A003BE6870F69BD23BC1CC2A3DEA77506C7A96F2E2A4EAD74D31DB16EB696A8565577B95C2A5BC89058114BDDD1" +
            "4D2AA790829B74BE2081080CFE4994EB1CBEE7F4B5001A8BC700EEE1511F25CDE5A08853F2DB10A477EFFB40E22B9B846DED" +
            "A126CF159281A4889C0A59EF26A475ED193B853858944A7CA2750040DD8578418A38C733B7EFC6C18E7E9ABCCB5E0540D3A1" +
            "2741968D89F7EFECBE709C7A451AACF070F6143ACC51B19B28CA49492C3A5EC322C9FFFA").HexToByteArray();

        public static byte[] Key_DSA2048Key = (
            "445056320001000001000000010000002000000020000000000001272B26541C50E35AFA386C959ED174D2DD70849A07D8EC" +
            "E24838315A48E1679AD08125795599A6117EC55C2248303207C7C008CE1A86E9145D47810E621145871DF5C780BF53EAA45D" +
            "8ADA28AACC40E101C4DB01645B4CA46A83754ECA82B94F48A04469CA40F61AB8E2CB10992A4062D5156C6F4E7FA50C6A3403" +
            "3D6ED75BCFD68DCA234417B586FE1FB5B43751F6280C1D37AF6E07EB3F19A58E6762FDD73406CFA4B206A314D884A021F3F3" +
            "167BDDA066F65C08E0E9CB37315DC428F5F14390F566BEE35E03D118210E2EF39272EA2EE1B060CAAA1D1919BB1DEE4149CE" +
            "C45B5A4A995B984B0964A631DC44EABD918C75007C6844F03F3623BD27CAFE7B92F7AF041C834A58863482546972FBCB0F5A" +
            "42625706D7088EE327BA8117AC115E671A8BCAD940D7A2E5A12021CC6E5133C9388120E58061C11C161078437CB15DBD1AE5" +
            "4CF46DC427A79A34246EA8A8037038537119DA906808CFE18D827090C78A5D0061944465A955B764297DD0F0AC68E859414A" +
            "08447CD91562A4B378DBDCCDC7B5118F3B5BC565D7F0198C24AD7CF6DEF4F3FF91302627BA6330FE7007BDAFEC34D644E010" +
            "AA4F380F6322303430550E7D457041309C2F3E7323CE2EB8DE6DE31B68E07565BADD08A97159858725E474A9B2DC76556DA1" +
            "1E124903BF21520CF288545ED4A53247E67EFB7620FE7F5DBEEF190569A88E29F50FAE343563FB49314FFEB7DFFA212AFF0E" +
            "C280098A21AED0482057A97DA0AD4BA7E25A44376763EB035DCF065BDF187E2BF71E1A41BD1533F3F4D5432833BEB6485987" +
            "2A28AB0A20C6E4C7F09843A443C1FD02444913B8362041E17962D8F29B77F4DF763ED97ACEE7978B7ECA8DE8C7D4EB7C46F4" +
            "B274C3A780E4336A48E3526AF5F611A11FDB204069EA4212C8131D90D9158DDD29540B22B799A993652A8666E8BA2B6B9660" +
            "02EAC030FE3D01CE4DA543ACAE89106564EA81FC1CEE358B613ED0D1A106BEE8458CCAB7D55C79532A80F8F5ED27C4FED459" +
            "BC15B39737F3EDAF5F4F3260E34CFD75D790C3D404722A527B1D6AC52E704CD7FB4086820994CA88AA6DE32987882DFF6991" +
            "CE0D641DE425FD0ACD614B824F75F58D84D96F8AA92E4C12F9233F0F70629295361993A7371E3352B6393072BB34F91C3B1D" +
            "08C7B239ED66FDE828870C03AD24E3805D6E0B708AE0EF9E17474A1AEE754C6F865B943407B2CCD39B3E").HexToByteArray();
    }
}
