#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "platform/devtools_g07edit.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include <iterator>
#include <utility>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosMapoo.hpp"
#include "kingdom/game/g07/cParamStage.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharMeteo.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "kingdom/game/g07/cTaskPanel.hpp"
#include "kingdom/game/g07/MAP_HIT_RESULT.hpp"
#include "kingdom/game/g07/MO_METEO_INFO.hpp"
#include "kingdom/game/g07/MO_SLOPE_INFO.hpp"
#include "kingdom/game/g07/MO_SLOPE_PAT.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

cTaskPanelMan::EDGE_INFO* cTaskPanelMan::EdgeInfo[4] = {[]() { cTaskPanelMan::EDGE_INFO* _o1 = new cTaskPanelMan::EDGE_INFO(); _o1->nFlag = 1; _o1->vNormal = Vector4(-1.0f, 0.0f, 0.0f); _o1->vInV0 = Vector4(0.0f, 0.0f, 0.0f); _o1->vInV1 = Vector4(0.0f, 0.0f, 0.0f); _o1->vOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o1->vOutV1 = Vector4(0.0f, 0.0f, 0.0f); _o1->vGOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o1->vGOutV1 = Vector4(0.0f, 0.0f, 0.0f); return _o1; }(), []() { cTaskPanelMan::EDGE_INFO* _o2 = new cTaskPanelMan::EDGE_INFO(); _o2->nFlag = 2; _o2->vNormal = Vector4(0.0f, 0.0f, -1.0f); _o2->vInV0 = Vector4(0.0f, 0.0f, 0.0f); _o2->vInV1 = Vector4(0.0f, 0.0f, 0.0f); _o2->vOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o2->vOutV1 = Vector4(0.0f, 0.0f, 0.0f); _o2->vGOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o2->vGOutV1 = Vector4(0.0f, 0.0f, 0.0f); return _o2; }(), []() { cTaskPanelMan::EDGE_INFO* _o3 = new cTaskPanelMan::EDGE_INFO(); _o3->nFlag = 1; _o3->vNormal = Vector4(1.0f, 0.0f, 0.0f); _o3->vInV0 = Vector4(0.0f, 0.0f, 0.0f); _o3->vInV1 = Vector4(0.0f, 0.0f, 0.0f); _o3->vOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o3->vOutV1 = Vector4(0.0f, 0.0f, 0.0f); _o3->vGOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o3->vGOutV1 = Vector4(0.0f, 0.0f, 0.0f); return _o3; }(), []() { cTaskPanelMan::EDGE_INFO* _o4 = new cTaskPanelMan::EDGE_INFO(); _o4->nFlag = 2; _o4->vNormal = Vector4(0.0f, 0.0f, 1.0f); _o4->vInV0 = Vector4(0.0f, 0.0f, 0.0f); _o4->vInV1 = Vector4(0.0f, 0.0f, 0.0f); _o4->vOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o4->vOutV1 = Vector4(0.0f, 0.0f, 0.0f); _o4->vGOutV0 = Vector4(0.0f, 0.0f, 0.0f); _o4->vGOutV1 = Vector4(0.0f, 0.0f, 0.0f); return _o4; }()};
const int cTaskPanelMan::s_nDirs[8] = {1, 2, 4, 8, 5, 9, 6, 10};
int cTaskPanelMan::level[9] = {};
float cTaskPanelMan::dist[4] = {};

cTaskPanelMan::cTaskPanelMan()
{
	m_pMotosGame = nullptr;
	CustomKingdomUtil::Assert(m_pMotosGame != nullptr, "nullpo");
	Init();
}

cTaskPanelMan::cTaskPanelMan(cAppG07MotosGame* pGame)
{
	m_pMotosGame = pGame;
	CustomKingdomUtil::Assert(m_pMotosGame != nullptr, "nullpo");
	Init();
}

uint cTaskPanelMan::nWidth()
{
	return m_nWidth;
}

uint cTaskPanelMan::nDepth()
{
	return m_nDepth;
}

float cTaskPanelMan::fPanelSize()
{
	return m_fPanelSize;
}

float cTaskPanelMan::nStageWidth()
{
	return m_sStageRect.w - m_sStageRect.x;
}

float cTaskPanelMan::nStageDepth()
{
	return m_sStageRect.h - m_sStageRect.y;
}

Constants::PANEL_INFO* cTaskPanelMan::SearchPanel(Vector4 vec)
{
	return SearchPanel(vec[0], vec[2]);
}

Constants::sMOTOS_S32_RECT& cTaskPanelMan::pStageRect()
{
	return m_sStageRect;
}

uint cTaskPanelMan::HitFlag(int x, int z)
{
	return 0u;
}

uint cTaskPanelMan::HitFlag(Vector4 pMTrans)
{
	return 0u;
}

MO_SLOPE_INFO*& cTaskPanelMan::pSlopeInfo()
{
	return m_SlopeInfo;
}

MAP_HIT_RESULT* cTaskPanelMan::HitResult(int n)
{
	return m_HitResult[n];
}

int cTaskPanelMan::StrikePanel(int id, int level, float range, Vector4 vec, bool spawnEffect)
{
	return StrikePanel(id, level, range, vec[0], vec[2], spawnEffect);
}

int cTaskPanelMan::StrikePanel(int id, int level, int PanelID)
{
	return StrikePanel(id, level, SearchPanelID(PanelID));
}

void cTaskPanelMan::Init()
{
	Mapoo = new cMotosMapoo();
	m_nWidth = 16u;
	m_nDepth = 16u;
	m_fPanelSize = 1.0f;
	m_pMeteoTask = nullptr;
	m_sStageRect = Constants::sMOTOS_S32_RECT();
	m_SlopeInfo = new MO_SLOPE_INFO();
	m_sStageRect.x = 255;
	m_sStageRect.y = 255;
	for (int i = 0; i < (int)std::size(m_HitResult); i++)
	{
		m_HitResult[i] = new MAP_HIT_RESULT();
	}
	m_nPanelPattern = SearchObjPanelPattern();
	if (!Constants::IS_LANPLAY())
	{
		uint stageNum = KingdomGlobal::MotosSys->nStageIdx();
		cParamStage* cParamStage2 = new cParamStage(devtools::G07LevelCsv("CSV_STAGE_PARAM_NORMAL_PARAM"), stageNum); // devtools: dev window difficulty (not 1:1)
		m_MeteoInfo = cParamStage2->pMeteoInfo();
		m_SlopeInfo = cParamStage2->pSlopeInfo();
		m_pMeteoTask = new cTaskCharMeteo();
		m_pMeteoTask->Create(0u);
		ConnectChild(m_pMeteoTask);
		m_pMeteoTask->vMTrans() = Vector4::one * 100.0f;
		m_pMeteoTask->vSpeed() = Vector4(0.0f, 0.01f, 0.0f, 1.0f);
	}
	SetupPanel();
	m_sStageRect.w++;
	m_sStageRect.h++;
}

void cTaskPanelMan::Term()
{
	for (int i = 0; i < (int)std::size(m_pTaskPanels); i++)
	{
		if (m_pTaskPanels[i] != nullptr)
		{
			m_pTaskPanels[i]->TermOnce();
		}
	}
	if (m_pMeteoTask != nullptr)
	{
		m_pMeteoTask->TermOnce();
	}
}

void cTaskPanelMan::SlopeMan()
{
	if (m_SlopeInfo->m_nPatNum == 0 || KingdomGlobal::g_pMotosGame->nStep(0) != 8)
	{
		return;
	}
	int num = (int)((long)Constants::get_GAME_COUNTER() % (long)m_SlopeInfo->m_nTotalTime);
	MO_SLOPE_PAT* mO_SLOPE_PAT = nullptr;
	for (int i = 0; i < 5; i++)
	{
		if (m_SlopeInfo->m_Pat[i]->m_nPatNo > 0)
		{
			mO_SLOPE_PAT = m_SlopeInfo->m_Pat[i];
			if (mO_SLOPE_PAT->m_nStart <= num && num < mO_SLOPE_PAT->m_nEnd)
			{
				break;
			}
		}
	}
	if (mO_SLOPE_PAT == nullptr || !CustomKingdomUtil::IF(mO_SLOPE_PAT->m_nTotal))
	{
		return;
	}
	int nTime = mO_SLOPE_PAT->m_nTime;
	int num2 = nTime + mO_SLOPE_PAT->m_nStopTime;
	int num3 = num2 + mO_SLOPE_PAT->m_nRevTime;
	(void)(mO_SLOPE_PAT->m_nRevStopTime);
	int num4 = num - mO_SLOPE_PAT->m_nStart;
	float num5 = mO_SLOPE_PAT->m_fRot * (float)((!CustomKingdomUtil::IF(num4 / mO_SLOPE_PAT->m_nTotal % 2)) ? 1 : (-1));
	num4 %= mO_SLOPE_PAT->m_nTotal;
	if (0 <= num4 && num4 < nTime)
	{
		KingdomGlobal::g_pMotosGame->nSlopeState() = 1;
		KingdomGlobal::g_pMotosGame->fSlopeRad() = num5 / (float)nTime * (float)num4;
	}
	else if (nTime <= num4 && num4 < num2)
	{
		KingdomGlobal::g_pMotosGame->nSlopeState() = 3;
		KingdomGlobal::g_pMotosGame->fSlopeRad() = num5;
	}
	else if (num2 <= num4 && num4 < num3)
	{
		KingdomGlobal::g_pMotosGame->nSlopeState() = 2;
		KingdomGlobal::g_pMotosGame->fSlopeRad() = num5 / (float)(num3 - num2) * (float)(num3 - num4);
	}
	else
	{
		KingdomGlobal::g_pMotosGame->nSlopeState() = 0;
		KingdomGlobal::g_pMotosGame->fSlopeRad() = 0.0f;
	}
	KingdomGlobal::g_pMotosGame->fConsSlopeRadOLD() = KingdomGlobal::g_pMotosGame->fConsSlopeRad();
	num = (int)((Constants::get_GAME_COUNTER() + 60) % m_SlopeInfo->m_nTotalTime);
	mO_SLOPE_PAT = nullptr;
	for (int j = 0; j < 5; j++)
	{
		if (m_SlopeInfo->m_Pat[j]->m_nPatNo > 0)
		{
			mO_SLOPE_PAT = m_SlopeInfo->m_Pat[j];
			if (mO_SLOPE_PAT->m_nStart <= num && num < mO_SLOPE_PAT->m_nEnd)
			{
				break;
			}
		}
	}
	if (mO_SLOPE_PAT != nullptr && CustomKingdomUtil::IF(mO_SLOPE_PAT->m_nTotal))
	{
		int nTime2 = mO_SLOPE_PAT->m_nTime;
		int num6 = nTime2 + mO_SLOPE_PAT->m_nStopTime;
		int num7 = num6 + mO_SLOPE_PAT->m_nRevTime;
		(void)(mO_SLOPE_PAT->m_nRevStopTime);
		int num8 = num - mO_SLOPE_PAT->m_nStart;
		float num9 = mO_SLOPE_PAT->m_fRot * (float)((!CustomKingdomUtil::IF(num8 / mO_SLOPE_PAT->m_nTotal % 2)) ? 1 : (-1));
		num8 %= mO_SLOPE_PAT->m_nTotal;
		if (0 <= num8 && num8 < nTime2)
		{
			KingdomGlobal::g_pMotosGame->fConsSlopeRad() = num9 / (float)nTime2 * (float)num8;
		}
		else if (nTime2 <= num8 && num8 < num6)
		{
			KingdomGlobal::g_pMotosGame->fConsSlopeRad() = num9;
		}
		else if (num6 <= num8 && num8 < num7)
		{
			KingdomGlobal::g_pMotosGame->fConsSlopeRad() = num9 / (float)(num7 - num6) * (float)(num7 - num8);
		}
		else
		{
			KingdomGlobal::g_pMotosGame->fConsSlopeRad() = 0.0f;
		}
	}
}

void cTaskPanelMan::SetupPanel()
{
	for (uint num = 0u; (long)num < (long)::Constants::ARRAY_SIZE(m_pTaskPanels); num++)
	{
		m_pTaskPanels[num] = nullptr;
	}
	Mapoo->SearchObj(196608u, cMotosMapoo::SearchCB([this](DObjParam::sObj* o, cJetTask* a) { return SetupPanelCB(o, a); }), this);
}

bool cTaskPanelMan::SetupPanelCB(DObjParam::sObj* obj, cJetTask* args)
{
	int kind = -1;
	CustomKingdomUtil::Assert(obj != nullptr, "nullpo");
	CustomKingdomUtil::Assert(args != nullptr, "nullpo");
	Constants::ePANEL ePANEL = Constants::ePANEL::ePANEL_INVALID;
	int num = 0;
	switch (obj->m_type)
	{
	case 65537u:
		ePANEL = Constants::ePANEL::ePANEL_TYPE_N;
		kind = 0;
		break;
	case 65538u:
		ePANEL = Constants::ePANEL::ePANEL_N_B;
		kind = 1;
		break;
	case 65539u:
		ePANEL = Constants::ePANEL::ePANEL_N_C;
		kind = 2;
		break;
	case 65540u:
		ePANEL = Constants::ePANEL::ePANEL_TYPE_X;
		kind = 3;
		break;
	case 65541u:
		ePANEL = Constants::ePANEL::ePANEL_TYPE_W;
		kind = 5;
		break;
	case 65542u:
		ePANEL = Constants::ePANEL::ePANEL_W_B;
		kind = 6;
		break;
	case 65543u:
		ePANEL = Constants::ePANEL::ePANEL_W_C;
		kind = 7;
		num = 0;
		break;
	case 65545u:
		ePANEL = Constants::ePANEL::ePANEL_W_C;
		kind = 7;
		num = 90;
		break;
	case 65547u:
		ePANEL = Constants::ePANEL::ePANEL_W_D;
		kind = 8;
		num = 0;
		break;
	case 65548u:
		ePANEL = Constants::ePANEL::ePANEL_W_D;
		num = 180;
		kind = 8;
		break;
	case 65549u:
		ePANEL = Constants::ePANEL::ePANEL_W_D;
		kind = 8;
		num = 90;
		break;
	case 65550u:
		ePANEL = Constants::ePANEL::ePANEL_W_D;
		kind = 8;
		num = 270;
		break;
	case 65551u:
		ePANEL = Constants::ePANEL::ePANEL_W_E;
		kind = 9;
		break;
	case 65555u:
		ePANEL = Constants::ePANEL::ePANEL_W_F;
		kind = 10;
		num = 0;
		break;
	case 65556u:
		ePANEL = Constants::ePANEL::ePANEL_W_F;
		num = 180;
		kind = 10;
		break;
	case 65557u:
		ePANEL = Constants::ePANEL::ePANEL_W_F;
		num = 90;
		kind = 10;
		break;
	case 65558u:
		ePANEL = Constants::ePANEL::ePANEL_W_F;
		num = 270;
		kind = 10;
		break;
	case 131076u:
		ePANEL = Constants::ePANEL::ePANEL_TYPE_G;
		kind = 4;
		switch (obj->m_param[0])
		{
		case 0:
			num = 0;
			break;
		case 1:
			num = 180;
			break;
		case 2:
			num = 90;
			break;
		case 3:
			num = 270;
			break;
		}
		break;
	}
	if (ePANEL != Constants::ePANEL::ePANEL_INVALID)
	{
		cTaskPanelMan* cTaskPanelMan2 = static_cast<cTaskPanelMan*>(args);
		cTaskPanelMan2->SetupPanel(ePANEL, kind, obj->m_pos[0] >> 4, obj->m_pos[1] >> 4, ::Constants::JET_DEG2RAD(num));
		int num2 = obj->m_pos[0] >> 4;
		int num3 = obj->m_pos[1] >> 4;
		if (cTaskPanelMan2->pStageRect().x > num2)
		{
			cTaskPanelMan2->pStageRect().x = num2;
		}
		if (cTaskPanelMan2->pStageRect().w < num2)
		{
			cTaskPanelMan2->pStageRect().w = num2;
		}
		if (cTaskPanelMan2->pStageRect().y > num3)
		{
			cTaskPanelMan2->pStageRect().y = num3;
		}
		if (cTaskPanelMan2->pStageRect().h < num3)
		{
			cTaskPanelMan2->pStageRect().h = num3;
		}
	}
	return true;
}

bool cTaskPanelMan::SetupPanel(Constants::ePANEL type, int kind, int x, int z, float r)
{
	if (x < 0 || (long)x >= (long)m_nWidth || z < 0 || (long)z >= (long)m_nDepth)
	{
		return false;
	}
	uint num = (uint)(x + (long)z * (long)m_nWidth);
	CustomKingdomUtil::Assert((long)num < (long)::Constants::ARRAY_SIZE(m_pTaskPanels), "error");
	if (m_pTaskPanels[num] != nullptr)
	{
		return true;
	}
	m_pTaskPanels[num] = new cTaskPanel(type);
	cTaskPanel* cTaskPanel2 = m_pTaskPanels[num];
	cTaskPanel2->nID() = num + 1;
	Constants::PANEL_INFO* pANEL_INFO = cTaskPanel2->pPanelInfo();
	pANEL_INFO->m_nFlag = 67u;
	pANEL_INFO->m_nMapPos[0] = (uint)x;
	pANEL_INFO->m_nMapPos[1] = (uint)z;
	pANEL_INFO->m_vWTrans[0] = (float)x * m_fPanelSize + m_fPanelSize / 2.0f;
	pANEL_INFO->m_vWTrans[1] = 0.0f;
	pANEL_INFO->m_vWTrans[2] = (float)z * m_fPanelSize + m_fPanelSize / 2.0f;
	pANEL_INFO->m_vRot[0] = 0.0f;
	pANEL_INFO->m_vRot[1] = 0.0f;
	pANEL_INFO->m_vRot[2] = 0.0f;
	pANEL_INFO->m_nKind = (ushort)kind;
	switch (pANEL_INFO->m_nKind)
	{
	case 3:
	case 4:
		pANEL_INFO->m_nFlag |= 16u;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		pANEL_INFO->m_nFlag |= 32u;
		break;
	}
	cTaskPanel2->vTrans(0) = (float)x * m_fPanelSize + m_fPanelSize / 2.0f;
	cTaskPanel2->vTrans(1) = 0.0f;
	cTaskPanel2->vTrans(2) = (float)z * m_fPanelSize + m_fPanelSize / 2.0f;
	cTaskPanel2->vTrans(3) = 1.0f;
	cTaskPanel2->nMapPos(0u) = x;
	cTaskPanel2->nMapPos(1u) = z;
	cTaskPanel2->vRotate(1) = r;
	cTaskPanel2->fFrame(PSP_PanelFrameSet(x, z, m_nPanelPattern));
	cTaskPanel2->StepChange(Constants::ePANEL_STEP::ePANEL_STEP_NORMAL);
	ConnectChild(cTaskPanel2);
	return true;
}

uint cTaskPanelMan::SearchObjPanelPattern()
{
	Ref<uint>* obj = new Ref<uint>(2097153u);
	if (Mapoo->SearchObj(2097152u, cTaskPanelMan::PanelPatternCB, obj))
	{
		return obj->Value;
	}
	return 2097153u;
}

bool cTaskPanelMan::PanelPatternCB(DObjParam::sObj* obj, Ref<uint>* args)
{
	CustomKingdomUtil::Assert(obj != nullptr, "nullpo");
	CustomKingdomUtil::Assert(args != nullptr, "nullpo");
	CustomKingdomUtil::Assert(CustomKingdomUtil::IF(obj->m_type & 0x200000), "error");
	args->Value = obj->m_type;
	return false;
}

Constants::PANEL_INFO* cTaskPanelMan::pPanel(uint no)
{
	CustomKingdomUtil::Assert(no < 256, "array over!");
	if (m_pTaskPanels[no] != nullptr)
	{
		return m_pTaskPanels[no]->pPanelInfo();
	}
	return nullptr;
}

bool cTaskPanelMan::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->IsPause())
	{
		return true;
	}
	if (!Constants::MOTOS_IS_PAUSE())
	{
		MeteoMan();
		SlopeMan();
	}
	return true;
}

cTaskPanel* cTaskPanelMan::pTaskPanel(int x, int z)
{
	if (x < 0 || (long)x >= (long)m_nWidth || z < 0 || (long)z >= (long)m_nDepth)
	{
		return nullptr;
	}
	CustomKingdomUtil::Assert((long)((uint)(x + (long)z * (long)m_nWidth)) < (long)::Constants::ARRAY_SIZE(m_pTaskPanels), "error");
	return m_pTaskPanels[x + (long)z * (long)m_nWidth];
}

cTaskPanel* cTaskPanelMan::pTaskPanel(int id)
{
	id--;
	int x = (int)((long)id % (long)m_nWidth);
	int z = (int)((long)id / (long)m_nWidth);
	return pTaskPanel(x, z);
}

int cTaskPanelMan::FrameSet_random(int x, int z, int max, int w, int d)
{
	return Random::Range(0, max);
}

int cTaskPanelMan::FrameSet_sync(int x, int z, int max, int w, int d)
{
	return 0;
}

int cTaskPanelMan::FrameSet_none(int x, int z, int max, int w, int d)
{
	return 0;
}

int cTaskPanelMan::FrameSet_up(int x, int z, int max, int w, int d)
{
	return z * max / d;
}

int cTaskPanelMan::FrameSet_down(int x, int z, int max, int w, int d)
{
	return max - z * max / d;
}

int cTaskPanelMan::FrameSet_left(int x, int z, int max, int w, int d)
{
	return x * max / w;
}

int cTaskPanelMan::FrameSet_right(int x, int z, int max, int w, int d)
{
	return max - x * max / w;
}

int cTaskPanelMan::FrameSet_leftup(int x, int z, int max, int w, int d)
{
	return x * max / w + z * max / d;
}

int cTaskPanelMan::FrameSet_leftdown(int x, int z, int max, int w, int d)
{
	return x * max / w + (max - z * max / d);
}

int cTaskPanelMan::FrameSet_rightup(int x, int z, int max, int w, int d)
{
	return max - x * max / w + z * max / d;
}

int cTaskPanelMan::FrameSet_rightdown(int x, int z, int max, int w, int d)
{
	return max - x * max / w + (max - z * max / d);
}

int cTaskPanelMan::FrameSet_cross_v(int x, int z, int max, int w, int d)
{
	if (!CustomKingdomUtil::IF(x & 1))
	{
		return max - z * max / d;
	}
	return z * max / d;
}

int cTaskPanelMan::FrameSet_cross_h(int x, int z, int max, int w, int d)
{
	if (!CustomKingdomUtil::IF(z & 1))
	{
		return max - x * max / w;
	}
	return x * max / w;
}

int cTaskPanelMan::FrameSet_cross_ru(int x, int z, int max, int w, int d)
{
	if (!CustomKingdomUtil::IF((x + z) & 1))
	{
		return max - x * max / w + z * max / d;
	}
	return x * max / w + (max - z * max / d);
}

int cTaskPanelMan::FrameSet_cross_rd(int x, int z, int max, int w, int d)
{
	if (!CustomKingdomUtil::IF((x + z) & 1))
	{
		return max - x * max / w + (max - z * max / d);
	}
	return x * max / w + z * max / d;
}

int cTaskPanelMan::FrameSet_cross_x(int x, int z, int max, int w, int d)
{
	if (!CustomKingdomUtil::IF((x + z) & 1))
	{
		return max >> 1;
	}
	return 0;
}

int cTaskPanelMan::FrameSet_converge(int x, int z, int max, int w, int d)
{
	if (x < w / 2)
	{
		x++;
		if (z >= d / 2)
		{
			return max - x * max / w + z * max / d + 60;
		}
		return max - x * max / w + (max - z * max / d) + 60;
	}
	if (z >= d / 2)
	{
		return x * max / w + z * max / d + 60;
	}
	return x * max / w + (max - z * max / d) + 60;
}

int cTaskPanelMan::FrameSet_diverge(int x, int z, int max, int w, int d)
{
	if (x < w / 2)
	{
		if (z >= d / 2)
		{
			return x * max / w + (max - z * max / d);
		}
		return x * max / w + z * max / d;
	}
	if (z >= d / 2)
	{
		return max - x * max / w + (max - z * max / d);
	}
	return max - x * max / w + z * max / d;
}

float cTaskPanelMan::PSP_PanelFrameSet(int x, int z, uint nPatern)
{
	int max = 240;
	nPatern %= 2097152;
	CustomKingdomUtil::Assert(nPatern != 2097155, "error! panel type none....");
	using dir_func = int (*)(int, int, int, int, int);
	static const dir_func funcs[19] = {
		&cTaskPanelMan::FrameSet_random, &cTaskPanelMan::FrameSet_random, &cTaskPanelMan::FrameSet_sync, &cTaskPanelMan::FrameSet_none, &cTaskPanelMan::FrameSet_up,
		&cTaskPanelMan::FrameSet_down, &cTaskPanelMan::FrameSet_left, &cTaskPanelMan::FrameSet_right, &cTaskPanelMan::FrameSet_leftup, &cTaskPanelMan::FrameSet_leftdown,
		&cTaskPanelMan::FrameSet_rightup, &cTaskPanelMan::FrameSet_rightdown, &cTaskPanelMan::FrameSet_cross_v, &cTaskPanelMan::FrameSet_cross_h,
		&cTaskPanelMan::FrameSet_cross_ru, &cTaskPanelMan::FrameSet_cross_rd, &cTaskPanelMan::FrameSet_cross_x, &cTaskPanelMan::FrameSet_converge,
		&cTaskPanelMan::FrameSet_diverge};
	CustomKingdomUtil::Assert(nPatern < 19, "error! panel type out of range");
	return (float)funcs[nPatern](x, z, max, (int)m_nWidth, (int)m_nDepth);
}

void cTaskPanelMan::MeteoMan()
{
	if (Constants::IS_LANPLAY() || Constants::IS_LANPLAY() || !Constants::IS_MASTER() || KingdomGlobal::g_pMotosGame->nStep(0) != 8 || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) || m_MeteoInfo->m_nLimit <= 0)
	{
		return;
	}
	int num = 0;
	if (KingdomGlobal::g_pMotosGame->nMeteoTimer() >= m_MeteoInfo->m_nLimit)
	{
		MeteoShoot();
		num = m_MeteoInfo->m_nInterval_Max - m_MeteoInfo->m_nInterval_Min;
		if (num != 0)
		{
			num = Random::Range(0, num);
		}
		m_MeteoInfo->m_nLimit = KingdomGlobal::g_pMotosGame->nMeteoTimer() + m_MeteoInfo->m_nInterval_Min + num;
	}
}

bool cTaskPanelMan::IsBreakPanel(Constants::PANEL_INFO* pPanel)
{
	if (pPanel == nullptr || !CustomKingdomUtil::IF(pPanel->m_nFlag & 1) || !CustomKingdomUtil::IF(pPanel->m_nFlag & 2) || CustomKingdomUtil::IF(pPanel->m_nFlag & 8) || pPanel->m_nKind >= 3 || CustomKingdomUtil::IF(pPanel->m_nFlag & 0x10))
	{
		return false;
	}
	return true;
}

void cTaskPanelMan::MeteoShoot()
{
	Vector4 v = Vector4();
	int num = -1;
	int num2 = -1;
	if (Random::Range(0, 100) < 25)
	{
		int num3 = 0;
		int num4 = 0;
		int value = Random::Range(0, 2);
		switch (Random::Range(0, 4))
		{
		case 0:
			if (num3 == 0)
			{
				num4 = (int)Random::Range(0.0f, nWidth());
			}
			while (num3 < (int)nWidth())
			{
				num3++;
				for (int k = 0; k < (int)nDepth(); k++)
				{
					if (IsBreakPanel(SearchPanelNo(num4, k)))
					{
						num = num4;
						num2 = k;
						num3 = (int)nWidth();
						break;
					}
				}
				num4 = (int)(CustomKingdomUtil::IF(value) ? ((num4 - 1 < 0) ? ((int)(nWidth() - 1)) : (num4 - 1)) : ((long)(num4 + 1) % (long)nWidth()));
			}
			break;
		case 1:
			if (num3 == 0)
			{
				num4 = (int)Random::Range(0.0f, nWidth());
			}
			while (num3 < (int)nWidth())
			{
				num3++;
				for (int j = 0; j < (int)nDepth(); j++)
				{
					if (IsBreakPanel(SearchPanelNo(num4, (int)((long)nDepth() - (long)(j + 1)))))
					{
						num = num4;
						num2 = (int)((long)nDepth() - (long)(j + 1));
						num3 = (int)nWidth();
						break;
					}
				}
				num4 = (int)(CustomKingdomUtil::IF(value) ? ((num4 - 1 < 0) ? ((int)(nWidth() - 1)) : (num4 - 1)) : ((long)(num4 + 1) % (long)nWidth()));
			}
			break;
		case 2:
			if (num3 == 0)
			{
				num4 = (int)Random::Range(0.0f, nDepth());
			}
			while (num3 < (int)nDepth())
			{
				num3++;
				for (int l = 0; l < (int)nWidth(); l++)
				{
					if (IsBreakPanel(SearchPanelNo(l, num4)))
					{
						num = l;
						num2 = num4;
						num3 = (int)nDepth();
						break;
					}
				}
				num4 = (int)(CustomKingdomUtil::IF(value) ? ((num4 - 1 < 0) ? ((int)(nDepth() - 1)) : (num4 - 1)) : ((long)(num4 + 1) % (long)nDepth()));
			}
			break;
		default:
			if (num3 == 0)
			{
				num4 = (int)Random::Range(0.0f, nDepth());
			}
			while (num3 < (int)nDepth())
			{
				num3++;
				for (int i = 0; i < (int)nWidth(); i++)
				{
					if (IsBreakPanel(SearchPanelNo((int)((long)nWidth() - (long)(i + 1)), num4)))
					{
						num = (int)((long)nWidth() - (long)(i + 1));
						num2 = num4;
						num3 = (int)nDepth();
						break;
					}
				}
				num4 = (int)(CustomKingdomUtil::IF(value) ? ((num4 - 1 < 0) ? ((int)(nDepth() - 1)) : (num4 - 1)) : ((long)(num4 + 1) % (long)nDepth()));
			}
			break;
		}
	}
	float yuragi;
	if (num != -1 && num2 != -1)
	{
		KingdomGlobal::g_pMotosGame->pMap()->World2Map(v, SearchPanelNo(num, num2)->m_vWTrans);
		yuragi = 0.3f;
	}
	else
	{
		v[0] = 0.0f;
		v[1] = 0.0f;
		v[2] = -0.4f;
		v[3] = 1.0f;
		Constants::tspVector3RotY(v, v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vRotate()[1]);
		Constants::VEC3_ADD(v, v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		yuragi = 0.8f;
	}
	cTaskChar* cTaskChar2 = nullptr;
	for (int m = 0; m < 32; m++)
	{
		cTaskChar2 = KingdomGlobal::g_pMotosGame->pEnemys((uint)m);
		if (cTaskChar2 != nullptr && cTaskChar2->nKind() == 15 && (double)Constants::VEC3_DIST_SQUARE(cTaskChar2->vMTrans(), v) < 9.0)
		{
			Vector4 _out;
			Constants::VEC3_SUB(_out, v, cTaskChar2->vMTrans());
			Constants::Motos_P_vecNormalize(_out, _out);
			Constants::VEC3_MUL(_out, _out, 3.0f);
			Constants::VEC3_ADD(v, cTaskChar2->vMTrans(), _out);
		}
	}
	float num5 = nWidth();
	float num6 = nDepth();
	Constants::VEC3_SET(m_MeteoInfo->m_vShootPos[0], 1.0f, 10.0f, 1.0f);
	Constants::VEC3_SET(m_MeteoInfo->m_vShootPos[1], num5 - 1.0f, 10.0f, 1.0f);
	Constants::VEC3_SET(m_MeteoInfo->m_vShootPos[2], 1.0f, 10.0f, num6 - 1.0f);
	Constants::VEC3_SET(m_MeteoInfo->m_vShootPos[3], num5 - 1.0f, 10.0f, num6 - 1.0f);
	MeteoSpawn(v, 0.3f, m_MeteoInfo->m_vShootPos[nMeteoShootPoint(v)], yuragi, 100);
	SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE028_METEO);
}

int cTaskPanelMan::nMeteoShootPoint(const Vector4& target)
{
	int num = 0;
	Vector4 _out;
	Constants::VEC3_SET(_out, (float)nWidth() * 0.5f, 0.0f, (float)nDepth() * 0.5f);
	if (target[0] < _out[0])
	{
		if (target[2] < _out[2])
		{
			return 3;
		}
		return 1;
	}
	if (target[2] < _out[2])
	{
		return 2;
	}
	return 0;
}

int cTaskPanelMan::nMeteoShootPoint()
{
	return nMeteoShootPoint(KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
}

void cTaskPanelMan::Map2World(Vector4 pWorld, const Vector4& pMap)
{
	::Constants::jetGeoVecCopy(pWorld, pMap);
}

void cTaskPanelMan::World2Map(Vector4 pMap, const Vector4& pWorld)
{
	::Constants::jetGeoVecCopy(pMap, pWorld);
}

bool cTaskPanelMan::IsFall(float fx, float fy, float fz)
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	v[0] = fx;
	v[1] = fy;
	v[2] = fz;
	return IsFall(v);
}

bool cTaskPanelMan::IsFall(Vector4 pPos)
{
	Constants::PANEL_INFO* pANEL_INFO = SearchPanel(pPos[0], pPos[2]);
	if (pANEL_INFO != nullptr && CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 1) && CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 2) && (!CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 8) || (CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 8) && pANEL_INFO->m_nTimer < 15)))
	{
		return false;
	}
	{
		cTaskGimmickBase* gimmick = KingdomGlobal::g_pMotosGame->pMap()->SearchGimmick(pPos[0], pPos[2]);   // C# `SearchGimmick(..)?.IsFall(pPos) ?? true`
		return gimmick != nullptr ? gimmick->IsFall(pPos) : true;
	}
}

bool cTaskPanelMan::IsMove(Vector4 pPos)
{
	Constants::PANEL_INFO* pANEL_INFO = SearchPanel(pPos);
	if (pANEL_INFO != nullptr && CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 0x20))
	{
		return false;
	}
	return true;
}

Constants::PANEL_INFO* cTaskPanelMan::SearchPanel(float mx, float mz)
{
	CustomKingdomUtil::Assert(m_fPanelSize != 0.0f, "error!");
	int x = (int)Mathf::Floor(mx / m_fPanelSize);
	int z = (int)Mathf::Floor(mz / m_fPanelSize);
	return SearchPanelNo(x, z);
}

Constants::PANEL_INFO* cTaskPanelMan::SearchPanelNo(int x, int z)
{
	if (x < 0 || (long)x >= (long)m_nWidth || z < 0 || (long)z >= (long)m_nDepth)
	{
		return nullptr;
	}
	return pPanel((uint)(x + (long)z * (long)m_nWidth));
}

int cTaskPanelMan::HitCheck(Vector4 pPos, Vector4 pVec, float fRange)
{
	int num = 0;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pPos);
	Vector4 _out;
	Constants::VEC3_ADD(_out, pPos, pVec);
	Map2World(v, v);
	Map2World(_out, _out);
	int num2 = (int)((Constants::MIN(v[0], _out[0]) - fRange) / m_fPanelSize) - 1;
	int num3 = (int)((Constants::MIN(v[2], _out[2]) - fRange) / m_fPanelSize) - 1;
	int num4 = (int)((Constants::MAX(v[0], _out[0]) + fRange) / m_fPanelSize) + 1;
	int num5 = (int)((Constants::MAX(v[2], _out[2]) + fRange) / m_fPanelSize) + 1;
	(void)((v[0] - fRange) / m_fPanelSize);
	(void)((v[2] - fRange) / m_fPanelSize);
	(void)((v[0] + fRange) / m_fPanelSize);
	(void)((v[2] + fRange) / m_fPanelSize);
	for (int i = num3; i <= num5; i++)
	{
		for (int j = num2; j <= num4; j++)
		{
			if (num < 4)
			{
				Constants::PANEL_INFO* pANEL_INFO = SearchPanelNo(j, i);
				if (pANEL_INFO != nullptr && CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 0x20) && GetResHitCheck(pANEL_INFO, m_HitResult[num], pPos, pVec, fRange))
				{
					num++;
				}
			}
		}
	}
	return num;
}

bool cTaskPanelMan::GetResHitCheck(Constants::PANEL_INFO* pPanel, MAP_HIT_RESULT* pOut, Vector4 pPos, Vector4 pVec, float fRange)
{
	CustomKingdomUtil::Assert(KingdomGlobal::g_pMotosGame != nullptr, "nullpo!");
	Vector4 vWorld;
	KingdomGlobal::g_pMotosGame->pMap()->Map2World(vWorld, pPos);
	Vector4 vWorld2;
	KingdomGlobal::g_pMotosGame->pMap()->Map2World(vWorld2, pVec);
	Vector4 normal;
	Vector4 pCrs;
	uint num = (uint)HitCheckSub(pPanel, vWorld, vWorld2, fRange, normal, pCrs);
	if (num != 0)
	{
		pOut->m_nFlag = num;
		::Constants::jetGeoVecCopy(pOut->m_vNormal, normal);
		::Constants::jetGeoVecCopy(pOut->m_vCrs, pCrs);
		return true;
	}
	return false;
}

void cTaskPanelMan::SetPlayer()
{
}

void cTaskPanelMan::SetEnemy()
{
}

void cTaskPanelMan::SetItem()
{
	Constants::MOTOS_NO_SOURCE("SetItem");
}

int cTaskPanelMan::AvoidPingBall(cTaskChar* pChar0, cTaskChar* pChar1, int axis)
{
	if (pChar0 != nullptr && pChar1 != nullptr)
	{
		float num = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->fPanelSize();
		int array[2] = {};
		int array2[2] = {};
		array[0] = (int)Mathf::Floor(pChar0->vMTrans()[0] / num);
		array[1] = (int)Mathf::Floor(pChar0->vMTrans()[2] / num);
		array2[0] = (int)Mathf::Floor(pChar1->vMTrans()[0] / num);
		array2[1] = (int)Mathf::Floor(pChar1->vMTrans()[2] / num);
		Constants::PANEL_INFO* pANEL_INFO;
		Constants::PANEL_INFO* pANEL_INFO2;
		switch (axis)
		{
		case 0:
			pANEL_INFO = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array[0], array[1] + 1);
			pANEL_INFO2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array2[0], array2[1] - 1);
			break;
		case 4:
			pANEL_INFO = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array[0], array[1] - 1);
			pANEL_INFO2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array2[0], array2[1] + 1);
			break;
		case 2:
			pANEL_INFO = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array[0] + 1, array[1]);
			pANEL_INFO2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array2[0] - 1, array2[1]);
			break;
		case 6:
			pANEL_INFO = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array[0] - 1, array[1]);
			pANEL_INFO2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->SearchPanelNo(array2[0] + 1, array2[1]);
			break;
		default:
			pANEL_INFO = nullptr;
			pANEL_INFO2 = nullptr;
			break;
		}
		if (pANEL_INFO != nullptr && pANEL_INFO2 != nullptr && CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 0x20) && CustomKingdomUtil::IF(pANEL_INFO2->m_nFlag & 0x20))
		{
			axis = (axis + 2) & 7;
		}
	}
	return axis;
}

int cTaskPanelMan::HitCheckSubRect(Vector4 pP0, Vector4 pP1, float fRange, Vector4 pPoint)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pP0);
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, pP1);
	Vector4 v3;
	::Constants::jetGeoVecCopy(v3, v);
	Vector4 v4;
	::Constants::jetGeoVecCopy(v4, v2);
	v3[0] -= fRange;
	v3[2] -= fRange;
	v4[0] += fRange;
	v4[2] += fRange;
	if (v3[0] > pPoint[0] || v4[0] < pPoint[0])
	{
		return 0;
	}
	if (v3[2] > pPoint[2] || v4[2] < pPoint[2])
	{
		return 0;
	}
	Vector4 v5 = Vector4();
	int num = 0;
	if (v[0] > pPoint[0] && v[2] > pPoint[2])
	{
		::Constants::jetGeoVecCopy(v5, v);
		num = 1;
	}
	if (v2[0] < pPoint[0] && v[2] > pPoint[2])
	{
		Constants::VEC3_SET(v5, v2[0], v[1], v[2]);
		num = 2;
	}
	if (v[0] > pPoint[0] && v2[2] < pPoint[2])
	{
		Constants::VEC3_SET(v5, v[0], v[1], v2[2]);
		num = 3;
	}
	if (v2[0] < pPoint[0] && v2[2] < pPoint[2])
	{
		::Constants::jetGeoVecCopy(v5, v2);
		num = 4;
	}
	if (num != 0)
	{
		if (Constants::VEC3_DIST_SQUARE(v5, pPoint) > fRange * fRange)
		{
			return 0;
		}
		return num;
	}
	return 5;
}

bool cTaskPanelMan::HitCheckSubCrsLine(Vector4& pOut, const Vector4& pP0, const Vector4& pP1, const Vector4& pV0, const Vector4& pV1)
{
	pOut = Vector4();
	bool flag = false;
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	Vector4 v2;
	Vector4 v3;
	Vector4 v4;
	Vector4 v5;
	if (Mathf::Approximately(pV0[0], pV1[0]))
	{
		if ((pP0[0] <= pV0[0] && pP1[0] >= pV0[0]) || (pP1[0] <= pV0[0] && pP0[0] >= pV0[0]))
		{
			if (pP0[2] < pP1[2])
			{
				::Constants::jetGeoVecCopy(v2, pP0);
				::Constants::jetGeoVecCopy(v3, pP1);
			}
			else
			{
				::Constants::jetGeoVecCopy(v2, pP1);
				::Constants::jetGeoVecCopy(v3, pP0);
			}
			if (pV0[2] < pV1[2])
			{
				::Constants::jetGeoVecCopy(v4, pV0);
				::Constants::jetGeoVecCopy(v5, pV1);
			}
			else
			{
				::Constants::jetGeoVecCopy(v4, pV1);
				::Constants::jetGeoVecCopy(v5, pV0);
			}
			float num = v4[0] - v2[0];
			float num2 = v3[0] - v4[0];
			v[0] = v4[0];
			v[1] = v4[1];
			if (CustomKingdomUtil::IF(num + num2))
			{
				v[2] = v2[2] + (v3[2] - v2[2]) * num / (num + num2);
				if (v4[2] <= v[2] && v5[2] >= v[2])
				{
					flag = true;
				}
			}
			else
			{
				flag = true;
				if (v3[2] < v4[2])
				{
					flag = false;
				}
				if (v2[2] > v5[2])
				{
					flag = false;
				}
				if (flag)
				{
					if (v2[2] > v4[2])
					{
						v[2] = v2[2];
					}
					else
					{
						v[2] = v3[2];
					}
				}
			}
		}
	}
	else if (Mathf::Approximately(pV0[2], pV1[2]) && ((pP0[2] <= pV0[2] && pP1[2] >= pV0[2]) || (pP1[2] <= pV0[2] && pP0[2] >= pV0[2])))
	{
		if (pP0[0] < pP1[0])
		{
			::Constants::jetGeoVecCopy(v2, pP0);
			::Constants::jetGeoVecCopy(v3, pP1);
		}
		else
		{
			::Constants::jetGeoVecCopy(v2, pP1);
			::Constants::jetGeoVecCopy(v3, pP0);
		}
		if (pV0[0] < pV1[0])
		{
			::Constants::jetGeoVecCopy(v4, pV0);
			::Constants::jetGeoVecCopy(v5, pV1);
		}
		else
		{
			::Constants::jetGeoVecCopy(v4, pV1);
			::Constants::jetGeoVecCopy(v5, pV0);
		}
		float num = v4[2] - v2[2];
		float num2 = v3[2] - v4[2];
		v[1] = v4[1];
		v[2] = v4[2];
		if (CustomKingdomUtil::IF(num + num2))
		{
			v[0] = v2[0] + (v3[0] - v2[0]) * num / (num + num2);
			if (v4[0] <= v[0] && v5[0] >= v[0])
			{
				flag = true;
			}
		}
		else
		{
			flag = true;
			if (v3[0] < v4[0])
			{
				flag = false;
			}
			if (v2[0] > v5[0])
			{
				flag = false;
			}
			if (flag)
			{
				if (v2[0] > v4[0])
				{
					v[0] = v2[0];
				}
				else
				{
					v[0] = v3[0];
				}
			}
		}
	}
	if (flag)
	{
		::Constants::jetGeoVecCopy(pOut, v);
	}
	return flag;
}

int cTaskPanelMan::HitCheckSub(Constants::PANEL_INFO* pPanel, Vector4 pos, Vector4 vec, float fRange, Vector4& normal, Vector4& pCrs)
{
	normal = Vector4();
	pCrs = Vector4();
	int num = 0;
	Vector4 pOut = Vector4();
	Vector4 v = Vector4();
	Vector4 v2 = Vector4();
	Vector4 v3;
	::Constants::jetGeoVecCopy(v3, pos);
	Vector4 _out;
	Constants::VEC3_ADD(_out, pos, vec);
	v[0] = pPanel->m_vWTrans[0] - m_fPanelSize / 2.0f;
	v[1] = pPanel->m_vWTrans[1];
	v[2] = pPanel->m_vWTrans[2] - m_fPanelSize / 2.0f;
	v2[0] = pPanel->m_vWTrans[0] + m_fPanelSize / 2.0f;
	v2[1] = pPanel->m_vWTrans[1];
	v2[2] = pPanel->m_vWTrans[2] + m_fPanelSize / 2.0f;
	Vector4 v4;
	::Constants::jetGeoVecCopy(v4, v);
	Vector4 v5;
	::Constants::jetGeoVecCopy(v5, v2);
	v4[0] -= fRange;
	v4[2] -= fRange;
	v5[0] += fRange;
	v5[2] += fRange;
	if (v4[0] > _out[0] || v5[0] < _out[0])
	{
		return 0;
	}
	if (v4[2] > _out[2] || v5[2] < _out[2])
	{
		return 0;
	}
	Constants::VEC3_SET(EdgeInfo[0]->vOutV0, v4[0], v4[1], v[2]);
	Constants::VEC3_SET(EdgeInfo[0]->vOutV1, v4[0], v4[1], v2[2]);
	Constants::VEC3_SET(EdgeInfo[0]->vGOutV0, v4[0], v4[1], v4[2] - 1.0f);
	Constants::VEC3_SET(EdgeInfo[0]->vGOutV1, v4[0], v4[1], v5[2] + 1.0f);
	Constants::VEC3_SET(EdgeInfo[1]->vOutV0, v[0], v4[1], v4[2]);
	Constants::VEC3_SET(EdgeInfo[1]->vOutV1, v2[0], v4[1], v4[2]);
	Constants::VEC3_SET(EdgeInfo[1]->vGOutV0, v4[0] - 1.0f, v4[1], v4[2]);
	Constants::VEC3_SET(EdgeInfo[1]->vGOutV1, v5[0] + 1.0f, v4[1], v4[2]);
	Constants::VEC3_SET(EdgeInfo[2]->vOutV0, v5[0], v4[1], v[2]);
	Constants::VEC3_SET(EdgeInfo[2]->vOutV1, v5[0], v4[1], v2[2]);
	Constants::VEC3_SET(EdgeInfo[2]->vGOutV0, v5[0], v4[1], v4[2] - 1.0f);
	Constants::VEC3_SET(EdgeInfo[2]->vGOutV1, v5[0], v4[1], v5[2] + 1.0f);
	Constants::VEC3_SET(EdgeInfo[3]->vOutV0, v[0], v4[1], v5[2]);
	Constants::VEC3_SET(EdgeInfo[3]->vOutV1, v2[0], v4[1], v5[2]);
	Constants::VEC3_SET(EdgeInfo[3]->vGOutV0, v4[0] - 1.0f, v4[1], v5[2]);
	Constants::VEC3_SET(EdgeInfo[3]->vGOutV1, v5[0] + 1.0f, v4[1], v5[2]);
	int num2 = HitCheckSubRect(v, v2, fRange, _out);
	if (num2 != 0)
	{
		if (num2 == 5)
		{
			auto& edgeInfo = EdgeInfo;
			for (int i = 0; i < 4; i++)
			{
				if (HitCheckSubCrsLine(pOut, v3, _out, edgeInfo[i]->vOutV0, edgeInfo[i]->vOutV1))
				{
					::Constants::jetGeoVecCopy(normal, edgeInfo[i]->vNormal);
					::Constants::jetGeoVecCopy(pCrs, pOut);
					return edgeInfo[i]->nFlag;
				}
			}
			float num3 = 65535.0f;
			Vector4 _out2 = Vector4();
			for (int j = 0; j < 4; j++)
			{
				switch (j)
				{
				case 0:
					Constants::VEC3_SET(_out2, v[0], v[1], v[2]);
					break;
				case 1:
					Constants::VEC3_SET(_out2, v2[0], v[1], v[2]);
					break;
				case 2:
					Constants::VEC3_SET(_out2, v[0], v[1], v2[2]);
					break;
				case 3:
					Constants::VEC3_SET(_out2, v2[0], v[1], v2[2]);
					break;
				}
				float num4 = Constants::VEC3_DIST_SQUARE(_out2, v3);
				if (num3 == 65535.0f || num3 > num4)
				{
					num3 = num4;
					Vector4 _discard11;
					::Constants::jetGeoVecCopy(_discard11, _out2);
					num = j + 1;
				}
			}
		}
		else
		{
			num = num2;
		}
		Constants::PANEL_INFO* array[2] = {};
		cTaskPanelMan::EDGE_INFO* eDGE_INFO = nullptr;
		array[0] = (array[1] = nullptr);
		switch (num)
		{
		case 1:
			array[0] = SearchPanelNo((int)pPanel->m_nMapPos[0], (int)(pPanel->m_nMapPos[1] - 1));
			array[1] = SearchPanelNo((int)(pPanel->m_nMapPos[0] - 1), (int)pPanel->m_nMapPos[1]);
			break;
		case 2:
			array[0] = SearchPanelNo((int)pPanel->m_nMapPos[0], (int)(pPanel->m_nMapPos[1] - 1));
			array[1] = SearchPanelNo((int)(pPanel->m_nMapPos[0] + 1), (int)pPanel->m_nMapPos[1]);
			break;
		case 3:
			array[0] = SearchPanelNo((int)pPanel->m_nMapPos[0], (int)(pPanel->m_nMapPos[1] + 1));
			array[1] = SearchPanelNo((int)(pPanel->m_nMapPos[0] - 1), (int)pPanel->m_nMapPos[1]);
			break;
		case 4:
			array[0] = SearchPanelNo((int)pPanel->m_nMapPos[0], (int)(pPanel->m_nMapPos[1] + 1));
			array[1] = SearchPanelNo((int)(pPanel->m_nMapPos[0] + 1), (int)pPanel->m_nMapPos[1]);
			break;
		}
		if (array[0] != nullptr && CustomKingdomUtil::IF(array[0]->m_nFlag & 0x20) && array[1] != nullptr && CustomKingdomUtil::IF(array[1]->m_nFlag & 0x20))
		{
			return 0;
		}
		if (array[0] != nullptr && CustomKingdomUtil::IF(array[0]->m_nFlag & 0x20))
		{
			eDGE_INFO = ((!CustomKingdomUtil::IF(num & 1)) ? EdgeInfo[2] : EdgeInfo[0]);
		}
		if (array[1] != nullptr && CustomKingdomUtil::IF(array[1]->m_nFlag & 0x20))
		{
			eDGE_INFO = ((num > 2) ? EdgeInfo[3] : EdgeInfo[1]);
		}
		if (eDGE_INFO != nullptr && HitCheckSubCrsLine(pOut, v3, _out, eDGE_INFO->vGOutV0, eDGE_INFO->vGOutV1))
		{
			::Constants::jetGeoVecCopy(normal, eDGE_INFO->vNormal);
			::Constants::jetGeoVecCopy(pCrs, pOut);
			return eDGE_INFO->nFlag;
		}
		Vector4 _out3 = Vector4();
		switch (num)
		{
		case 1:
			Constants::VEC3_SET(_out3, v[0], v[1], v[2]);
			break;
		case 2:
			Constants::VEC3_SET(_out3, v2[0], v[1], v[2]);
			break;
		case 3:
			Constants::VEC3_SET(_out3, v[0], v[1], v2[2]);
			break;
		case 4:
			Constants::VEC3_SET(_out3, v2[0], v[1], v2[2]);
			break;
		}
		Vector4 _discard14;
		if (CustomKingdomUtil::IF(Constants::Motos_P_vecCrossCircle(pOut, _discard14, v3, _out, _out3, fRange)))
		{
			Vector4 _discard15;
			Constants::VEC3_SUB(_discard15, pOut, _out3);
			::Constants::jetGeoVecCopy(pCrs, pOut);
			Constants::VEC3_SUB(normal, pOut, _out3);
			Constants::Motos_P_vecNormalize(normal, normal);
			return 4;
		}
	}
	return 0;
}

int cTaskPanelMan::StrikePanel(int id, int level, float range, float mx, float mz, bool spawnEffect)
{
	float num = 0.70710677f;
	Constants::PANEL_INFO* array[4] = {};
	int num2 = 0;
	int num3 = 1;
	num *= range;
	for (int i = 0; i < 4; i++)
	{
		float mx2;
		float mz2;
		switch (i)
		{
		default:
			mx2 = mx - num;
			mz2 = mz - num;
			break;
		case 1:
			mx2 = mx + num;
			mz2 = mz - num;
			break;
		case 2:
			mx2 = mx - num;
			mz2 = mz + num;
			break;
		case 3:
			mx2 = mx + num;
			mz2 = mz + num;
			break;
		}
		Constants::PANEL_INFO* pANEL_INFO = SearchPanel(mx2, mz2);
		int j;
		for (j = 0; j < num2 && pANEL_INFO != array[j]; j++)
		{
		}
		if (j == num2)
		{
			array[num2] = pANEL_INFO;
			num2++;
		}
	}
	for (int i = 0; i < num2; i++)
	{
		num3 |= StrikePanel(id, level, array[i], spawnEffect);
	}
	return num3;
}

int cTaskPanelMan::StrikePanel(int id, int level, Constants::PANEL_INFO* pPanel, bool spawnEffect)
{
	if (pPanel != nullptr && level != 0)
	{
		cTaskPanel* cTaskPanel2 = pPanel->m_pTaskPanel;
		CustomKingdomUtil::Assert(cTaskPanel2 != nullptr, "nullpo");
		if (!CustomKingdomUtil::IF(pPanel->m_nFlag & 1) || !CustomKingdomUtil::IF(pPanel->m_nFlag & 2) || CustomKingdomUtil::IF(pPanel->m_nFlag & 8))
		{
			return 1;
		}
		if (pPanel->m_nKind >= 3 || CustomKingdomUtil::IF(pPanel->m_nFlag & 0x10))
		{
			return 2;
		}
		uint result = 4u;
		if (level == 1)
		{
			short nStep = pPanel->m_nStep;
			if (nStep != 0)
			{
				if (nStep == 1)
				{
					cTaskPanel2->StepChange(Constants::ePANEL_STEP::ePANEL_STEP_FALLING);
					if (spawnEffect)
					{
						Constants::SpawnEffect(18, pPanel->m_vWTrans);
					}
					result = 4u;
				}
				result = 1u;
			}
			else
			{
				if (spawnEffect)
				{
					Constants::SpawnEffect(18, pPanel->m_vWTrans);
				}
				cTaskPanel2->StepChange(Constants::ePANEL_STEP::ePANEL_STEP_CRACK);
			}
		}
		else
		{
			short nStep = pPanel->m_nStep;
			if ((uint)nStep <= 1u)
			{
				cTaskPanel2->StepChange(Constants::ePANEL_STEP::ePANEL_STEP_FALLING);
				if (spawnEffect)
				{
					Constants::SpawnEffect(18, pPanel->m_vWTrans);
				}
			}
			else
			{
				result = 1u;
			}
		}
		return (int)result;
	}
	return 1;
}

Constants::PANEL_INFO* cTaskPanelMan::SearchPanelID(int PanelID)
{
	PanelID--;
	int x = (int)((long)PanelID % (long)m_nWidth);
	int z = (int)((long)PanelID / (long)m_nWidth);
	return SearchPanelNo(x, z);
}

bool cTaskPanelMan::MeteoSpawn(Vector4 pTarget, float speed, Vector4 pShootPos, float yuragi, int seid)
{
	if (CustomKingdomUtil::IF(speed))
	{
		if (m_pMeteoTask == nullptr)
		{
			m_pMeteoTask = new cTaskCharMeteo();
			CustomKingdomUtil::Assert(m_pMeteoTask != nullptr, "nullpo");
			m_pMeteoTask->Create(0u);
			ConnectChild(m_pMeteoTask);
		}
		Vector4 v;
		::Constants::jetGeoVecCopy(v, pTarget);
		if (seid != 0)
		{
			v[0] += yuragi * ((float)(Random::Range(0, seid) * 2 - seid) / (float)seid);
			v[2] += yuragi * ((float)(Random::Range(0, seid) * 2 - seid) / (float)seid);
		}
		::Constants::jetGeoVecCopy(m_pMeteoTask->vMTrans(), pShootPos);
		::Constants::jetGeoVecCopy(m_pMeteoTask->vTarget(), v);
		m_pMeteoTask->fSpeed() = speed;
		Constants::VEC3_SUB(m_pMeteoTask->vSpeed(), m_pMeteoTask->vTarget(), m_pMeteoTask->vMTrans());
		Constants::Motos_P_vecNormalize(m_pMeteoTask->vSpeed(), m_pMeteoTask->vSpeed());
		Constants::VEC3_MUL(m_pMeteoTask->vSpeed(), m_pMeteoTask->vSpeed(), speed);
	}
	return true;
}

int cTaskPanelMan::AdjustPanel(Vector4& pOut, Vector4 pPos, float fRange, int _base)
{
	pOut = Vector4();
	int num = 0;
	Vector4 _out = Vector4();
	Constants::PANEL_INFO* pANEL_INFO = nullptr;
	Constants::PANEL_INFO* pANEL_INFO2 = nullptr;
	int num2 = Mathf::FloorToInt(pPos[0] / fPanelSize());
	int num3 = Mathf::FloorToInt(pPos[2] / fPanelSize());
	for (int i = 0; i < 9; i++)
	{
		int x;
		int z;
		switch (i)
		{
		default:
			x = num2;
			z = num3 - 1;
			break;
		case 1:
			x = num2;
			z = num3 + 1;
			break;
		case 2:
			x = num2 - 1;
			z = num3;
			break;
		case 3:
			x = num2 + 1;
			z = num3;
			break;
		case 4:
			x = num2 - 1;
			z = num3 - 1;
			break;
		case 5:
			x = num2 + 1;
			z = num3 - 1;
			break;
		case 6:
			x = num2 - 1;
			z = num3 + 1;
			break;
		case 7:
			x = num2 + 1;
			z = num3 + 1;
			break;
		case 8:
			x = num2;
			z = num3;
			break;
		}
		pANEL_INFO = SearchPanelNo(x, z);
		if (i == 8)
		{
			pANEL_INFO2 = pANEL_INFO;
		}
		if (pANEL_INFO == nullptr)
		{
			level[i] = -1;
		}
		else if (CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 8))
		{
			level[i] = -1;
		}
		else if (!CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 2))
		{
			level[i] = -1;
		}
		else if (CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 0x20))
		{
			level[i] = 1;
		}
		else
		{
			level[i] = 0;
		}
		cTaskGimmickBase* cTaskGimmickBase2 = KingdomGlobal::g_pMotosGame->pMap()->SearchGimmickNo(x, z);
		if (cTaskGimmickBase2 != nullptr)
		{
			if (cTaskGimmickBase2->IsFall(pPos))
			{
				level[i] = -1;
			}
			else
			{
				level[i] = 0;
			}
		}
	}
	for (int j = 0; j < 4; j++)
	{
		if (_base <= level[j] && level[8] < level[j])
		{
			num |= s_nDirs[j];
		}
	}
	for (int k = 4; k < 8; k++)
	{
		bool flag = false;
		if (_base <= level[k] && level[8] < level[k])
		{
			switch (k)
			{
			case 4:
			_switch18_4:
				if (level[8] >= level[0] && level[8] >= level[2])
				{
					flag = true;
					break;
				}
				goto _switch18_5;
			case 5:
			_switch18_5:
				if (level[8] >= level[0] && level[8] >= level[3])
				{
					flag = true;
					break;
				}
				goto _switch18_6;
			case 6:
			_switch18_6:
				if (level[8] >= level[1] && level[8] >= level[2])
				{
					flag = true;
					break;
				}
				goto _switch18_7;
			case 7:
			_switch18_7:
				if (level[8] >= level[1] && level[8] >= level[3])
				{
					flag = true;
				}
				break;
			}
		}
		if (flag)
		{
			num |= s_nDirs[k];
		}
	}
	if (num != 0)
	{
		if (pANEL_INFO2 != nullptr)
		{
			KingdomGlobal::g_pMotosGame->pMap()->Map2World(_out, pANEL_INFO2->m_vWTrans);
			_out[0] -= fPanelSize() / 2.0f;
			_out[2] -= fPanelSize() / 2.0f;
		}
		else
		{
			Constants::VEC3_SET(_out, Mathf::Floor(pPos[0] / fPanelSize()) * fPanelSize(), pPos[1], Mathf::Floor(pPos[2] / fPanelSize()) * fPanelSize());
		}
		::Constants::jetGeoVecCopy(pOut, pPos);
		if (CustomKingdomUtil::IF(num & 1) && pPos[2] < _out[2] + fRange && !Mathf::Approximately(pPos[2], _out[2] + fRange))
		{
			pOut[2] = _out[2] + fRange;
		}
		else
		{
			num &= -2;
		}
		if (CustomKingdomUtil::IF(num & 2) && pPos[2] > _out[2] + fPanelSize() - fRange && !Mathf::Approximately(pPos[2], _out[2] + fPanelSize() - fRange))
		{
			pOut[2] = _out[2] + fPanelSize() - fRange;
		}
		else
		{
			num &= -3;
		}
		if (CustomKingdomUtil::IF(num & 4) && pPos[0] < _out[0] + fRange && !Mathf::Approximately(pPos[0], _out[0] + fRange))
		{
			pOut[0] = _out[0] + fRange;
		}
		else
		{
			num &= -5;
		}
		if (CustomKingdomUtil::IF(num & 8) && pPos[0] > _out[0] + fPanelSize() - fRange && !Mathf::Approximately(pPos[0], _out[0] + fPanelSize() - fRange))
		{
			pOut[0] = _out[0] + fPanelSize() - fRange;
		}
		else
		{
			num &= -9;
		}
	}
	return num;
}

int cTaskPanelMan::ForceAdjustPanel(Vector4& pOut, Vector4 pPos, float fRange, int input_dir)
{
	int num = 0;
	Vector4 v = Vector4();
	Constants::PANEL_INFO* pANEL_INFO = nullptr;
	int num2 = (int)Mathf::Floor(pPos[0] / fPanelSize());
	int num3 = (int)Mathf::Floor(pPos[2] / fPanelSize());
	for (int i = 0; i < 5; i++)
	{
		int x;
		int z;
		switch (i)
		{
		default:
			x = num2;
			z = num3 - 1;
			break;
		case 1:
			x = num2;
			z = num3 + 1;
			break;
		case 2:
			x = num2 - 1;
			z = num3;
			break;
		case 3:
			x = num2 + 1;
			z = num3;
			break;
		case 4:
			x = num2;
			z = num3;
			break;
		}
		pANEL_INFO = SearchPanelNo(x, z);
		(void)(4);
		if (pANEL_INFO == nullptr)
		{
			level[i] = -1;
		}
		else if (CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 8))
		{
			level[i] = -1;
		}
		else if (!CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 2))
		{
			level[i] = -1;
		}
		else if (CustomKingdomUtil::IF(pANEL_INFO->m_nFlag & 0x20))
		{
			level[i] = 1;
		}
		else
		{
			level[i] = 0;
		}
		if (i < 4)
		{
			if (pANEL_INFO != nullptr)
			{
				::Constants::jetGeoVecCopy(v, pANEL_INFO->m_vWTrans);
				v[0] += fPanelSize() / 2.0f;
				v[2] += fPanelSize() / 2.0f;
				dist[i] = Constants::VEC3_DIST_SQUARE(v, pPos);
			}
			else
			{
				dist[i] = 65536.0f;
			}
		}
		cTaskGimmickBase* cTaskGimmickBase2 = KingdomGlobal::g_pMotosGame->pMap()->SearchGimmickNo(x, z);
		if (cTaskGimmickBase2 != nullptr)
		{
			if (cTaskGimmickBase2->IsFall(pPos))
			{
				level[i] = -1;
			}
			else
			{
				level[i] = 0;
			}
		}
	}
	for (int j = 0; j < 4; j++)
	{
		if (level[4] > level[j])
		{
			num |= 1 << j;
		}
	}
	if (num != 0 && CustomKingdomUtil::IF(pOut))
	{
		int num4 = -1;
		for (int k = 0; k < 4; k++)
		{
			if (((num != 0) & CustomKingdomUtil::IF(1 << k)) && (num4 < 0 || dist[num4] > dist[k] || CustomKingdomUtil::IF(input_dir & (1 << k))))
			{
				num4 = k;
			}
		}
		num = 1 << num4;
		::Constants::jetGeoVecCopy(pOut, pPos);
		switch (num)
		{
		case 1:
			pOut[2] = (float)num3 * fPanelSize() - fRange;
			break;
		case 2:
			pOut[2] = (float)(num3 + 1) * fPanelSize() + fRange;
			break;
		case 4:
			pOut[0] = (float)num2 * fPanelSize() - fRange;
			break;
		case 8:
			pOut[0] = (float)(num2 + 1) * fPanelSize() + fRange;
			break;
		}
	}
	return num;
}

}
