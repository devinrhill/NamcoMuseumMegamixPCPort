#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyPupaRed : public cTaskCharEnemy
{
public:
	cTaskCharEnemyPupaRed();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	uint Input() override;

	bool Create(uint no) override;

	void SetMMSParam(int para1, int para2, int para3) override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyPupaRed::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
