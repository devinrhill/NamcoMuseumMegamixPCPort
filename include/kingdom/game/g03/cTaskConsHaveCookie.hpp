#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"

namespace kingdom::game::g03
{

class cTaskConsHaveCookie : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eCOOKIE = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_OUT = 3,
		eSTATE_MAX = 4
	};

	enum EFigure
	{
		eFIGURE = 3
	};

	cTaskConsNum* m_pTaskNum{};

	uint m_nCookieMax{};

	uint m_nCookie{};

public:
	cTaskConsHaveCookie();

	virtual ~cTaskConsHaveCookie();

	bool IsUpdate(uint dat);

private:
	void Init();

	void Term();

public:
	void Start(uint have);

	void End();

	void Update(uint have);

	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsHaveCookie::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
