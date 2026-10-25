#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Galaga (g01) stage select: RETURN button console (DOL vtable 0x80437810). Ported from main.dol only.
namespace kingdom::game::g01
{

class cTaskConsMenuBack : public cTaskConsMenuBase
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

	// DOL 0x8036fd58
	static const int s_AnmTbl[eANM_MAX];

	// hit pattern, immediate in IsDpdHit (DOL 0x800bae1c li r5, 6)
	static constexpr int HIT_PAT = 6;

private:
	int m_n1d8{};       // +0x1d8 = 0 in the ctor, not read by this class. UNVERIFIED use

public:
	// DOL 0x800bac5c
	cTaskConsMenuBack(cTaskConsMenuManBase* const& pConsMan);

	// DOL 0x800bad0c (thunk 0x800bb088)
	virtual ~cTaskConsMenuBack();

	// DOL 0x800bad64
	bool IsDpdHit(float x, float y) override;

protected:
	// DOL 0x800bae9c
	void WaitExec() override;

	// DOL 0x800baee8
	bool AppearExec() override;

	// DOL 0x800baf9c
	void PointExec() override;

	// DOL 0x800bafe8
	bool DecideExec() override;
};

}
