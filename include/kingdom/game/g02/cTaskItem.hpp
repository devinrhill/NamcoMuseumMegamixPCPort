#pragma once

#include "kingdom/game/g02/cTaskGimmick.hpp"

// Rally-X Remix (g02) items and flags (cTaskGimmick subclasses; see cTaskGimmick.hpp). Ported from main.dol.
//
// Items (0x1e4 bytes; one model with ITEM_ANI_OTHER and the SHADOW_CIRCLE sub model; y = 0.04): taken by the collector unit (the player, or
// the red car on the boss stage: 0x800b1dd0) -> particle slot 9 at the item, hidden, inactive. Single player never brings them back (the
// Multi* variants respawn on a timer: NOT PORTED, multiplayer).
//
// Flags (cTaskItemFlag, 0x220 bytes): model 0 = ITEM_FLAG (hidden: the flag a car drops), model 1 = the flag of the class (ITEM_FLAG /
// ITEM_FLAG_SP / ITEM_FLAG_LUCKY / ITEM_FLAG_01..20), both with ITEM_ANI_FLAG + SHADOW_CIRCLE. Taking one (OnHit 0x800b2f38) hides both,
// makes it a plain flag (+0x1e4 = 1, type 0xb) and inactive; the flag manager can then drop it again (+0x5c Drop 0x800b30ac: it flies in a
// sin arc for 60 frames from the dropping car to the target cell, then shows model 0 and is active again).
namespace kingdom::game::g02
{
// native: the shared code of cTaskItemFuelBurst / cTaskItemChameleonPaint / cTaskItemSmokeTank / cTaskItemSpecialArmor (the four DOL classes
// have byte-identical Execute and +0x50 functions; there is no common DOL base class)
class cTaskItem : public cTaskGimmick
{
public:
	cTaskItem(const char* key);       // the factory entries 0x8007113c / 0x80071028 / 0x80070f14 / 0x80070e00 after the base part
	bool Execute(cJetTask* pParam) override;   // 0x80075684 / 0x80074fc0 / 0x8007489c / 0x800741d8
	bool OnHit(cTaskUnit* pUnit) override;     // 0x800752f8 / 0x80074bd4 / 0x80074510 / 0x80073dec
};

// kingdom::game::g02::cTaskItemFuelBurst (vtable 0x80392454; factory 0x8007113c, type 34): ITEM_FUEL
class cTaskItemFuelBurst : public cTaskItem
{
public:
	cTaskItemFuelBurst() : cTaskItem("ITEM_FUEL") { SetName("g02::cTaskItemFuelBurst"); }
	// DOL 0x800751b4
	const char* GetName() const override { return "FuelBurst"; }
};

// kingdom::game::g02::cTaskItemChameleonPaint (vtable 0x80392314; factory 0x80071028, type 35): ITEM_PAINT
class cTaskItemChameleonPaint : public cTaskItem
{
public:
	cTaskItemChameleonPaint() : cTaskItem("ITEM_PAINT") { SetName("g02::cTaskItemChameleonPaint"); }
	// DOL 0x80074a90
	const char* GetName() const override { return "ChameleonPaint"; }
};

// kingdom::game::g02::cTaskItemSmokeTank (vtable 0x803921d4; factory 0x80070f14, type 36): ITEM_SMOKE
class cTaskItemSmokeTank : public cTaskItem
{
public:
	cTaskItemSmokeTank() : cTaskItem("ITEM_SMOKE") { SetName("g02::cTaskItemSmokeTank"); }
	// DOL 0x800743cc
	const char* GetName() const override { return "SmokeTank"; }
};

// kingdom::game::g02::cTaskItemSpecialArmor (vtable 0x80392094; factory 0x80070e00, type 37): ITEM_ARMOR
class cTaskItemSpecialArmor : public cTaskItem
{
public:
	cTaskItemSpecialArmor() : cTaskItem("ITEM_ARMOR") { SetName("g02::cTaskItemSpecialArmor"); }
	// DOL 0x80073ca8
	const char* GetName() const override { return "SpecialArmor"; }
};

// kingdom::game::g02::cTaskItemFlag (0x220 bytes, vtable 0x80396b10)
class cTaskItemFlag : public cTaskGimmick
{
public:
	cTaskItemFlag();                  // the factory head: cTaskGimmick part, +0x210 sound actor, Init (vtable +0x3c)
	void InitFlag();                  // 0x800b28c0 (+0x3c)
	bool Execute(cJetTask* pParam) override;   // 0x800b2b3c
	bool OnHit(cTaskUnit* pUnit) override;     // 0x800b2f38 (+0x50)
	void Enable() override;           // 0x800b31e0 (+0x58): show model 1 (model 0 once taken), active
	bool Drop(const float pTo[4], const float pFrom[4]);   // 0x800b30ac (+0x5c; FlagMan FUN_800704c0): false while active / flying
	void HideModels();                // 0x800b3238 (both models invisible)

	bool IsTaken() const { return m_bTaken; }     // +0x1e4
	bool IsFlying() const { return m_bFlying; }   // +0x1e5 (FlagMan FUN_80070554 compares +0x1f0 while it is set)
	const float* vTo() const { return m_vTo; }    // +0x1f0

protected:
	void SetupFlag(const char* key);  // 0x800b2a20(this, 1, key): model 1

	bool m_bTaken{};                  // +0x1e4
	bool m_bFlying{};                 // +0x1e5
	float m_fTime{};                  // +0x1e8 (0..60)
	float m_vTo[4]{0.0f, 0.0f, 0.0f, 1.0f};     // +0x1f0
	float m_vFrom[4]{0.0f, 0.0f, 0.0f, 1.0f};   // +0x200
	// +0x210 the sound actor (cKingdomSndActor): NOT PORTED
};

// kingdom::game::g02::cTaskItemFlagNormal (vtable 0x80393440; factory 0x80072ca0, type 11): ITEM_FLAG
class cTaskItemFlagNormal : public cTaskItemFlag
{
public:
	cTaskItemFlagNormal();
	// DOL 0x8007697c
	const char* GetName() const override { return m_bTaken ? "Flag" : "NormalFlag"; }
	void Disable() override;          // 0x80076948 (+0x54): hide both models, then the base
};

// kingdom::game::g02::cTaskItemFlagSpecial (vtable 0x80393398; factory 0x80071250, type 33): ITEM_FLAG_SP
class cTaskItemFlagSpecial : public cTaskItemFlag
{
public:
	cTaskItemFlagSpecial();
	// DOL 0x80076888
	const char* GetName() const override { return m_bTaken ? "Flag" : "SpecialFlag"; }
};

// kingdom::game::g02::cTaskItemFlagLucky (vtable 0x803932f0; factory 0x80071414, type 32): ITEM_FLAG_LUCKY
class cTaskItemFlagLucky : public cTaskItemFlag
{
public:
	cTaskItemFlagLucky();
	// DOL 0x800767c8
	const char* GetName() const override { return m_bTaken ? "Flag" : "LuckyFlag"; }
};

// the twenty numbered flags cTaskItemFlagNumber01..20 (types 12..31; one native class): vtables 0x80393200 (01), 0x80393150 (02) ...
// 0x803924f0 (20) (0xb0 apart), factories 0x80072b20 (01), 0x800729f4 (02) ... 0x800714d8 (20) (table 0x80391f48 + 4 * type), names
// 0x80076704 (01), 0x80076640 (02) ... 0x80075878 (20) "NumberFlagNN" / "Flag". All share +0x54 0x80071604 (inactive only, the models stay)
// and +0x58 0x80072c60. The ctor puts ITEM_ANI_FLAGnn_NOTGET (table 0x8036c478 + 4 n) into model 1's slot 0 and disables the flag; only
// number 01 is then enabled at once (FlagMan enables the next one: W2).
class cTaskItemFlagNumber : public cTaskItemFlag
{
public:
	explicit cTaskItemFlagNumber(int n);   // n = 1..20
	const char* GetName() const override;
	// DOL 0x80071604
	void Disable() override { m_bActive = false; }
	void Enable() override;           // 0x80072c60: ReleaseAnmTexPat(model 1, 0) (the NOTGET texture), then cTaskItemFlag::Enable
	int nNumber() const { return m_nNumber; }
private:
	int m_nNumber{};                  // native (the class)
};
}
