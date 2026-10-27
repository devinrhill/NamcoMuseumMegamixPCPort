#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharAreaGhost.hpp"

namespace kingdom::game::g03
{

class cTaskCharGhostInky : public cTaskCharAreaGhost
{
private:
	void Init();

public:
	void Term() override;

	cTaskCharGhostInky();

	cTaskChar::eKIND GetKind() override;

	string GetModelKey() override;

	string GetAnmKey() override;

	string GetSRTAnmKey() override;

	uint GetPushOutLevel() override;

protected:
	void ReleaseTerm() override { cTaskCharGhostInky::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
