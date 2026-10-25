#pragma once

#include "common.hpp"

// Galaga's own object bases (DOL kingdom::game::g01::cObjectBase / cObject / cObjectAABB). Every Galaga game object and the menu's console
// manager derive from cObject (the manager through multiple inheritance, its cJetTask part at +0x50 of the DOL object). Field meanings that
// are not known yet keep their DOL offset in the name; they are renamed as the game code that reads them is ported.
// Destructors of the g01 classes that only free what they own (natively the members / the task tree do it): cTaskBullet 0x800d28b8,
// cTaskBulletDummy 0x800d2c78, cTaskBulletHit 0x800d2db8, cTaskMissile 0x800e7650, cTaskBossMissile 0x800e7990, cTaskBossSpMissile 0x800f2bd4,
// cTaskCookie 0x800eb290, cTaskCourse 0x800e0190, cTaskEffectExpand 0x800e1b70, cTaskGimmick 0x800eddbc, cTaskGimmickBreakComet 0x800edce0,
// cTaskGimmickMComet 0x800ede98, cTaskSS 0x800f90c8.
namespace kingdom::game::g01
{
// DOL vtable 0x80437b60 (+0xc of the object): destructor only
class cObjectBase
{
public:
	cObjectBase() = default;                       // DOL 0x801c16f4
	virtual ~cObjectBase();                        // DOL 0x800bc698 (calls the empty 0x801c1710)
	int nKind() const { return m_nKind; }          // DOL 0x801c1758 (reads cObject +0x10)
	// state machine of the object (e.g. the menu's console manager: Reset 0x800bd984 clears both; the app waits for state 0)
	int nState() const { return m_nState; }
	int nStep() const { return m_nStep; }

protected:
	s32 m_nState{};                                // +0x00
	s32 m_nStep{};                                 // +0x04
	bool m_b08{false};                             // +0x08
	bool m_b09{true};                              // +0x09
	// +0x0c vtable
	s32 m_nKind{};                                 // +0x10 (cObject's field; declared here because nKind is a cObjectBase method)
};

// DOL vtable 0x8044b0e8: ~cObject, HitCheck, HitCallback
class cObject : public cObjectBase
{
public:
	cObject();                                     // DOL 0x801c1714
	~cObject() override;                           // DOL 0x800bc6f8 (calls the empty 0x801c1754)
	virtual bool HitCheck(cObject* other) { (void)other; return false; }   // DOL 0x801c1760
	virtual void HitCallback(cObject* other) { (void)other; }             // DOL 0x801c1768
	bool bActive() const { return m_b08; }         // +0x08 (the collision pass skips inactive objects)
	bool bCollide() const { return m_b14; }        // +0x14
	const float* vPos() const { return &m_v20.x; } // +0x20 world position
	// native helper: cTaskPacman 0x800f4bc4 (+0x2b4 in {1, 2}: Pac-Man is in a jump, shots pass), called by the shots on the kind 3 object
	virtual bool bJumping() const { return false; }

protected:
	bool m_b14{};                                  // +0x14
	s32 m_n18{};                                   // +0x18
	Vector4 m_v20{0.0f, 0.0f, 0.0f, 1.0f};         // +0x20
	Vector4 m_v30{0.0f, 0.0f, 0.0f, 0.0f};         // +0x30
	Vector4 m_v40{1.0f, 1.0f, 1.0f, 1.0f};         // +0x40 scale (FUN_8013e968 = Vector4 ONE)
};

// DOL vtable 0x8044b0d0: + CalcAABB (slot 3). The box of a model's vertex positions (min +0x50, max +0x5c; copies at +0x68 / +0x74).
class cObjectAABB : public cObject
{
public:
	cObjectAABB() = default;                       // DOL 0x801c176c
	~cObjectAABB() override;                       // DOL 0x800c9d6c
	// DOL 0x801c1780: min / max over every position array of the model's ResMdl (NOT PORTED YET: needs the native model's vertex arrays)
	virtual void CalcAABB(void* pModel);
	// native split of 0x801c1780: the model-space box (+0x50 / +0x5c) from the minimum / maximum of every vertex position of the model
	void SetLocalAABB(const float mn[3], const float mx[3]);
	// DOL 0x801c1950 (r4 == 0): world box (+0x68 / +0x74) = AABB::Set(local box, T(+0x20) * Rz Ry Rx (+0x30) * S(+0x40))
	void UpdateWorldAABB();
	// DOL 0x801c1950 (r4 = a matrix): world box = AABB::Set(local box, m); m column major
	void UpdateWorldAABB(const float m[16]);
	bool bAABB() const { return m_bAABB; }
	const float* vWorldMin() const { return m_vMin0; }
	const float* vWorldMax() const { return m_vMax0; }

protected:
	float m_vMin[3]{};                             // +0x50
	float m_vMax[3]{};                             // +0x5c
	float m_vMin0[3]{};                            // +0x68
	float m_vMax0[3]{};                            // +0x74
	bool m_bAABB{false};                           // +0x80
	s32 m_n84{};                                   // +0x84
};
}
