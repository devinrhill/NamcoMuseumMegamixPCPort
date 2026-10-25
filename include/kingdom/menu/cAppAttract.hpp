#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "wii/ThpPlayer.hpp"

namespace kingdom::menu
{

// Job 136 (0x08800000): the attract movie. Ported from main.dol: RTTI 0x80478398, vtable 0x80449450, job init 0x8019a864 (0x2a8 byte object),
// setup 0x8019b468, Execute 0x8019b548, Draw 0x8019b624, dtor 0x8019b65c.
//
// It plays "g10_menu/attract.thp" (640x480, 29.97 fps, 2784 frames, stereo 32 kHz) through the SDK THP player and returns to the title:
//   * any button other than B (hold & ~0x400)      -> title with cKingdomSys+0xa91 = 1  (the title then picks layout clips mode 2)
//   * the movie ends (player state 3)               -> title with cKingdomSys+0xa91 = 0
//   * the pad word being 0xFFFF0000                 -> title (a disconnected-pad marker; never produced by this port)
//   * B released (pad word 3) -> Play, else B held -> Pause: holding B freezes the movie, letting go resumes it
// The retail Megamix never enters this job from the title (cAppTitle's idle stopwatch result is discarded, +0x2a0 is only ever cleared):
// it is a leftover of Namco Museum Remix. Reach it with `--job 0x08800000`.
class cAppAttract : public cAppKingdom
{
public:
	cAppAttract();
	~cAppAttract() override;

	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

private:
	wii::ThpPlayer m_Player;
	bool m_bOpen{false};
};

}
