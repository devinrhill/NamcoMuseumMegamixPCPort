#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// kingdom::game::g11::cTaskConsMenuPlayer (vtable 0x804500f0, 0x210 bytes, ctor 0x80206344): player i's panel of the multiplayer menu (338), layout
// GROBDA_BIN_ANT_MENU_SELECT02. Layer 0 = the frame (anim 0x28 + 2 i appear / 0x29 + 2 i wait), layer 1 = the crown of the best player (anims
// 0x3d..0x3f), layers 2..6 = the five digits of the player's last score (anim 3 * digit + 0 / 1 / 2). Children in the DOL: a cTaskFaceIcon (the Mii,
// +0x200, NOT PORTED) and a cTaskFontWriter (+0x204) that draws the name.
namespace kingdom::game::g11
{
class cTaskConsMenuPlayer : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMenuPlayer(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int player, const std::string& name);   // DOL 0x80206344
	bool Execute(cJetTask* pParam) override;   // DOL 0x80206d3c
	void SetTop(bool b) { m_bTop = b; }        // +0x1e4: the best score gets the crown
	void DrawName() const;                     // the font writer's text (priority 0x80000002)

protected:
	void WaitExec() override;                  // DOL 0x80206b70
	bool AppearExec() override;                // DOL 0x80206be8

private:
	int AnimNo(uint layer, int kind) const;    // DOL 0x80206954
	int m_nPlayer{};                           // +0x1dc
	int m_nScore{};                            // +0x1e0 (sys per-player record)
	bool m_bTop{};                             // +0x1e4
	std::string m_Name;                        // +0x1d8 (the matching info)
	Vector4 m_vPos{};                          // +0x1f0 the name's centre (layout pixels, y up)
	bool m_bShowName{};                        // font writer +0x268
};
}
