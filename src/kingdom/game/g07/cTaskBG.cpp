#include "kingdom/game/g07/cTaskBG.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "GamePad.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cParamBG.hpp"
#include "kingdom/game/g07/cTaskBGEffect.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ResourceCache.hpp>

namespace kingdom::game::g07
{

cTaskBG::cTaskBG()
{
	Init();
}

uint& cTaskBG::nStageIdx()
{
	return m_nWorldIdx;
}

// DOL 0x801e0cd4 [audited: described by the 43rd-session pass]: CSV_BG rows 1..4 (trans, rotate in degrees, scale, frame rate default 1), MDL_BG_n + its TSA / CLA / CHA when present,
// light set 0, fog 0, then PtkInit (vtable +0x44)
void cTaskBG::Init()
{
	m_nWorldIdx = KingdomGlobal::MotosSys->nWorld();
	cParamBG* cParamBG2 = new cParamBG("CSV_BG");
	m_nUseNum = 0;
	m_pTaskBGEffect = nullptr;
	m_nStagePtk = 0;
	CustomKingdomUtil::Assert("fatal error");
	string array[4][4] = {{"MDL_BG_1", "TSA_BG_1", "CLA_BG_1", "CHA_BG_1"}, {"MDL_BG_2", "TSA_BG_2", "CLA_BG_2", "CHA_BG_2"}, {"MDL_BG_3", "TSA_BG_3", "CLA_BG_3", "CHA_BG_3"}, {"MDL_BG_4", "TSA_BG_4", "CLA_BG_4", "CHA_BG_4"}};
	for (uint num = 0u; num < 4; num++)
	{
		if ("null" != ResourceDatabase::GetPath(array[num][0]))
		{
			Object* data = ResourceDatabase::Get<Object>(array[num][0], ResourceCache::EType::Temporary);
			cKingdomModel* cKingdomModel2 = CreateModel(num);
			cKingdomModel2->RegistModel(data);
			cKingdomModel2->RegistAnm(0u, array[num][1]);
			cKingdomModel2->RegistAnm(0u, array[num][2]);
			cKingdomModel2->RegistAnm(0u, array[num][3]);
			if (num == 3)
			{
				::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), cParamBG2->vGetTrans(num));
				::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), cParamBG2->vGetRotate(num));
				::Constants::jetGeoVecCopy(cKingdomModel2->vScale(), cParamBG2->vGetScale(num));
			}
			else
			{
				::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), cParamBG2->vGetTrans(num));
				::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), cParamBG2->vGetRotate(num));
				::Constants::jetGeoVecCopy(cKingdomModel2->vScale(), cParamBG2->vGetScale(num));
			}
			cKingdomModel2->SetUpdateRateAll(0u, cParamBG2->fGetFrameRate(num));
			cKingdomModel2->SetLightSetIdx(0);
			cKingdomModel2->SetFogIdx(0);
		}
	}
	PtkInit();
}

void cTaskBG::Term()
{
	cTaskModel::Term();
}

// DOL 0x801e1270 [audited: described by the 43rd-session pass]: the BG effect manager (8 kinds BG_PTK01..08), then per world the emitters at the children of the named nodes of BG model 3
// (only when model 3 exists); then 20 pre-roll steps, each skipped while pad 0 holds HOME. The C# left the world cases empty (no BG particles)
void cTaskBG::PtkInit()
{
	for (int i = 0; i < (int)std::size(m_hPtk); i++)
	{
		m_hPtk[i] = new cKingdomPtkHnd();
	}
	CustomKingdomUtil::Assert(m_pTaskBGEffect == nullptr, "multiple init");
	ConnectChild(m_pTaskBGEffect = new cTaskBGEffect());
	const bool bModel3 = pModel(3u) != nullptr;
	switch (m_nWorldIdx)
	{
	case 2u:
		if (bModel3)
		{
			PtkSet("ef_water01", 0u, 3u);
			PtkSet("ef_bird01", 1u, 3u);
			PtkSet("ef_rain01", 7u, 3u);
		}
		break;
	case 3u:
		if (bModel3)
		{
			m_nStagePtk = PtkSet("ef_bolon01", 3u, 3u);
		}
		break;
	case 4u:
		if (bModel3)
		{
			PtkSet("ef_fire01", 4u, 3u);
		}
		break;
	case 5u:
		if (bModel3)
		{
			PtkSet("ef_star01", 5u, 3u);
		}
		break;
	default:
		break;
	}
	for (uint num = 0u; num < 20; num++)
	{
		if ((GamePad::Get(0).hold & GamePad::PAD_HOME) == 0)
		{
			m_pTaskBGEffect->get_KingdomPtkMan()->UpdateFrame(1.0f);
		}
	}
}

// DOL 0x801e17ac [audited: described by the 43rd-session pass]: one emitter per child of node `name` (first child +0x60, then siblings +0x64) at the child's local translate (ResNodeData
// +0x38, as stored: ResNode::GetTranslate mirrors X for Unity) + BG model 3's position; returns the first handle index, -1 when the node is missing
int cTaskBG::PtkSet(const string& name, uint nKind, uint ModelNo)
{
	cKingdomModel* cKingdomModel2 = pModel(ModelNo);
	if (cKingdomModel2 == nullptr)
	{
		return -1;
	}
	ResMdl* resMdl = new ResMdl(cKingdomModel2->pScnMdl());
	if (!resMdl->IsValid())
	{
		return -1;
	}
	ResNode* resNode = resMdl->GetResNode(name);
	if (!resNode->IsValid())
	{
		return -1;
	}
	int num = 0;
	ResNode* childNode = resNode->GetChildNode(num);
	if (!childNode->IsValid())
	{
		return -1;
	}
	int nUseNum = m_nUseNum;
	do
	{
		const float* tr = childNode->Mdl->nodes[childNode->Idx].translate;
		Vector4 v(tr[0], tr[1], tr[2], 1.0f);
		if (pModel(3u) != nullptr)
		{
			::Constants::jetGeoVecAdd(v, v, pModel(3u)->vTrans());
		}
		CustomKingdomUtil::Assert(m_nUseNum < 32, "error");
		CustomKingdomUtil::Assert(m_pTaskBGEffect != nullptr, "nullpo");
		m_pTaskBGEffect->get_KingdomPtkMan()->Generate(m_hPtk[m_nUseNum], nKind, false, true, true, 0.0f, 0, 0, 0);
		m_hPtk[m_nUseNum]->vSetTrans(v);
		num++;
		childNode = resNode->GetChildNode(num);
		m_nUseNum++;
	}
	while (childNode->IsValid());
	return nUseNum;
}

// DOL 0x801e1a9c [audited: described by the 43rd-session pass]: the local translate of the first child of node `name` (copied to +0xbe0), nothing when the model / node / child is missing
Nullable<Vector4> cTaskBG::pModelNodeTrans(const string& name, uint ModelNo)
{
	cKingdomModel* cKingdomModel2 = pModel(ModelNo);
	if (cKingdomModel2 == nullptr)
	{
		return nullptr;
	}
	ResMdl* resMdl = new ResMdl(cKingdomModel2->pScnMdl());
	if (!resMdl->IsValid())
	{
		return nullptr;
	}
	ResNode* resNode = resMdl->GetResNode(name);
	if (!resNode->IsValid())
	{
		return nullptr;
	}
	ResNode* childNode = resNode->GetChildNode(0);
	if (!childNode->IsValid())
	{
		return nullptr;
	}
	const float* tr = childNode->Mdl->nodes[childNode->Idx].translate;
	m_vNodeTrans = Vector4(tr[0], tr[1], tr[2], 1.0f);
	return m_vNodeTrans;
}

// DOL 0x801e1c14 [audited: blr]: empty
void cTaskBG::PtkExecute()
{
}

// DOL 0x801e1c18 [audited: described by the 43rd-session pass]: PtkExecute, then the model animations unless paused / HOME (SYSTEM_IS_PAUSE) or game step 1. Draw = cTaskModel::Draw (0x801e1c90 is
// a copy of 0x8009a650)
bool cTaskBG::Execute(cJetTask* pParam)
{
	PtkExecute();
	if (!Constants::SYSTEM_IS_PAUSE() && KingdomGlobal::g_pMotosGame->nStep(0) != 1)
	{
		UpdateFrame();
	}
	return true;
}

void cTaskBG::Draw(int nStatus, cJetTask* pParam)
{
	cTaskModel::Draw(nStatus, pParam);
}

}
