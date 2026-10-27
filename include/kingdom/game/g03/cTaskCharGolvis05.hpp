#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskCharGear2.hpp"

namespace kingdom::game::g03
{

class cTaskCharGolvis05 : public cTaskCharGolvis
{
public:
	enum gmEMonsterStateGolvisGear2
	{
		STATE_GOLVISGEAR2_MOVE = 0,
		STATE_GOLVISGEAR2_ESCAPE = 1,
		STATE_GOLVISGEAR2_MAX = 2
	};

private:
	enum EEscape
	{
		eESCAPE_DIR_NONE = 0,
		eESCAPE_DIR_LEFT = 1,
		eESCAPE_DIR_RIGHT = 2
	};

public:
	static const float s_fHeight[3];

	static constexpr float ROTATE_ACCEL = 5E-05f;

	static constexpr float MOVE_SPEED_DELTA = 0.01f;

	static constexpr float GEARS_RADIUS_L = 10.0f;

	static const Constants::gmSGolvisAttack* s_AttackTBL[2];

	static constexpr int ROTATE_GEAR_RIVISE = 3;

private:
	cTaskCharGear2* m_pGear{};

	int m_nGearTimer{};

	int m_nEscapeMoveTimer{};

	int m_nEscapeDir{};

	int m_nMoveWait{};

	Vector4 m_vButtOldTrans{};

public:
	static constexpr int ESCAPE_TYPE2_MOVE_TIME = 30;

	static const int m_aaGerasPatern1[4][4];

	static const int m_aaGerasPatern2[3][4];

protected:
	cTaskCharGolvis05::gmEMonsterStateGolvisGear2 m_iState{};

public:
	static bool s_CheckLimit(const Vector4& vPos);

	cTaskCharGolvis05(cJetTask* pParent);

	bool IsIzike();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void Setting(bool bRelocate) override;

	void InitStateFirst() override;

	void InitStateStart() override;

protected:
	void MonsterFrameMove_PreProcess() override;

public:
	Vector4 CalcButtCursorTrans(Vector4 v) override;

protected:
	void MonsterFrameMove_Free() override;

public:
	void GearsExecuteFrameMove();

protected:
	void MonsterFrameMove_Escape() override;

	void ChangeToDamage() override;

public:
	void GearsExecuteLife3();

	void GearsExecuteLife2();

	void GearsExecuteLife1();

	void IjikeStart() override;

	Constants::gmSGolvisAttack* GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum) override;

protected:
	bool ChangeToAttack(int iAttackKind) override;

protected:
	void ReleaseTerm() override { cTaskCharGolvis05::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
