#include "kingdom/game/g07/cTaskBG.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <iterator>

#include "cJetTask.hpp"
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

void cTaskBG::PtkInit()
{
	for (int i = 0; i < (int)std::size(m_hPtk); i++)
	{
		m_hPtk[i] = new cKingdomPtkHnd();
	}
	CustomKingdomUtil::Assert(m_pTaskBGEffect == nullptr, "multiple init");
	ConnectChild(m_pTaskBGEffect = new cTaskBGEffect());
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	cKingdomModel* cKingdomModel2 = pModel(3u);
	switch (m_nWorldIdx)
	{
	case 2u:
		if (cKingdomModel2 == nullptr)
		{
		}
		break;
	case 3u:
		if (cKingdomModel2 == nullptr)
		{
		}
		break;
	case 4u:
		if (cKingdomModel2 == nullptr)
		{
		}
		break;
	case 5u:
		if (cKingdomModel2 == nullptr)
		{
		}
		break;
	default:
		CustomKingdomUtil::Assert("error");
		break;
	case 0u:
	case 1u:
		break;
	}
	for (uint num = 0u; num < 20; num++)
	{
		m_pTaskBGEffect->get_KingdomPtkMan()->UpdateFrame();
	}
}

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
		Vector4 v;
		::Constants::jetGeoVecCopy(v, childNode->GetTranslate());
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

Nullable<Vector4> cTaskBG::pModelNodeTrans(const string& name, uint ModelNo)
{
	return nullptr;
}

void cTaskBG::PtkExecute()
{
	switch (m_nWorldIdx)
	{
	case 0u:
	case 1u:
	case 2u:
	case 3u:
	case 4u:
	case 5u:
		return;
	}
	CustomKingdomUtil::Assert("error");
}

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
