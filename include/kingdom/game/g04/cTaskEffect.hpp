#pragma once

#include "kingdom/cTaskPBPtkMan.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskKingdom.hpp"

#include <random>

// Gator Panic Remix (g04) particle manager: kingdom::game::g04::cTaskEffect (0xd50 bytes, vtable 0x80449e80; builder 0x80093ae0 at app +0x29c,
// priority 0x10003, also the global particle manager (KingSys +0xa40 -> +0x1bc)). Setup 0x801a86d0: Create(27 slots, 0x5a, 0xfa, 0x9c4)
// (0x80021b80), slot i = PTK_EFFECTnn of the CRC table 0x8037ce30 (0x80021c58; 25..27 are not in the archives). Execute 0x801a8784 = the
// particle calc unless paused / HOME. The builder's two other tasks, cTaskEffectRensFlear and cTaskEffectBird, are below.
namespace kingdom::game::g04
{
class cTaskEffect : public kingdom::cTaskPBPtkMan
{
public:
	cTaskEffect();                              // DOL 0x801a86d0
	bool Execute(cJetTask* pParam) override;    // DOL 0x801a8784
};

// The lens flare: kingdom::game::g04::cTaskEffectRensFlear (0x240 bytes, vtable 0x80449f88, app +0x2ac), built by the effect builder 0x80093ae0
// at the BG's rense_set (0x8019d238 / 0x801a90cc) and connected to the app. Between Start (the game starts) and Stop (time up) it fires the
// one-time particle PTK_EFFECT17_LENS01 (slot 0x10) every (rand % CSV_EFFECT row 1 + row 0) seconds, scaled by a random mix of rows 2 / 3.
class cTaskEffectRensFlear : public kingdom::cTaskKingdom
{
public:
	explicit cTaskEffectRensFlear(const Vector4& pos);   // DOL 0x801a8e04 (Init) + 0x801a90cc (position)
	bool Execute(cJetTask* pParam) override;            // DOL 0x801a8ee0
	void Start();                                       // DOL 0x801a9064
	void Stop() { m_nOn = 0; }                          // DOL 0x801a90c0

private:
	Vector4 m_vPos;                    // +0x1c0
	int m_nWait{};                     // +0x1d0
	kingdom::cKingdomPtkHnd m_Hnd;     // +0x1e0
	int m_nOn{};                       // +0x230
	std::mt19937 m_Rand{0x20061206u};  // +0x234 (cKingdomRand, seed 0x20061206; UNVERIFIED: = std::mt19937)
	uint m_nMin{}, m_nRange{1};        // cParamEffect +0x20 / +0x24 (rows 0 / 1, seconds)
	float m_fScale[2]{1.0f, 1.0f};     // cParamEffect +0x28 / +0x2c (rows 2 / 3)
};

// The birds: kingdom::game::g04::cTaskEffectBird (0x240 bytes, vtable 0x80449f28), built at the end of the effect builder 0x80093ae0 and
// connected to the app. Every (rand % CSV_EFFECT row 5 + row 4) seconds it fires the one-time particle PTK_EFFECT23_TORI01 (slot 0x16) at the
// BG's bird_set node. The DOL seeds a new cKingdomRand with 0x20061206 for every draw, so the interval is always the same.
class cTaskEffectBird : public kingdom::cTaskKingdom
{
public:
	explicit cTaskEffectBird(const Vector4& pos);   // DOL 0x801a8b50 (cParamEffect, CSV_EFFECT 0x8019bac8) + 0x801a8d1c (position)
	bool Execute(cJetTask* pParam) override;        // DOL 0x801a8be0

private:
	Vector4 m_vPos;                    // +0x1c0
	int m_nWait{};                     // +0x1d0 frames to the next bird
	kingdom::cKingdomPtkHnd m_Hnd;     // +0x1e0
	uint m_nMin{}, m_nRange{1};        // cParamEffect +0x30 / +0x34 (CSV_EFFECT rows 4 / 5, seconds)
};
}
