#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom
{

// The pointer sprite of the menus (kingdom::cTaskCursor, DOL vtable 0x80394450; the Pac-N-Roll menu uses the derived class with vtable 0x803961a8).
//   Execute 0x80095be4, Draw 0x80095e0c, Init 0x800956e4 (this, padNo, layout crc, idle anim), AddButton 0x800958b8 (mask, anim),
//   GetPadNo 0x80095bcc, GetPointer 0x80095ef4 (float* x, float* y -> bool). Further engine slots of its vtable, not overridden by the games:
//   0x8009585c (anita 0's animation with three arguments), 0x80095bd4 / 0x80095bdc (+0x1c8 set / get), 0x80095f64 (a vector (0, 0, 0, 1), then a
//   virtual call).
// One layout (anita 0, BIN_COMMON_ANT_CURSOR: anim 0 idle, anim 1 pressed) is moved to the pointer every frame; it is hidden while the remote
// points off screen or B is held. Pressing a registered button plays that button's animation, then the idle one again.
class cTaskCursor : public cTaskAnita
{
public:
	cTaskCursor();
	virtual ~cTaskCursor();

	// DOL 0x800956e4 / 0x800957a0: pad number, layout name (the DOL passes its CRC), idle animation. Hidden until the first Execute.
	void Init(int padNo, const char* layoutName, int idleAnim);
	// DOL 0x800958b8: pressing any bit of `mask` (pad trig) plays animation `anim`
	void AddButton(unsigned mask, int anim);
	int GetPadNo() const { return m_nPadNo; }
	void SetPadNo(int padNo) { m_nPadNo = padNo; }     // DOL 0x80095bc4 (vtable slot 0x58)
	void SetEnable(bool b) { m_bEnable = b; }          // +0x1e1: a disabled cursor is not drawn (the multiplayer menu's unused players)
	// DOL 0x80095ef4: pointer in -1..1 (y down), false when the remote does not point at the screen
	bool GetPointer(float& x, float& y) const;

	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

private:
	struct Button { unsigned mask; int anim; };
	int m_nPadNo{};          // +0x1cc
	int m_nIdleAnim{};       // +0x1d0
	std::vector<Button> m_buttons; // +0x1d4
	bool m_bPressed{};       // +0x1e0: a press animation is playing
	bool m_bEnable{true};    // +0x1e1
};

}
