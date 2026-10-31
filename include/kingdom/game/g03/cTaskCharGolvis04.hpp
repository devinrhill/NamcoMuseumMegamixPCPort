#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharGear1.hpp"
#include "kingdom/game/g03/cDemoSceneCamera.hpp"

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

	// the death demo's cTaskSceneCamera (the DOL keeps it in the app's camera slot +0x2b0 and connects it to this task)
	cDemoSceneCamera* m_pDemoCamera{};

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
	void DemoUpdateFrame();

protected:
	// DOL dtor 0x8011d5e0: Term (blr), the +0xa10 effect handle's dtor, then ~cTaskCharGolvis (its Term)
	void ReleaseTerm() override;
};

}
