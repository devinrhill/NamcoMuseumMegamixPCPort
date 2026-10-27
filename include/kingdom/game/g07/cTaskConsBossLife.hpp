#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsBossLife : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_LIFE = 0,
		eANM_LIFE_FLASH = 1,
		eANM_MAX = 2
	};

	enum EStep
	{
		eSTEP_INIT = 0,
		eSTEP_VISIBLE = 1,
		eSTEP_HPUP = 2,
		eSTEP_MAIN = 3,
		eSTEP_END = 4
	};

	int m_nTimer{};

	int m_nOldHp{};

public:
	cTaskConsBossLife();

	virtual ~cTaskConsBossLife();

	void Free() override;

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

private:
	IEnumerator* WaitOneFramePlaySE();

protected:
	void ReleaseTerm() override { cTaskConsBossLife::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
