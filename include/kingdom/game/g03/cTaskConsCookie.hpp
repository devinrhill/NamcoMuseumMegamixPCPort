#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"

namespace kingdom::game::g03
{

class cTaskConsCookie : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eCOOKIE = 0,
		eSLASH = 1
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

	cTaskConsNum* m_pTaskNumMax{};

	cTaskConsNum* m_pTaskNum{};

	uint m_nCookieMax{};

	uint m_nCookie{};

	bool m_bComplete{};

public:
	cTaskConsCookie();

	virtual ~cTaskConsCookie();

	bool IsUpdate(uint dat);

private:
	void Init();

	void Term();

public:
	void Start(uint max, uint now);

	void End();

	void Update(uint max, uint now);

private:
	bool IsComplete();

public:
	bool Execute(cJetTask* pParam) override;
};

}
