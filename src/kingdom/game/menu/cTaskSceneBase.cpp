#include "kingdom/game/menu/cTaskSceneBase.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "kingdom/sMatchingData.hpp"
#include "KingdomGlobal.hpp"
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::menu
{

void cTaskSceneBase::AllWait()
{
}

void cTaskSceneBase::KeySelect(const int& padNum)
{
}

void cTaskSceneBase::SelectProc(const int& padNum)
{
}

void cTaskSceneBase::DecideCheck(const int& padNum)
{
}

int& cTaskSceneBase::nId()
{
	return m_nId;
}

int& cTaskSceneBase::nState()
{
	return m_nState;
}

int& cTaskSceneBase::nStep()
{
	return m_nStep;
}

int& cTaskSceneBase::nSelectNow()
{
	return m_nSelectNow;
}

int& cTaskSceneBase::nSelectOld()
{
	return m_nSelectOld;
}

sMatchingData*& cTaskSceneBase::pMatchingData()
{
	return m_pMatchingData;
}

std::vector<cTaskConsMatching*>& cTaskSceneBase::vecHit()
{
	return m_vecHit;
}

cTaskSceneBase::cTaskSceneBase(int nId)
{
	this->nId() = nId;
	nState() = 0;
	nStep() = 0;
	nSelectNow() = 0;
	nSelectOld() = 0;
	pMatchingData() = (static_cast<cAppGameBaseMenuMatching*>(KingdomGlobal::KingSys->pAppMain()))->pGetMatchingData();
}

cTaskSceneBase::~cTaskSceneBase()
{
}

void cTaskSceneBase::Reset()
{
}

void cTaskSceneBase::Appear()
{
}

bool cTaskSceneBase::IsAppearEnd()
{
	return false;
}

void cTaskSceneBase::Wait()
{
}

void cTaskSceneBase::Delete()
{
}

bool cTaskSceneBase::IsDeleteEnd()
{
	return false;
}

bool cTaskSceneBase::ErrorCheck()
{
	return false;
}

int cTaskSceneBase::nGetId()
{
	return nId();
}

int cTaskSceneBase::nGetConnectCtrl(int nPlayerNo)
{
	return (static_cast<cAppGameBaseMenuMatching*>(KingdomGlobal::KingSys->pAppMain()))->nGetConnectCtrl(nPlayerNo);
}

int cTaskSceneBase::nGetConnectCtrlCt()
{
	return (static_cast<cAppGameBaseMenuMatching*>(KingdomGlobal::KingSys->pAppMain()))->nGetConnectCtrlCt();
}

void cTaskSceneBase::nSetMessageIdx(uint nIdx)
{
	(static_cast<cAppGameBaseMenuMatching*>(KingdomGlobal::KingSys->pAppMain()))->nSetMessageIdx(nIdx);
}

void cTaskSceneBase::nSetErrorCode(uint nIdx)
{
	(static_cast<cAppGameBaseMenuMatching*>(KingdomGlobal::KingSys->pAppMain()))->nSetErrorCode(nIdx);
}

cTaskConsMatching* cTaskSceneBase::pGetHitObj(const int& nId)
{
	CustomKingdomUtil::Assert(0 <= nId, "error");
	for (cTaskConsMatching* item : vecHit())
	{
		if (item->nGetId() == nId)
		{
			return item;
		}
	}
	return nullptr;
}

bool cTaskSceneBase::DpdSelect(const int& padNum)
{
	int num = -1;
	for (cTaskConsMatching* item : vecHit())
	{
		(void)(item);
	}
	return false;
}

void cTaskSceneBase::Select()
{
	nSelectNow() = nSelectOld();
	int padNum = InGamePadManager::GetPad1No();
	if (!DpdSelect(padNum))
	{
		KeySelect(padNum);
	}
	SelectProc(padNum);
	DecideCheck(padNum);
	nSelectOld() = nSelectNow();
}

void cTaskSceneBase::PlaySndSe(const ESEKind& pSndName, int ctrl)
{
	CustomKingdomUtil::Assert(ctrl < 4, "error");
	SoundManager::get_SE()->PlaySE(pSndName);
}

bool cTaskSceneBase::GetPadTrg(int padNum, cTaskSceneBase::EKey key)
{
	switch (key)
	{
	case cTaskSceneBase::EKey::eUP:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_UP))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP);
		}
		return true;
	case cTaskSceneBase::EKey::eDOWN:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_DOWN))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_DOWN);
		}
		return true;
	case cTaskSceneBase::EKey::eLEFT:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT);
		}
		return true;
	case cTaskSceneBase::EKey::eRIGHT:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT);
		}
		return true;
	default:
		return false;
	}
}

}
