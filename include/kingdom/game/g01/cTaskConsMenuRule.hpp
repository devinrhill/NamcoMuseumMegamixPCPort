#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g01
{

// Galaga stage select: the rule (how to play) button (layout ANT_MENU, crc 0xd3a909fe). vtable 0x80437d18.
// Created by the console manager ctor (0x800bc114) as Rule(man).
class cTaskConsMenuRule : public cTaskConsMenuBase
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

	static constexpr int HIT_PAT = 10;              // IsDpdHit 0x800bf114: li r5, 0xa

	static const int ANM_LIST[eANM_MAX];            // DOL 0x80370400

private:
	int m_n1d8{};   // +0x1d8: 0 in the ctor, never read by this class (UNVERIFIED meaning)

public:
	cTaskConsMenuRule(cTaskConsMenuManBase* const& pConsMan);   // DOL 0x800bef54

	virtual ~cTaskConsMenuRule();   // DOL 0x800bf004 (thunk 0x800bf380)

	bool IsDpdHit(float x, float y) override;   // DOL 0x800bf05c

protected:
	void WaitExec() override;     // DOL 0x800bf194

	bool AppearExec() override;   // DOL 0x800bf1e0

	void PointExec() override;    // DOL 0x800bf294

	bool DecideExec() override;   // DOL 0x800bf2e0
};

}
