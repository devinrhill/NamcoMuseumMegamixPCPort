#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyGiga : public cTaskCharEnemy
{
private:
	Matrix4x4 m_mRoll{};

	ushort m_nRotDir{};

public:
	cTaskCharEnemyGiga();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint Input() override;

	bool ActionBoth() override;

	void SetMMSParam(int para1, int para2, int para3) override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyGiga::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
