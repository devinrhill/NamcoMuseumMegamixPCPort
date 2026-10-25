#include "kingdom/game/g07/Constants.hpp"

#include <random>
#include "kingdom/game/g07/cTaskMap.hpp"

#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cAppG07MotosGameOver.hpp"
#include "kingdom/game/g07/cAppG07MotosMatching.hpp"
#include "kingdom/game/g07/cAppG07MotosMenu.hpp"
#include "kingdom/game/g07/cAppG07MotosMultiMenu.hpp"
#include "kingdom/game/g07/cAppG07MotosStaffRoll.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cParamPlayer.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskPanel.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <InGamePadManager.hpp>

namespace kingdom::game::g07
{

cAppG07MotosMenu* Constants::m_motosMenu = nullptr;
cAppG07MotosMultiMenu* Constants::m_motosMultiMenu = nullptr;
cAppG07MotosGame* Constants::m_motosGame = nullptr;
cAppG07MotosGameOver* Constants::m_motosGameOver = nullptr;
cAppG07MotosStaffRoll* Constants::m_motosStaffRoll = nullptr;
cAppG07MotosMatching* Constants::m_motosMatching = nullptr;
const uint Constants::MULTI_ITEM_LIST[6] = {5u, 10u, 15u, 20u, 25u, 30u};
const uint Constants::MULTI_TIME_LIST[5] = {30000u, 60000u, 90000u, 120000u, 150000u};
const uint Constants::MULTI_ROUND_LIST[4] = {1u, 2u, 3u, 10u};
const uint Constants::g_aMotosStage2RebirthTime[13][2] = {{(uint)Constants::FPS_TIME(5.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(5.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(5.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(5.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(5.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(5.0f), (uint)Constants::FPS_TIME(15.0f)}, {(uint)Constants::FPS_TIME(3.0f), (uint)Constants::FPS_TIME(15.0f)}};
const Vector4 Constants::s_Vec[9] = {Vector4(0.0f, 0.0f, 1.0f, 0.0f), Vector4(0.7071f, 0.0f, 0.7071f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 0.0f), Vector4(0.7071f, 0.0f, -0.7071f, 0.0f), Vector4(0.0f, 0.0f, -1.0f, 0.0f), Vector4(-0.7071f, 0.0f, -0.7071f, 0.0f), Vector4(-1.0f, 0.0f, 0.0f, 0.0f), Vector4(-0.7071f, 0.0f, 0.7071f, 0.0f), Vector4(0.0f, 0.0f, 0.0f, 0.0f)};
const uint Constants::s_TransFlagTbl[6] = {1024u, 64u, 32u, 524288u, 2097152u, 0u};

uint Constants::get_MO_SELECT_DEFAULT_COLOR_1P()
{
	return ::Constants::JET_RGBA8888(20u, 100u, 20u, 160u);
}

uint Constants::get_MO_SELECT_DEFAULT_COLOR_2P()
{
	return ::Constants::JET_RGBA8888(20u, 100u, 20u, 160u);
}

uint Constants::get_MO_SELECT_DEFAULT_COLOR_3P()
{
	return ::Constants::JET_RGBA8888(20u, 100u, 20u, 160u);
}

uint Constants::get_MO_SELECT_DEFAULT_COLOR_4P()
{
	return ::Constants::JET_RGBA8888(20u, 100u, 20u, 160u);
}

MOTOS_VS_PARAM* Constants::get_g_MoVSParam()
{
	return KingdomGlobal::pMotosGameSys->GetVsParam();
}

float* Constants::get_g_AxisAttack()
{
	return KingdomGlobal::pMotosGameSys->pAxisAttack();
}

void Constants::set_g_AxisAttack(float* value)
{
	std::copy(value, value + 6, KingdomGlobal::pMotosGameSys->pAxisAttack());
}

float* Constants::get_g_PowerLevel2Power()
{
	return KingdomGlobal::pMotosGameSys->SysPowerLevel2Power();
}

short* Constants::get_g_HoverLevel2Time()
{
	return KingdomGlobal::pMotosGameSys->SysHoverLevel2Time();
}

short* Constants::get_g_ChargeLevel2Time()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2Time();
}

float* Constants::get_g_ChargeLevel2Power()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2Power();
}

float* Constants::get_g_ChargeLevel2MyRush()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2MyRush();
}

float* Constants::get_g_ChargeLevel2Heretic()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2Heretic();
}

float* Constants::get_g_ChargeLevel2Speed()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2Speed();
}

short* Constants::get_g_ChargeLevel2Weight()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2Weight();
}

short* Constants::get_g_ChargeLevel2PiyoTime()
{
	return KingdomGlobal::pMotosGameSys->SysChargeLevel2PiyoTime();
}

MO_CHAR_PARAMS* Constants::get_g_MoPlayerParam()
{
	return KingdomGlobal::pMotosGameSys->SysMoPlayerParam();
}

void Constants::set_g_MoPlayerParam(MO_CHAR_PARAMS* value)
{
	KingdomGlobal::pMotosGameSys->SysMoPlayerParam() = value;
}

MO_CHAR_PARAMS** Constants::get_g_MoEnemyParam()
{
	return KingdomGlobal::pMotosGameSys->SysMoEnemyParam();
}

uint Constants::get_AXIS_ATTACK_SIZE()
{
	return 24u;
}

int Constants::get_MO_VS_ENEMY_TOUCH()
{
	return get_g_MoVSParam()->m_nEnemyTouch;
}

int Constants::get_MO_VS_MOTOS_TOUCH()
{
	return get_g_MoVSParam()->m_nMotosTouch;
}

int Constants::get_MO_VS_LAND_POINT_FRAME()
{
	return get_g_MoVSParam()->m_nLPFrame;
}

int Constants::get_MO_VS_LAND_POINT_POINT()
{
	return get_g_MoVSParam()->m_nLPPoint;
}

float Constants::get_MO_VS_ROB_POINT_RATE()
{
	return get_g_MoVSParam()->m_fRobPoint;
}

float Constants::get_MO_VS_FALL_POINT_RATE()
{
	return get_g_MoVSParam()->m_fFallPoint;
}

int Constants::get_MO_VS_POINT_DECEL_POINT()
{
	return get_g_MoVSParam()->m_nPDPoint;
}

float Constants::get_MO_VS_POINT_DECEL_DECEL()
{
	return get_g_MoVSParam()->m_fPDDecel;
}

int Constants::get_MO_VS_PANEL_REBIRTH()
{
	return Constants::FPS_TIME(get_g_MoVSParam()->m_fPanelRebirth);
}

int Constants::get_MO_VS_PLAYER_MUTEKI()
{
	return Constants::FPS_TIME(get_g_MoVSParam()->m_fPlayerMuteki);
}

int Constants::get_MO_VS_SP_CHARGE_POINT()
{
	return get_g_MoVSParam()->m_nSPCharge;
}

int Constants::get_MO_VS_SP_PIYO_ENEMY()
{
	return get_g_MoVSParam()->m_nSPPiyoEnemy;
}

int Constants::get_MO_VS_SP_PIYO_SELF()
{
	return get_g_MoVSParam()->m_nSPPiyoMySelf;
}

uint Constants::get_SYNC_COUNTER()
{
	return KingdomGlobal::pMotosGameSys->SyncCounter();
}

uint Constants::get_GAME_COUNTER()
{
	return KingdomGlobal::pMotosGameSys->GameCounter();
}

uint Constants::get_SYNC_COUNTER_LOCAL()
{
	return KingdomGlobal::pMotosGameSys->SyncCounterLocal();
}

uint Constants::get_GAME_COUNTER_LOCAL()
{
	return KingdomGlobal::pMotosGameSys->GameCounterLocal();
}

void Constants::set_SYNC_COUNTER(uint v) { KingdomGlobal::pMotosGameSys->SyncCounter() = v; }
void Constants::set_GAME_COUNTER(uint v) { KingdomGlobal::pMotosGameSys->GameCounter() = v; }
void Constants::set_SYNC_COUNTER_LOCAL(uint v) { KingdomGlobal::pMotosGameSys->SyncCounterLocal() = v; }
void Constants::set_GAME_COUNTER_LOCAL(uint v) { KingdomGlobal::pMotosGameSys->GameCounterLocal() = v; }

int Constants::get_STAGE_SELECT_NO()
{
	return KingdomGlobal::MotosSys->nForceVsRoundCount() - KingdomGlobal::MotosSys->nVsRoundCount();
}

uint Constants::get_MO_HITFLAG_REFLECT()
{
	return 1u;
}

uint Constants::get_MO_HITFLAG_REPEL()
{
	return 2u;
}

void Constants::Kingdom_G07_Init_ChangeInit()
{
	Constants::Motos_System_Init();
	kingdom::Constants::Kingdom_SetJobNo(287309824u);
}

void Constants::Kingdom_G07_Term_ChangeInit()
{
	Constants::Motos_System_Term();
	kingdom::Constants::Kingdom_SetJobNo(144703488u);
}

void Constants::Kingdom_G07_Menu_ChangeInit()
{
	m_motosMenu = new cAppG07MotosMenu();
	KingdomGlobal::SysCore->RegistMainTask(m_motosMenu, Constants::IsExecGCCollect());
}

void Constants::Kingdom_G07_Menu_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_motosMenu->Free();
	m_motosMenu = nullptr;
}

void Constants::Kingdom_G07_MultiMenu_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_motosMultiMenu = new cAppG07MotosMultiMenu(), Constants::IsExecGCCollect());
}

void Constants::Kingdom_G07_MultiMenu_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_motosMultiMenu->Free();
	m_motosMultiMenu = nullptr;
}

void Constants::Kingdom_G07_Game_ChangeInit()
{
	m_motosGame = new cAppG07MotosGame();
	KingdomGlobal::SysCore->RegistMainTask(m_motosGame);
}

void Constants::Kingdom_G07_Game_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_motosGame->Free();
	m_motosGame = nullptr;
}

void Constants::Kingdom_G07_GameOver_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_motosGameOver = new cAppG07MotosGameOver());
}

void Constants::Kingdom_G07_GameOver_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_motosGameOver->Free();
	m_motosGameOver = nullptr;
}

void Constants::Kingdom_G07_StaffRoll_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_motosStaffRoll = new cAppG07MotosStaffRoll());
}

void Constants::Kingdom_G07_StaffRoll_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_motosStaffRoll->Free();
	m_motosStaffRoll = nullptr;
}

void Constants::Kingdom_G07_Matching_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_motosMatching = new cAppG07MotosMatching(), Constants::IsExecGCCollect());
}

void Constants::Kingdom_G07_Matching_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_motosMatching->Free();
	m_motosMatching = nullptr;
}

void Constants::Kingdom_G07_Nunchuk_ChangeInit()
{
}

void Constants::Kingdom_G07_Nunchuk_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
}

void Constants::ReleaseMotos()
{
	if (m_motosMenu != nullptr)
	{
		m_motosMenu->Free();
	}
	m_motosMenu = nullptr;
	if (m_motosMatching != nullptr)
	{
		m_motosMatching->Free();
	}
	m_motosMatching = nullptr;
	if (m_motosMultiMenu != nullptr)
	{
		m_motosMultiMenu->Free();
	}
	m_motosMultiMenu = nullptr;
	if (m_motosGame != nullptr)
	{
		m_motosGame->Free();
	}
	m_motosGame = nullptr;
	if (m_motosGameOver != nullptr)
	{
		m_motosGameOver->Free();
	}
	m_motosGameOver = nullptr;
	if (m_motosStaffRoll != nullptr)
	{
		m_motosStaffRoll->Free();
	}
	m_motosStaffRoll = nullptr;
}

bool Constants::IsExecGCCollect()
{
	if (KingdomGlobal::SysCore->GetLastJobNo() != 287309824 && KingdomGlobal::SysCore->GetLastJobNo() != 291504128)
	{
		return KingdomGlobal::SysCore->GetLastJobNo() != 292552704;
	}
	return false;
}

void Constants::MOTOS_CAUTION(string str)
{
}

void Constants::MOTOS_NO_SOURCE(string str)
{
}

int Constants::MO_VS_REBIRTH_LIMIT()
{
	return Constants::FPS_TIME(5.0f);
}

void Constants::Motos_System_Init()
{
	CustomKingdomUtil::Assert(KingdomGlobal::MotosSys == nullptr, "multiple init");
	KingdomGlobal::MotosSys = new cG07MotosSys();
}

void Constants::Motos_System_Term()
{
	if (KingdomGlobal::MotosSys != nullptr)
	{
		KingdomGlobal::MotosSys->Free();
		KingdomGlobal::MotosSys = nullptr;
	}
}

cG07MotosSys* Constants::Motos_System()
{
	return KingdomGlobal::MotosSys;
}

namespace
{
// `new System.Random().Next()`: a non-negative int from a fresh time seeded generator
uint SysRandomNext()
{
	static std::mt19937 gen{std::random_device{}()};
	return (uint)(gen() & 0x7fffffffu);
}
}

uint Constants::MOTOS_RANDU()
{
	return KingdomGlobal::pMotosGameSys->RandU(SysRandomNext());
}

float Constants::MOTOS_RANDF()
{
	return KingdomGlobal::pMotosGameSys->RandF(SysRandomNext());
}

uint Constants::MOTOS_RANDRANGEU(uint min, uint max)
{
	return KingdomGlobal::pMotosGameSys->RandRangeU(min, max, SysRandomNext());
}

float Constants::MOTOS_RANDRANGEF(float min, float max)
{
	return KingdomGlobal::pMotosGameSys->RandRangeF(min, max, SysRandomNext());
}

uint Constants::MOTOS_IRANDU(uint index)
{
	return KingdomGlobal::pMotosGameSys->RandU(index);
}

float Constants::MOTOS_IRANDF(uint index)
{
	return KingdomGlobal::pMotosGameSys->RandF(index);
}

uint Constants::MOTOS_IRANDRANGEU(uint min, uint max, uint index)
{
	return KingdomGlobal::pMotosGameSys->RandRangeU(min, max, index);
}

float Constants::MOTOS_IRANDRANGEF(float min, float max, uint index)
{
	return KingdomGlobal::pMotosGameSys->RandRangeF(min, max, index);
}

bool Constants::IS_MYCHAR(cTaskChar* p)
{
	return KingdomGlobal::pMotosGameSys->IsGameSysMyChar(p);
}

bool Constants::IS_LANPLAY()
{
	return KingdomGlobal::pMotosGameSys->IsGameSysLanPlay();
}

bool Constants::IS_MASTER()
{
	return KingdomGlobal::pMotosGameSys->IsGameSysMaster();
}

bool Constants::IS_JOIN(int n)
{
	return ((long)KingdomGlobal::pMotosGameSys->JoinFlag() & (long)(1 << n)) != 0;
}

uint Constants::MY_ID()
{
	return KingdomGlobal::pMotosGameSys->ID();
}

bool Constants::IS_BOSS_STAGE()
{
	return KingdomGlobal::MotosSys->nStage() == 7;
}

long Constants::SYNC_COUNTER_FROMBYTE(int n)
{
	if ((get_SYNC_COUNTER() & 0xFF) < (uint)(n & 0xFF))
	{
		return ((long)get_SYNC_COUNTER() & (long)(-256)) + (uint)(n & 0xFF) - 256;
	}
	return ((long)get_SYNC_COUNTER() & (long)(-256)) + (uint)(n & 0xFF);
}

long Constants::GAME_COUNTER_FROMBYTE(int n)
{
	if ((get_GAME_COUNTER() & 0xFF) < (uint)(n & 0xFF))
	{
		return ((long)get_GAME_COUNTER() & (long)(-256)) + (uint)(n & 0xFF) - 256;
	}
	return ((long)get_GAME_COUNTER() & (long)(-256)) + (uint)(n & 0xFF);
}

long Constants::COUNTER_FROMBYTE(int c, int n)
{
	if ((uint)(c & 0xFF) < (uint)(n & 0xFF))
	{
		return ((long)((uint)c) & (long)(-256)) + (uint)(n & 0xFF) - 256;
	}
	return ((long)((uint)c) & (long)(-256)) + (uint)(n & 0xFF);
}

void Constants::Motos_Game_System_Init()
{
	CustomKingdomUtil::Assert(KingdomGlobal::pMotosGameSys == nullptr, "multiple init");
	KingdomGlobal::pMotosGameSys = new cMotosGameSys();
}

void Constants::Motos_Game_System_Term()
{
	if (KingdomGlobal::pMotosGameSys != nullptr)
	{
		KingdomGlobal::pMotosGameSys->Free();
		KingdomGlobal::pMotosGameSys = nullptr;
	}
}

cMotosGameSys* Constants::Motos_Game_System()
{
	return KingdomGlobal::pMotosGameSys;
}

int Constants::MO_GAME_LEVEL_C2M(int n)
{
	return ((n == 1) ? 0 : ((n == 0) ? 1 : (2)));
}

cKingdomPtkHnd* Constants::SpawnEffect(int nKind, Vector4 pMPos, cJetTask* pParam, int nDelay, bool bDead, float loadFactor)
{
	return KingdomGlobal::g_pMotosGame->pTaskEffect()->SpawnEffect(nKind, pMPos, pParam, nDelay, bDead, loadFactor);
}

void Constants::SpawnColEffect(cTaskChar* pChar0, cTaskChar* pChar1)
{
	KingdomGlobal::g_pMotosGame->pTaskEffect()->SpawnColEffect(pChar0, pChar1);
}

int Constants::motosParamRebirthTime()
{
	if (KingdomGlobal::MotosSys->nStageIdx() >= 36 && KingdomGlobal::MotosSys->nStageIdx() <= 45)
	{
		return (int)g_aMotosStage2RebirthTime[KingdomGlobal::MotosSys->nStageIdx() - 36][0];
	}
	return 0;
}

bool Constants::motosIsRebirth()
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		const uint* array = ((KingdomGlobal::MotosSys->nStageIdx() < 36 || KingdomGlobal::MotosSys->nStageIdx() > 45) ? nullptr : g_aMotosStage2RebirthTime[KingdomGlobal::MotosSys->nStageIdx() - 36]);
		int num = KingdomGlobal::g_pMotosGame->VSRestTimer();
		int num2 = (int)array[1];
		if (array != nullptr && num2 < num)
		{
			return true;
		}
	}
	return false;
}

bool Constants::IS_CLEARPAUSE()
{
	return KingdomGlobal::g_pMotosGame->IsGameClearPause();
}

bool Constants::MOTOS_IS_PAUSE()
{
	if (!kingdom::Constants::IS_PAUSE() && KingdomGlobal::g_pMotosGame->IsGameStart())
	{
		return KingdomGlobal::g_pMotosGame->IsPause();
	}
	return true;
}

bool Constants::SYSTEM_IS_PAUSE()
{
	if (!KingdomGlobal::g_pMotosGame->IsPause())
	{
		return InGamePadManager::get_Disable();
	}
	return true;
}

float Constants::FPS_LEN(float f)
{
	return f / 60.0f;
}

float Constants::FPS_SPEED(float f)
{
	return f / 60.0f;
}

int Constants::FPS_TIME(float t)
{
	return (int)(t * 60.0f);
}

float Constants::MOTOS_P_ABS(float x)
{
	if (!(x < 0.0f))
	{
		return x;
	}
	return 0.0f - x;
}

float Constants::MOTOS_P_MIN(float a, float b)
{
	if (!(a < b))
	{
		return b;
	}
	return a;
}

float Constants::MOTOS_P_MAX(float a, float b)
{
	if (!(a > b))
	{
		return b;
	}
	return a;
}

void Constants::VEC4_SET(Vector4& _out, float x1, float y1, float z1)
{
	_out = Vector4();
	_out[0] = x1;
	_out[1] = y1;
	_out[2] = z1;
}

void Constants::VEC4_ADD(Vector4& _out, Vector4 v1, Vector4 v2)
{
	_out = Vector4();
	_out[0] = v1[0] + v2[0];
	_out[1] = v1[1] + v2[1];
	_out[2] = v1[2] + v2[2];
}

void Constants::VEC4_SUB(Vector4& _out, Vector4 v1, Vector4 v2)
{
	_out = Vector4();
	_out[0] = v1[0] - v2[0];
	_out[1] = v1[1] - v2[1];
	_out[2] = v1[2] - v2[2];
}

void Constants::VEC4_MUL(Vector4& _out, Vector4 v1, float n)
{
	_out = Vector4();
	_out[0] = v1[0] * n;
	_out[1] = v1[1] * n;
	_out[2] = v1[2] * n;
}

void Constants::VEC4_DIV(Vector4& _out, Vector4 v1, float n)
{
	_out = Vector4();
	_out[0] = v1[0] / n;
	_out[1] = v1[1] / n;
	_out[2] = v1[2] / n;
}

float Constants::VEC4_LENGTH_SQUARE(float x, float y, float z)
{
	return x * x + y * y + z * z;
}

float Constants::VEC4_DIST_SQUARE(Vector4 v0, Vector4 v1)
{
	return (v0[0] - v1[0]) * (v0[0] - v1[0]) + (v0[1] - v1[1]) * (v0[1] - v1[1]) + (v0[2] - v1[2]) * (v0[2] - v1[2]);
}

void Constants::Motos_P_vecCross(Vector4& _out, const Vector4& vl, const Vector4& vr)
{
	_out = Vector4(vl[1] * vr[2] - vr[1] * vl[2], vl[2] * vr[0] - vr[2] * vl[0], vl[0] * vr[1] - vr[0] * vl[1], 1.0f);
}

float Constants::Motos_P_vecDot(const Vector4& vl, const Vector4& vr)
{
	return vl[0] * vr[0] + vl[1] * vr[1] + vl[2] * vr[2];
}

float Constants::Motos_P_sqrtf_c(float a)
{
	return Mathf::Sqrt(a);
}

float Constants::Motos_P_vecLength_c(Vector4 v)
{
	return Mathf::Sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float Constants::Motos_P_vecLength2(const Vector4& v)
{
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

float Constants::Motos_P_vecDistance(const Vector4& v1, const Vector4& v2)
{
	Vector4 _out;
	Constants::VEC4_SUB(_out, v1, v2);
	return _out.magnitude();
}

void Constants::Motos_P_vecNormalize(Vector4& _out, const Vector4& v)
{
	Vector4 vector = v;
	_out = Vector4();
	Vector4 vector2 = vector;
	float magnitude = vector2.magnitude();
	if (CustomKingdomUtil::IF(magnitude))
	{
		float num = 1.0f / magnitude;
		_out[0] = vector[0] * num;
		_out[1] = vector[1] * num;
		_out[2] = vector[2] * num;
	}
	else
	{
		_out[0] = 0.0f;
		_out[1] = 0.0f;
		_out[2] = 0.0f;
	}
}

void Constants::Motos_P_vecFVec2SVec(ushort* pOut, const Vector4& pVec)
{
	pOut[0] = Constants::fl2fw(pVec[0]);
	pOut[1] = Constants::fl2fw(pVec[1]);
	pOut[2] = Constants::fl2fw(pVec[2]);
}

void Constants::Motos_P_vecSVec2FVec(Vector4& pOut, const ushort* pVec)
{
	pOut = Vector4();
	pOut[0] = Constants::fw2fl(pVec[0]);
	pOut[1] = Constants::fw2fl(pVec[1]);
	pOut[2] = Constants::fw2fl(pVec[2]);
}

float Constants::Motos_P_vecGetRad(const Vector4& v0_ptr, const Vector4& v1_ptr)
{
	Vector4 _out = Vector4();
	Vector4 _out2;
	Constants::Motos_P_vecNormalize(_out2, v0_ptr);
	_out[0] = 0.0f;
	_out[1] = 1.0f;
	_out[2] = 0.0f;
	_out[3] = 1.0f;
	Constants::Motos_P_vecCross(_out, _out2, _out);
	Vector4 _out3;
	Constants::Motos_P_vecNormalize(_out3, v1_ptr);
	float y = Constants::Motos_P_vecDot(_out3, _out);
	float x = Constants::Motos_P_vecDot(_out3, _out2);
	return Mathf::Atan2(y, x);
}

float Constants::Motos_P_vecGetRad2(const Vector4& v0_ptr, const Vector4& v1_ptr, int axis)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_vecGetRad2");
	return 0.0f;
}

void Constants::Motos_P_vecReflect(Vector4& pOut, const Vector4& pNormal, const Vector4& pVec)
{
	float n = Constants::Motos_P_vecDot(pVec, pNormal);
	Vector4 _out = pNormal;
	Constants::VEC4_MUL(_out, _out, 2.0f);
	Constants::VEC4_MUL(_out, _out, n);
	Constants::VEC4_SUB(_out, pVec, _out);
	pOut = _out;
}

bool Constants::Motos_P_vecPointCheck(const Vector4& pPos, const Vector4& pV1, const Vector4& pV2)
{
	Vector4 vector = pV1;
	Vector4 vector2 = pV2;
	if (Mathf::Approximately(vector[2], vector2[2]))
	{
		if (vector[0] < vector2[0])
		{
			if (pPos[0] < vector[0] || pPos[0] > vector2[0])
			{
				return false;
			}
		}
		else if (pPos[0] > vector[0] || pPos[0] < vector2[0])
		{
			return false;
		}
	}
	else if (Mathf::Approximately(vector[0], vector2[0]))
	{
		if (vector[2] < vector2[2])
		{
			if (pPos[2] < vector[2] || pPos[2] > vector2[2])
			{
				return false;
			}
		}
		else if (pPos[2] > vector[2] || pPos[2] < vector2[2])
		{
			return false;
		}
	}
	else
	{
		if (vector[0] < vector2[0])
		{
			if (pPos[0] < vector[0] || pPos[0] > vector2[0])
			{
				return false;
			}
		}
		else if (pPos[0] > vector[0] || pPos[0] < vector2[0])
		{
			return false;
		}
		if (vector[2] < vector2[2])
		{
			if (pPos[2] < vector[2] || pPos[2] > vector2[2])
			{
				return false;
			}
		}
		else if (pPos[2] > vector[2] || pPos[2] < vector2[2])
		{
			return false;
		}
	}
	return true;
}

bool Constants::Motos_P_vecEquation(float& pA, float& pB, float& pC, const Vector4& pVec1, const Vector4& pVec2)
{
	float num = pVec1[0];
	float num2 = pVec1[2];
	float num3 = pVec2[0];
	float num4 = pVec2[2];
	float num5 = num3 - num;
	float num6 = num4 - num2;
	if (Constants::MOTOS_P_ABS(num5) < 0.0001f)
	{
		num5 = 0.0f;
	}
	if (Constants::MOTOS_P_ABS(num6) < 0.0001f)
	{
		num6 = 0.0f;
	}
	if (CustomKingdomUtil::IF(num5) && CustomKingdomUtil::IF(num6))
	{
		pA = 1.0f / num5;
		pB = -1.0f / num6;
		pC = (0.0f - num) * pA - num2 * pB;
		return true;
	}
	if (Mathf::Approximately(num5, 0.0f))
	{
		pA = 1.0f;
		pB = 0.0f;
		pC = ((Constants::MOTOS_P_ABS(num) < Constants::MOTOS_P_ABS(num3)) ? num : num3);
	}
	else
	{
		pA = 0.0f;
		pB = 1.0f;
		pC = ((Constants::MOTOS_P_ABS(num2) < Constants::MOTOS_P_ABS(num4)) ? num2 : num4);
	}
	return false;
}

int Constants::Motos_P_vecCrossCircleEx(Vector4& pOut0, Vector4& pOut1, const Vector4& pVec1, const Vector4& pVec2, const Vector4& pOrg, const float& fRange, bool flag)
{
	pOut0 = Vector4();
	pOut1 = Vector4();
	int num = 0;
	float pA = 0.0f;
	float pB = 0.0f;
	float pC = 0.0f;
	Vector4 _out = Vector4();
	Vector4 _out2 = Vector4();
	Vector4 pPos = Vector4();
	Vector4 pPos2 = Vector4();
	Constants::VEC4_SET(_out, pVec1[0] - pOrg[0], pVec1[1], pVec1[2] - pOrg[2]);
	Constants::VEC4_SET(_out2, pVec2[0] - pOrg[0], pVec2[1], pVec2[2] - pOrg[2]);
	if (Constants::Motos_P_vecEquation(pA, pB, pC, _out, _out2))
	{
		float num2 = fRange;
		float num3 = pA * pA + pB * pB;
		float num4 = pA * pC;
		float num5 = pB * pC;
		float num6 = num4 * num4 - num3 * (pC * pC - pB * pB * num2 * num2);
		float num7 = num5 * num5 - num3 * (pC * pC - pA * pA * num2 * num2);
		if (num3 == 0.0f || pB == 0.0f || pA == 0.0f)
		{
			// TODO(unity): Debug.LogFormat("ERROR: Motos_P_vecCrossCircle ZERO divide\n");
			// TODO(unity): Debug.LogFormat("  aa_bb = {0}  a = {1}  b = {2}\n", num3, pA, pB);
		}
		if (num6 < 0.0f || num7 < 0.0f)
		{
			return 0;
		}
		num = ((num6 == 0.0f && num7 == 0.0f) ? 1 : 2);
		if (Constants::MOTOS_P_ABS(pB) >= Constants::MOTOS_P_ABS(pA))
		{
			float num8 = (CustomKingdomUtil::IF(num6) ? Mathf::Sqrt(num6) : 0.0f);
			pPos[0] = (0.0f - num4 + num8) / num3;
			pPos[1] = _out[1];
			pPos[2] = (0.0f - (pA * pPos[0] + pC)) / pB;
			if (num == 2)
			{
				pPos2[0] = (0.0f - num4 - num8) / num3;
				pPos2[1] = _out2[1];
				pPos2[2] = (0.0f - (pA * pPos2[0] + pC)) / pB;
			}
		}
		else
		{
			float num9 = (CustomKingdomUtil::IF(num7) ? Mathf::Sqrt(num7) : 0.0f);
			pPos[2] = (0.0f - num5 + num9) / num3;
			pPos[1] = _out[1];
			pPos[0] = (0.0f - (pB * pPos[2] + pC)) / pA;
			if (num == 2)
			{
				pPos2[2] = (0.0f - num5 - num9) / num3;
				pPos2[1] = _out2[1];
				pPos2[0] = (0.0f - (pB * pPos2[2] + pC)) / pA;
			}
		}
		pPos[0] += pOrg[0];
		pPos[2] += pOrg[2];
		pPos2[0] += pOrg[0];
		pPos2[2] += pOrg[2];
	}
	else
	{
		if (!(Constants::MOTOS_P_ABS(pC) <= fRange))
		{
			return 0;
		}
		float num10 = fRange * fRange - pC * pC;
		num = ((Constants::MOTOS_P_ABS(pC) == fRange) ? 1 : 2);
		if (pA == 1.0f)
		{
			pPos[0] = pC;
			pPos[1] = _out[1];
			pPos[2] = (CustomKingdomUtil::IF(num10) ? Mathf::Sqrt(num10) : 0.0f);
			if (num == 2)
			{
				pPos2 = pPos;
				pPos2[2] = 0.0f - pPos2[2];
			}
		}
		else
		{
			pPos[2] = pC;
			pPos[1] = _out[1];
			pPos[0] = (CustomKingdomUtil::IF(num10) ? Mathf::Sqrt(num10) : 0.0f);
			if (num == 2)
			{
				pPos2 = pPos;
				pPos2[0] = 0.0f - pPos2[0];
			}
		}
		pPos[0] += pOrg[0];
		pPos[2] += pOrg[2];
		pPos2[0] += pOrg[0];
		pPos2[2] += pOrg[2];
	}
	if (CustomKingdomUtil::IF(num))
	{
		if (flag)
		{
			if (num == 2 && !Constants::Motos_P_vecPointCheck(pPos2, pVec1, pVec2))
			{
				num--;
			}
			if (!Constants::Motos_P_vecPointCheck(pPos, pVec1, pVec2) && CustomKingdomUtil::IF(--num))
			{
				pPos = pPos2;
			}
		}
		if (num == 2 && Constants::VEC4_DIST_SQUARE(pVec1, pPos2) < Constants::VEC4_DIST_SQUARE(pVec1, pPos))
		{
			Vector4 vector = pPos;
			pPos = pPos2;
			pPos2 = vector;
		}
		if (num != 0)
		{
			pOut0 = pPos;
		}
		if (num == 2)
		{
			pOut1 = pPos2;
		}
	}
	return num;
}

int Constants::Motos_P_vecCrossCircle(Vector4& pOut0, Vector4& pOut1, const Vector4& pVec1, const Vector4& pVec2, const Vector4& pOrg, const float& fRange)
{
	return Constants::Motos_P_vecCrossCircleEx(pOut0, pOut1, pVec1, pVec2, pOrg, fRange, true);
}

void Constants::Motos_P_vecLookRotate(Vector4& pOut, const Vector4& pP1, const Vector4& pP2)
{
	Constants::MOTOS_CAUTION("Motos_P_vecLookRotate");
	Vector4 _out = Vector4();
	Vector4 vector = Vector4();
	Vector4 v1_ptr = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	Constants::VEC4_SET(pOut, 0.0f, 0.0f, 0.0f);
	Constants::VEC4_SUB(_out, pP2, pP1);
	vector = _out;
	vector[1] = 0.0f;
	pOut[1] = Constants::Motos_P_vecGetRad(vector, v1_ptr);
	float value = vector[0];
	vector[0] = vector[1];
	vector[1] = value;
	value = _out[0];
	_out[0] = _out[1];
	_out[1] = value;
	pOut[0] = Constants::Motos_P_vecGetRad(vector, _out);
}

void Constants::Motos_P_vecCalcBBMatrix(Matrix4x4 pOut, Vector4 pEye, Vector4 pLookAt, Vector4 pUp)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_vecCalcBBMatrix");
}

bool Constants::Motos_P_rectHitPoint(const Constants::sMOTOS_P_RECT& rect, const float& x, float y)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_rectHitPoint");
	return true;
}

bool Constants::Motos_P_rectHitRect(const Constants::sMOTOS_P_RECT& rect1, const Constants::sMOTOS_P_RECT& rect2)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_rectHitRect");
	return true;
}

int Constants::Motos_P_bitGet(const byte* pData, int bitnum, int num)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_bitGet");
	return 0;
}

void Constants::Motos_P_bitSet(byte* pData, int bitnum, int num, int wdata)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_bitSet");
}

float Constants::fabs(float f)
{
	if (!(f < 0.0f))
	{
		return f;
	}
	return 0.0f - f;
}

float Constants::fw2fl(ushort s)
{
	if (CustomKingdomUtil::IF(s))
	{
		Constants::UNI_FL uNI_FL = [&]() { Constants::UNI_FL _o1 = Constants::UNI_FL(); _o1.u = (uint)(((s & 0x8000) << 16) | (((s & 0x7FFF) << 13) + 939524096)); return _o1; }();
		return uNI_FL.f;
	}
	return 0.0f;
}

ushort Constants::fl2fw(float f)
{
	Constants::UNI_FL uNI_FL = Constants::UNI_FL();
	ushort num = 0;
	if (Constants::fabs(f) > 0.0001f)
	{
		uNI_FL.f = f;
		num |= (ushort)(CustomKingdomUtil::IF(uNI_FL.u & 0x80000000u) ? 32768 : 0);
		num |= (ushort)(((uNI_FL.u & 0x7F800000) >> 23) - 112 << 10);
		return (ushort)(num | (ushort)((uNI_FL.u & 0x7FFFFF) >> 13));
	}
	return 0;
}

float Constants::Motos_P_radGap(float src, float dist)
{
	Constants::MOTOS_NO_SOURCE("Motos_P_radGap");
	return 0.0f;
}

float Constants::ABS(float x)
{
	return Mathf::Abs(x);
}

int Constants::ABS(int x)
{
	return Mathf::Abs(x);
}

float Constants::MIN(float a, float b)
{
	return Mathf::Min(a, b);
}

int Constants::MIN(int a, int b)
{
	return Mathf::Min(a, b);
}

float Constants::MAX(float a, float b)
{
	return Mathf::Max(a, b);
}

int Constants::MAX(int a, int b)
{
	return Mathf::Max(a, b);
}

void Constants::VEC3_SET(Vector4& _out, float x1, float y1, float z1)
{
	_out = Vector4();
	_out.x = x1;
	_out.y = y1;
	_out.z = z1;
}

Vector4 Constants::VEC3_GET(float x1, float y1, float z1)
{
	return Vector4(x1, y1, z1, 1.0f);
}

void Constants::VEC3_ADD(Vector4& _out, Vector4 v1, Vector4 v2)
{
	_out = Vector4();
	_out.x = v1.x + v2.x;
	_out.y = v1.y + v2.y;
	_out.z = v1.z + v2.z;
}

void Constants::VEC3_SUB(Vector4& _out, Vector4 v1, Vector4 v2)
{
	_out = Vector4();
	_out.x = v1.x - v2.x;
	_out.y = v1.y - v2.y;
	_out.z = v1.z - v2.z;
}

void Constants::VEC3_MUL(Vector4& _out, Vector4 v1, float n)
{
	_out = Vector4();
	_out.x = v1.x * n;
	_out.y = v1.y * n;
	_out.z = v1.z * n;
}

void Constants::VEC3_DIV(Vector4& _out, Vector4 v1, float n)
{
	_out = Vector4();
	_out.x = v1.x / n;
	_out.y = v1.y / n;
	_out.z = v1.z / n;
}

float Constants::VEC3_LENGTH_SQUARE(float x, float y, float z)
{
	return x * x + y * y + z * z;
}

Vector4 Constants::MOTOS_VEC4_SET(Vector4& v, float x, float y, float z, float w)
{
	v = Vector4();
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

float Constants::VEC3_DIST_SQUARE(Vector4 v0, Vector4 v1)
{
	return (v0.x - v1.x) * (v0.x - v1.x) + (v0.y - v1.y) * (v0.y - v1.y) + (v0.z - v1.z) * (v0.z - v1.z);
}

float Constants::TSP_DEG2RAD(float deg)
{
	return deg * ((float)std::numbers::pi * 2.0f) / 360.0f;
}

float Constants::TSP_RAD2DEG(float rad)
{
	return rad * 360.0f / ((float)std::numbers::pi * 2.0f);
}

int Constants::motosAxisReverse(int axis)
{
	return (axis + 4) & 7;
}

bool Constants::MO_DEBUG_GPI(int no)
{
	return false;
}

bool Constants::motosNetJoin(int id)
{
	return false;
}

bool Constants::motosNetRoundStart(int id)
{
	return false;
}

int Constants::motosNetJoinNum()
{
	int num = 0;
	for (int i = 0; i < 4; i++)
	{
		if (Constants::motosNetJoin(i))
		{
			num++;
		}
	}
	return num;
}

void Constants::motosDefaultEnv3D()
{
	Constants::MOTOS_NO_SOURCE("motosDefaultEnv3D");
}

void Constants::motosDefaultEnv2D()
{
	Constants::MOTOS_NO_SOURCE("motosDefaultEnv2D");
}

void Constants::motosDrawGrid(float step, int num)
{
	Constants::MOTOS_NO_SOURCE("motosDrawGrid");
}

int Constants::motosVector2Dir(const Vector4& v)
{
	int num = 0;
	if (v[0] > 0.0f)
	{
		num |= 8;
	}
	else if (v[0] < 0.0f)
	{
		num |= 4;
	}
	if (v[2] > 0.0f)
	{
		num |= 2;
	}
	else if (v[2] < 0.0f)
	{
		num |= 1;
	}
	return num;
}

Vector4 Constants::motosDir2Vector(int dir)
{
	Constants::MOTOS_CAUTION("motosDir2Vector");
	return ((dir == 2) ? s_Vec[0] : ((dir == 10) ? s_Vec[1] : ((dir == 8) ? s_Vec[2] : ((dir == 9) ? s_Vec[3] : ((dir == 1) ? s_Vec[4] : ((dir == 5) ? s_Vec[5] : ((dir == 4) ? s_Vec[6] : ((dir == 6) ? s_Vec[7] : (s_Vec[8])))))))));
}

Vector4 Constants::motosAxis2Vector(int axis)
{
	return s_Vec[axis];
}

int Constants::motosRad2Axis(float rad)
{
	float num;
	for (num = ::Constants::JET_RAD2DEG(rad); num < 0.0f; num += 360.0f)
	{
	}
	while (num > 360.0f)
	{
		num -= 360.0f;
	}
	if (num < 22.5f)
	{
		return 0;
	}
	if (num < 67.5f)
	{
		return 1;
	}
	if (num < 112.5f)
	{
		return 2;
	}
	if (num < 157.5f)
	{
		return 3;
	}
	if (num < 202.5f)
	{
		return 4;
	}
	if (num < 247.5f)
	{
		return 5;
	}
	if (num < 292.5f)
	{
		return 6;
	}
	if (num < 337.5f)
	{
		return 7;
	}
	return 0;
}

float Constants::motosAxis2Rad(int axis)
{
	return ((axis == 1) ? Constants::TSP_DEG2RAD(45.0f) : ((axis == 2) ? Constants::TSP_DEG2RAD(90.0f) : ((axis == 3) ? Constants::TSP_DEG2RAD(135.0f) : ((axis == 4) ? Constants::TSP_DEG2RAD(180.0f) : ((axis == 5) ? Constants::TSP_DEG2RAD(225.0f) : ((axis == 6) ? Constants::TSP_DEG2RAD(270.0f) : ((axis == 7) ? Constants::TSP_DEG2RAD(315.0f) : (Constants::TSP_DEG2RAD(0.0f)))))))));
}

int Constants::motosAxisRotate(int axis, float rad)
{
	return Constants::motosRad2Axis(Constants::motosAxis2Rad(axis) + rad);
}

Vector4 Constants::Motos_P_vecRotateVector(Vector4& pOut, Vector4 pVec, float rad)
{
	Vector4 vector = Vector4();
	float num = Mathf::Cos(rad);
	float num2 = Mathf::Sin(rad);
	vector[0] = pVec[0] * num - pVec[2] * num2;
	vector[1] = pVec[1];
	vector[2] = pVec[0] * num2 + pVec[2] * num;
	pOut = vector;
	return pOut;
}

bool Constants::Motos_P_vecCheckRect(Vector4& pOutCross, const Constants::sMOTOS_P_RECT& pRect, const Vector4& pVec1, const Vector4& pVec2)
{
	pOutCross = Vector4();
	Constants::MOTOS_NO_SOURCE("Motos_P_vecCheckRect");
	return false;
}

bool Constants::motosCharMove(cTaskChar* pChar, Vector4 pGoal, float fRange, float fAccel)
{
	if (pChar != nullptr)
	{
		Vector4 _out;
		Constants::VEC3_SUB(_out, pGoal, pChar->vMTrans());
		_out[1] = 0.0f;
		float num = ::Constants::jetGeoVecLength(_out);
		if (num < fRange)
		{
			return true;
		}
		float n = ((!(fAccel > 0.0f)) ? pChar->fAccelSpeed() : fAccel);
		Vector4 _discard2;
		Constants::VEC3_DIV(_discard2, _out, num);
		Constants::VEC3_MUL(_out, _out, n);
		Constants::VEC3_ADD(pChar->vSpeed(), pChar->vSpeed(), _out);
	}
	return false;
}

int Constants::motosReverseDir(int dir)
{
	int num = 0;
	if (CustomKingdomUtil::IF(dir & 1))
	{
		num |= 2;
	}
	if (CustomKingdomUtil::IF(dir & 2))
	{
		num |= 1;
	}
	if (CustomKingdomUtil::IF(dir & 4))
	{
		num |= 8;
	}
	if (CustomKingdomUtil::IF(dir & 8))
	{
		num |= 4;
	}
	return num;
}

void Constants::tspVuMtxCopy(Matrix4x4& m0, Matrix4x4 m1)
{
	::Constants::jetGeoMtxCopy(m0, m1);
}

void Constants::tspVector3Scale(Vector4& v0, const Vector4& v1, float t)
{
	v0 = Vector4();
	v0[0] = v1[0] * t;
	v0[1] = v1[1] * t;
	v0[2] = v1[2] * t;
}

void Constants::tspVuMtxMultTrans(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2)
{
	// the C# built a new Matrix4x4 with an object initializer and copied it (tspVuMtxCopy); the temporary also keeps m0 == m1 / m2 aliasing safe
	Matrix4x4 t = Matrix4x4::identity();
	t[0][0] = m1[0][0];
	t[0][1] = m1[0][1];
	t[0][2] = m1[0][2];
	t[0][3] = m1[0][3];
	t[1][0] = m1[1][0];
	t[1][1] = m1[1][1];
	t[1][2] = m1[1][2];
	t[1][3] = m1[1][3];
	t[2][0] = m1[2][0];
	t[2][1] = m1[2][1];
	t[2][2] = m1[2][2];
	t[2][3] = m1[2][3];
	t[3][0] = m1[0][0] * m2[3][0] + m1[1][0] * m2[3][1] + m1[2][0] * m2[3][2] + m1[3][0];
	t[3][1] = m1[0][1] * m2[3][0] + m1[1][1] * m2[3][1] + m1[2][1] * m2[3][2] + m1[3][1];
	t[3][2] = m1[0][2] * m2[3][0] + m1[1][2] * m2[3][1] + m1[2][2] * m2[3][2] + m1[3][2];
	t[3][3] = m1[0][3] * m2[3][0] + m1[1][3] * m2[3][1] + m1[2][3] * m2[3][2] + m1[3][3];
	tspVuMtxCopy(m0, t);
}

void Constants::tspVuMtxMultRot(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2)
{
	// the C# built a new Matrix4x4 with an object initializer and copied it (tspVuMtxCopy); the temporary also keeps m0 == m1 / m2 aliasing safe
	Matrix4x4 t = Matrix4x4::identity();
	t[0][0] = m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2];
	t[0][1] = m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2];
	t[0][2] = m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2];
	t[0][3] = m1[0][3] * m2[0][0] + m1[1][3] * m2[0][1] + m1[2][3] * m2[0][2];
	t[1][0] = m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2];
	t[1][1] = m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2];
	t[1][2] = m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2];
	t[1][3] = m1[0][3] * m2[1][0] + m1[1][3] * m2[1][1] + m1[2][3] * m2[1][2];
	t[2][0] = m1[0][0] * m2[2][0] + m1[1][0] * m2[2][1] + m1[2][0] * m2[2][2];
	t[2][1] = m1[0][1] * m2[2][0] + m1[1][1] * m2[2][1] + m1[2][1] * m2[2][2];
	t[2][2] = m1[0][2] * m2[2][0] + m1[1][2] * m2[2][1] + m1[2][2] * m2[2][2];
	t[2][3] = m1[0][3] * m2[2][0] + m1[1][3] * m2[2][1] + m1[2][3] * m2[2][2];
	t[3][0] = m1[3][0] * m2[3][3];
	t[3][1] = m1[3][1] * m2[3][3];
	t[3][2] = m1[3][2] * m2[3][3];
	t[3][3] = m1[3][3] * m2[3][3];
	tspVuMtxCopy(m0, t);
}

void Constants::tspVuMtxUnit(Matrix4x4& m0)
{
	m0 = Matrix4x4();
	m0[0][0] = 1.0f;
	m0[0][1] = 0.0f;
	m0[0][2] = 0.0f;
	m0[0][3] = 0.0f;
	m0[1][0] = 0.0f;
	m0[1][1] = 1.0f;
	m0[1][2] = 0.0f;
	m0[1][3] = 0.0f;
	m0[2][0] = 0.0f;
	m0[2][1] = 0.0f;
	m0[2][2] = 1.0f;
	m0[2][3] = 0.0f;
	m0[3][0] = 0.0f;
	m0[3][1] = 0.0f;
	m0[3][2] = 0.0f;
	m0[3][3] = 1.0f;
}

float Constants::tspVector3Length(const Vector4& v0)
{
	return Mathf::Sqrt(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2]);
}

float Constants::tspVector2Length(const Vector2& v0)
{
	return Mathf::Sqrt(v0[0] * v0[0] + v0[1] * v0[1]);
}

float Constants::tspVector3Distance(Vector4 v0, Vector4 v1)
{
	float num = v0[0] - v1[0];
	float num2 = v0[1] - v1[1];
	float num3 = v0[2] - v1[2];
	return Mathf::Sqrt(num * num + num2 * num2 + num3 * num3);
}

void Constants::tspVector3Normalize(Vector4& v0, const Vector4& v1)
{
	::Constants::jetGeoVecNormalize(v0, v1);
}

void Constants::tspVuMtxApply(Vector4& v0, const Matrix4x4& m0, const Vector4& v1)
{
	Vector4 v2 = Vector4(v1[0] * m0[0][0] + v1[1] * m0[1][0] + v1[2] * m0[2][0] + m0[3][0], v1[0] * m0[0][1] + v1[1] * m0[1][1] + v1[2] * m0[2][1] + m0[3][1], v1[0] * m0[0][2] + v1[1] * m0[1][2] + v1[2] * m0[2][2] + m0[3][2], 0.0f);
	::Constants::jetGeoVecCopy(v0, v2);
}

void Constants::tspVector3RotX(Vector4& v0, const Vector4& v1, float rx)
{
	float num = Mathf::Sin(rx);
	float value = Mathf::Cos(rx);
	Matrix4x4 m;
	Constants::tspVuMtxUnit(m);
	m[1][1] = value;
	m[1][2] = num;
	m[2][1] = 0.0f - num;
	m[2][2] = value;
	Constants::tspVuMtxApply(v0, m, v1);
}

void Constants::tspVector3RotY(Vector4& v0, const Vector4& v1, float ry)
{
	float num = Mathf::Sin(ry);
	float value = Mathf::Cos(ry);
	Matrix4x4 m;
	Constants::tspVuMtxUnit(m);
	m[0][0] = value;
	m[2][0] = num;
	m[0][2] = 0.0f - num;
	m[2][2] = value;
	Constants::tspVuMtxApply(v0, m, v1);
}

void Constants::tspVector3RotZ(Vector4& v0, const Vector4& v1, float rz)
{
	float num = Mathf::Sin(rz);
	float value = Mathf::Cos(rz);
	Matrix4x4 m;
	Constants::tspVuMtxUnit(m);
	m[0][0] = value;
	m[0][1] = 0.0f - num;
	m[1][0] = num;
	m[1][1] = value;
	Constants::tspVuMtxApply(v0, m, v1);
}

void Constants::tspVuMtxRotZ(Matrix4x4& m0, const Matrix4x4& m1, float rz)
{
	float num = Mathf::Sin(rz);
	float value = Mathf::Cos(rz);
	Matrix4x4 m2;
	Constants::tspVuMtxUnit(m2);
	m2[0][0] = value;
	m2[0][1] = num;
	m2[1][0] = 0.0f - num;
	m2[1][1] = value;
	Constants::tspVuMtxMultRot(m0, m1, m2);
}

void Constants::tspVector3RotXYZ(Vector4& v0, const Vector4& v1, const Vector4& rot)
{
	Constants::tspVector3RotX(v0, v1, rot[0]);
	Constants::tspVector3RotY(v0, v0, rot[1]);
	Constants::tspVector3RotZ(v0, v0, rot[2]);
}

void Constants::tspVector3RotZYX(Vector4& v0, const Vector4& v1, const Vector4& rot)
{
	Constants::tspVector3RotZ(v0, v1, rot[0]);
	Constants::tspVector3RotY(v0, v0, rot[1]);
	Constants::tspVector3RotX(v0, v0, rot[2]);
}

void Constants::tspVuMtxTrans(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& tv)
{
	Matrix4x4 m2;
	Constants::tspVuMtxUnit(m2);
	m2[3][0] = tv[0];
	m2[3][1] = tv[1];
	m2[3][2] = tv[2];
	Constants::tspVuMtxMultTrans(m0, m1, m2);
}

bool Constants::SEARCH_GIMMICK(int n, float x, float z)
{
	if (KingdomGlobal::g_pMotosGame->pMap()->SearchGimmick(x, z) != nullptr)
	{
		return (long)KingdomGlobal::g_pMotosGame->pMap()->SearchGimmick(x, z)->nTaskKind() == (long)n;
	}
	return false;
}

void Constants::DefaultPlayerParam()
{
	cParamPlayer* obj = new cParamPlayer();
	obj->RegistData("CSV_PLAYER_PARAM");
	set_g_MoPlayerParam(obj->pMotosParams());
}

}
