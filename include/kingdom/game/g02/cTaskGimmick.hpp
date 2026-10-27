#pragma once

#include <vector>

#include "WiiSnd.hpp"
#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cKingdomSndActor.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/cTaskModel.hpp"

// Rally-X Remix (g02) gimmicks: kingdom::game::g02::cTaskGimmick (vtable 0x80396538, a cTaskModel) and the road hazards, plus the gimmick
// manager cTaskGimmickMan (0x1c0 bytes, vtable 0x803936e8). The items and flags (also cTaskGimmick subclasses) are in cTaskItem.hpp.
// Ported from main.dol (no C# exists for Rally-X).
//
// Every gimmick sits on a MAPOO cell: cell (x, z) (+0x1cc / +0x1d0) = world (x - 4, 0, z - 8). The factory 0x800736cc (table 0x80391f48, 40
// entries) news the class of a type (+0x1d4), then writes the cell and the direction (+0x1d8: 0 / 1 / 2 / 3 = rot Y 0, pi, 3pi/2, pi/2).
// The vtable slots past the cTaskModel ones:
//   +0x3c Init 0x800b1944    +0x40 Term 0x800b1a48    +0x44 name (debug strings)   +0x48 HitTest(cell[2]) 0x800b1d08
//   +0x4c IsAt(x, z) 0x800b1d24 (cell equality)   +0x50 OnHit(unit) 0x800b1d4c (base: return 1)
//   +0x54 Disable 0x800b1d54 (+0x1dc = 0)   +0x58 Enable 0x800b1d60 (+0x1dc = 1)   (+0x5c: the smoke's Start(index) / the flag's Drop(to, from))
// +0x1dc = "active": the contact test skips inactive gimmicks, the radar hides a taken flag until it is active again (cTaskConsole::Hooks.
// GimmickRespawn = GimmickActive below).
// The gimmick Executes return at once while the app is paused (+0x1b4), the stage is ending (+0x284) or HOME is held.
namespace kingdom::game::g02
{
class cTaskUnit;

// kingdom::game::g02::cParamGimmick (0x28 bytes, a cJetCSV, vtable 0x80396594): CSV_GIMMICK, shared by every gimmick (reference counted by
// cTaskGimmick Init / Term: DAT_80478fc8 count, DAT_80478fcc the object). +0x20 = row 0 column 1, +0x24 = 1. Nothing in the DOL reads it
// (the only r13 - 0x5754 references are Init / Term).
class cParamGimmick : public kingdom::cKingdomCSV
{
public:
	cParamGimmick();                  // the Init part of 0x800b1944
	// DOL 0x800b1aa0 (the deleting dtor: cJetCSV dtor)
	~cParamGimmick() override = default;
	float fValue{};                   // +0x20 (UNVERIFIED: read as F32; the decomp shows cJetCSV::S32 returning a double)
	bool bLoaded{};                   // +0x24

	static void AddRef();             // 0x800b1944 (count +1; new on the first)
	static void Release();            // 0x800b1a48 (count -1; delete on the last)
	static cParamGimmick* Get() { return s_pParam; }

private:
	static int s_nRef;                // DAT_80478fc8
	static cParamGimmick* s_pParam;   // DAT_80478fcc
};

// kingdom::game::g02::cTaskGimmick (0x1e4 bytes)
class cTaskGimmick : public kingdom::cTaskModel
{
public:
	cTaskGimmick();                   // the shared head of every factory entry: cTaskModel ctor, +0x1cc / +0x1d0 = 0, Init (vtable +0x3c)
	~cTaskGimmick() override;         // DOL 0x80070830 (Term not run natively for non-app tasks)

	// vtable 0x80396538
	void InitGimmick();               // 0x800b1944 (+0x3c)
	void Term() override;             // 0x800b1a48 (+0x40)
	bool Execute(cJetTask* pParam) override;   // 0x800b1b38 (+0x18)
	// DOL 0x800b1cec (+0x44)
	virtual const char* GetName() const { return "unkown"; }
	bool HitTest(const int pCell[2]) { return IsAt(pCell[0], pCell[1]); }   // 0x800b1d08 (+0x48)
	virtual bool IsAt(int x, int z) const;     // 0x800b1d24 (+0x4c)
	// DOL 0x800b1d4c (+0x50)
	virtual bool OnHit(cTaskUnit* pUnit) { (void)pUnit; return true; }
	// DOL 0x800b1d54 (+0x54)
	virtual void Disable() { m_bActive = false; }
	// DOL 0x800b1d60 (+0x58)
	virtual void Enable() { m_bActive = true; }

	// DOL 0x800b1d6c
	bool IsActive() const { return m_bActive; }
	// DOL 0x800b1cdc (&+0x1cc)
	int* pCell() { return &m_nX; }
	// DOL 0x800b1ce4 (&+0x1d4)
	int nType() const { return m_nType; }
	// DOL 0x800b1d00 (&+0x1e0: the smoke screen's owner unit id; FUN_80085f50 reads it for type 0)
	int nOwner() const { return m_nOwner; }

	int m_nX{};                       // +0x1cc
	int m_nZ{};                       // +0x1d0
	int m_nType{-1};                  // +0x1d4
	int m_nDir{};                     // +0x1d8
	bool m_bActive{true};             // +0x1dc
	int m_nOwner{};                   // +0x1e0

	// the shared helpers of the subclasses
	static bool IsFrozen();           // the Execute head: app +0x1b4 || app +0x284 || get_pad1(0) & 0x400
	void RotY(Vector4& rot) const;    // 0x800b1d74 (+ 0x800b1dc4 for dir 3): rot.y by +0x1d8
	static bool IsCollector(cTaskUnit* pUnit);   // 0x800b1dd0: the unit that takes items / flags (boss stage: id & 0x20, else id & 0x10)
	static const char* ShadowClip();  // 0x800c5ce0: SHADOW_COLOR_WORLDn by world (table 0x80371188)
	// the flag / item model setup: RegistModel(key, 0x36f), light set 1, fog 1, clip slot 0, SHADOW_CIRCLE expanded on node "shadow" with
	// the world's colour clip (0x8001f72c / 0x8001e1f8 / 0x8001e400 / 0x8001a4d8 / 0x8001fe58)
	kingdom::cKingdomModel* SetupModel(uint nIdx, const char* key, const char* clip, bool bShadow);
	static void Fx(kingdom::cKingdomPtkHnd& h, uint nSlot, bool bOnetime, bool bDepend, bool bClip, const Vector4& pos,
	               const Vector4* pRot = nullptr);   // FUN_80021a9c
	static void FxOnce(uint nSlot, kingdom::cKingdomModel* m);   // a temporary handle, slot at the model (trans + rotate), one-time
	static void Se(const char* cue);  // FUN_80129d78 with a temporary cRallyXSndHnd and no actor (the smoke plays on its own actor)

protected:
	void ReleaseTerm() override { cTaskGimmick::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

// DOL 0x800b1d6c: cTaskConsole::Hooks.GimmickRespawn (false when g is not a cTaskGimmick)
bool GimmickActive(kingdom::cTaskModel* g);

// kingdom::game::g02::cTaskGimmickStone (0x1e4, vtable 0x80393650; factory 0x8007352c): GIMMICK_STONE. +0x50 0x80076b3c = the base.
class cTaskGimmickStone : public cTaskGimmick
{
public:
	cTaskGimmickStone();
	// DOL 0x80076b40
	const char* GetName() const override { return "Stone"; }
};

// kingdom::game::g02::cTaskGimmickOil (0x1e4, vtable 0x803935b8; factory 0x80073474): GIMMICK_OIL
class cTaskGimmickOil : public cTaskGimmick
{
public:
	cTaskGimmickOil();
	// DOL 0x80076abc
	const char* GetName() const override { return "Oil"; }
};

// kingdom::game::g02::cTaskGimmickSpike (0x1e4, vtable 0x80393520; factory 0x800733bc): GIMMICK_SPIKE
class cTaskGimmickSpike : public cTaskGimmick
{
public:
	cTaskGimmickSpike();
	// DOL 0x80076a3c
	const char* GetName() const override { return "Spike"; }
};

// the seven mud classes (0x240 bytes each; one native class, picked by type 4..10):
//   4 cTaskGimmickMud1 (vtable 0x803969a0, factory 0x800732d4, GIMMICK_MUD_ONE, name 0x800b22d8 "Mud ONE")
//   5 cTaskGimmickMud_RD_0 (0x803967c0, 0x800731ec, GIMMICK_MUD_RD_0, 0x800b20a4 "Mud_0_RIGHT_OR_DOWN")
//   6 cTaskGimmickMud_RD_1 (0x803965e0, 0x80073104, GIMMICK_MUD_RD_1, 0x800b1e70 "Mud_1_RIGHT_OR_DOWN")
//   7 cTaskGimmickMud_C_0 (0x80396860, 0x8007301c, GIMMICK_MUD_C_0, 0x800b2160 "Mud_0_CENTER")
//   8 cTaskGimmickMud_C_1 (0x80396680, 0x80072f34, GIMMICK_MUD_C_1, 0x800b1f2c "Mud_1_CENTER")
//   9 cTaskGimmickMud_LU_0 (0x80396900, 0x80072e4c, GIMMICK_MUD_LU_0, 0x800b221c "Mud_0_LEFT_OR_UP")
//  10 cTaskGimmickMud_LU_1 (0x80396720, 0x80072d64, GIMMICK_MUD_LU_1, 0x800b1fe8 "Mud_1_LEFT_OR_UP")
// Each has a cKingdomPtkHnd at +0x1f0 that nothing uses (not ported); +0x50 = the base (0x800b1e54 .. 0x800b1e6c branch to 0x800b1d4c).
class cTaskGimmickMud : public cTaskGimmick
{
public:
	explicit cTaskGimmickMud(int nType);
	const char* GetName() const override;
private:
	int m_nKind{};                    // type - 4 (native: picks the class)
};

// kingdom::game::g02::cTaskGimmickSmoke (0x250 bytes, vtable 0x80396ab0; factory 0x800735e4): the smoke screen a car drops. No model: only
// the particle (+0x1f0) on its cell. The map keeps a pool (0x20 made by the map Init, more on demand) in the smoke manager.
class cTaskGimmickSmoke : public cTaskGimmick
{
public:
	cTaskGimmickSmoke();
	void InitSmoke();                 // 0x800b2390 (+0x3c): owner 0x20070222, timer 0, Disable
	bool Execute(cJetTask* pParam) override;   // 0x800b23f0: alive for 0xb4 frames
	// DOL 0x800b27dc (+0x44)
	const char* GetName() const override { return "Smoke"; }
	void Disable() override;          // 0x800b261c (+0x54): particle detach + base
	void Enable() override;           // 0x800b24bc (+0x58): Start(0)
	void Start(int nIndex);           // 0x800b24d0 (+0x5c): the particle (slot 1; multiplayer 0xc / 0xd / 0xe / 1 by index), timer 0, Enable
	bool Put(int nOwner, const int pCell[2], int nIndex);   // 0x800b2658 (called by the smoke manager FUN_8007c47c)

	kingdom::cKingdomPtkHnd m_Fx;     // +0x1f0
	kingdom::cKingdomSndActor m_SndActor;   // +0x244 (FUN_8002654c)
	int m_nTime{};                    // +0x240
	// +0x244 the sound actor (cKingdomSndActor): NOT PORTED
};

// kingdom::game::g02::cTaskGimmickMan (0x1c0 bytes): owns every gimmick (+0x1b4 vector)
class cTaskGimmickMan : public kingdom::cTaskKingdom
{
public:
	cTaskGimmickMan();
	// DOL 0x800736c4
	bool Execute(cJetTask* pParam) override { (void)pParam; return true; }
	cTaskGimmick* Create(int nType, const int* pCell, int nDir);   // 0x800736cc
	cTaskGimmick* Create(int nType, int x, int z, int nDir) { const int c[2] = {x, z}; return Create(nType, c, nDir); }
	// DOL 0x80073a2c: every active gimmick on (x, z); with bReact the unit's reaction (cTaskUnit::OnGimmick 0x80085f50) then the gimmick's
	// +0x50. True when one was there.
	bool Contact(cTaskUnit* pUnit, int x, int z, bool bReact);
	bool IsCellFree(int x, int z) const;   // 0x80073af4 (no active gimmick on the cell)
	bool IsTypeActive(int nType) const;    // DOL 0x80073ba0 (type 0x29) / 0x80073c24 (type 0x27): is any gimmick of that type active

	std::vector<cTaskGimmick*> m_vec; // +0x1b4
};
}
