#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g11/cGrobdaCollision.hpp"
#include "kingdom/game/g11/cTaskGrobdaModel.hpp"

namespace kingdom { class cKingdomPtkHnd; }

// Grobda Remix (g11) shots: kingdom::game::g11::cTaskGrobdaShot (base, 0xb40 bytes: cGrobdaObject + cTaskKingdom at +0x97c; ctor 0x800aaa0c, Execute 0x800aac50, fire
// 0x800ab090, move 0x800aadfc, Resolve 0x800ab4b4 / 0x800ab930, hit stop 0x800ab424), cTaskGrobdaShotPlayer (ctor 0x800ac4a8, fire wrapper 0x800ac640 = the sound m11se_laser +
// 0x800ab090, register 0x800ac6f8 = list 5, Resolve 0x800ac738) and cTaskGrobdaShotEnemy (ctor 0x800ab5b8, fire wrapper 0x800ab6e0 = the effect (slot 0x10 + type - 0x2000001) + the
// sound m11se_beam + 0x800ab090, register 0x800ab8f0 = list 6, Resolve 0x800ab930). The shot is a PTK effect whose handle (+0xb38) follows it; the object has no model: position /
// rotation live in members (vtable +0x18 / +0x28 of the DOL read the handle's translate / rotate). Radius 0.2.
// NOT PORTED: the homing mode 1 of the enemy shots (FUN_800aadfc, FireAimed 0x800ab7cc / 0x800ab254).
namespace kingdom::game::g11
{
class cTaskGrobdaShot : public kingdom::cTaskKingdom, public cGrobdaObject
{
public:
	cTaskGrobdaShot();                          // DOL 0x800aaa0c (state 0, mode 0, the effect handle +0xb38)
	~cTaskGrobdaShot() override;
	bool IsActive() const { return m_nState == 1; }
	int nState() const { return m_nState; }
	bool Execute(cJetTask* pParam) override;    // DOL 0x800aac50
	void Resolve(int a, int b) override;        // DOL 0x800ac738 / 0x800ab930 (the same body)

	float* GetPos() override { return m_vPos; }
	float* GetRot() override { return m_vRot; }

protected:
	virtual int Layer() const = 0;              // the list the shot registers in (vtable +0x44: 0x800ac6f8 -> 5, 0x800ab8f0 -> 6); < 0: none (the grenade)
	void BaseFire(float speed, const float pos[3], float rotY);   // DOL 0x800ab090
	void GenerateHandle(uint slot);             // the effect of the shot into its handle (FUN_8009ff60(app, slot, pos, 0, handle, 0))
	void Stop();                                // DOL 0x800ab424
	void HitEffect();                           // DOL 0x800ab4b4: the nearest hit of a kind < 3 spawns the effect 0x17 at its point
	int m_nState{};                             // the state word (GXInitTexObjUserData(this, n)): 0 idle, 1 flying
	int m_nMode{};                              // +0xb30 (1 = homing)
	cTaskGrobdaModel* m_pTarget{};              // +0xb34 the player a homing shot follows
	float m_fHomeSpeed{};                       // +0x28 of the DOL (SetCharSpace)
	float m_vPos[4]{};
	float m_vRot[4]{};
	kingdom::cKingdomPtkHnd* m_pHnd{};          // +0xb38
};

class cTaskGrobdaShotPlayer : public cTaskGrobdaShot
{
public:
	explicit cTaskGrobdaShotPlayer(int player);   // DOL 0x800ac4a8 (type = the player index)
	void Fire(float speed, const float pos[3], float rotY);   // DOL 0x800ac640
protected:
	int Layer() const override { return 5; }
private:
	int m_nPlayer{};
};

// kingdom::game::g11::cTaskGrobdaShotGrenade (0xb50, vtable 0x80395478; ctor 0x800abb8c / 0x800abc90, throw 0x800ac050, Execute 0x800abddc, blast 0x800ac320): a ball on a ballistic arc
// (gravity = the player's GRE_GRAVITY), the trail effect (slot 0x1a + player); where it lands the blast (cTaskGrobdaCombo::Hit kind 2) goes off. Type 0x3000001 + player. No list.
class cTaskGrobdaShotGrenade : public cTaskGrobdaShot
{
public:
	explicit cTaskGrobdaShotGrenade(uint type);
	void Throw(float speed, const float pos[3], const float rot[3]);   // DOL 0x800ac050
	bool Execute(cJetTask* pParam) override;                           // DOL 0x800abddc
	static float s_fGravity;                                           // FLOAT_804772dc (set by the player's Init)
protected:
	int Layer() const override { return -1; }
private:
	void Explode();                                                    // DOL 0x800ac320
	bool m_bFlight{};                                                  // +0xb3c
};

class cTaskGrobdaShotEnemy : public cTaskGrobdaShot
{
public:
	explicit cTaskGrobdaShotEnemy(uint type);     // DOL 0x800ab5b8 (type 0x2000001 + the enemy kind)
	bool Fire(float speed, const float pos[3], float rotY);   // DOL 0x800ab6e0: false when the effect could not be made
	bool FireAimed(float speed, const float pos[3], const float rot[3]);   // DOL 0x800ab7cc: a homing shot at the nearest player (nothing when there is none)
protected:
	int Layer() const override { return 6; }
};
}
