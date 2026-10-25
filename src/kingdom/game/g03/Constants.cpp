#include "kingdom/game/g03/Constants.hpp"
#include "GamePad.hpp"

#include <limits>

#include "CustomKingdomUtil.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/game/g03/AREA_POINT.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGameOver.hpp"
#include "kingdom/game/g03/cAppG03PacnRollMenu.hpp"
#include "kingdom/game/g03/cAppG03PacnRollStaffRoll.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <InGamePadManager.hpp>

namespace kingdom::game::g03
{

cAppG03PacnRollMenu* Constants::m_pacnRollMenu = nullptr;
cAppG03PacnRollGame* Constants::m_pacnRollGame = nullptr;
cAppG03PacnRollGameOver* Constants::m_pacnRollGameOver = nullptr;
cAppG03PacnRollStaffRoll* Constants::m_pacnRollStaffRoll = nullptr;
const float Constants::scAudioMinDistance = 1.0f;
const float Constants::scAudioMaxDistance = 30.0f;
AREA_POINT* Constants::StageArea3_5_0 = []() { AREA_POINT* _o1 = new AREA_POINT(); _o1->m_vPos = Vector3(-16.0f, 0.0f, -19.0f); _o1->m_fRangeSQ = 0.25f; _o1->m_nNeighbor[0] = 1; _o1->m_nNeighbor[1] = 3; _o1->m_nNeighbor[2] = -1; _o1->m_nNeighbor[3] = -1; return _o1; }();
AREA_POINT* Constants::StageArea3_5_1 = []() { AREA_POINT* _o2 = new AREA_POINT(); _o2->m_vPos = Vector3(-19.0f, 0.0f, -16.0f); _o2->m_fRangeSQ = 0.25f; _o2->m_nNeighbor[0] = 0; _o2->m_nNeighbor[1] = 2; _o2->m_nNeighbor[2] = -1; _o2->m_nNeighbor[3] = -1; return _o2; }();
AREA_POINT* Constants::StageArea3_5_2 = []() { AREA_POINT* _o3 = new AREA_POINT(); _o3->m_vPos = Vector3(-16.0f, 0.0f, -13.0f); _o3->m_fRangeSQ = 0.25f; _o3->m_nNeighbor[0] = 1; _o3->m_nNeighbor[1] = 3; _o3->m_nNeighbor[2] = -1; _o3->m_nNeighbor[3] = -1; return _o3; }();
AREA_POINT* Constants::StageArea3_5_3 = []() { AREA_POINT* _o4 = new AREA_POINT(); _o4->m_vPos = Vector3(-13.0f, 0.0f, -16.0f); _o4->m_fRangeSQ = 0.25f; _o4->m_nNeighbor[0] = 0; _o4->m_nNeighbor[1] = 2; _o4->m_nNeighbor[2] = 4; _o4->m_nNeighbor[3] = -1; return _o4; }();
AREA_POINT* Constants::StageArea3_5_4 = []() { AREA_POINT* _o5 = new AREA_POINT(); _o5->m_vPos = Vector3(-5.0f, 0.0f, -16.0f); _o5->m_fRangeSQ = 0.25f; _o5->m_nNeighbor[0] = 5; _o5->m_nNeighbor[1] = 7; _o5->m_nNeighbor[2] = 3; _o5->m_nNeighbor[3] = -1; return _o5; }();
AREA_POINT* Constants::StageArea3_5_5 = []() { AREA_POINT* _o6 = new AREA_POINT(); _o6->m_vPos = Vector3(0.0f, 0.0f, -21.0f); _o6->m_fRangeSQ = 0.25f; _o6->m_nNeighbor[0] = 4; _o6->m_nNeighbor[1] = 6; _o6->m_nNeighbor[2] = 10; _o6->m_nNeighbor[3] = -1; return _o6; }();
AREA_POINT* Constants::StageArea3_5_6 = []() { AREA_POINT* _o7 = new AREA_POINT(); _o7->m_vPos = Vector3(5.0f, 0.0f, -16.0f); _o7->m_fRangeSQ = 0.25f; _o7->m_nNeighbor[0] = 5; _o7->m_nNeighbor[1] = 7; _o7->m_nNeighbor[2] = 17; _o7->m_nNeighbor[3] = -1; return _o7; }();
AREA_POINT* Constants::StageArea3_5_7 = []() { AREA_POINT* _o8 = new AREA_POINT(); _o8->m_vPos = Vector3(0.0f, 0.0f, -10.0f); _o8->m_fRangeSQ = 0.25f; _o8->m_nNeighbor[0] = 4; _o8->m_nNeighbor[1] = 6; _o8->m_nNeighbor[2] = 12; _o8->m_nNeighbor[3] = -1; return _o8; }();
AREA_POINT* Constants::StageArea3_5_8 = []() { AREA_POINT* _o9 = new AREA_POINT(); _o9->m_vPos = Vector3(0.0f, 0.0f, -35.0f); _o9->m_fRangeSQ = 0.25f; _o9->m_nNeighbor[0] = 9; _o9->m_nNeighbor[1] = 11; _o9->m_nNeighbor[2] = -1; _o9->m_nNeighbor[3] = -1; return _o9; }();
AREA_POINT* Constants::StageArea3_5_9 = []() { AREA_POINT* _o10 = new AREA_POINT(); _o10->m_vPos = Vector3(-3.0f, 0.0f, -32.0f); _o10->m_fRangeSQ = 0.25f; _o10->m_nNeighbor[0] = 8; _o10->m_nNeighbor[1] = 10; _o10->m_nNeighbor[2] = -1; _o10->m_nNeighbor[3] = -1; return _o10; }();
AREA_POINT* Constants::StageArea3_5_10 = []() { AREA_POINT* _o11 = new AREA_POINT(); _o11->m_vPos = Vector3(0.0f, 0.0f, -29.0f); _o11->m_fRangeSQ = 0.25f; _o11->m_nNeighbor[0] = 9; _o11->m_nNeighbor[1] = 11; _o11->m_nNeighbor[2] = 5; _o11->m_nNeighbor[3] = -1; return _o11; }();
AREA_POINT* Constants::StageArea3_5_11 = []() { AREA_POINT* _o12 = new AREA_POINT(); _o12->m_vPos = Vector3(3.0f, 0.0f, -32.0f); _o12->m_fRangeSQ = 0.25f; _o12->m_nNeighbor[0] = 8; _o12->m_nNeighbor[1] = 10; _o12->m_nNeighbor[2] = -1; _o12->m_nNeighbor[3] = -1; return _o12; }();
AREA_POINT* Constants::StageArea3_5_12 = []() { AREA_POINT* _o13 = new AREA_POINT(); _o13->m_vPos = Vector3(0.0f, 0.0f, -3.0f); _o13->m_fRangeSQ = 0.25f; _o13->m_nNeighbor[0] = 13; _o13->m_nNeighbor[1] = 15; _o13->m_nNeighbor[2] = 7; _o13->m_nNeighbor[3] = -1; return _o13; }();
AREA_POINT* Constants::StageArea3_5_13 = []() { AREA_POINT* _o14 = new AREA_POINT(); _o14->m_vPos = Vector3(-3.0f, 0.0f, 0.0f); _o14->m_fRangeSQ = 0.25f; _o14->m_nNeighbor[0] = 12; _o14->m_nNeighbor[1] = 14; _o14->m_nNeighbor[2] = -1; _o14->m_nNeighbor[3] = -1; return _o14; }();
AREA_POINT* Constants::StageArea3_5_14 = []() { AREA_POINT* _o15 = new AREA_POINT(); _o15->m_vPos = Vector3(0.0f, 0.0f, 3.0f); _o15->m_fRangeSQ = 0.25f; _o15->m_nNeighbor[0] = 13; _o15->m_nNeighbor[1] = 15; _o15->m_nNeighbor[2] = -1; _o15->m_nNeighbor[3] = -1; return _o15; }();
AREA_POINT* Constants::StageArea3_5_15 = []() { AREA_POINT* _o16 = new AREA_POINT(); _o16->m_vPos = Vector3(3.0f, 0.0f, 0.0f); _o16->m_fRangeSQ = 0.25f; _o16->m_nNeighbor[0] = 12; _o16->m_nNeighbor[1] = 14; _o16->m_nNeighbor[2] = -1; _o16->m_nNeighbor[3] = -1; return _o16; }();
AREA_POINT* Constants::StageArea3_5_16 = []() { AREA_POINT* _o17 = new AREA_POINT(); _o17->m_vPos = Vector3(16.0f, 0.0f, -19.0f); _o17->m_fRangeSQ = 0.25f; _o17->m_nNeighbor[0] = 17; _o17->m_nNeighbor[1] = 19; _o17->m_nNeighbor[2] = -1; _o17->m_nNeighbor[3] = -1; return _o17; }();
AREA_POINT* Constants::StageArea3_5_17 = []() { AREA_POINT* _o18 = new AREA_POINT(); _o18->m_vPos = Vector3(13.0f, 0.0f, -16.0f); _o18->m_fRangeSQ = 0.25f; _o18->m_nNeighbor[0] = 16; _o18->m_nNeighbor[1] = 18; _o18->m_nNeighbor[2] = 6; _o18->m_nNeighbor[3] = -1; return _o18; }();
AREA_POINT* Constants::StageArea3_5_18 = []() { AREA_POINT* _o19 = new AREA_POINT(); _o19->m_vPos = Vector3(16.0f, 0.0f, -13.0f); _o19->m_fRangeSQ = 0.25f; _o19->m_nNeighbor[0] = 17; _o19->m_nNeighbor[1] = 19; _o19->m_nNeighbor[2] = -1; _o19->m_nNeighbor[3] = -1; return _o19; }();
AREA_POINT* Constants::StageArea3_5_19 = []() { AREA_POINT* _o20 = new AREA_POINT(); _o20->m_vPos = Vector3(19.0f, 0.0f, -16.0f); _o20->m_fRangeSQ = 0.25f; _o20->m_nNeighbor[0] = 16; _o20->m_nNeighbor[1] = 18; _o20->m_nNeighbor[2] = -1; _o20->m_nNeighbor[3] = -1; return _o20; }();
AREA_POINT* Constants::StageArea3_5[21] = {StageArea3_5_0, StageArea3_5_1, StageArea3_5_2, StageArea3_5_3, StageArea3_5_4, StageArea3_5_5, StageArea3_5_6, StageArea3_5_7, StageArea3_5_8, StageArea3_5_9, StageArea3_5_10, StageArea3_5_11, StageArea3_5_12, StageArea3_5_13, StageArea3_5_14, StageArea3_5_15, StageArea3_5_16, StageArea3_5_17, StageArea3_5_18, StageArea3_5_19, nullptr};
std::vector<std::vector<AREA_POINT*>> Constants::AllStageArea;
string Constants::s_aModelName[5] = {"BIN_BOS_NORMALN_MDL", "BIN_BOS_NORMALB_MDL", "BIN_BOS_BIJIKEN_MDL", "BIN_BOS_BIJIKEB_MDL", "BIN_BOS_EYE_MDL"};
string Constants::s_aAnimeName[34] = {"BIN_BOS_ANM_WAIT", "BIN_BOS_ANM_MOVE", "BIN_BOS_ANM_ESCAPE", "BIN_BOS_ANM_APPEAR", "BIN_BOS_ANM_REBIRTH", "BIN_BOS_ANM_RECOVER", "BIN_BOS_ANM_LAUGH", "BIN_BOS_ANM_DAMAGE", "BIN_BOS_ANM_SUFFER", "BIN_BOS_ANM_PLAY00", "BIN_BOS_ANM_PLAY01", "BIN_BOS_ANM_PLAY02", "BIN_BOS_ANM_INSTRUCT", "BIN_BOS_ANM_KNOCK_H0", "BIN_BOS_ANM_KNOCK_V0", "BIN_BOS_ANM_KNOCK_H1", "BIN_BOS_ANM_KNOCK_V1", "BIN_BOS_ANM_HEADBUTT0", "BIN_BOS_ANM_HEADBUTT1", "BIN_BOS_ANM_HEADBUTT2L", "BIN_BOS_ANM_HEADBUTT3", "BIN_BOS_ANM_FAN_START", "BIN_BOS_ANM_FAN", "BIN_BOS_ANM_FAN_END", "BIN_BOS_ANM_DIZZ", "BIN_BOS_ANM_DOWN", "BIN_BOS_ANM_DOWNNING", "BIN_BOS_ANM_WAKEUP", "BIN_BOS_ANM_BOMB_KNOCK", "BIN_BOS_ANM_BOMB_DRIBBLE", "BIN_BOS_ANM_BIJIKE_MOVE", "BIN_BOS_ANM_BIJIKE_EATEN", "BIN_BOS_ANM_EYE_MOVE", "BIN_DEMO_BOS_ANM_GOL"};
float Constants::iAttackPermitSubY[15] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
sbyte Constants::iCarryGhostSet[18][3] = {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}, {3, -1, -1}, {4, -1, -1}, {5, -1, -1}, {6, -1, -1}, {7, -1, -1}, {8, -1, -1}, {9, -1, -1}, {0, 1, -1}, {2, 3, -1}, {4, 5, -1}, {6, 7, -1}, {8, 9, -1}, {0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
const Vector4 Constants::CENTER_GEAR_POS{};
// C# static constructor of Constants: runs once before first use

Constants::sFace::sFace()
{
	::Constants::jetGeoVecUnit(vVrt[0]);
	::Constants::jetGeoVecUnit(vVrt[1]);
	::Constants::jetGeoVecUnit(vVrt[2]);
	::Constants::jetGeoVecUnit(vNorm);
	nKind = Constants::eFACE_KIND::eFACE_KIND_NORM;
	fWork = 0.0f;
	nHangFlag = 0u;
	nKindEx = 0u;
}

Constants::CAM_PARAM* Constants::CAM_PARAM::Clone()
{
	// FIXME(cs2cpp): unresolved identifier MemberwiseClone
	// TODO(unity): return (CAM_PARAM)MemberwiseClone();
	return new CAM_PARAM(*this);   // MemberwiseClone
}

Constants::CAM_PARAM::CAM_PARAM()
	
{
}

cG03PacnRollSys* Constants::PacnRoll_System()
{
	return KingdomGlobal::PacnRollSys;
}

void Constants::SetPacnRoll_System(cG03PacnRollSys* system)
{
	if (KingdomGlobal::PacnRollSys != nullptr)
	{
		KingdomGlobal::PacnRollSys->Free();
		KingdomGlobal::PacnRollSys = nullptr;
	}
	KingdomGlobal::PacnRollSys = system;
}

void Constants::Kingdom_G03_Init_ChangeInit()
{
	CustomKingdomUtil::Assert(KingdomGlobal::PacnRollSys == nullptr, "mulutiple init");
	KingdomGlobal::PacnRollSys = new cG03PacnRollSys();
	kingdom::Constants::Kingdom_SetJobNo(220200960u);
}

void Constants::Kingdom_G03_Term_ChangeInit()
{
	if (KingdomGlobal::PacnRollSys != nullptr)
	{
		KingdomGlobal::PacnRollSys->Free();
		KingdomGlobal::PacnRollSys = nullptr;
	}
	kingdom::Constants::Kingdom_SetJobNo(144703488u);
}

void Constants::Kingdom_G03_Menu_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_pacnRollMenu = new cAppG03PacnRollMenu());
}

void Constants::Kingdom_G03_Menu_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_pacnRollMenu->Free();
	m_pacnRollMenu = nullptr;
}

void Constants::Kingdom_G03_Game_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_pacnRollGame = new cAppG03PacnRollGame());
}

void Constants::Kingdom_G03_Game_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_pacnRollGame->Free();
	m_pacnRollGame = nullptr;
}

void Constants::Kingdom_G03_GameOver_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_pacnRollGameOver = new cAppG03PacnRollGameOver());
}

void Constants::Kingdom_G03_GameOver_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_pacnRollGameOver->Free();
	m_pacnRollGameOver = nullptr;
}

void Constants::Kingdom_G03_StaffRoll_ChangeInit()
{
	KingdomGlobal::SysCore->RegistMainTask(m_pacnRollStaffRoll = new cAppG03PacnRollStaffRoll());
}

void Constants::Kingdom_G03_StaffRoll_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
	m_pacnRollStaffRoll->Free();
	m_pacnRollStaffRoll = nullptr;
}

void Constants::Kingdom_G03_Nunchuk_ChangeInit()
{
}

void Constants::Kingdom_G03_Nunchuk_ChangeTerm()
{
	KingdomGlobal::SysCore->ReleaseMainTask();
}

void Constants::ReleasePacnRoll()
{
	if (m_pacnRollMenu != nullptr)
	{
		m_pacnRollMenu->Free();
	}
	m_pacnRollMenu = nullptr;
	if (m_pacnRollGame != nullptr)
	{
		m_pacnRollGame->Free();
	}
	m_pacnRollGame = nullptr;
	if (m_pacnRollGameOver != nullptr)
	{
		m_pacnRollGameOver->Free();
	}
	m_pacnRollGameOver = nullptr;
	if (m_pacnRollStaffRoll != nullptr)
	{
		m_pacnRollStaffRoll->Free();
	}
	m_pacnRollStaffRoll = nullptr;
}

bool Constants::IS_PAC_PAUSE()
{
	// DOL: every g03 task starts with `app->+0x1b4 (IsPause) != 0 || (get_pad1(0)->hold & 0x400) != 0` (0x80164f94 for Pac-Man, ...): the pause task or the open Home Button Menu
	// (HOME held here) freezes it. InGamePadManager::get_Disable is the Unity input lock, no DOL counterpart (kept: it is never set natively).
	if (KingdomGlobal::KingSys->pAppMain()->IsPause() || (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0)
	{
		return true;
	}
	return InGamePadManager::get_Disable();
}

void Constants::ON_PAUSE()
{
	KingdomGlobal::KingSys->pAppMain()->IsPause() = true;
}

void Constants::OFF_PAUSE()
{
	KingdomGlobal::KingSys->pAppMain()->IsPause() = false;
}

bool Constants::IS_NAKAJIMA_PAUSE()
{
	return false;
}

bool Constants::IsVecNormalize(const Vector4& v)
{
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] >= (-std::numeric_limits<float>::infinity());
}

cAppG03PacnRollGame* Constants::PACN_APP()
{
	return dynamic_cast<cAppG03PacnRollGame*>(KingdomGlobal::KingSys->pAppMain());
}

cTaskCharPacman* Constants::PACMAN_OBJ()
{
	return (static_cast<cAppG03PacnRollGame*>(KingdomGlobal::KingSys->pAppMain()))->pTaskCharPacman();
}

bool Constants::CAM_PARAM_CHECK(Constants::CAM_PARAM* ptr, int flg)
{
	return CustomKingdomUtil::IF((long)ptr->m_nFlag & (long)(1 << flg));
}

AREA_POINT* Constants::areaGetInfo(const uint& nStage, const int& index)
{
	return AllStageArea[nStage][index];
}

int Constants::areaGetNo(const uint& nStage, const Vector4& vPos)
{
	float num = std::numeric_limits<float>::max();
	int num2 = -1;
	std::vector<AREA_POINT*>& array = AllStageArea[nStage];
	if (!array.empty())
	{
		for (int i = 0; i < (int)array.size(); i++)
		{
			if (array[i] == nullptr)
			{
				continue;
			}
			float num3 = array[i]->m_vPos[0] - vPos[0];
			float num4 = array[i]->m_vPos[2] - vPos[2];
			float num5 = num3 * num3 + num4 * num4;
			if (num2 == -1 || num > num5)
			{
				num2 = i;
				num = num5;
				if (array[i]->m_fRangeSQ >= num)
				{
					break;
				}
			}
		}
	}
	return num2;
}

int Constants::areaGetFarNo(const uint& nStage, const Vector4& vPos)
{
	float num = std::numeric_limits<float>::lowest();
	int num2 = -1;
	std::vector<AREA_POINT*>& array = AllStageArea[nStage];
	if (!array.empty())
	{
		for (int i = 0; i < (int)array.size(); i++)
		{
			if (array[i] != nullptr)
			{
				float num3 = array[i]->m_vPos[0] - vPos[0];
				float num4 = array[i]->m_vPos[2] - vPos[2];
				float num5 = num3 * num3 + num4 * num4;
				if (num2 == -1 || num < num5)
				{
					num2 = i;
					num = num5;
				}
			}
		}
	}
	return num2;
}

int Constants::areaGetRevNo(const uint& nStage, const Vector4& vSrc, const Vector4& vPos)
{
	float num = std::numeric_limits<float>::lowest();
	int num2 = -1;
	std::vector<AREA_POINT*>& array = AllStageArea[nStage];
	Constants::areaGetNo(nStage, vSrc);
	Vector4 v;
	::Constants::jetGeoVecSub(v, vPos, vSrc);
	if (!array.empty())
	{
		for (int i = 0; i < (int)array.size(); i++)
		{
			if (array[i] == nullptr)
			{
				continue;
			}
			Vector4 v2;
			::Constants::jetGeoVecSub(v2, array[i]->m_vPos, vSrc);
			if (0.0f > ::Constants::jetGeoVecInnerProduct(v, v2))
			{
				float num3 = array[i]->m_vPos[0] - vPos[0];
				float num4 = array[i]->m_vPos[2] - vPos[2];
				float num5 = num3 * num3 + num4 * num4;
				if (num2 == -1 || num < num5)
				{
					num2 = i;
					num = num5;
				}
			}
		}
	}
	return num2;
}

int Constants::s_areaSearch(uint nStage, AREA_POINT** pArray, uint nMax, int level, ulong flag, int nSrc, int nDst)
{
	std::vector<AREA_POINT*>& array = AllStageArea[nStage];
	CustomKingdomUtil::Assert(0 < nMax, "nullpo");
	if ((long)level == (long)nMax)
	{
		return -1;
	}
	flag |= (ulong)(1L << nSrc);
	pArray[level] = array[nSrc];
	if (nSrc == nDst)
	{
		return level + 1;
	}
	for (int i = 0; i < 4 && -1 != array[nSrc]->m_nNeighbor[i]; i++)
	{
		int num = array[nSrc]->m_nNeighbor[i];
		if (0 >= (flag & (ulong)(1L << num)))
		{
			int num2 = Constants::s_areaSearch(nStage, pArray, nMax, level + 1, flag, num, nDst);
			if (num2 >= 0)
			{
				return num2;
			}
		}
	}
	return -1;
}

int Constants::areaSearch(const uint& nStage, AREA_POINT** pArray, const uint& nMax, const int& nSrc, const int& nDst, const ulong& exflag)
{
	ulong num = exflag;
	std::vector<AREA_POINT*>& array = AllStageArea[nStage];
	CustomKingdomUtil::Assert(0 < nMax, "nullpo");
	if (nSrc == -1 || nDst == -1)
	{
		return -1;
	}
	pArray[0] = array[nSrc];
	if (nSrc == nDst)
	{
		return 0;
	}
	num |= (ulong)(1L << nSrc);
	for (uint num2 = 1u; num2 <= nMax; num2++)
	{
		for (int i = 0; i < 4 && -1 != array[nSrc]->m_nNeighbor[i]; i++)
		{
			int num3 = array[nSrc]->m_nNeighbor[i];
			if (0 >= (num & (ulong)(1L << num3)))
			{
				int num4 = Constants::s_areaSearch(nStage, pArray, num2, 0, num, num3, nDst);
				if (num4 >= 0)
				{
					return num4;
				}
			}
		}
	}
	return -1;
}

bool Constants::areaCheckArrive(const uint& nStage, const int& nSrc, const Vector4& vPos)
{
	std::vector<AREA_POINT*>& array = AllStageArea[nStage];
	Vector4 v;
	::Constants::jetGeoVecSub(v, vPos, array[nSrc]->m_vPos);
	if (v[0] * v[0] + v[1] * v[1] + v[2] * v[2] <= array[nSrc]->m_fRangeSQ)
	{
		return true;
	}
	return false;
}

void Constants::_StaticCtor()
{
	AllStageArea.resize(26);
	AllStageArea[13].assign(std::begin(StageArea3_5), std::end(StageArea3_5));
	CENTER_GEAR_POS = Vector4(0.5f, 0.4f, 3.7f, 0.0f);
}

}
