// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using Test.Cryptography;

namespace System.Security.Cryptography.Pkcs.Tests.Pkcs12
{
    internal static class Pkcs12Documents
    {
        internal static readonly ReadOnlyMemory<byte> EmptyPfx = (
            "304F020103301106092A864886F70D010701A004040230003037301F30070605" +
            "2B0E03021A0414822078BC83E955E314BDA908D76D4C5177CC94EB0414711018" +
            "F2897A44A90E92779CB655EA11814EC598").HexToByteArray();

        // Extracted from https://github.com/dotnet/runtime/issues/18254
        internal static readonly ReadOnlyMemory<byte> IndefiniteEncodingNoMac = (
            "3080020103308006092A864886F70D010701A0802480048203E8308030800609" +
            "2A864886F70D010701A0802480048203E83082051530820511060B2A864886F7" +
            "0D010C0A0101A08204C0308204BC020100300D06092A864886F70D0101010500" +
            "048204A6308204A202010002820101009F48C352AE217B90067400DC6BE53EED" +
            "B2BD7654F016CCEB2070BB2A6C51D3A4E22B1BF12DBE088FE9DC4CFCC9631BF8" +
            "764B4354015C3BE4D95CD9508247FB1635A16AAEEC7BE8F62003925B4A6DF675" +
            "CA37C6E43CBB15AB4D9D51E51A8ABB1D32BD5A5C44559BADAC776904A1CD7475" +
            "A85EBD31AB14B862D7179E705930B943A55871509E8BFD7D16447639D9070BA7" +
            "A6587C039B535FA5F88861ECE54FD6BF1CD079881FEA8BDC5690A6586B36316B" +
            "EAAAC26AB038D836637204A1B8D39FBAF84425E8E680E2FB412D23F21DAC4FF1" +
            "5AE680CDC11349579BEB9E2F063DB8DDE126DC87EF74F8ECF7F6DC8EF1E72F22" +
            "C28A6A4EBBA146D1F341325CD8DBD1E30203010001028201003C0BF0264122BA" +
            "34075ABFE05884697EAE3D2258CED1A99A91F62D78C6B0EA3A6450A00E01927C" +
            "23D4A38A1A59C915084B7FFFF7B1339618A5A08D03FEB927FCAD671692AEEFDF" +
            "58B9A4DF0DAF37590BFA99A04EF65E08C3355533444D073396C44401C67AB453" +
            "25DBB8804C90BAA5469D9C697249EE5BBC571DBE4AD834B80222037E453D8AA9" +
            "14C5AFE1106E07A6E781817BF5E2845AE73E213F34DB88871E8F5F1FD67DC824" +
            "0D52B004D9DD18D4E41E769E2FE56002EE7DD9C025E07270B0D4F66ED46EA4DF" +
            "5D2A5BDAE58CF7224A08E00B1D0673139D3FA56DC3450A5D7DB92868CB9CA07A" +
            "944AC2D26D32E461DB87A316FD062EC0EAD8114D016D3AF7E102818100D0BADF" +
            "D704884E3149CAC3DECF40FF9FFCA51A0DF288CB1150E4E388A018C1F3FAEB07" +
            "7F1C69F1842F998779512D4FEAA3C05F5B1C1CAA870DBA2477B68387FE4BED32" +
            "A5C31A3E8AFEDA38F41625BA449ADB17012B0C61F2A77A27D4531038EC6B4460" +
            "B1D0F58CB7900ADAB536871FD2B76BAC77C1BB24F35E69EFB86547281B028181" +
            "00C35B4529A6C7FE776F5615E8BC4EBB1CD14B208FE6A62CC251B68BB1FA03E3" +
            "0B9949F162EAC5BD2A6D93EBC49E099B3895BDADDE3D6C7ACAE8F52D2813582F" +
            "14568D152D9202B667D3D082E73C183D16ED28C765A7E914F11355D2B12153FA" +
            "6B5F110798CDBC8B8E3C0F19D3DFCA87CD1DC3D7443D7B3510542E6F110543A9" +
            "D9028180399B0981C6FF734D09078BCD2737D214AE5E467588F515CE1E9C3CEB" +
            "8CFBA83394915ECD46E33A4377FB9036FC1F5C49EE8D7E03A01B8D389EA23BC4" +
            "4A3BBCC182E4E0B07CCAF66DD7EE13FFD148F24252D69A5FB928BEB380632730" +
            "E307BC1E0B70A09B128762219B0053E1E53D9F9BC1015764E9B3A37C03178B90" +
            "416B048203E844F70281805A6BDE615EFEE6BB655F1518FA3FFEBF61E8048201" +
            "3142141910055B93F2C84A028CD6044259454F219790ED18770503A69A8FABEB" +
            "33364CCD656E9888A722D58FCE9B4BF09BB097BD23604642761A80E901D714CB" +
            "84BA7644F7CF679E297531E359396985411EA10D691DB721F9635CFC128434DE" +
            "780D255AC7B251F284E3879F462851028180627F8F0152C35A648A326EAB2436" +
            "C99DED76358BE2A8E6BA9F764983437D23CA340BFC455F5A3F233245D043DDC9" +
            "59428FF95A218A145F73B53F923C61EB9B711FC9BD8FCB9C232EB60B1696475B" +
            "CF973D81F6C555A1175D03C5C8AB9268C79AEA99FE92FF4462D55F822438AD9E" +
            "1A6707B40969337C3BC813A00662030808C2313E301706092A864886F70D0109" +
            "14310A1E080063006500720074302306092A864886F70D01091531160414EDF3" +
            "D122CF623CF0CFC9CD226261E8415A83E630000000000000308006092A864886" +
            "F70D010701A0802480048203E8308206113082060D060B2A864886F70D010C0A" +
            "0103A08205BC308205B8060A2A864886F70D01091601A08205A8048205A43082" +
            "05A030820488A00302010202081BA5AE3003BE6D65300D06092A864886F70D01" +
            "01050500308196310B300906035504061302555331133011060355040A0C0A41" +
            "70706C6520496E632E312C302A060355040B0C234170706C6520576F726C6477" +
            "69646520446576656C6F7065722052656C6174696F6E73314430420603550403" +
            "0C3B4170706C6520576F726C647769646520446576656C6F7065722052656C61" +
            "74696F6E732043657274696669636174696F6E20417574686F72697479301E17" +
            "0D3135303131323132313231305A170D3136303131323132313231305A308193" +
            "311A3018060A0992268993F22C6401010C0A524A585654453836353331383036" +
            "06035504030C2F6950686F6E6520446576656C6F7065723A2046726564657269" +
            "6B204361726C6965722028385439554B55424759392931133011060355040B0C" +
            "0A5443444B35454C41483731193017060355040A0C10467265646572696B2043" +
            "61726C696572310B300906035504061302555330820122300D06092A864886F7" +
            "0D01010105000382010F003082010A02820101009F48C352AE217B90067400DC" +
            "6BE53EEDB2BD7654F016CCEB2070BB2A6C51D3A4E22B1BF12DBE088FE9DC4CFC" +
            "C9631BF8764B4354015C3BE4D95CD9508247FB1635A16AAEEC7BE8F62003925B" +
            "4A6DF675CA37C6E43CBB15AB4D9D51E51A8ABB1D32BD5A5C44559BADAC776904" +
            "A1CD7475A85EBD31AB14B862D7179E705930B943A55871509E8BFD7D16447639" +
            "D9070BA7A6587C039B535FA5F88861ECE54FD6BF1CD079881FEA8BDC5690A658" +
            "6B36316BEAAAC26AB038D8366372048203A004A1B8D39FBAF84425E8E680E2FB" +
            "412D23F21DAC4FF15AE680CDC11349579BEB9E2F063DB8DDE126DC87EF74F8EC" +
            "F7F6DC8EF1E72F22C28A6A4EBBA146D1F341325CD8DBD1E30203010001A38201" +
            "F1308201ED301D0603551D0E04160414EDF3D122CF623CF0CFC9CD226261E841" +
            "5A83E630300C0603551D130101FF04023000301F0603551D2304183016801488" +
            "271709A9B618608BECEBBAF64759C55254A3B73082010F0603551D2004820106" +
            "308201023081FF06092A864886F7636405013081F13081C306082B0601050507" +
            "02023081B60C81B352656C69616E6365206F6E20746869732063657274696669" +
            "6361746520627920616E7920706172747920617373756D657320616363657074" +
            "616E6365206F6620746865207468656E206170706C696361626C65207374616E" +
            "64617264207465726D7320616E6420636F6E646974696F6E73206F6620757365" +
            "2C20636572746966696361746520706F6C69637920616E64200482022D636572" +
            "74696669636174696F6E2070726163746963652073746174656D656E74732E30" +
            "2906082B06010505070201161D687474703A2F2F7777772E6170706C652E636F" +
            "6D2F6170706C6563612F304D0603551D1F044630443042A040A03E863C687474" +
            "703A2F2F646576656C6F7065722E6170706C652E636F6D2F6365727469666963" +
            "6174696F6E617574686F726974792F7777647263612E63726C300E0603551D0F" +
            "0101FF04040302078030160603551D250101FF040C300A06082B060105050703" +
            "033013060A2A864886F763640601020101FF04020500300D06092A864886F70D" +
            "01010505000382010100688B8CC9D1D24F89757CCB1A5EBE75123A9AD347C1DC" +
            "F358F51C3AA06D5C5A4492E91831033EC027B74908ED2D65DA8F097E952A74C7" +
            "86032542B23CB487D4D96706FAC577B6F9C0CACA91B433EA8475A24788EE765A" +
            "74113CE82C72450DB6E712987736939855F530C82B6160BFF1BF7705F6885C1E" +
            "F7D2D363B832C425D502A15332D710E9E350680F21BEF198620FE78F9335E3E1" +
            "11D3A2D398A32ABD403B806008A8DEBBBB4762F22EC98F4718E7C6CAE5ECA03A" +
            "CFE85F898F7551BD808266E288BC39E87061EC8BD0206B1B8E49DA79190383E8" +
            "E42F5DA38F78934B7FCFB56754B887FF654BA56E6EA1FEB69F67BE052EABECDB" +
            "987BF98AA45A74C9B3E1313E301706092A864886F70D010914310A1E08006300" +
            "6500720074302306092A864886F70D01091531160414EDF3D122CF623CF0CFC9" +
            "CD226261E8415A83E63000000000000000000000000000000000").HexToByteArray();

        // orapki wallet create -wallet ewallet.p12
        // mkstore -wrl ewallet.p12 -createUserCredential a b c d
        // mkstore -wrl ewallet.p12 -createCredential a_prod_db a_test_user
        //  (prompted secret: "potatos are tasty")
        internal static readonly ReadOnlyMemory<byte> SimpleOracleWallet = (
            "308202AC0201033082027206092A864886F70D010701A08202630482025F3082" +
            "025B3082025706092A864886F70D010706A0820248308202440201003082023D" +
            "06092A864886F70D010701301C060A2A864886F70D010C0103300E0408967CA6" +
            "2B186E7BB00202040080820210B62C4961FCD9978CF84C83F4C8F32030725F11" +
            "69D25B53CC875E4D25D115D84B659D210E4D26F1FD1A3B85D5B4E0F61402A44E" +
            "71D642A563F0BEB10BA974897C22AE037E6509C7E4F5100E201781E1E7C2A0AB" +
            "181A65A330148FC0618D28D019FA37E32803A71936AE79ADF1AA6D10B112819B" +
            "A9D84D0E797B213466B8590A0B2A8F13E6B39BB32FD76377E87DD5DEC88AB716" +
            "D3983C78BECE79A089A42D6AF3E30DF72084DA605E68CEFD326FFF9532EFE8E9" +
            "862E08AD72427F356CCAE049945B1001EB1D20C404609F1D0942534548F076C2" +
            "BC812CD6BD30F9ECB38F72C7101310267A12902DE5FAAAFF304E886F4511A13A" +
            "8990F433FE4B5C1BD04A6DFB6567C49C9E23168D42CEF6135EA6679A29D013CA" +
            "0E4F5F1A9FD5D2BC42A672A8E5C376506292C4849D4DF75D8E36CE8C24189D80" +
            "877F355AE5A5F94B6B763635D9D492F2593571EBBBABA6A5A84DA3EBB3804DAF" +
            "7EAF2B798DDDA0B66CF09524E0D5D3CEDE5AC09CE0EE0C17D6508CA1367BD8AF" +
            "1333F847C06410DA32C87AC2795778F23393C07C7629D137791328A36E2042C6" +
            "A966F24C70E4E4001ABF1FCFA83E5A5E4D44A2DFAF1C2E5D48FB52D4A9AB775B" +
            "40C2CA3BB3205DCD7053C33C3712DEDE5B7D9A31FDB9206823CB6874B3D49E0E" +
            "295FA05D635BD07F132306512DE4D27E640FF033F9F26F7E3B89709ED174A6BD" +
            "ECE2283B5B97C4A4640DE37905C183FE5D28846859B453BEBF39F4F99E303130" +
            "21300906052B0E03021A050004148B12EE39C54B03EF4C1B0C2D8A3A9624D629" +
            "285A0408E398C69C57E4782102020400").HexToByteArray();

        internal const string OracleWalletPassword = "123Wallet";
    }
}
