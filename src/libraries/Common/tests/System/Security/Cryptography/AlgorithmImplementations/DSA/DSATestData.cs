// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Text;
using Test.Cryptography;

namespace System.Security.Cryptography.Dsa.Tests
{
    // Note to contributors:
    //   Keys contained in this file should be randomly generated for the purpose of inclusion here,
    //   or obtained from some fixed set of test data. (Please) DO NOT use any key that has ever been
    //   used for any real purpose.
    //
    // Note to readers:
    //   The keys contained in this file should all be treated as compromised. That means that you
    //   absolutely SHOULD NOT use these keys on anything that you actually want to be protected.
    internal class DSATestData
    {
        public static readonly byte[] HelloBytes = new ASCIIEncoding().GetBytes("Hello");

        internal static DSAParameters Dsa512Parameters = new DSAParameters
        {
            P = (
                "D6A8B7F1CAF7A6964D07663FC691D22F6ABCD55C37AEF58D20746740D82FE14E" +
                "146363627D91925142DCDEE384BE0A1E04ED5BF5F471486F4D986D42A2E7DF95").HexToByteArray(),

            Q = "FAB5F625D5D5E16430A1EF630EBE33897CC224F9".HexToByteArray(),

            G = (
                "0844C490E52EF58E05902C636D64D1D5EB2C6082A0D4F3BFD1CE078E87B43A7E" +
                "F7BBECE19A4EFE2A6D9C229D360083CEA9F721F39B05BAF97052DEFC67A58A2B").HexToByteArray(),

            X = "2E3D7A84C85B66785E1F6FE796982B22B0CB98BC".HexToByteArray(),

            Y = (
                "C300E0E67D877E6CED39FEEAAAC1F2C2BD568E6A32467227E12B6AE45A8D9478" +
                "541A480AC80038AAC863827D6E3984061A25905C18BD2499A839663C3CA45605").HexToByteArray(),
        };

        internal static DSAParameters Dsa576Parameters = new DSAParameters
        {
            P = (
                "E2167306BFFD86BB62F4327B778BBFA07BA42323EC567B106B9563882BDDD6D7" +
                "F2EE7360F299888DE9F40A61C78D0BD8442EFA9C322B868AD367B3941D72B7A3" +
                "32C954EB1629132B").HexToByteArray(),

            Q = "CCDCECCF5F0B2C8FE238E2F06F22137F17FAEB1B".HexToByteArray(),

            G = (
                "AF17D4061302079E33034A77A058DDB4B832ACB114B7B8D2D3AE4451DFF85EB8" +
                "DD75D4474218369D485B2206506406044AB4E6407FDAA5A29E95D4964CA559E8" +
                "1C6F7CFCDA872665").HexToByteArray(),

            X = "AC32693E1CD72AD63E1A0B6E8157EBBCA671D3DB".HexToByteArray(),

            Y = (
                "815A549B6FD0CEDAF044B00B7CFE1351902D7727D6D7FB736003A4E1C4CD8DFB" +
                "F431E4FF4733F3FA92C765F0CFF944E3ED56A85B75953EB16901248985BB5F89" +
                "1398EAB5E39645E7").HexToByteArray(),
        };

        internal static DSAParameters GetDSA1024Params()
        {
            DSAParameters p = new DSAParameters();
            p.G = (
                "6BC366B66355545E098F1FE90E5469B567E09FA79D817F2B367B45DECD4301A59C81D6911F7691D370E15AC692C04BC11872" +
                "C171A7FE654E963D7DDA575A9E98CE026FB7D3934A258608134A8EC5ED69A2AEDC89401B67ADDE427F17EDAEB72D7AF45D9A" +
                "B1D59E1B13D4EFBD17C764330267DDE352C20E05B80DB3C109FE8B9C").HexToByteArray();
            p.P = (
                "C16D26C74D6C1627799C0918548E553FE58C7881DA484629CAF64311F4B27CFEF6BDB0F21206B0FFC4999A2FED53B43B9EE2" +
                "910C68DA2C436A8018F4938F6472369F5647D005BCC96E22590CC15E3CD4EA0D132F5DA5AF6AAA0807B0CC4EF3404AF542F4" +
                "546B37BDD6A47E641130837DB99397C845635D7DC36D0537E4A84B31").HexToByteArray();
            p.Q = "D83C0ECB73551E2FE30D51FCF4236C651883ADD7".HexToByteArray();
            p.X = "C02678007779E52E360682214BD47F8FAF42BC2D".HexToByteArray();
            p.Y = (
                "690BB37A9145E05D6E7B47C457898AAEDD72501C9D16E79B1AD75A872CF017AA90BBFB90F1B3B7F5C03C87E46E8725665526" +
                "FD34157B26F596A1F0997F59F3E65EFC615A552D5E7569C5FFC4593D5A0299110E71C97E1221A5A03FE9A6935AEDD88EF0B3" +
                "B2F79D3A99ED75F7B871E6EAF2680D96D574A5F4C13BACE3B4B44DE1").HexToByteArray();
            return p;
        }

        internal static DSAParameters GetDSA2048Params()
        {
            DSAParameters p = new DSAParameters();
            p.G = (
                "44A7D22DEBA29CE19D678D2DC11F118BAA10E3BEA94DE29C3EC36C10AB4D688004A1B7F4387FC1CC9613E6851FEDBBD54531" +
                "9EDE544B94E4FAE9C1069E7734F9E6AFC8A0B840696CDFFE286E1AF1AD6E39629D0C8C6016AC625F100BACF5FF74B2325C9D" +
                "99A6D8B0310B268F63E35F5D1C8DA663F94ABA90244CECCF9A8CE5DB5479B006F9131E5EA78222A32E2A103C1FEF16929B15" +
                "6E3230C954295CDA3E5F91F71B567FA3B774B842F128CD0D343D5468DB90734A678035E65E6A21CC7301F4F53E6B66718A83" +
                "FF285A6FDE89CA5AE1D1B9633A25A34A926F9D2808F9BF795D936787FF4C7286BC7FC4A82AFA9B06C9125109A923BAF3E377" +
                "55F1574BAFB5").HexToByteArray();
            p.P = (
                "AF8FB9B2B147C96F392360639BDAA6544FF2CD086B604A3324F485595F02CEAF32D2D49F2DD3AD9F5384DD09D03182DB8F0A" +
                "A06CE0A0FCA2366A5B031BD0E2FAD797BADC874A0C6781529C01E0D69704B43DF371AE4A9B3DD81EB03F5DAE283780112568" +
                "66337B73A824F7E8ACA61981A7C67649363D2D7D12CC2305308D084BEC68CC3B1B3EF577053A5E23449FBA3C3EB6F8CD8EA6" +
                "9F116B3748BD237F97F4F7E911C41C394D6F7D2D53B767618F0DED48E7BD7F30C6568948264A54C00D358A76E9826ED791C6" +
                "B6CBF8C29C245173B1D8D219438E59373CE7554EF49C7840A8C55CE2E5E2C33C10AAD8D90F28C7CB2EF14AD5ED8C4E6992B4" +
                "1ECEC65288F5").HexToByteArray();
            p.Q = "C93AB229237282997F23541A399BBF75CECD30BE0BE9592C07043ED30221EACB".HexToByteArray();
            p.X = "B8BFF7B3328E6B6DBC8325A275A1193EF81C975ECF88B340C468B770FB5E2658".HexToByteArray();
            p.Y = (
                "011BD85986A2C41315FF01C554A45E5A9C45B38EBDCC2660B6D17889604E800A6FDE8E017CED3793F4A6FBAFBB7613FEB7BA" +
                "87841ABD59935D18858939C00A6E4619B6562475955D6D72B2134BECF5AB34118F60D84B1FF268753F188E861255132D84CA" +
                "0AAB681B8551873394F18E0DAEF6B5F9577EC7CA2AA63281B539AF5894283E3FB6DCC5537D92C8BFE7C0723AC212EEF3B771" +
                "8D35C3FDD99F7AE0912CC7C47521A56FA511BE31712D979AE67F17A7BCC4E5810EA9CA138DEED3D78239EEEF239A772942D2" +
                "3C2810C2972219525646127F9C4B81612C12AEDC2DC91E81153B0A207C1FBC00795B3EB8E86796CED29AC1247FF7122A1EAF" +
                "081025751572").HexToByteArray();
            return p;
        }

        internal static DSAParameters Dsa2048DeficientXParameters =
            new DSAParameters
            {
                P = (
                    "94D3E4B39F17273E8F27B32692F7C34D35618BD279548B4736939C2BA4E72911" +
                    "6CAEAC6001E95705C27D7CF0C308BCA702BB4712D4BB448A1078B631150EAB45" +
                    "5C91678A61C51C2D371E02BBAC70E9C2C6DA757B57820EC8FD096E105EC36C07" +
                    "8A4FD7DD8724D275D3007F94F12FF96167293C667B467707F1DC9A7C0B34BC4F" +
                    "C475B15D51240A789A53A56B64D69C1E02B7CA3AD341B3B6F6C986F1E065BFE6" +
                    "80D87DEE6D9591D8E38E57C37AC2983B2794BEAFFE2C7B5B6CA49B9812487332" +
                    "5C61DE6F820411018BE9723FE2BDA8332C13088AA32A086D14D6F00201013F51" +
                    "678643F32ACC00525DB22F0020D92F9C2F4088B8D35A1E764DCD8B70C0B3D2EB").HexToByteArray(),

                G = (
                    "3CF0F144B70ABBD44260D92C4E032ADF0A598E68323E254AFF8710C9E8EA9BE1" +
                    "9D0D2AF939AA0DEC76D207B6EACEF7824E366575356301CA5F985EF8FF7AF551" +
                    "8B8DF0885A6AC570FE2E5C5956170E71F2740194018C8A0A4A7B0F9CF96AFD7E" +
                    "25026FA80B9F9726B6C4DFCBC4105EB98687CA697F797940C1335B144311D47F" +
                    "F21E4D5C43C00584786BC1AAE4AD858AC063351EE4423470BE5115528694EFCF" +
                    "312D9E58D4DDD1C01E283833F4F149E5F9BD4549304431B61BE0DAD717B538C5" +
                    "CD52FDE1A8E66193E89D09C1E07968E6F707EF5614E83E61126F6A5F3939E076" +
                    "7374544BE3225CC4F8F3C501D5A2526F7BB64D656A7BBDFE7E72CB16D71E8A38").HexToByteArray(),

                Q = "DB70A03A158E9CCB9D93DBF76796CBEBCA45A8703E82A45801588EE4B6985AB3".HexToByteArray(),

                Y = (
                    "7072CEE0A830F2150302F8B0CF0F07147B6C6B132CE64DF4F6E13B9E87F0F2DC" +
                    "5E05744119B6958095351A7742A9E45C72E23EE8BF0098C82FD2D0787F219747" +
                    "0BFB0CBC21462AD3893DEF7C1A0DF34FD52957FF01C69F76C564C268EBCD43DE" +
                    "C376DF538A8874AF34ED007B4E46D0660068137931AA59A32E875B29429DB556" +
                    "35DEB6DC8CBF867D4494CAD5EDEE7E7F205EE8E8B2C8A052B4528F47C92B1F8C" +
                    "FEC9ED053C21E3A731BFF269AD099C413D5FC8785351165E79AA7A143A0C7E55" +
                    "61EF03BCE3CF0DED9CED8316802F99DDAD032DE8C19270F3BAF5A8260888917F" +
                    "CD73E4B1D56FAD1563C71DBC92F6725692DDD6A39141844D11FDA230B7BE6BAC").HexToByteArray(),

                X = "00C871B7E389685DB97D934DD7011FAB076FF27EC4282FB00C0D646D29D5E4E5".HexToByteArray(),
            };

        // The parameters and signature come from FIPS 186-2 APPENDIX 5. EXAMPLE OF THE DSA
        internal static void GetDSA1024_186_2(out DSAParameters parameters, out byte[] signature, out byte[] data)
        {
            parameters = new DSAParameters()
            {
                P = (
                "8df2a494492276aa3d25759bb06869cbeac0d83afb8d0cf7cbb8324f0d7882e5d0762fc5b7210eafc2e9adac32ab7aac" +
                "49693dfbf83724c2ec0736ee31c80291").HexToByteArray(),
                Q = ("c773218c737ec8ee993b4f2ded30f48edace915f").HexToByteArray(),
                G = (
                "626d027839ea0a13413163a55b4cb500299d5522956cefcb3bff10f399ce2c2e71cb9de5fa24babf58e5b79521925c9c" +
                "c42e9f6f464b088cc572af53e6d78802").HexToByteArray(),
                X = ("2070b3223dba372fde1c0ffc7b2e3b498b260614").HexToByteArray(),
                Y = (
                "19131871d75b1612a819f29d78d1b0d7346f7aa77bb62a859bfd6c5675da9d212d3a36ef1672ef660b8c7c255cc0ec74" +
                "858fba33f44c06699630a76b030ee333").HexToByteArray()
            };

            signature = (
                // r
                "8bac1ab66410435cb7181f95b16ab97c92b341c0" +
                // s
                "41e2345f1f56df2458f426d155b4ba2db6dcd8c8"
                ).HexToByteArray();

            data = "abc"u8.ToArray();
        }
    }
}
