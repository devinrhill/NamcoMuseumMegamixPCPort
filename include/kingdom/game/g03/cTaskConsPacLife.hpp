#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsPacLife : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eLIFE = 0,
		eICON = 1
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_ADD = 3,
		eSTATE_MINUS = 4,
		eSTATE_OUT = 5,
		eSTATE_MAX = 6
	};

	enum EPac
	{
		ePAC_HPMAX = 7
	};

	uint m_nMaxHP{};

	uint m_nMaxHPOld{};

	uint m_nHP{};

	uint m_nHPOld{};

	bool m_bMaxAnime{};

public:
	cTaskConsPacLife();

	virtual ~cTaskConsPacLife();

private:
	void Init();

	void Term();

public:
	void Start(uint max);

	void End();

	bool Execute(cJetTask* pParam) override;

private:
	void UpdateOld();

	void UpHP();

	void DownHP();

	void UpMaxHP();

	void _UpMaxHP();

public:
	void Update(uint max, uint HP);

private:
	void SetPosIcon();

	int nGetAnimFlameIn(uint max);

	int nGetAnimFlameOut(uint max, uint HP);

	int nGetAnimHPUp(uint max, uint HP);

	int nGetAnimHPStay(uint max, uint HP);

	int nGetAnimHPDown(uint max, uint HP);

	int nGetAnimFlameUP(uint max);

protected:
	void ReleaseTerm() override { cTaskConsPacLife::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
