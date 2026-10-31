#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskPanel.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "kingdom/game/g07/sMOTOS_POS2D.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomAnimManager.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ResourceCache.hpp>

namespace kingdom::game::g07
{

const int cTaskPanel::rot[4] = {0, 90, 180, 270};

bool cTaskPanel::CHECK_PANEL_WARNING(string xxx)
{
	return true;
}

bool cTaskPanel::CHECK_PANEL_ASSERT(string xxx)
{
	return false;
}

cTaskPanel::cTaskPanel()
{
	Init();
}

cTaskPanel::cTaskPanel(Constants::ePANEL type)
{
	Init();
	CustomKingdomUtil::Assert(Create(type), "error");
}

uint& cTaskPanel::nID()
{
	return m_nID;
}

uint& cTaskPanel::nFlag()
{
	return m_nFlag;
}

Constants::ePANEL cTaskPanel::nType()
{
	return m_nType;
}

void cTaskPanel::fFrame(float fFrame)
{
	m_fAnimFrame = fFrame;
}

float cTaskPanel::fFrame()
{
	return m_fAnimFrame;
}

int& cTaskPanel::nMapPos(uint n)
{
	CustomKingdomUtil::Assert(2 > n, "error");
	return m_nMapPos[n];
}

Constants::PANEL_INFO*& cTaskPanel::pPanelInfo()
{
	return m_Panel;
}

// DOL 0x801fd774 [audited: per AUDIT_G03_G07 row] (max frame <= frame)
bool cTaskPanel::AnimateModel(uint model)
{
	if (pModel(model)->GetFrameAnmChr(0u) >= pModel(model)->GetFrameMaxAnmChr(0u))
	{
		return true;
	}
	return false;
}

// DOL 0x801fc858 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskPanel::Init()
{
	m_nID = 0u;
	m_nFlag = 66u;
	m_nStep = Constants::ePANEL_STEP::ePANEL_STEP_NORMAL;
	m_nType = Constants::ePANEL::ePANEL_INVALID;
	m_nModel = cTaskPanel::eMODEL::eMODEL_UNION;
	m_fAnimFrame = 0.0f;
	m_nCurModelIdx = 0u;
	m_crackType = 0;   // chosen per crack step (DOL 0x801fd8f4 [audited: described by the 43rd-session pass]), not at construction
	for (int i = 0; i < (int)std::size(m_nMapPos); i++)
	{
		m_nMapPos[i] = 0;
	}
	m_Panel = new Constants::PANEL_INFO();
	m_Panel->m_pTaskPanel = this;
	::Constants::jetGeoVecUnit(m_vTrans);
	::Constants::jetGeoVecUnit(m_vRotate);
	SetName("cTaskPanel");
}

void cTaskPanel::Term()
{
	Release();
	cTaskModel::Term();
}

bool cTaskPanel::Create(Constants::ePANEL type)
{
	m_nType = type;
	switch ((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_MASK)))
	{
	case 4096u:
		return CreatePanelN(type);
	case 8192u:
		return CreatePanelX(type);
	case 16384u:
		return CreatePanelG(type);
	case 32768u:
		return CreatePanelW(type);
	default:
		CustomKingdomUtil::Assert("unknown");
		return false;
	}
}

bool cTaskPanel::CreatePanelN(Constants::ePANEL type)
{
	CustomKingdomUtil::Assert(CustomKingdomUtil::IF((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_N))), "erorr");
	uint num = (uint)((Constants::ePANEL)((int)type & (int)(Constants::ePANEL)0xFFFFEFFFu));
	CustomKingdomUtil::Assert(num < 3, "error");
	string array[3][4] = {{"MDL_PANEL_A", "TSA_PANEL_A", "CLA_PANEL_A", "CHA_PANEL_A"}, {"MDL_PANEL_B", "TSA_PANEL_B", "CLA_PANEL_B", "CHA_PANEL_B"}, {"MDL_PANEL_C", "TSA_PANEL_C", "CLA_PANEL_C", "CHA_PANEL_C"}};
	CHECK_PANEL_WARNING(array[num][0]);
	Object* data = ResourceDatabase::Get<Object>(array[num][0], ResourceCache::EType::Temporary);
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel(data);
	cKingdomModel2->RegistAnm(0u, array[num][1]);
	cKingdomModel2->RegistAnm(0u, array[num][2]);
	cKingdomModel2->RegistAnm(0u, array[num][3]);
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	// TODO(unity): m_animMgr = cKingdomModel2.pScnMdl().AddMissingComponent<KingdomAnimManager>();
	m_animMgr->SetChildAnim(true);
	CustomKingdomUtil::Assert(num < 3, "error");
	string array2[3][3] = {{"MDL_PANEL_A_CRACK_1", "MDL_PANEL_A_CRACK_2", "MDL_PANEL_A_CRACK_3"}, {"MDL_PANEL_B_CRACK_1", "MDL_PANEL_B_CRACK_2", "MDL_PANEL_B_CRACK_3"}, {"MDL_PANEL_C_CRACK_1", "MDL_PANEL_C_CRACK_2", "MDL_PANEL_C_CRACK_3"}};
	// DOL 0x801fc970 [audited: described by the 43rd-session pass]: all three crack models (slots 1..3) are created up front; the crack step picks one with rand() % 3 + 1 (0x801fd8f4). The C# created only a
	// random one at construction
	for (uint k = 0u; k < 3u; k++)
	{
		data = ResourceDatabase::Get<Object>(array2[num][k], ResourceCache::EType::Temporary);
		CHECK_PANEL_ASSERT(array2[num][k]);
		cKingdomModel2 = CreateModel(1u + k);
		cKingdomModel2->RegistModel(data);
		cKingdomModel2->AttachShadow();
		cKingdomModel2->SetLightSetIdx(1);
		cKingdomModel2->SetFogIdx(1);
	}
	CustomKingdomUtil::Assert(num < 3, "error");
	string array3[3][4] = {{"MDL_PANEL_A_BREAK", "TSA_PANEL_A_BREAK", "CLA_PANEL_A_BREAK", "CHA_PANEL_A_BREAK"}, {"MDL_PANEL_B_BREAK", "TSA_PANEL_B_BREAK", "CLA_PANEL_B_BREAK", "CHA_PANEL_B_BREAK"}, {"MDL_PANEL_C_BREAK", "TSA_PANEL_C_BREAK", "CLA_PANEL_C_BREAK", "CHA_PANEL_C_BREAK"}};
	data = ResourceDatabase::Get<Object>(array3[num][0], ResourceCache::EType::Temporary);
	CHECK_PANEL_ASSERT(array3[num][0]);
	cKingdomModel2 = CreateModel(4u);
	cKingdomModel2->RegistModel(data);
	cKingdomModel2->RegistAnm(0u, array3[num][1]);
	cKingdomModel2->RegistAnm(0u, array3[num][2]);
	cKingdomModel2->RegistAnm(0u, array3[num][3]);
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	m_nFlag |= 1u;
	ModelChange(cTaskPanel::eMODEL::eMODEL_UNION);
	return true;
}

bool cTaskPanel::CreatePanelX(Constants::ePANEL type)
{
	CustomKingdomUtil::Assert(CustomKingdomUtil::IF((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_X))), "erorr");
	uint num = (uint)((Constants::ePANEL)((int)type & (int)(Constants::ePANEL)0xFFFFDFFFu));
	CustomKingdomUtil::Assert(num < 1, "error");
	string array[1][4] = {{"MDL_PANEL_X", "TSA_PANEL_X", "CLA_PANEL_X", "CHA_PANEL_X"}};
	Object* data = ResourceDatabase::Get<Object>(array[num][0], ResourceCache::EType::Temporary);
	CHECK_PANEL_WARNING(array[num][0]);
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel(data);
	cKingdomModel2->RegistAnm(0u, array[num][1]);
	cKingdomModel2->RegistAnm(0u, array[num][2]);
	cKingdomModel2->RegistAnm(0u, array[num][3]);
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	// TODO(unity): m_animMgr = cKingdomModel2.pScnMdl().AddMissingComponent<KingdomAnimManager>();
	m_animMgr->SetChildAnim(true);
	ModelChange(cTaskPanel::eMODEL::eMODEL_UNION);
	m_nFlag |= 1u;
	m_nFlag |= 16u;
	return true;
}

bool cTaskPanel::CreatePanelG(Constants::ePANEL type)
{
	CustomKingdomUtil::Assert(CustomKingdomUtil::IF((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_G))), "erorr");
	uint num = (uint)((Constants::ePANEL)((int)type & (int)(Constants::ePANEL)0xFFFFBFFFu));
	CustomKingdomUtil::Assert(num < 1, "error");
	string array[1][4] = {{"MDL_GIMMICK_EXPLODED", "TSA_GIMMICK_EXPLODED", "CLA_GIMMICK_EXPLODED", "CHA_GIMMICK_EXPLODED"}};
	Object* data = ResourceDatabase::Get<Object>(array[num][0], ResourceCache::EType::Temporary);
	CHECK_PANEL_WARNING(array[num][0]);
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel(data);
	cKingdomModel2->RegistAnm(0u, array[num][1]);
	cKingdomModel2->RegistAnm(0u, array[num][2]);
	cKingdomModel2->RegistAnm(0u, array[num][3]);
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	// TODO(unity): m_animMgr = cKingdomModel2.pScnMdl().AddMissingComponent<KingdomAnimManager>();
	m_animMgr->SetChildAnim(true);
	CustomKingdomUtil::Assert(num < 1, "error");
	string array2[1][4] = {{"MDL_GIMMICK_EXPLODED_BREAK", "TSA_GIMMICK_EXPLODED_BREAK", "CLA_GIMMICK_EXPLODED_BREAK", "CHA_GIMMICK_EXPLODED_BREAK"}};
	data = ResourceDatabase::Get<Object>(array2[num][0], ResourceCache::EType::Temporary);
	CHECK_PANEL_ASSERT(array2[num][0]);
	cKingdomModel2 = CreateModel(4u);
	cKingdomModel2->RegistModel(data);
	cKingdomModel2->RegistAnm(0u, array2[num][1]);
	cKingdomModel2->RegistAnm(0u, array2[num][2]);
	cKingdomModel2->RegistAnm(0u, array2[num][3]);
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	ModelChange(cTaskPanel::eMODEL::eMODEL_UNION);
	m_nFlag |= 1u;
	m_nFlag |= 16u;
	return true;
}

bool cTaskPanel::CreatePanelW(Constants::ePANEL type)
{
	CustomKingdomUtil::Assert(CustomKingdomUtil::IF((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_W))), "erorr");
	uint num = (uint)((Constants::ePANEL)((int)type & (int)(Constants::ePANEL)0xFFFF7FFFu));
	CustomKingdomUtil::Assert(num < 6, "error");
	string array[6][4] = {{"MDL_PANEL_WALL_A", "TSA_PANEL_WALL_A", "CLA_PANEL_WALL_A", "CHA_PANEL_WALL_A"}, {"MDL_PANEL_WALL_B", "TSA_PANEL_WALL_B", "CLA_PANEL_WALL_B", "CHA_PANEL_WALL_B"}, {"MDL_PANEL_WALL_C", "TSA_PANEL_WALL_C", "CLA_PANEL_WALL_C", "CHA_PANEL_WALL_C"}, {"MDL_PANEL_WALL_D", "TSA_PANEL_WALL_D", "CLA_PANEL_WALL_D", "CHA_PANEL_WALL_D"}, {"MDL_PANEL_WALL_E", "TSA_PANEL_WALL_E", "CLA_PANEL_WALL_E", "CHA_PANEL_WALL_E"}, {"MDL_PANEL_WALL_F", "TSA_PANEL_WALL_F", "CLA_PANEL_WALL_F", "CHA_PANEL_WALL_F"}};
	Object* data = ResourceDatabase::Get<Object>(array[num][0], ResourceCache::EType::Temporary);
	CHECK_PANEL_WARNING(array[num][0]);
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel(data);
	cKingdomModel2->RegistAnm(0u, array[num][1]);
	cKingdomModel2->RegistAnm(0u, array[num][2]);
	cKingdomModel2->RegistAnm(0u, array[num][3]);
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	// TODO(unity): m_animMgr = cKingdomModel2.pScnMdl().AddMissingComponent<KingdomAnimManager>();
	m_animMgr->SetChildAnim(true);
	ModelChange(cTaskPanel::eMODEL::eMODEL_UNION);
	m_nFlag |= 32u;
	m_nFlag |= 1u;
	m_nFlag |= 16u;
	return true;
}

bool cTaskPanel::Release()
{
	return false;
}

uint cTaskPanel::StrikePanel()
{
	if (!CustomKingdomUtil::IF(m_nFlag & 2) || CustomKingdomUtil::IF(m_nFlag & 8))
	{
		return 1u;
	}
	if (!CustomKingdomUtil::IF((uint)((Constants::ePANEL)((int)m_nType & (int)Constants::ePANEL::ePANEL_TYPE_N))))
	{
		return 1u;
	}
	uint result = 4u;
	switch (m_nStep)
	{
	case Constants::ePANEL_STEP::ePANEL_STEP_NORMAL:
		StepChange(Constants::ePANEL_STEP::ePANEL_STEP_CRACK);
		break;
	case Constants::ePANEL_STEP::ePANEL_STEP_CRACK:
		StepChange(Constants::ePANEL_STEP::ePANEL_STEP_FALLING);
		break;
	default:
		result = 1u;
		break;
	}
	return result;
}

// DOL 0x801fd304 [audited: described by the 43rd-session pass] (audited 44th: matches; both rotation tables are 0 / 90 / 180 / 270, picked with rand() & 3)
void cTaskPanel::StepChange(Constants::ePANEL_STEP step)
{
	switch (step)
	{
	case Constants::ePANEL_STEP::ePANEL_STEP_NORMAL:
		{
			m_nFlag |= 64u;
			m_nFlag &= 4294967283u;
			float num2 = fFrame();
			pModel()->SetFrameAnmAll(0u, num2);   // native frames (the C# passed Unity seconds: num2 / 60)
			if (nullptr != m_animMgr)
			{
				m_animMgr->PlayAnim(num2 / 60.0f);
			}
			break;
		}
	case Constants::ePANEL_STEP::ePANEL_STEP_CRACK:
		{
			m_nFlag &= 4294967231u;
			m_nFlag &= 4294967283u;
			m_nFlag |= 4u;
			int num3 = rot[kingdom::MslRandRange(0, ::Constants::ARRAY_SIZE(rot))];
			vRotate(1) = ::Constants::JET_DEG2RAD(num3);
			break;
		}
	case Constants::ePANEL_STEP::ePANEL_STEP_FALLING:
		{
			m_nFlag |= 64u;
			m_nFlag &= 4294967283u;
			m_nFlag |= 8u;
			if (pModel(4u) != nullptr)
			{
				pModel(4u)->SetFrameAnmAll(0u, 0.0f);
			}
			int num = rot[kingdom::MslRandRange(0, ::Constants::ARRAY_SIZE(rot))];
			vRotate(1) = ::Constants::JET_DEG2RAD(num);
			break;
		}
	case Constants::ePANEL_STEP::ePANEL_STEP_EXPLODE:
		m_nFlag &= 4294967283u;
		m_nFlag |= 1u;
		m_Panel->m_nExplodeCount = 50;
		break;
	case Constants::ePANEL_STEP::ePANEL_STEP_REBIRTH:
		m_nFlag |= 67u;
		m_nFlag |= 64u;
		m_nFlag &= 4294967283u;
		m_Panel->m_nTimer = 5;
		break;
	}
	m_nStep = step;
	m_Panel->m_nFlag = m_nFlag;
	m_Panel->m_nStep = (short)m_nStep;
	ModelChange(StepModelIdx(m_nType, m_nStep));
	if (step == Constants::ePANEL_STEP::ePANEL_STEP_HIDE)
	{
		pModel((uint)StepModelIdx(m_nType, m_nStep))->Invisible();
	}
}

// DOL 0x801fd564 [audited: described by the 43rd-session pass] (audited 44th: steps 2..5 equal; kind 4 keeps its break model)
void cTaskPanel::ExecPanel()
{
	Constants::PANEL_INFO* panel = m_Panel;
	KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	switch (panel->m_nStep)
	{
	case 2:
		panel->m_nFlag |= 8u;
		panel->m_nTimer++;
		if (panel->m_nTimer < 10)
		{
			pModel(4u)->SetFrameAnmAll(0u, 0.0f);
		}
		if (!AnimateModel(4u))
		{
			break;
		}
		if (panel->m_nKind != 4)
		{
			StepChange(Constants::ePANEL_STEP::ePANEL_STEP_HIDE);
			if (KingdomGlobal::MotosSys->IsBossStage() || KingdomGlobal::g_pMotosGame->bMultiPlay())
			{
				panel->m_nTimer = (int)Constants::get_GAME_COUNTER();
			}
		}
		// DOL 0x801fd564: nothing for kind 4 (the C# hid the break model here)
		break;
	case 3:
		if (++panel->m_nTimer >= 30)
		{
			Constants::SpawnEffect(20, panel->m_vWTrans);
			StepChange(Constants::ePANEL_STEP::ePANEL_STEP_FALLING);
		}
		break;
	case 4:
		if (KingdomGlobal::MotosSys->IsBossStage() || KingdomGlobal::g_pMotosGame->bMultiPlay())
		{
			int mO_VS_PANEL_REBIRTH = Constants::get_MO_VS_PANEL_REBIRTH();
			if (panel->m_nTimer != 0 && panel->m_nTimer + mO_VS_PANEL_REBIRTH <= (int)Constants::get_GAME_COUNTER())
			{
				StepChange(Constants::ePANEL_STEP::ePANEL_STEP_REBIRTH);
				Constants::SpawnEffect(24, panel->m_vWTrans);
				panel->m_nComa = panel->m_nNComa;
			}
		}
		break;
	case 5:
		if (!CustomKingdomUtil::IF(--panel->m_nTimer))
		{
			StepChange(Constants::ePANEL_STEP::ePANEL_STEP_NORMAL);
			panel->m_nComa = panel->m_nNComa;
		}
		break;
	}
	if (panel->m_nExplodeCount != 0 && !CustomKingdomUtil::IF(--panel->m_nExplodeCount))
	{
		BigExplodePanel();
	}
}

void cTaskPanel::ExplodePanel()
{
	Constants::PANEL_INFO* panel = m_Panel;
	if (panel != nullptr && panel->m_nKind == 4 && panel->m_nStep == 0)
	{
		StepChange(Constants::ePANEL_STEP::ePANEL_STEP_EXPLODE);
	}
}

void cTaskPanel::BigExplodePanel()
{
	Constants::PANEL_INFO* panel = m_Panel;
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	int num = (int)panel->m_nMapPos[0];
	int num2 = (int)panel->m_nMapPos[1];
	sMOTOS_POS2D array[4] = {[&]() { sMOTOS_POS2D _o9 = sMOTOS_POS2D(); _o9.pos[0] = 1; _o9.pos[1] = 0; return _o9; }(), [&]() { sMOTOS_POS2D _o10 = sMOTOS_POS2D(); _o10.pos[0] = -1; _o10.pos[1] = 0; return _o10; }(), [&]() { sMOTOS_POS2D _o11 = sMOTOS_POS2D(); _o11.pos[0] = 0; _o11.pos[1] = 1; return _o11; }(), [&]() { sMOTOS_POS2D _o12 = sMOTOS_POS2D(); _o12.pos[0] = 0; _o12.pos[1] = -1; return _o12; }()};
	for (uint num3 = 0u; num3 < 4; num3++)
	{
		int x = num + array[num3].pos[0];
		int z = num2 + array[num3].pos[1];
		((cTaskPanelMan2->SearchPanelNo(x, z) != nullptr) ? cTaskPanelMan2->SearchPanelNo(x, z)->m_pTaskPanel : nullptr)->ExplodePanel();
	}
}

// DOL 0x801fd8f4 (audited 44th: type N crack step = rand() % 3 + 1, the other tables equal)
cTaskPanel::eMODEL cTaskPanel::StepModelIdx(Constants::ePANEL type, Constants::ePANEL_STEP step)
{
	return (((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_MASK)) == 4096u) ? ((step == Constants::ePANEL_STEP::ePANEL_STEP_NORMAL) ? cTaskPanel::eMODEL::eMODEL_UNION : ((step == Constants::ePANEL_STEP::ePANEL_STEP_CRACK) ? (cTaskPanel::eMODEL)(1 + (m_crackType = kingdom::MslRand() % 3)) : ((step == Constants::ePANEL_STEP::ePANEL_STEP_FALLING) ? cTaskPanel::eMODEL::eMODEL_BREAK : ((step == Constants::ePANEL_STEP::ePANEL_STEP_HIDE) ? cTaskPanel::eMODEL::eMODEL_BREAK : ((step == Constants::ePANEL_STEP::ePANEL_STEP_REBIRTH) ? cTaskPanel::eMODEL::eMODEL_UNION : (cTaskPanel::eMODEL::eMODEL_UNION)))))) : (((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_MASK)) == 8192u) ? cTaskPanel::eMODEL::eMODEL_UNION : (((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_MASK)) == 16384u) ? ((step == Constants::ePANEL_STEP::ePANEL_STEP_NORMAL) ? cTaskPanel::eMODEL::eMODEL_UNION : ((step == Constants::ePANEL_STEP::ePANEL_STEP_CRACK) ? cTaskPanel::eMODEL::eMODEL_UNION : ((step == Constants::ePANEL_STEP::ePANEL_STEP_FALLING) ? cTaskPanel::eMODEL::eMODEL_BREAK : (cTaskPanel::eMODEL::eMODEL_UNION)))) : (((uint)((Constants::ePANEL)((int)type & (int)Constants::ePANEL::ePANEL_TYPE_MASK)) == 32768u) ? cTaskPanel::eMODEL::eMODEL_UNION : (cTaskPanel::eMODEL::eMODEL_UNION)))));
}

bool cTaskPanel::ModelChange(cTaskPanel::eMODEL idx)
{
	if ((ulong)m_nCurModelIdx != (ulong)idx)
	{
		m_bUpdateMtx = true;
	}
	m_nCurModelIdx = (uint)idx;
	for (uint num = 0u; num < 5; num++)
	{
		if (pModel(num) != nullptr)
		{
			if (num == (uint)idx)
			{
				pModel(num)->Visible();
			}
			else
			{
				pModel(num)->Invisible();
			}
		}
	}
	return true;
}

bool cTaskPanel::Execute(cJetTask* pParam)
{
	if (Constants::SYSTEM_IS_PAUSE())
	{
		return true;
	}
	if (CustomKingdomUtil::IF(m_Panel->m_nFlag & 1))
	{
		ExecPanel();
	}
	// DOL 0x801fda08 [audited: described by the 43rd-session pass]: then cTaskModel::UpdateFrame (0x8009a478) = every model of the panel advances its animations (crack / break / fall); the
	// C# gated it on a Unity clip of the current model, replaced by `false` in the conversion, so breaking panels never animated
	cTaskModel::UpdateFrame();
	return true;
}

void cTaskPanel::Draw(int nStatus, cJetTask* pParam)
{
	if (m_bUpdateMtx || !CustomKingdomUtil::IsEqualFloat(KingdomGlobal::g_pMotosGame->fSlopeRad(), 0.0f))
	{
		Matrix4x4 m = (CustomKingdomUtil::IsEqualFloat(KingdomGlobal::g_pMotosGame->fSlopeRad(), 0.0f) ? Matrix4x4::Translate(m_vTrans) : (KingdomGlobal::g_pMotosGame->pSlopeMatrix() * Matrix4x4::Translate(m_vTrans)));
		if (!CustomKingdomUtil::IsEqualFloat(m_vRotate.x, 0.0f) || !CustomKingdomUtil::IsEqualFloat(m_vRotate.y, 0.0f) || !CustomKingdomUtil::IsEqualFloat(m_vRotate.z, 0.0f))
		{
			::Constants::jetGeoMtxRotZYX(m, m, m_vRotate);
		}
		cTaskModel::cPBModel* cPBModel = pPBModel(m_nCurModelIdx);
		// native: the model takes the whole matrix (Unity set position + euler angles of the transform, converted to its own axes)
		if (cPBModel != nullptr) DummyNW4R::SetMtx(cPBModel->pScnMdl(), m);
		m_bUpdateMtx = false;
	}
	// DOL 0x801fdbdc [audited: described by the 43rd-session pass]: the current model's push-back (+0xd8) goes into this frame's scene list (0x8009c3f4). (The DOL rebuilds the matrix
	// every frame; the C# only when it changed, which gives the same matrix.)
	if (cTaskModel::cPBModel* cur = pPBModel(m_nCurModelIdx)) cur->UpdateMatrix();
}

void cTaskPanel::UpdateMatrix()
{
	for (uint num = 0u; num < 5; num++)
	{
		cKingdomModel* cKingdomModel2;
		if ((cKingdomModel2 = pModel(num)) != nullptr)
		{
			::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), m_vTrans);
			::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), m_vRotate);
		}
	}
}

Vector4& cTaskPanel::vTrans()
{
	return m_vTrans;
}

float& cTaskPanel::vTrans(int n)
{
	m_bUpdateMtx = true;
	switch (n)
	{
	case 0:
		return m_vTrans.x;
	case 1:
		return m_vTrans.y;
	case 2:
		return m_vTrans.z;
	case 3:
		return m_vTrans.w;
	default:
		CustomKingdomUtil::Assert(4 > n, "error");
		return m_vTrans.w;
	}
}

Vector4& cTaskPanel::vRotate()
{
	return m_vRotate;
}

float& cTaskPanel::vRotate(int n)
{
	m_bUpdateMtx = true;
	switch (n)
	{
	case 0:
		return m_vRotate.x;
	case 1:
		return m_vRotate.y;
	case 2:
		return m_vRotate.z;
	case 3:
		return m_vRotate.w;
	default:
		CustomKingdomUtil::Assert(4 > n, "error");
		return m_vRotate.w;
	}
}

}
