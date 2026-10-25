#pragma once

#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g04/cAnimeBase.hpp"

// Gator Panic Remix (g04) the tortoise host: kingdom::game::g04::cTaskTortoise (0x2e0 bytes, vtable 0x8044a7bc; builder 0x80093260 at app
// +0x280 puts BIN_MODEL_KAME at the BG node kame_set, turned by pi; Init 0x801b7768 light / fog 1). SetState 0x801b7c7c:
//   0 / 3 / 10 idle (ANIME_KAME_A03)   1 / 2 the READY talk (A02, PTK_EFFECT10_KAME01)   4 a player was bitten (A04 + A04_CLA, PTK_EFFECT07_HIT)
//   5..9 the result reaction by rank (A05..A08 / A10 with TEX_ANIME_KAME_05..10, PTK_EFFECT12..16)   10 the result: he turns (rot.y -0.1745 a
//   frame) and +0x2d1 is set once he faces the camera (rot.y < 0).
namespace kingdom::game::g04
{
class cTaskTortoise : public kingdom::cTaskModel
{
public:
	explicit cTaskTortoise(const Vector4& pos);   // the builder 0x80093260
	void Init();                                  // DOL 0x801b7768
	void Reset();                                 // DOL 0x801b7810
	bool Execute(cJetTask* pParam) override;      // DOL 0x801b7898
	void SetState(int state);                     // DOL 0x801b7c7c
	void Result(int rank);                        // DOL 0x801b86b8
	bool bTurned() const { return m_bTurned; }    // +0x2d1

private:
	cAnimeBase m_Anime;                           // +0x1cc
	int m_nState{};                               // +0x1d0
	int m_nOldState{};                            // +0x1d4
	int m_nTimer{};                               // +0x1d8
	kingdom::cKingdomPtkHnd m_HitFx;              // +0x1e0
	kingdom::cKingdomPtkHnd m_TalkFx;             // +0x230
	kingdom::cKingdomPtkHnd m_ResultFx;           // +0x280
	bool m_b2d0{}, m_bTurned{}, m_bResultFx{}, m_b2d3{};   // +0x2d0..+0x2d3
};
}
