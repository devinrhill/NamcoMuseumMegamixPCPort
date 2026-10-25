#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g01
{

// Galaga stage select: the single / multi player panel (layout ANT_MENU, crc 0xd3a909fe). vtable 0x80437c48.
// Created by the console manager ctor (0x800bc114) as Multi(man, bMulti man+0x20c).
class cTaskConsMenuMulti : public cTaskConsMenuBase
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

	static constexpr int HIT_PAT = 12;              // IsDpdHit 0x800beb8c: li r5, 0xc

	static const int ANM_LIST[2][eANM_MAX];         // DOL 0x803703b8: [bMulti != 0][anm]

private:
	uint m_bMulti{};   // +0x1d8 (cG01GalagaSys::IsMulti of the manager, +0x20c)
	int m_n1dc{};      // +0x1dc: 0 in the ctor and in DecideExec step 0, never read here (UNVERIFIED meaning)

public:
	cTaskConsMenuMulti(cTaskConsMenuManBase* const& pConsMan, uint bMulti);   // DOL 0x800be9b0

	virtual ~cTaskConsMenuMulti();   // DOL 0x800bea7c (thunk 0x800bef4c)

	bool IsDpdHit(float x, float y) override;   // DOL 0x800bead4

protected:
	int nGetAnm(int nAnm);   // inlined table read (0x803703b8) of every *Exec

	void WaitExec() override;     // DOL 0x800bec0c

	bool AppearExec() override;   // DOL 0x800beca8

	void PointExec() override;    // DOL 0x800bedb0

	bool DecideExec() override;   // DOL 0x800bee4c
};

}
