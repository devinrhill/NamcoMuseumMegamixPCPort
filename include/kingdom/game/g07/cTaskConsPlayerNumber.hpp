#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsPlayerNumber : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_PLAYER1 = 0,
		eANM_MAX = 1
	};

	uint m_nPlayerNo{};

	bool m_bExec{};

public:
	cTaskConsPlayerNumber();

	virtual ~cTaskConsPlayerNumber();

	bool& bExec();

	uint& nPlayerNo();

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsPlayerNumber::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
