#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsReady : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_READY = 0,
		eANM_MAX = 1
	};

	uint m_nTimer{};

	bool m_bGo{};

public:
	cTaskConsReady();

	virtual ~cTaskConsReady();

	void Free() override;

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsReady::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
