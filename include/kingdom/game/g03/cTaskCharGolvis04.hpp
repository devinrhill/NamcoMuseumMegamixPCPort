#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharGear1.hpp"

namespace kingdom::game::g03
{

class cTaskCharGolvis04 : public cTaskCharGolvis
{
public:
	enum gmEMonsterStateGolvisGear1
	{
		STATE_GOLVISGEAR1_MOVE = 0,
		STATE_GOLVISGEAR1_ESCAPE = 1,
		STATE_GOLVISGEAR1_MAX = 2
	};

	enum EProc
	{
		eProc_Init = 0,
		eProc_DemoInit = 1,
		eProc_Main = 2,
		eProc_End = 3
	};

private:
	enum EDemoStep
	{
		Entry = 0,
		AttackBefore = 1,
		Attack = 2,
		Escape = 3,
		Jump1 = 4,
		Jump2 = 5,
		Jump3 = 6,
		Jump4 = 7,
		Dash = 8,
		FadeOut = 9,
		End = 10,
		MAX = 11
	};

protected:
	static constexpr float MOVE_SPEED_DELTA = 0.01f;

	static const Constants::gmSGolvisAttack* s_AttackTBL[4];

public:
	static constexpr int eDMDL_Pacman = 0;

	static constexpr int MOVE_TIMER = 40;

private:
	cTaskChar* m_pDemoPacMan{};

	cTaskCharGear1* m_pGear{};

	int m_nDemoTimer{};

	int m_nMoveTimer{};

	int m_nPacNowGearNo{};

	int m_nAttackPatern{};

	cKingdomPtkHnd* m_hDemoPtk{};

	GameObject* m_dummyDemoCamObj{};

	Animation* m_dummyDemoCamAnim{};

	GameObject* m_dummyDemoCamAimObj{};

	Animation* m_dummyDemoCamAimAnim{};

	static const int scDemoFrameCnts[11];

	cTaskCharGolvis04::EDemoStep m_demoStep{};

protected:
	cTaskCharGolvis04::gmEMonsterStateGolvisGear1 m_iState{};

public:
	cTaskCharGolvis04(cJetTask* pParent);

private:
	void Init();

protected:
	void DemoPacManCreate();

public:
	void Term() override;

protected:
	bool IsForceAttack() override;

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	bool ChangeToAttack(int iAttackKind) override;

public:
	void Setting(bool bRelocate) override;

	void InitStateFirst() override;

	void InitStateStart() override;

protected:
	void MonsterFrameMove_PreProcess() override;

	void MonsterFrameMove_Free() override;

	void MonsterFrameMove_Escape() override;

	void ChangeToDamage() override;

	bool MonsterFrameMove_Death() override;

public:
	void IjikeStart() override;

	Constants::gmSGolvisAttack* GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum) override;

private:
	void CreateDummyDemoCamera();

	void UpdateDummyDemoCamera();

	float GetDemoCurrentFrameCnt();

protected:
	void ReleaseTerm() override { cTaskCharGolvis04::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
