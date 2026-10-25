#pragma once

#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"
#include "kingdom/game/g01/cTaskShootMarker.hpp"

// Galaga Remix (g01) the last boss's shells: kingdom::game::g01::cTaskShell (0x2c0 bytes, vtable 0x80438c68; ctor 0x800c9e04, made by the
// boss's model setup 0x800cb308 for shell1..4). Kind 0xb, CHAR_ENEMY_SHELL_MDL expanded onto the boss model at its node (0x800ca35c), HP from
// ShellParam_%dp.csv. Not registered for collision: the boss tests the players' shots against them first (0x800cf250 -> vtable +0x68) and
// hands the hit on (cTaskBoss::HitCallback). The vomit attack opens them (CHA_01 / CLA_01, collidable); breaking all four before o_Vomit01
// cancels the vomit, else they close; the struggle breaks them for good (CLA_02). Not ported: the 3D sound actor (its shoot marker, cTaskShootMarker
// 0x800f84d4, is).
namespace kingdom::game::g01
{
class cTaskBoss;

class cTaskShell : public cObjectAABB, public kingdom::cTaskModel
{
public:
	cTaskShell(int nIndex, cTaskBoss* pBoss, cKingdomModel* pBossModel, const char* node);   // DOL 0x800c9e04 (+ setup 0x800ca35c)
	bool Execute(cJetTask* pParam) override;   // DOL 0x800ca464
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x800ca4f4 (natively the boss model draws the expanded shell)
	void HitCallback(cObject* other) override; // DOL 0x800ca548
	bool RayHit(const float org[3], const float dir[3], float& t) const;   // DOL 0x800ca518 (vtable +0x68)
	void Open();                               // DOL 0x800ca6ac (vtable +0x78)
	void Activate();                           // DOL 0x800ca664 (vtable +0x74)
	void Close();                              // DOL 0x800ca7e8 (vtable +0x7c)
	void Break();                              // DOL 0x800ca760 (vtable +0x80)
	bool IsClosed() const { return m_nState == 0; }   // DOL 0x800ce330 (vtable +0x8c)

private:
	void Hide();                               // DOL 0x800ca690 (vtable +0x60): not collidable (the marker hides)
	void UpdatePos();                          // DOL 0x800ca208 (vtable +0x5c): +0x270 = the model's world matrix, +0x20 its origin
	int m_nIndex{};                            // +0x258
	u32 m_nAnm{};                              // +0x25c bit 0 chr, bit 1 clr registered
	int m_nHp{1};                              // ShellParam HP (+0x254 -> +0)
	cTaskBoss* m_pBoss{};
	cTaskShootMarker* m_pMarker{};             // +0x2b0
	float m_mWorld[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};   // +0x270 (column major)
	bool m_bMtx{};
};
}
