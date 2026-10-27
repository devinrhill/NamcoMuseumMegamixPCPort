#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsEnemyLife : public cTaskConsoleBase
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

	enum EMaxHp
	{
		MAX_HP = 0
	};

	static constexpr int g_izike = 3;

	uint m_nHP{};

	uint m_nHPOld{};

	uint m_nLevel{};

public:
	cTaskConsEnemyLife();

	virtual ~cTaskConsEnemyLife();

private:
	void Init();

	void Term();

public:
	void Start(uint HP);

	void End();

	bool Execute(cJetTask* pParam) override;

private:
	void _Update();

	void DownHP();

public:
	void Update(uint HP, uint level);

private:
	void SetPosIcon();

	int nGetAnimHPStay(uint HP);

	int nGetAnimHPDown(uint HP);

public:
	void Cry();

	void Stay();

	void Dead();

protected:
	void ReleaseTerm() override { cTaskConsEnemyLife::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
