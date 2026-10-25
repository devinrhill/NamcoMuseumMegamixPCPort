#pragma once

#include "kingdom/cTaskFade.hpp"

// Rally-X Remix (g02) the screen fade: kingdom::game::g02::cTaskRallyXFade (0x620 bytes, vtable 0x80441124, parent kingdom::cTaskFade; ctor
// 0x801242c4, Execute 0x801272c8, dtor 0x80127660). Made by the app ctor 0x801236bc at +0x2a4 (priority 0x8fffffef, Fade(2, 500)).
// Execute is cTaskFade::Execute (0x80009e5c) plus a pause test: the fade does not advance while the app is paused (app +0x1b4).
namespace kingdom::game::g02
{
class cTaskRallyXFade : public kingdom::cTaskFade
{
public:
	cTaskRallyXFade();                                  // DOL 0x801242c4
	bool Execute(cJetTask* pParam) override;            // DOL 0x801272c8
};
}
