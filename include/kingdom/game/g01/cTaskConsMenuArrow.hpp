#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Galaga (g01) stage select: left / right arrow console (DOL vtable 0x80437740, 0x1e4 bytes). Ported from main.dol only.
namespace kingdom::game::g01
{

class cTaskConsMenuArrow : public cTaskConsMenuBase
{
protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_MAX = 4
	};

	// DOL 0x8036fd10: animation per arrow index (0/1) and EAnm
	static const int s_AnmTbl[2][eANM_MAX];

	// DOL r2-0x6b30 (0x8047b0d0): hit pattern per arrow index
	static const int s_HitPatTbl[2];

private:
	int m_nIndex{};     // +0x1d8 arrow index 0/1 (ctor arg)
	bool m_bActive{};   // +0x1dc ctor arg (manager +0x21c != 0); false = never shown, never hit. UNVERIFIED meaning
	int m_nHitPat{};    // +0x1e0 s_HitPatTbl[m_nIndex]

public:
	// DOL 0x800ba6d0
	cTaskConsMenuArrow(cTaskConsMenuManBase* const& pConsMan, int nIndex, bool bActive);
	bool bActive() const { return m_bActive; }   // read by cTaskConsMenuMan::KeySelect (0x800bdac8)

	// DOL 0x800ba7a0 (thunk 0x800bac54)
	virtual ~cTaskConsMenuArrow();

	// DOL 0x800ba7f8
	bool IsDpdHit(float x, float y) override;

protected:
	// DOL 0x800ba950
	void WaitExec() override;

	// DOL 0x800ba9e4
	bool AppearExec() override;

	// DOL 0x800baae4
	void PointExec() override;

	// DOL 0x800bab78
	bool DecideExec() override;
};

}
