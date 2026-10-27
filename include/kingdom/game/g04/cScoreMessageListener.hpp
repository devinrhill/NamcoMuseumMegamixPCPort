#pragma once

#include "common.hpp"

// Gator Panic Remix (g04) kingdom::game::g04::cScoreMessageListener: the interface a Pac-Man reports to (+0x70c); cTaskConsScore (single,
// +0x1c8 of it, vtable 0x80449cd8) and cTaskConsMultiScore implement it. Slots as the Pac-Man calls them:
//   +0x0c / +0x24 SetBonus (0x801a6d40: +0x1d0; multi: blr 0x801a4274)   +0x10 (single blr 0x801a6bc0, multi 0x801a37a4; not called by the Pac-Man)
//   +0x14 SetHits (0x801a6bc4)   +0x18 SetDamage (0x801a6c38)
//   +0x1c Combo (0x801a6cdc: chain >= 2 sets the combo bonus and shows the combo number)   +0x20 CanPay (0x801a6cb8: score - n >= 0)
namespace kingdom::game::g04
{
class cTaskPacmanBase;

struct sScoreListener
{
	virtual ~sScoreListener() = default;
	virtual void SetBonus(cTaskPacmanBase* p, int n) = 0;
	virtual void SetHits(cTaskPacmanBase* p, int n) = 0;
	virtual void SetDamage(cTaskPacmanBase* p, int n) = 0;
	virtual void Combo(cTaskPacmanBase* p, int combo, uint chain) = 0;
	virtual bool CanPay(cTaskPacmanBase* p, int n) = 0;
};
}
