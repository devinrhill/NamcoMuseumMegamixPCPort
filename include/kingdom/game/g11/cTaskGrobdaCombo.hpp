#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g11/cGrobdaCollision.hpp"

namespace kingdom { class cKingdomPtkHnd; }

// Grobda Remix (g11) explosions and combos.
//   cTaskGrobdaBomb (the blast of a dead enemy; ctor 0x8002fbb0 / 0x8002fd7c, Start 0x80030194, Execute 0x8002ff10): a circle (list 3, tag = the combo) whose radius grows from min to
//     max, holds, then its effect (slot 0x18, 0x19 for the grenade size) burns out. kind 0: grow 1800 ms, hold 1800 ms, radius 0.5 .. 2.5; kind 1 / 2: grow 1000 ms, hold 1800 ms, 1.5 .. 4.5.
//     Phases +0xb34: 1 grow, 2 hold, 3 effect, 4 done; the object flag word is 0 / 1 / 2 / 3 after each phase. Type = the owner's (0x4000001 + player; kind 2 -> 0x4000006 + player).
//   cTaskGrobdaCombo (0x1cc bytes, vtable 0x8038bca8; ctor 0x80034224, Hit 0x8003458c, Execute 0x8003440c): one per shooter run; every kill adds a bomb; the count (+0x1b8) resets when
//     the last bomb is gone; the stage releases it in state 2.
namespace kingdom::game::g11
{
class cTaskGrobdaCombo;

class cTaskGrobdaBomb : public kingdom::cTaskKingdom, public cGrobdaObject
{
public:
	cTaskGrobdaBomb(cTaskGrobdaCombo* combo, int kind);   // DOL 0x8002fbb0 + 0x8002fd7c
	~cTaskGrobdaBomb() override;
	void Start(uint type, const float pos[3]);           // DOL 0x80030194
	bool Execute(cJetTask* pParam) override;             // DOL 0x8002ff10
	int nFlags2() const { return (int)nFlags(); }
	bool IsDone() const { return nFlags() == 3u; }       // the combo releases the bombs whose flag word is 3

	float* GetPos() override { return m_vPos; }
	float* GetRot() override { return m_vRot; }

private:
	int m_nKind{};                // +0xb30
	int m_nPhase{};               // +0xb34
	bool m_bEnter{};              // +0xb38
	uint m_nGrow{}, m_nHold{};    // +0xb3c / +0xb40
	float m_fMin{}, m_fMax{};     // +0xb44 / +0xb48
	cTaskGrobdaCombo* m_pCombo{}; // +0xb4c (the tag)
	float m_vPos[4]{};
	float m_vRot[4]{};
	kingdom::cKingdomPtkHnd* m_pHnd{};   // +0xb50
};

class cTaskGrobdaCombo : public kingdom::cTaskKingdom
{
public:
	explicit cTaskGrobdaCombo(uint type);                // DOL 0x80034224 (+0x1b4 = the owner's type 0x4000001 + player)
	bool Execute(cJetTask* pParam) override;             // DOL 0x8003440c
	void Hit(const float pos[3], int kind, bool count);  // DOL 0x8003458c: a blast at the place; kind 2 turns the type to 0x4000006 + player
	int nCount() const { return m_nCount; }              // +0x1b8
	int nMode() const { return m_nMode; }                // +0x1c8: 1 active, 2 done
	void SetMode(int m) { m_nMode = m; }
	uint nType() const { return m_nType; }
	int nBombs() const { return (int)m_vBomb.size(); }

private:
	uint m_nType{};                       // +0x1b4
	int m_nCount{};                       // +0x1b8
	std::vector<cTaskGrobdaBomb*> m_vBomb;   // +0x1bc
	int m_nMode{};                        // +0x1c8
};
}
