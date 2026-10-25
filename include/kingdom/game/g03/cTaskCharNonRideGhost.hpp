#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"

namespace kingdom::game::g03
{

class cTaskCharNonRideGhost : public cTaskCharRouteGhost
{
private:
	void Init();

public:
	void Term() override;

	cTaskCharNonRideGhost();

	cTaskCharNonRideGhost(uint type);

	cTaskChar::eKIND GetKind() override;

	string GetModelKey() override;

	string GetAnmKey() override;

	string GetNodeKey0() override;

	string GetNodeKey1() override;

	uint GetEffectType0() override;

	uint GetEffectType1() override;
};

}
