/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __JOIN_DTOA_HPP__
#define __JOIN_DTOA_HPP__

// libjoin.
#include <join/diyfp.hpp>

// C.
#include <cstring>
#include <cmath>

namespace join
{
    namespace details
    {
        inline const DiyFp& cachedPower (int index)
        {
            static const DiyFp result[] =
            {
                {0xbf29dcaba82fdeae, -1203}, {0xeef453d6923bd65a, -1200},
                {0x9558b4661b6565f8, -1196}, {0xbaaee17fa23ebf76, -1193},
                {0xe95a99df8ace6f54, -1190}, {0x91d8a02bb6c10594, -1186},
                {0xb64ec836a47146fa, -1183}, {0xe3e27a444d8d98b8, -1180},
                {0x8e6d8c6ab0787f73, -1176}, {0xb208ef855c969f50, -1173},
                {0xde8b2b66b3bc4724, -1170}, {0x8b16fb203055ac76, -1166},
                {0xaddcb9e83c6b1794, -1163}, {0xd953e8624b85dd79, -1160},
                {0x87d4713d6f33aa6c, -1156}, {0xa9c98d8ccb009506, -1153},
                {0xd43bf0effdc0ba48, -1150}, {0x84a57695fe98746d, -1146},
                {0xa5ced43b7e3e9188, -1143}, {0xcf42894a5dce35ea, -1140},
                {0x818995ce7aa0e1b2, -1136}, {0xa1ebfb4219491a1f, -1133},
                {0xca66fa129f9b60a7, -1130}, {0xfd00b897478238d1, -1127},
                {0x9e20735e8cb16382, -1123}, {0xc5a890362fddbc63, -1120},
                {0xf712b443bbd52b7c, -1117}, {0x9a6bb0aa55653b2d, -1113},
                {0xc1069cd4eabe89f9, -1110}, {0xf148440a256e2c77, -1107},
                {0x96cd2a865764dbca, -1103}, {0xbc807527ed3e12bd, -1100},
                {0xeba09271e88d976c, -1097}, {0x93445b8731587ea3, -1093},
                {0xb8157268fdae9e4c, -1090}, {0xe61acf033d1a45df, -1087},
                {0x8fd0c16206306bac, -1083}, {0xb3c4f1ba87bc8697, -1080},
                {0xe0b62e2929aba83c, -1077}, {0x8c71dcd9ba0b4926, -1073},
                {0xaf8e5410288e1b6f, -1070}, {0xdb71e91432b1a24b, -1067},
                {0x892731ac9faf056f, -1063}, {0xab70fe17c79ac6ca, -1060},
                {0xd64d3d9db981787d, -1057}, {0x85f0468293f0eb4e, -1053},
                {0xa76c582338ed2622, -1050}, {0xd1476e2c07286faa, -1047},
                {0x82cca4db847945ca, -1043}, {0xa37fce126597973d, -1040},
                {0xcc5fc196fefd7d0c, -1037}, {0xff77b1fcbebcdc4f, -1034},
                {0x9faacf3df73609b1, -1030}, {0xc795830d75038c1e, -1027},
                {0xf97ae3d0d2446f25, -1024}, {0x9becce62836ac577, -1020},
                {0xc2e801fb244576d5, -1017}, {0xf3a20279ed56d48a, -1014},
                {0x9845418c345644d7, -1010}, {0xbe5691ef416bd60c, -1007},
                {0xedec366b11c6cb8f, -1004}, {0x94b3a202eb1c3f39, -1000},
                {0xb9e08a83a5e34f08,  -997}, {0xe858ad248f5c22ca,  -994},
                {0x91376c36d99995be,  -990}, {0xb58547448ffffb2e,  -987},
                {0xe2e69915b3fff9f9,  -984}, {0x8dd01fad907ffc3c,  -980},
                {0xb1442798f49ffb4b,  -977}, {0xdd95317f31c7fa1d,  -974},
                {0x8a7d3eef7f1cfc52,  -970}, {0xad1c8eab5ee43b67,  -967},
                {0xd863b256369d4a41,  -964}, {0x873e4f75e2224e68,  -960},
                {0xa90de3535aaae202,  -957}, {0xd3515c2831559a83,  -954},
                {0x8412d9991ed58092,  -950}, {0xa5178fff668ae0b6,  -947},
                {0xce5d73ff402d98e4,  -944}, {0x80fa687f881c7f8e,  -940},
                {0xa139029f6a239f72,  -937}, {0xc987434744ac874f,  -934},
                {0xfbe9141915d7a922,  -931}, {0x9d71ac8fada6c9b5,  -927},
                {0xc4ce17b399107c23,  -924}, {0xf6019da07f549b2b,  -921},
                {0x99c102844f94e0fb,  -917}, {0xc0314325637a193a,  -914},
                {0xf03d93eebc589f88,  -911}, {0x96267c7535b763b5,  -907},
                {0xbbb01b9283253ca3,  -904}, {0xea9c227723ee8bcb,  -901},
                {0x92a1958a7675175f,  -897}, {0xb749faed14125d37,  -894},
                {0xe51c79a85916f485,  -891}, {0x8f31cc0937ae58d3,  -887},
                {0xb2fe3f0b8599ef08,  -884}, {0xdfbdcece67006ac9,  -881},
                {0x8bd6a141006042be,  -877}, {0xaecc49914078536d,  -874},
                {0xda7f5bf590966849,  -871}, {0x888f99797a5e012d,  -867},
                {0xaab37fd7d8f58179,  -864}, {0xd5605fcdcf32e1d7,  -861},
                {0x855c3be0a17fcd26,  -857}, {0xa6b34ad8c9dfc070,  -854},
                {0xd0601d8efc57b08c,  -851}, {0x823c12795db6ce57,  -847},
                {0xa2cb1717b52481ed,  -844}, {0xcb7ddcdda26da269,  -841},
                {0xfe5d54150b090b03,  -838}, {0x9efa548d26e5a6e2,  -834},
                {0xc6b8e9b0709f109a,  -831}, {0xf867241c8cc6d4c1,  -828},
                {0x9b407691d7fc44f8,  -824}, {0xc21094364dfb5637,  -821},
                {0xf294b943e17a2bc4,  -818}, {0x979cf3ca6cec5b5b,  -814},
                {0xbd8430bd08277231,  -811}, {0xece53cec4a314ebe,  -808},
                {0x940f4613ae5ed137,  -804}, {0xb913179899f68584,  -801},
                {0xe757dd7ec07426e5,  -798}, {0x9096ea6f3848984f,  -794},
                {0xb4bca50b065abe63,  -791}, {0xe1ebce4dc7f16dfc,  -788},
                {0x8d3360f09cf6e4bd,  -784}, {0xb080392cc4349ded,  -781},
                {0xdca04777f541c568,  -778}, {0x89e42caaf9491b61,  -774},
                {0xac5d37d5b79b6239,  -771}, {0xd77485cb25823ac7,  -768},
                {0x86a8d39ef77164bd,  -764}, {0xa8530886b54dbdec,  -761},
                {0xd267caa862a12d67,  -758}, {0x8380dea93da4bc60,  -754},
                {0xa46116538d0deb78,  -751}, {0xcd795be870516656,  -748},
                {0x806bd9714632dff6,  -744}, {0xa086cfcd97bf97f4,  -741},
                {0xc8a883c0fdaf7df0,  -738}, {0xfad2a4b13d1b5d6c,  -735},
                {0x9cc3a6eec6311a64,  -731}, {0xc3f490aa77bd60fd,  -728},
                {0xf4f1b4d515acb93c,  -725}, {0x991711052d8bf3c5,  -721},
                {0xbf5cd54678eef0b7,  -718}, {0xef340a98172aace5,  -715},
                {0x9580869f0e7aac0f,  -711}, {0xbae0a846d2195713,  -708},
                {0xe998d258869facd7,  -705}, {0x91ff83775423cc06,  -701},
                {0xb67f6455292cbf08,  -698}, {0xe41f3d6a7377eeca,  -695},
                {0x8e938662882af53e,  -691}, {0xb23867fb2a35b28e,  -688},
                {0xdec681f9f4c31f31,  -685}, {0x8b3c113c38f9f37f,  -681},
                {0xae0b158b4738705f,  -678}, {0xd98ddaee19068c76,  -675},
                {0x87f8a8d4cfa417ca,  -671}, {0xa9f6d30a038d1dbc,  -668},
                {0xd47487cc8470652b,  -665}, {0x84c8d4dfd2c63f3b,  -661},
                {0xa5fb0a17c777cf0a,  -658}, {0xcf79cc9db955c2cc,  -655},
                {0x81ac1fe293d599c0,  -651}, {0xa21727db38cb0030,  -648},
                {0xca9cf1d206fdc03c,  -645}, {0xfd442e4688bd304b,  -642},
                {0x9e4a9cec15763e2f,  -638}, {0xc5dd44271ad3cdba,  -635},
                {0xf7549530e188c129,  -632}, {0x9a94dd3e8cf578ba,  -628},
                {0xc13a148e3032d6e8,  -625}, {0xf18899b1bc3f8ca2,  -622},
                {0x96f5600f15a7b7e5,  -618}, {0xbcb2b812db11a5de,  -615},
                {0xebdf661791d60f56,  -612}, {0x936b9fcebb25c996,  -608},
                {0xb84687c269ef3bfb,  -605}, {0xe65829b3046b0afa,  -602},
                {0x8ff71a0fe2c2e6dc,  -598}, {0xb3f4e093db73a093,  -595},
                {0xe0f218b8d25088b8,  -592}, {0x8c974f7383725573,  -588},
                {0xafbd2350644eead0,  -585}, {0xdbac6c247d62a584,  -582},
                {0x894bc396ce5da772,  -578}, {0xab9eb47c81f5114f,  -575},
                {0xd686619ba27255a3,  -572}, {0x8613fd0145877586,  -568},
                {0xa798fc4196e952e7,  -565}, {0xd17f3b51fca3a7a1,  -562},
                {0x82ef85133de648c5,  -558}, {0xa3ab66580d5fdaf6,  -555},
                {0xcc963fee10b7d1b3,  -552}, {0xffbbcfe994e5c620,  -549},
                {0x9fd561f1fd0f9bd4,  -545}, {0xc7caba6e7c5382c9,  -542},
                {0xf9bd690a1b68637b,  -539}, {0x9c1661a651213e2d,  -535},
                {0xc31bfa0fe5698db8,  -532}, {0xf3e2f893dec3f126,  -529},
                {0x986ddb5c6b3a76b8,  -525}, {0xbe89523386091466,  -522},
                {0xee2ba6c0678b597f,  -519}, {0x94db483840b717f0,  -515},
                {0xba121a4650e4ddec,  -512}, {0xe896a0d7e51e1566,  -509},
                {0x915e2486ef32cd60,  -505}, {0xb5b5ada8aaff80b8,  -502},
                {0xe3231912d5bf60e6,  -499}, {0x8df5efabc5979c90,  -495},
                {0xb1736b96b6fd83b4,  -492}, {0xddd0467c64bce4a1,  -489},
                {0x8aa22c0dbef60ee4,  -485}, {0xad4ab7112eb3929e,  -482},
                {0xd89d64d57a607745,  -479}, {0x87625f056c7c4a8b,  -475},
                {0xa93af6c6c79b5d2e,  -472}, {0xd389b47879823479,  -469},
                {0x843610cb4bf160cc,  -465}, {0xa54394fe1eedb8ff,  -462},
                {0xce947a3da6a9273e,  -459}, {0x811ccc668829b887,  -455},
                {0xa163ff802a3426a9,  -452}, {0xc9bcff6034c13053,  -449},
                {0xfc2c3f3841f17c68,  -446}, {0x9d9ba7832936edc1,  -442},
                {0xc5029163f384a931,  -439}, {0xf64335bcf065d37d,  -436},
                {0x99ea0196163fa42e,  -432}, {0xc06481fb9bcf8d3a,  -429},
                {0xf07da27a82c37088,  -426}, {0x964e858c91ba2655,  -422},
                {0xbbe226efb628afeb,  -419}, {0xeadab0aba3b2dbe5,  -416},
                {0x92c8ae6b464fc96f,  -412}, {0xb77ada0617e3bbcb,  -409},
                {0xe55990879ddcaabe,  -406}, {0x8f57fa54c2a9eab7,  -402},
                {0xb32df8e9f3546564,  -399}, {0xdff9772470297ebd,  -396},
                {0x8bfbea76c619ef36,  -392}, {0xaefae51477a06b04,  -389},
                {0xdab99e59958885c5,  -386}, {0x88b402f7fd75539b,  -382},
                {0xaae103b5fcd2a882,  -379}, {0xd59944a37c0752a2,  -376},
                {0x857fcae62d8493a5,  -372}, {0xa6dfbd9fb8e5b88f,  -369},
                {0xd097ad07a71f26b2,  -366}, {0x825ecc24c8737830,  -362},
                {0xa2f67f2dfa90563b,  -359}, {0xcbb41ef979346bca,  -356},
                {0xfea126b7d78186bd,  -353}, {0x9f24b832e6b0f436,  -349},
                {0xc6ede63fa05d3144,  -346}, {0xf8a95fcf88747d94,  -343},
                {0x9b69dbe1b548ce7d,  -339}, {0xc24452da229b021c,  -336},
                {0xf2d56790ab41c2a3,  -333}, {0x97c560ba6b0919a6,  -329},
                {0xbdb6b8e905cb600f,  -326}, {0xed246723473e3813,  -323},
                {0x9436c0760c86e30c,  -319}, {0xb94470938fa89bcf,  -316},
                {0xe7958cb87392c2c3,  -313}, {0x90bd77f3483bb9ba,  -309},
                {0xb4ecd5f01a4aa828,  -306}, {0xe2280b6c20dd5232,  -303},
                {0x8d590723948a535f,  -299}, {0xb0af48ec79ace837,  -296},
                {0xdcdb1b2798182245,  -293}, {0x8a08f0f8bf0f156b,  -289},
                {0xac8b2d36eed2dac6,  -286}, {0xd7adf884aa879177,  -283},
                {0x86ccbb52ea94baeb,  -279}, {0xa87fea27a539e9a5,  -276},
                {0xd29fe4b18e88640f,  -273}, {0x83a3eeeef9153e89,  -269},
                {0xa48ceaaab75a8e2b,  -266}, {0xcdb02555653131b6,  -263},
                {0x808e17555f3ebf12,  -259}, {0xa0b19d2ab70e6ed6,  -256},
                {0xc8de047564d20a8c,  -253}, {0xfb158592be068d2f,  -250},
                {0x9ced737bb6c4183d,  -246}, {0xc428d05aa4751e4d,  -243},
                {0xf53304714d9265e0,  -240}, {0x993fe2c6d07b7fac,  -236},
                {0xbf8fdb78849a5f97,  -233}, {0xef73d256a5c0f77d,  -230},
                {0x95a8637627989aae,  -226}, {0xbb127c53b17ec159,  -223},
                {0xe9d71b689dde71b0,  -220}, {0x9226712162ab070e,  -216},
                {0xb6b00d69bb55c8d1,  -213}, {0xe45c10c42a2b3b06,  -210},
                {0x8eb98a7a9a5b04e3,  -206}, {0xb267ed1940f1c61c,  -203},
                {0xdf01e85f912e37a3,  -200}, {0x8b61313bbabce2c6,  -196},
                {0xae397d8aa96c1b78,  -193}, {0xd9c7dced53c72256,  -190},
                {0x881cea14545c7575,  -186}, {0xaa242499697392d3,  -183},
                {0xd4ad2dbfc3d07788,  -180}, {0x84ec3c97da624ab5,  -176},
                {0xa6274bbdd0fadd62,  -173}, {0xcfb11ead453994ba,  -170},
                {0x81ceb32c4b43fcf5,  -166}, {0xa2425ff75e14fc32,  -163},
                {0xcad2f7f5359a3b3e,  -160}, {0xfd87b5f28300ca0e,  -157},
                {0x9e74d1b791e07e48,  -153}, {0xc612062576589ddb,  -150},
                {0xf79687aed3eec551,  -147}, {0x9abe14cd44753b53,  -143},
                {0xc16d9a0095928a27,  -140}, {0xf1c90080baf72cb1,  -137},
                {0x971da05074da7bef,  -133}, {0xbce5086492111aeb,  -130},
                {0xec1e4a7db69561a5,  -127}, {0x9392ee8e921d5d07,  -123},
                {0xb877aa3236a4b449,  -120}, {0xe69594bec44de15b,  -117},
                {0x901d7cf73ab0acd9,  -113}, {0xb424dc35095cd80f,  -110},
                {0xe12e13424bb40e13,  -107}, {0x8cbccc096f5088cc,  -103},
                {0xafebff0bcb24aaff,  -100}, {0xdbe6fecebdedd5bf,   -97},
                {0x89705f4136b4a597,   -93}, {0xabcc77118461cefd,   -90},
                {0xd6bf94d5e57a42bc,   -87}, {0x8637bd05af6c69b6,   -83},
                {0xa7c5ac471b478423,   -80}, {0xd1b71758e219652c,   -77},
                {0x83126e978d4fdf3b,   -73}, {0xa3d70a3d70a3d70a,   -70},
                {0xcccccccccccccccd,   -67}, {0x8000000000000000,   -63},
                {0xa000000000000000,   -60}, {0xc800000000000000,   -57},
                {0xfa00000000000000,   -54}, {0x9c40000000000000,   -50},
                {0xc350000000000000,   -47}, {0xf424000000000000,   -44},
                {0x9896800000000000,   -40}, {0xbebc200000000000,   -37},
                {0xee6b280000000000,   -34}, {0x9502f90000000000,   -30},
                {0xba43b74000000000,   -27}, {0xe8d4a51000000000,   -24},
                {0x9184e72a00000000,   -20}, {0xb5e620f480000000,   -17},
                {0xe35fa931a0000000,   -14}, {0x8e1bc9bf04000000,   -10},
                {0xb1a2bc2ec5000000,    -7}, {0xde0b6b3a76400000,    -4},
                {0x8ac7230489e80000,     0}, {0xad78ebc5ac620000,     3},
                {0xd8d726b7177a8000,     6}, {0x878678326eac9000,    10},
                {0xa968163f0a57b400,    13}, {0xd3c21bcecceda100,    16},
                {0x84595161401484a0,    20}, {0xa56fa5b99019a5c8,    23},
                {0xcecb8f27f4200f3a,    26}, {0x813f3978f8940984,    30},
                {0xa18f07d736b90be5,    33}, {0xc9f2c9cd04674edf,    36},
                {0xfc6f7c4045812296,    39}, {0x9dc5ada82b70b59e,    43},
                {0xc5371912364ce305,    46}, {0xf684df56c3e01bc7,    49},
                {0x9a130b963a6c115c,    53}, {0xc097ce7bc90715b3,    56},
                {0xf0bdc21abb48db20,    59}, {0x96769950b50d88f4,    63},
                {0xbc143fa4e250eb31,    66}, {0xeb194f8e1ae525fd,    69},
                {0x92efd1b8d0cf37be,    73}, {0xb7abc627050305ae,    76},
                {0xe596b7b0c643c719,    79}, {0x8f7e32ce7bea5c70,    83},
                {0xb35dbf821ae4f38c,    86}, {0xe0352f62a19e306f,    89},
                {0x8c213d9da502de45,    93}, {0xaf298d050e4395d7,    96},
                {0xdaf3f04651d47b4c,    99}, {0x88d8762bf324cd10,   103},
                {0xab0e93b6efee0054,   106}, {0xd5d238a4abe98068,   109},
                {0x85a36366eb71f041,   113}, {0xa70c3c40a64e6c52,   116},
                {0xd0cf4b50cfe20766,   119}, {0x82818f1281ed44a0,   123},
                {0xa321f2d7226895c8,   126}, {0xcbea6f8ceb02bb3a,   129},
                {0xfee50b7025c36a08,   132}, {0x9f4f2726179a2245,   136},
                {0xc722f0ef9d80aad6,   139}, {0xf8ebad2b84e0d58c,   142},
                {0x9b934c3b330c8577,   146}, {0xc2781f49ffcfa6d5,   149},
                {0xf316271c7fc3908b,   152}, {0x97edd871cfda3a57,   156},
                {0xbde94e8e43d0c8ec,   159}, {0xed63a231d4c4fb27,   162},
                {0x945e455f24fb1cf9,   166}, {0xb975d6b6ee39e437,   169},
                {0xe7d34c64a9c85d44,   172}, {0x90e40fbeea1d3a4b,   176},
                {0xb51d13aea4a488dd,   179}, {0xe264589a4dcdab15,   182},
                {0x8d7eb76070a08aed,   186}, {0xb0de65388cc8ada8,   189},
                {0xdd15fe86affad912,   192}, {0x8a2dbf142dfcc7ab,   196},
                {0xacb92ed9397bf996,   199}, {0xd7e77a8f87daf7fc,   202},
                {0x86f0ac99b4e8dafd,   206}, {0xa8acd7c0222311bd,   209},
                {0xd2d80db02aabd62c,   212}, {0x83c7088e1aab65db,   216},
                {0xa4b8cab1a1563f52,   219}, {0xcde6fd5e09abcf27,   222},
                {0x80b05e5ac60b6178,   226}, {0xa0dc75f1778e39d6,   229},
                {0xc913936dd571c84c,   232}, {0xfb5878494ace3a5f,   235},
                {0x9d174b2dcec0e47b,   239}, {0xc45d1df942711d9a,   242},
                {0xf5746577930d6501,   245}, {0x9968bf6abbe85f20,   249},
                {0xbfc2ef456ae276e9,   252}, {0xefb3ab16c59b14a3,   255},
                {0x95d04aee3b80ece6,   259}, {0xbb445da9ca61281f,   262},
                {0xea1575143cf97227,   265}, {0x924d692ca61be758,   269},
                {0xb6e0c377cfa2e12e,   272}, {0xe498f455c38b997a,   275},
                {0x8edf98b59a373fec,   279}, {0xb2977ee300c50fe7,   282},
                {0xdf3d5e9bc0f653e1,   285}, {0x8b865b215899f46d,   289},
                {0xae67f1e9aec07188,   292}, {0xda01ee641a708dea,   295},
                {0x884134fe908658b2,   299}, {0xaa51823e34a7eedf,   302},
                {0xd4e5e2cdc1d1ea96,   305}, {0x850fadc09923329e,   309},
                {0xa6539930bf6bff46,   312}, {0xcfe87f7cef46ff17,   315},
                {0x81f14fae158c5f6e,   319}, {0xa26da3999aef774a,   322},
                {0xcb090c8001ab551c,   325}, {0xfdcb4fa002162a63,   328},
                {0x9e9f11c4014dda7e,   332}, {0xc646d63501a1511e,   335},
                {0xf7d88bc24209a565,   338}, {0x9ae757596946075f,   342},
                {0xc1a12d2fc3978937,   345}, {0xf209787bb47d6b85,   348},
                {0x9745eb4d50ce6333,   352}, {0xbd176620a501fc00,   355},
                {0xec5d3fa8ce427b00,   358}, {0x93ba47c980e98ce0,   362},
                {0xb8a8d9bbe123f018,   365}, {0xe6d3102ad96cec1e,   368},
                {0x9043ea1ac7e41393,   372}, {0xb454e4a179dd1877,   375},
                {0xe16a1dc9d8545e95,   378}, {0x8ce2529e2734bb1d,   382},
                {0xb01ae745b101e9e4,   385}, {0xdc21a1171d42645d,   388},
                {0x899504ae72497eba,   392}, {0xabfa45da0edbde69,   395},
                {0xd6f8d7509292d603,   398}, {0x865b86925b9bc5c2,   402},
                {0xa7f26836f282b733,   405}, {0xd1ef0244af2364ff,   408},
                {0x8335616aed761f1f,   412}, {0xa402b9c5a8d3a6e7,   415},
                {0xcd036837130890a1,   418}, {0x802221226be55a65,   422},
                {0xa02aa96b06deb0fe,   425}, {0xc83553c5c8965d3d,   428},
                {0xfa42a8b73abbf48d,   431}, {0x9c69a97284b578d8,   435},
                {0xc38413cf25e2d70e,   438}, {0xf46518c2ef5b8cd1,   441},
                {0x98bf2f79d5993803,   445}, {0xbeeefb584aff8604,   448},
                {0xeeaaba2e5dbf6785,   451}, {0x952ab45cfa97a0b3,   455},
                {0xba756174393d88e0,   458}, {0xe912b9d1478ceb17,   461},
                {0x91abb422ccb812ef,   465}, {0xb616a12b7fe617aa,   468},
                {0xe39c49765fdf9d95,   471}, {0x8e41ade9fbebc27d,   475},
                {0xb1d219647ae6b31c,   478}, {0xde469fbd99a05fe3,   481},
                {0x8aec23d680043bee,   485}, {0xada72ccc20054aea,   488},
                {0xd910f7ff28069da4,   491}, {0x87aa9aff79042287,   495},
                {0xa99541bf57452b28,   498}, {0xd3fa922f2d1675f2,   501},
                {0x847c9b5d7c2e09b7,   505}, {0xa59bc234db398c25,   508},
                {0xcf02b2c21207ef2f,   511}, {0x8161afb94b44f57d,   515},
                {0xa1ba1ba79e1632dc,   518}, {0xca28a291859bbf93,   521},
                {0xfcb2cb35e702af78,   524}, {0x9defbf01b061adab,   528},
                {0xc56baec21c7a1916,   531}, {0xf6c69a72a3989f5c,   534},
                {0x9a3c2087a63f6399,   538}, {0xc0cb28a98fcf3c80,   541},
                {0xf0fdf2d3f3c30b9f,   544}, {0x969eb7c47859e744,   548},
                {0xbc4665b596706115,   551}, {0xeb57ff22fc0c795a,   554},
                {0x9316ff75dd87cbd8,   558}, {0xb7dcbf5354e9bece,   561},
                {0xe5d3ef282a242e82,   564}, {0x8fa475791a569d11,   568},
                {0xb38d92d760ec4455,   571}, {0xe070f78d3927556b,   574},
                {0x8c469ab843b89563,   578}, {0xaf58416654a6babb,   581},
                {0xdb2e51bfe9d0696a,   584}, {0x88fcf317f22241e2,   588},
                {0xab3c2fddeeaad25b,   591}, {0xd60b3bd56a5586f2,   594},
                {0x85c7056562757457,   598}, {0xa738c6bebb12d16d,   601},
                {0xd106f86e69d785c8,   604}, {0x82a45b450226b39d,   608},
                {0xa34d721642b06084,   611}, {0xcc20ce9bd35c78a5,   614},
                {0xff290242c83396ce,   617}, {0x9f79a169bd203e41,   621},
                {0xc75809c42c684dd1,   624}, {0xf92e0c3537826146,   627},
                {0x9bbcc7a142b17ccc,   631}, {0xc2abf989935ddbfe,   634},
                {0xf356f7ebf83552fe,   637}, {0x98165af37b2153df,   641},
                {0xbe1bf1b059e9a8d6,   644}, {0xeda2ee1c7064130c,   647},
                {0x9485d4d1c63e8be8,   651}, {0xb9a74a0637ce2ee1,   654},
                {0xe8111c87c5c1ba9a,   657}, {0x910ab1d4db9914a0,   661},
                {0xb54d5e4a127f59c8,   664}, {0xe2a0b5dc971f303a,   667},
                {0x8da471a9de737e24,   671}, {0xb10d8e1456105dad,   674},
                {0xdd50f1996b947519,   677}, {0x8a5296ffe33cc930,   681},
                {0xace73cbfdc0bfb7b,   684}, {0xd8210befd30efa5a,   687},
                {0x8714a775e3e95c78,   691}, {0xa8d9d1535ce3b396,   694},
                {0xd31045a8341ca07c,   697}, {0x83ea2b892091e44e,   701},
                {0xa4e4b66b68b65d61,   704}, {0xce1de40642e3f4b9,   707},
                {0x80d2ae83e9ce78f4,   711}, {0xa1075a24e4421731,   714},
                {0xc94930ae1d529cfd,   717}, {0xfb9b7cd9a4a7443c,   720},
                {0x9d412e0806e88aa6,   724}, {0xc491798a08a2ad4f,   727},
                {0xf5b5d7ec8acb58a3,   730}, {0x9991a6f3d6bf1766,   734},
                {0xbff610b0cc6edd3f,   737}, {0xeff394dcff8a948f,   740},
                {0x95f83d0a1fb69cd9,   744}, {0xbb764c4ca7a44410,   747},
                {0xea53df5fd18d5514,   750}, {0x92746b9be2f8552c,   754},
                {0xb7118682dbb66a77,   757}, {0xe4d5e82392a40515,   760},
                {0x8f05b1163ba6832d,   764}, {0xb2c71d5bca9023f8,   767},
                {0xdf78e4b2bd342cf7,   770}, {0x8bab8eefb6409c1a,   774},
                {0xae9672aba3d0c321,   777}, {0xda3c0f568cc4f3e9,   780},
                {0x8865899617fb1871,   784}, {0xaa7eebfb9df9de8e,   787},
                {0xd51ea6fa85785631,   790}, {0x8533285c936b35df,   794},
                {0xa67ff273b8460357,   797}, {0xd01fef10a657842c,   800},
                {0x8213f56a67f6b29c,   804}, {0xa298f2c501f45f43,   807},
                {0xcb3f2f7642717713,   810}, {0xfe0efb53d30dd4d8,   813},
                {0x9ec95d1463e8a507,   817}, {0xc67bb4597ce2ce49,   820},
                {0xf81aa16fdc1b81db,   823}, {0x9b10a4e5e9913129,   827},
                {0xc1d4ce1f63f57d73,   830}, {0xf24a01a73cf2dcd0,   833},
                {0x976e41088617ca02,   837}, {0xbd49d14aa79dbc82,   840},
                {0xec9c459d51852ba3,   843}, {0x93e1ab8252f33b46,   847},
                {0xb8da1662e7b00a17,   850}, {0xe7109bfba19c0c9d,   853},
                {0x906a617d450187e2,   857}, {0xb484f9dc9641e9db,   860},
                {0xe1a63853bbd26451,   863}, {0x8d07e33455637eb3,   867},
                {0xb049dc016abc5e60,   870}, {0xdc5c5301c56b75f7,   873},
                {0x89b9b3e11b6329bb,   877}, {0xac2820d9623bf429,   880},
                {0xd732290fbacaf134,   883}, {0x867f59a9d4bed6c0,   887},
                {0xa81f301449ee8c70,   890}, {0xd226fc195c6a2f8c,   893},
                {0x83585d8fd9c25db8,   897}, {0xa42e74f3d032f526,   900},
                {0xcd3a1230c43fb26f,   903}, {0x80444b5e7aa7cf85,   907},
                {0xa0555e361951c367,   910}, {0xc86ab5c39fa63441,   913},
                {0xfa856334878fc151,   916}, {0x9c935e00d4b9d8d2,   920},
                {0xc3b8358109e84f07,   923}, {0xf4a642e14c6262c9,   926},
                {0x98e7e9cccfbd7dbe,   930}, {0xbf21e44003acdd2d,   933},
                {0xeeea5d5004981478,   936}, {0x95527a5202df0ccb,   940},
                {0xbaa718e68396cffe,   943}, {0xe950df20247c83fd,   946},
                {0x91d28b7416cdd27e,   950}, {0xb6472e511c81471e,   953},
                {0xe3d8f9e563a198e5,   956}, {0x8e679c2f5e44ff8f,   960},
                {0xb201833b35d63f73,   963}, {0xde81e40a034bcf50,   966},
                {0x8b112e86420f6192,   970}, {0xadd57a27d29339f6,   973},
                {0xd94ad8b1c7380874,   976}, {0x87cec76f1c830549,   980},
                {0xa9c2794ae3a3c69b,   983}, {0xd433179d9c8cb841,   986},
                {0x849feec281d7f329,   990}, {0xa5c7ea73224deff3,   993},
                {0xcf39e50feae16bf0,   996}, {0x81842f29f2cce376,  1000},
                {0xa1e53af46f801c53,  1003}, {0xca5e89b18b602368,  1006},
                {0xfcf62c1dee382c42,  1009}, {0x9e19db92b4e31ba9,  1013},
                {0xc5a05277621be294,  1016}, {0xf70867153aa2db39,  1019},
                {0x9a65406d44a5c903,  1023}, {0xc0fe908895cf3b44,  1026},
                {0xf13e34aabb430a15,  1029}, {0x96c6e0eab509e64d,  1033},
                {0xbc789925624c5fe1,  1036}, {0xeb96bf6ebadf77d9,  1039},
                {0x933e37a534cbaae8,  1043}, {0xb80dc58e81fe95a1,  1046},
                {0xe61136f2227e3b0a,  1049}, {0x8fcac257558ee4e6,  1053},
                {0xb3bd72ed2af29e20,  1056}, {0xe0accfa875af45a8,  1059},
                {0x8c6c01c9498d8b89,  1063}, {0xaf87023b9bf0ee6b,  1066},
                {0xdb68c2ca82ed2a06,  1069}, {0x892179be91d43a44,  1073},
                {0xab69d82e364948d4,  1076}
            };

            return result[index + 343];
        }

        inline char* writeExponent (char* buffer, int k)
        {
            if (k < 0)
            {
                *buffer++ = '-';
                k = -k;
            }

            if (k >= 100)
            {
                *buffer++ = '0' + k / 100;
                k %= 100;

                *buffer++ = '0' + k / 10;
                k %= 10;
            }
            else if (k >= 10)
            {
                *buffer++ = '0' + k / 10;
                k %= 10;
            }

            *buffer++ = '0' + k;

            return buffer;
        }

        inline char* prettify (char* buffer, int length, int k)
        {
            int kk = length + k;

            if ((length <= kk) && (kk <= 21))
            {
                for (int i = length; i < kk; ++i)
                {
                    buffer[i] = '0';
                }
                buffer[kk] = '.';
                buffer[kk + 1] = '0';
                return &buffer[kk + 2];
            }
            else if ((0 < kk) && (kk <= 21))
            {
                memmove (&buffer[kk + 1], &buffer[kk], length - kk);
                buffer[kk] = '.';
                return &buffer[length + 1];
            }
            else if ((-6 < kk) && (kk <= 0))
            {
                int offset = 2 - kk;
                memmove (&buffer[offset], &buffer[0], length);
                buffer[0] = '0';
                buffer[1] = '.';
                for (int i = 2; i < offset; ++i)
                {
                    buffer[i] = '0';
                }
                return &buffer[length + offset];
            }
            else if (length == 1)
            {
                buffer[1] = 'e';
                return writeExponent (&buffer[2], kk - 1);
            }
            else
            {
                memmove (&buffer[2], &buffer[1], length - 1);
                buffer[1] = '.';
                buffer[length + 1] = 'e';
                return writeExponent (&buffer[length + 2], kk - 1);
            }
        }

        inline void grisuRound (char* buffer, int length, uint64_t delta, uint64_t rest, uint64_t ten_kappa, uint64_t wp_w)
        {
            while (rest < wp_w && delta - rest >= ten_kappa && (rest + ten_kappa < wp_w || wp_w - rest > rest + ten_kappa - wp_w))
            {
                --buffer[length - 1];
                rest += ten_kappa;
            }
        }

        inline size_t digitsCount (uint32_t n)
        {
            if (n < 10) return 1;
            if (n < 100) return 2;
            if (n < 1000) return 3;
            if (n < 10000) return 4;
            if (n < 100000) return 5;
            if (n < 1000000) return 6;
            if (n < 10000000) return 7;
            if (n < 100000000) return 8;
            if (n < 1000000000) return 9;
            return 10;
        }

        inline void digitsGen (DiyFp W, DiyFp Mp, uint64_t delta, char* buffer, int& length, int& k)
        {
            static const uint32_t kPow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
            DiyFp one (static_cast <uint64_t> (1) << -Mp._exponent, Mp._exponent);
            DiyFp wp_w = Mp - W;
            uint32_t p1 = static_cast <uint32_t> (Mp._mantissa >> -one._exponent);
            uint64_t p2 = Mp._mantissa & (one._mantissa - 1);
            int kappa = static_cast <int> (digitsCount (p1));
            length = 0;

            while (kappa > 0)
            {
                uint32_t d = 0;
                switch (kappa)
                {
                    case 10: d = p1 / 1000000000; p1 %= 1000000000; break;
                    case  9: d = p1 / 100000000;  p1 %= 100000000;  break;
                    case  8: d = p1 / 10000000;   p1 %= 10000000;   break;
                    case  7: d = p1 / 1000000;    p1 %= 1000000;    break;
                    case  6: d = p1 / 100000;     p1 %= 100000;     break;
                    case  5: d = p1 / 10000;      p1 %= 10000;      break;
                    case  4: d = p1 / 1000;       p1 %= 1000;       break;
                    case  3: d = p1 / 100;        p1 %= 100;        break;
                    case  2: d = p1 / 10;         p1 %= 10;         break;
                    case  1: d = p1;              p1  = 0;          break;
                    default:                                        break;
                }
                if (d || length)
                {
                    buffer[length++] = '0' + d;
                }
                --kappa;
                uint64_t tmp = (static_cast <uint64_t> (p1) << -one._exponent) + p2;
                if (tmp <= delta)
                {
                    k += kappa;
                    grisuRound (buffer, length, delta, tmp, static_cast <uint64_t> (kPow10[kappa]) << -one._exponent, wp_w._mantissa);
                    return;
                }
            }

            uint64_t unit = 1;
            for (;;)
            {
                p2 *= 10; delta *= 10; unit *= 10;
                char d = static_cast <char> (p2 >> -one._exponent);
                if (d || length)
                {
                    buffer[length++] = '0' + d;
                }
                p2 &= one._mantissa - 1;
                --kappa;
                if (p2 < delta)
                {
                    k += kappa;
                    grisuRound (buffer, length, delta, p2, one._mantissa, wp_w._mantissa * unit);
                    return;
                }
            }
        }

        inline int kComputation (int exp, int alpha)
        {
            return ::ceil ((alpha - exp + 63) * 0.30102999566398114);
        }

        inline void grisu2 (char* buffer, int& length, int& k, double value)
        {
            DiyFp val (value), minus, plus;
            val.normalizedBoundaries (minus, plus);

            int mk = kComputation (plus._exponent + 64, -59);
            const DiyFp& c_mk = cachedPower (mk);

            minus *= c_mk;
            plus  *= c_mk;

            ++minus._mantissa;
            --plus._mantissa;

            k = -mk;

            digitsGen (val.normalize () * c_mk, plus, plus._mantissa - minus._mantissa, buffer, length, k);
        }
    }

    /**
     * @brief double to string conversion.
     * @param buffer buffer to write the string representation to.
     * @param value value to convert.
     * @return end position.
     */
    inline char* dtoa (char* buffer, double value)
    {
        if (std::signbit (value))
        {
            *buffer++ = '-';
            value = -value;
        }

        if (value == 0)
        {
            buffer[0] = '0';
            buffer[1] = '.';
            buffer[2] = '0';
            return &buffer[3];
        }
        else
        {
            int length = 0, k = 0;
            details::grisu2 (buffer, length, k, value);
            return details::prettify (buffer, length, k);
        }
    }
}

#endif
