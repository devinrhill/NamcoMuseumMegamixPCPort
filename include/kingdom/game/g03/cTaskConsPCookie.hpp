#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsPCookie : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eGAUGE = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_ADD = 3,
		eSTATE_OUT = 4,
		eSTATE_MAX = 5
	};

	enum EMaxCookie
	{
		MAX_COOKIE = 3
	};

	int m_nCookie{};

	int m_nCookieOld{};

public:
	cTaskConsPCookie();

	virtual ~cTaskConsPCookie();

private:
	void Init();

	void Term();

public:
	void Start();

	void End();

	bool Execute(cJetTask* pParam) override;

private:
	void Update();

	void UpCookie();

public:
	void Update(int Cookie, uint HP);

private:
	int nGetAnimCookieStay(int Cookie);

	int nGetAnimCookieUP(int Cookie);

	int nGetAnimCookieFlameOut(int Cookie);

protected:
	void ReleaseTerm() override { cTaskConsPCookie::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
