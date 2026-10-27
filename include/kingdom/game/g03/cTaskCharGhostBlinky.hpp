#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharAreaGhost.hpp"

namespace kingdom::game::g03
{

class cTaskCharGhostBlinky : public cTaskCharAreaGhost
{
private:
	void Init();

public:
	void Term() override;

	cTaskCharGhostBlinky();

	cTaskChar::eKIND GetKind() override;

	string GetModelKey() override;

	string GetAnmKey() override;

	string GetSRTAnmKey() override;

	uint GetPushOutLevel() override;

protected:
	void ReleaseTerm() override { cTaskCharGhostBlinky::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
