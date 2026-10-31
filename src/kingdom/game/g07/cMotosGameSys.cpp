#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>
#include <limits>

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cParamPlayerLevel.hpp"
#include "kingdom/game/g07/cParamVS.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/MO_KEEP_DATA.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"
#include "kingdom/game/g07/MOTOS_VS_SETTING.hpp"

namespace kingdom::game::g07
{

const uint cMotosGameSys::MOTOS_RAND_TBL[512] = {1067595299u, 955945823u, 477289528u, 4107218783u, 4228976476u, 3344332714u, 3355579695u, 227628506u, 810200273u, 2591290167u, 2560260675u, 3242736208u, 646746669u, 1479517882u, 4245472273u, 1143372638u, 3863670494u, 3221021970u, 1773610557u, 1138697238u, 1421897700u, 1269916527u, 2859934041u, 1764463362u, 3874892047u, 3965319921u, 72549643u, 2383988930u, 2600218693u, 3237492380u, 2792901476u, 725331109u, 605841842u, 271258942u, 715137098u, 3297999536u, 1322965544u, 4229579109u, 1395091102u, 3735697720u, 2101727825u, 3730287744u, 2950434330u, 1661921839u, 2895579582u, 2370511479u, 1004092106u, 2247096681u, 2111242379u, 3237345263u, 4082424759u, 219785033u, 2454039889u, 3709582971u, 835606218u, 2411949883u, 2735205030u, 756421180u, 2175209704u, 1873865952u, 2762534237u, 4161807854u, 3351099340u, 181129879u, 3269891896u, 776029799u, 2218161979u, 3001745796u, 1866825872u, 2133627728u, 34862734u, 1191934573u, 3102311354u, 2916517763u, 1012402762u, 2184831317u, 4257399449u, 2899497138u, 3818095062u, 3030756734u, 1282161629u, 420003642u, 2326421477u, 2741455717u, 1278020671u, 3744179621u, 271777016u, 2626330018u, 2560563991u, 3055977700u, 4233527566u, 1228397661u, 3595579322u, 1077915006u, 2395931898u, 1851927286u, 3013683506u, 1999971931u, 3006888962u, 1049781534u, 1488758959u, 3491776230u, 104418065u, 2448267297u, 3075614115u, 3872332600u, 891912190u, 3936547759u, 2269180963u, 2633455084u, 1047636807u, 2604612377u, 2709305729u, 1952216715u, 207593580u, 2849898034u, 670771757u, 2210471108u, 467711165u, 263046873u, 3569667915u, 1042291111u, 3863517079u, 1464270005u, 2758321352u, 3790799816u, 2301278724u, 3106281430u, 7974801u, 2792461636u, 555991332u, 621766759u, 1322453093u, 853629228u, 686962251u, 1455120532u, 957753161u, 1802033300u, 1021534190u, 3486047311u, 1902128914u, 3701138056u, 4176424663u, 1795608698u, 560858864u, 3737752754u, 3141170998u, 1553553385u, 3367807274u, 711546358u, 2475125503u, 262969859u, 251416325u, 2980076994u, 1806565895u, 969527843u, 3529327173u, 2736343040u, 2987196734u, 1649016367u, 2206175811u, 3048174801u, 3662503553u, 3138851612u, 2660143804u, 1663017612u, 1816683231u, 411916003u, 3887461314u, 2347044079u, 1015311755u, 1203592432u, 2170947766u, 2569420716u, 813872093u, 1105387678u, 1431142475u, 220570551u, 4243632715u, 4179591855u, 2607469131u, 3090613241u, 282341803u, 1734241730u, 1391822177u, 1001254810u, 827927915u, 1886687171u, 3935097347u, 2631788714u, 3905163266u, 110554195u, 2447955646u, 3717202975u, 3304793075u, 3739614479u, 3059127468u, 953919171u, 2590123714u, 1132511021u, 3795593679u, 2788030429u, 982155079u, 3472349556u, 859942552u, 2681007391u, 2299624053u, 647443547u, 233600422u, 608168955u, 3689327453u, 1849778220u, 1608438222u, 3968158357u, 2692977776u, 2851872572u, 246750393u, 3582818628u, 3329652309u, 4036366910u, 1012970930u, 950780808u, 3959768744u, 2538550045u, 191422718u, 2658142375u, 3276369011u, 2927737484u, 1234200027u, 1920815603u, 3536074689u, 1535612501u, 2184142071u, 3276955054u, 428488088u, 2378411984u, 4059769550u, 3913744741u, 2732139246u, 64369859u, 3755670074u, 842839565u, 2819894466u, 2414718973u, 1010060670u, 1839715346u, 2410311136u, 152774329u, 3485009480u, 4102101512u, 2852724304u, 879944024u, 1785007662u, 2748284463u, 1354768064u, 3267784736u, 2269127717u, 3001240761u, 3179796763u, 895723219u, 865924942u, 4291570937u, 89355264u, 1471026971u, 4114180745u, 3201939751u, 2867476999u, 2460866060u, 3603874571u, 2238880432u, 3308416168u, 2072246611u, 2755653839u, 3773737248u, 1709066580u, 4282731467u, 2746170170u, 2832568330u, 433439009u, 3175778732u, 26248366u, 2551382801u, 183214346u, 3893339516u, 1928168445u, 1337157619u, 3429096554u, 3275170900u, 1782047316u, 4264403756u, 1876594403u, 4289659572u, 3223834894u, 1728705513u, 4068244734u, 2867840287u, 1147798696u, 302879820u, 1730407747u, 1923824407u, 1180597908u, 1569786639u, 198796327u, 560793173u, 2107345620u, 2705990316u, 3448772106u, 3678374155u, 758635715u, 884524671u, 486356516u, 1774865603u, 3881226226u, 2635213607u, 1181121587u, 1508809820u, 3178988241u, 1594193633u, 1235154121u, 326117244u, 2304031425u, 937054774u, 2687415945u, 3192389340u, 2003740439u, 1823766188u, 2759543402u, 10067710u, 1533252662u, 4132494984u, 82378136u, 420615890u, 3467563163u, 541562091u, 3535949864u, 2277319197u, 3330822853u, 3215654174u, 4113831979u, 4204996991u, 2162248333u, 3255093522u, 2219088909u, 2978279037u, 255818579u, 2859348628u, 3097280311u, 2569721123u, 1861951120u, 2907080079u, 2719467166u, 998319094u, 2521935127u, 2404125338u, 259456032u, 2086860995u, 1839848496u, 1893547357u, 2527997525u, 1489393124u, 2860855349u, 76448234u, 2264934035u, 744914583u, 2586791259u, 1385380501u, 66529922u, 1819103258u, 1899300332u, 2098173828u, 1793831094u, 276463159u, 360132945u, 4178212058u, 595015228u, 177071838u, 2800080290u, 1573557746u, 1548998935u, 378454223u, 1460534296u, 1116274283u, 3112385063u, 3709761796u, 827999348u, 3580042847u, 1913901014u, 614021289u, 4278528023u, 1905177404u, 45407939u, 3298183234u, 1184848810u, 3644926330u, 3923635459u, 1627046213u, 3677876759u, 969772772u, 1160524753u, 1522441192u, 452369933u, 1527502551u, 832490847u, 1003299676u, 1071381111u, 2891255476u, 973747308u, 4086897108u, 1847554542u, 3895651598u, 2227820339u, 1621250941u, 2881344691u, 3583565821u, 3510404498u, 849362119u, 862871471u, 797858058u, 2867774932u, 2821282612u, 3272403146u, 3997979905u, 209178708u, 1805135652u, 6783381u, 2823361423u, 792580494u, 4263749770u, 776439581u, 3798193823u, 2853444094u, 2729507474u, 1071873341u, 1329010206u, 1289336450u, 3327680758u, 2011491779u, 80157208u, 922428856u, 1158943220u, 1667230961u, 2461022820u, 2608845159u, 387516115u, 3345351910u, 1495629111u, 4098154157u, 3156649613u, 3525698599u, 4134908037u, 446713264u, 2137537399u, 3617403512u, 813966752u, 1157943946u, 3734692965u, 1680301658u, 3180398473u, 3509854711u, 2228114612u, 1008102291u, 486805123u, 863791847u, 3189125290u, 1050308116u, 3777341526u, 4291726501u, 844061465u, 1347461791u, 2826481581u, 745465012u, 2055805750u, 4260209475u, 2386693097u, 2980646741u, 447229436u, 2077782664u, 1232942813u, 4023002732u, 1399011509u, 3140569849u, 2579909222u, 3794857471u, 900758066u, 2887199683u, 1720257997u, 3367494931u, 2668921229u, 955539029u, 3818726432u, 1105704962u, 3889207255u, 2277369307u, 2746484505u, 1761846513u, 2413916784u, 2685127085u, 4240257943u, 1166726899u, 4215215715u, 3082092067u, 3960461946u, 1663304043u, 2087473241u, 4162589986u, 2507310778u, 1579665506u, 767234210u, 970676017u, 492207530u, 1441679602u, 1314785090u};
int cMotosGameSys::m_nInstance = 0;
bool cMotosGameSys::m_nIdentityCard{};

int cMotosGameSys::get_DeadEnemyCnt()
{
	return m_deadEnemyCnt;
}

int cMotosGameSys::get_GetItemCnt()
{
	return m_getItemCnt;
}

cMotosGameSys::cMotosGameSys()
	
{
	Init();
}

void cMotosGameSys::Free()
{
	Term();
}

uint cMotosGameSys::ID()
{
	return m_nID;
}

void cMotosGameSys::ID(uint n)
{
	m_nID = n;
}

uint& cMotosGameSys::SyncCounter()
{
	return m_nSyncCounter;
}

uint& cMotosGameSys::GameCounter()
{
	return m_nGameCounter;
}

uint& cMotosGameSys::SyncCounterLocal()
{
	return m_nSyncCounterLocal;
}

uint& cMotosGameSys::GameCounterLocal()
{
	return m_nGameCounterLocal;
}

MO_KEEP_DATA** cMotosGameSys::KeepData()
{
	return m_KeepData;
}

uint cMotosGameSys::RandMax()
{
	return std::numeric_limits<uint>::max();
}

uint cMotosGameSys::GameKind()
{
	return m_nGameKind;
}

uint cMotosGameSys::TestMasterFlag(uint flag)
{
	return m_nMGFlagRecieve & flag;
}

bool cMotosGameSys::bTestMasterFlag(uint flag)
{
	return (m_nMGFlagRecieve & flag) != 0;
}

void cMotosGameSys::OnMasterFlag(uint flag)
{
	if (m_nID == 0)
	{
		m_nMGFlagSend |= flag;
	}
}

void cMotosGameSys::OffMasterFlag(uint flag)
{
	if (m_nID == 0)
	{
		m_nMGFlagSend &= ~flag;
	}
}

void cMotosGameSys::XchgMasterFlag(uint flag)
{
	if (m_nID == 0)
	{
		m_nMGFlagSend ^= flag;
	}
}

uint cMotosGameSys::TestSlaveFlag(int id, uint flag)
{
	return m_nSGFlagRecieve[id] & flag;
}

bool cMotosGameSys::bTestSlaveFlag(int id, uint flag)
{
	return (m_nSGFlagRecieve[id] & flag) != 0;
}

void cMotosGameSys::OnSlaveFlag(uint id, uint flag)
{
	if (m_nID == id)
	{
		m_nSGFlagSend |= flag;
	}
}

void cMotosGameSys::OffSlaveFlag(uint id, uint flag)
{
	if (m_nID == id)
	{
		m_nSGFlagSend &= ~flag;
	}
}

void cMotosGameSys::XchgSlaveFlag(uint id, uint flag)
{
	if (m_nID == id)
	{
		m_nSGFlagSend ^= flag;
	}
}

MOTOS_VS_PARAM*& cMotosGameSys::GetVsParam()
{
	return m_MoVSParam;
}

float* cMotosGameSys::pAxisAttack()
{
	return m_AxisAttack;
}

int& cMotosGameSys::nGameLevel()
{
	return m_nGameLevel;
}

uint cMotosGameSys::JoinFlag()
{
	return m_nJoinFlag;
}

void cMotosGameSys::JoinFlag(uint n)
{
	m_nJoinFlag = n;
}

int& cMotosGameSys::nStartMode()
{
	return m_nStartMode;
}

MOTOS_VS_PARAM* cMotosGameSys::pVSParam()
{
	return m_MoVSParam;
}

MOTOS_VS_SETTING* cMotosGameSys::pVSSetting()
{
	return m_VSSetting;
}

bool& cMotosGameSys::bVSSharing()
{
	return m_bVSSharing;
}

sbyte& cMotosGameSys::nVSStage(int no)
{
	return m_VSSetting->m_nStage[no];
}

sbyte& cMotosGameSys::nVSSharingMap()
{
	return m_VSSetting->m_nSharingMap;
}

ushort& cMotosGameSys::nVSLimitTime()
{
	return m_VSSetting->m_nLimitTime;
}

sbyte& cMotosGameSys::nVSLimitNo()
{
	return m_VSSetting->m_nLimitTimeNo;
}

byte& cMotosGameSys::nVSWinNum(int num)
{
	return m_VSSetting->m_nWinNum[num];
}

byte& cMotosGameSys::nVSColor(int num)
{
	return m_VSSetting->m_nColor[num];
}

byte& cMotosGameSys::nVSItemNum()
{
	return m_VSSetting->m_nItemNum;
}

byte& cMotosGameSys::nVSRoundWinNum(int num)
{
	return m_VSSetting->m_nRoundWinNum[num];
}

byte& cMotosGameSys::nVSSPCount(int num)
{
	return m_VSSetting->m_nSPCount[num];
}

sbyte& cMotosGameSys::nVSSPLimit()
{
	return m_VSSetting->m_nSPLimit;
}

float* cMotosGameSys::SysPowerLevel2Power()
{
	return m_PowerLevel2Power;
}

short* cMotosGameSys::SysHoverLevel2Time()
{
	return m_HoverLevel2Time;
}

short* cMotosGameSys::SysChargeLevel2Time()
{
	return m_ChargeLevel2Time;
}

float* cMotosGameSys::SysChargeLevel2Power()
{
	return m_ChargeLevel2Power;
}

float* cMotosGameSys::SysChargeLevel2MyRush()
{
	return m_ChargeLevel2MyRush;
}

float* cMotosGameSys::SysChargeLevel2Heretic()
{
	return m_ChargeLevel2Heretic;
}

float* cMotosGameSys::SysChargeLevel2Speed()
{
	return m_ChargeLevel2Speed;
}

short* cMotosGameSys::SysChargeLevel2Weight()
{
	return m_ChargeLevel2Weight;
}

short* cMotosGameSys::SysChargeLevel2PiyoTime()
{
	return m_ChargeLevel2PiyoTime;
}

MO_CHAR_PARAMS*& cMotosGameSys::SysMoPlayerParam()
{
	return m_MoPlayerParam;
}

MO_CHAR_PARAMS** cMotosGameSys::SysMoEnemyParam()
{
	return m_MoEnemyParam;
}

void cMotosGameSys::ResetDeadEnemyCnt()
{
	m_deadEnemyCnt = 0;
}

void cMotosGameSys::AddDeadEnemyCnt(int add)
{
	m_deadEnemyCnt += add;
}

void cMotosGameSys::ResetGetItemCnt()
{
	m_getItemCnt = 0;
}

void cMotosGameSys::AddGetItemCnt(int add)
{
	m_getItemCnt += add;
}

uint cMotosGameSys::TestSlaveFlagAll(uint flag)
{
	return std::numeric_limits<uint>::max();
}

uint cMotosGameSys::TestSlaveFlagSomebody(uint flag)
{
	return 0u;
}

// DOL 0x800a1400 [audited: per AUDIT_G03_G07 row] (ctor body inside the 0xe3c-byte MotosSys block, job init 0x800a1e70): keep data life 3, VS defaults (sel color 0, sharing map -1,
// item / round / time index 3, colours 0..3, selected stages -2, limit 1800 frames), then NeedInit; the RSAR group load (0x80137648) is the sound layer (E1)
void cMotosGameSys::Init()
{
	CustomKingdomUtil::Assert(m_nInstance++ == 0, "fatal error");
	m_nGameKind = 0u;
	m_nMGFlagRecieve = 0u;
	m_nGameLevel = 0;
	m_nMGFlagRecieve = 0u;
	m_nMGFlagSend = 0u;
	m_nSGFlagSend = 0u;
	m_nID = 0u;
	m_nSyncCounter = 0u;
	m_nGameCounter = 0u;
	m_nSyncCounterLocal = 0u;
	m_nGameCounterLocal = 0u;
	std::fill(std::begin(m_nSGFlagRecieve), std::end(m_nSGFlagRecieve), 0u);
	m_nJoinFlag = 1u;
	m_nMGFlagRecieve = 0u;
	m_nMGFlagSend = 0u;
	m_nSGFlagSend = 0u;
	std::fill(std::begin(m_nSGFlagRecieve), std::end(m_nSGFlagRecieve), 0u);
	m_nVSRound = 0u;
	m_nGameKind = 0u;
	m_nStartMode = 0;
	m_nGameLevel = 0;
	m_nPacketSndRev = false;
	m_nLoadedData = false;
	m_nSharingDone = false;
	m_bVSSharing = false;
	for (int i = 0; i < (int)std::size(m_KeepData); i++)
	{
		m_KeepData[i] = new MO_KEEP_DATA();
	}
	m_KeepData[0]->m_nLife = 3;
	m_VSSetting = new MOTOS_VS_SETTING();
	m_VSSetting->m_nSelColor = 0;
	m_VSSetting->m_nSharingMap = -1;
	m_VSSetting->m_nItemNumNo = 3;
	m_VSSetting->m_nRoundNum = 3;
	m_VSSetting->m_nLimitTimeNo = 3;
	for (int j = 0; j < 4; j++)
	{
		m_VSSetting->m_nWinNum[j] = 0;
		m_VSSetting->m_nRoundWinNum[j] = 0;
		m_VSSetting->m_nColor[j] = (byte)j;
		m_VSSetting->m_nSPCount[j] = 0;
	}
	for (int k = 0; k < 3; k++)
	{
		m_VSSetting->m_nStage[k] = 0;
		m_VSSetting->m_nSelStage[k] = -2;
	}
	m_VSSetting->m_nItemNum = 0;
	m_VSSetting->m_nLimitTime = (ushort)Constants::FPS_TIME(30.0f);
	m_VSSetting->m_nRestRound = 0;
	NeedInit();
}

void cMotosGameSys::Term()
{
	CustomKingdomUtil::Assert(--m_nInstance == 0, "fatal error");
}

// DOL 0x800a1570 [audited: per AUDIT_G03_G07 row] (also called by the game Init 0x800a4288): it also zeroes the enemy / player parameter blocks (+0x1c0 / +0xe4), which the game
// Init reloads right after (all 27 kinds), so the port keeps its pointers
void cMotosGameSys::NeedInit()
{
	m_nID = 0u;
	m_nSyncCounter = 0u;
	m_nGameCounter = 0u;
	m_nSyncCounterLocal = 0u;
	m_nGameCounterLocal = 0u;
	std::fill(std::begin(m_nSGFlagRecieve), std::end(m_nSGFlagRecieve), 0u);
	cParamPlayerLevel* cParamPlayerLevel2 = new cParamPlayerLevel();
	cParamPlayerLevel2->RegistData("CSV_PLAYER_LEVEL");
	Constants::MOTOS_CAUTION("Init");
	for (int i = 0; i < cParamPlayerLevel2->MaxLevel(); i++)
	{
		m_PowerLevel2Power[i] = cParamPlayerLevel2->PowerLevel2Power(i);
		m_HoverLevel2Time[i] = cParamPlayerLevel2->HoverLevel2Time(i);
		m_ChargeLevel2Time[i] = cParamPlayerLevel2->ChargeLevel2Time(i);
		m_ChargeLevel2Power[i] = cParamPlayerLevel2->ChargeLevel2Power(i);
		m_ChargeLevel2MyRush[i] = cParamPlayerLevel2->ChargeLevel2MyRush(i);
		m_ChargeLevel2Heretic[i] = cParamPlayerLevel2->ChargeLevel2Heretic(i);
		m_ChargeLevel2Speed[i] = cParamPlayerLevel2->ChargeLevel2Speed(i);
		m_ChargeLevel2Weight[i] = cParamPlayerLevel2->ChargeLevel2Weight(i);
		m_ChargeLevel2PiyoTime[i] = cParamPlayerLevel2->ChargeLevel2PiyoTime(i);
	}
	cParamVS* cParamVS2 = new cParamVS();
	cParamVS2->RegistData("CSV_VS_PARAM");
	m_MoVSParam = cParamVS2->pVSInfo();
}

void cMotosGameSys::MultiDataInit()
{
}

uint cMotosGameSys::RandU(uint index)
{
	return MOTOS_RAND_TBL[index & 0x1FF];
}

float cMotosGameSys::RandF(uint index)
{
	return 1.0f / (float)RandMax() * (float)RandU(index);
}

uint cMotosGameSys::RandRangeU(uint min, uint max, uint index)
{
	float num = (float)(max - min) / 4.2949673E+09f;
	uint num2 = RandU(index);
	return min + (uint)((float)num2 * num);
}

float cMotosGameSys::RandRangeF(float min, float max, uint index)
{
	float num = (max - min) / 1.0f;
	float num2 = RandF(index);
	return min + num2 * num;
}

bool cMotosGameSys::IsGameSysMyChar(cTaskChar* p)
{
	if (!p->bTestFlag(2u))
	{
		if (!p->bTestFlag(2u))
		{
			return Constants::IS_MASTER();
		}
		return false;
	}
	return true;
}

bool cMotosGameSys::IsGameSysLanPlay()
{
	return (GameKind() & 4) != 0;
}

bool cMotosGameSys::IsGameSysMaster()
{
	return true;
}

void cMotosGameSys::NET_Sync()
{
	if (!Constants::IS_LANPLAY())
	{
		if (m_nID == 0)
		{
			m_nMGFlagRecieve = m_nMGFlagSend;
		}
		m_nSGFlagRecieve[m_nID] = m_nSGFlagSend;
	}
}

}
