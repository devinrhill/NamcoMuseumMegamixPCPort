#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g01
{

// Galaga stage select: the stage map picture (layout ANT_STAGE, crc 0xcabf0277). vtable 0x80437b70 (2D only, no model).
// Created by the console manager ctor (0x800bc114) as Map(man, world man+0x218, stage man+0x210).
class cTaskConsMenuMap : public cTaskConsMenuBase
{
protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_DELETE = 4,
		eANM_MAX = 5
	};

	static constexpr int HIT_MAP_PAT = 1;           // IsDpdHit 0x800be5b8: li r5, 1

	static const int ANM_LIST[3][3][eANM_MAX];      // DOL 0x803702d8 (45 words, [world][stage][anm] = 0..44)

private:
	uint m_nWorldNo{};   // +0x1d8
	uint m_nStageNo{};   // +0x1dc

protected:
	uint& nWorldNo();

	uint& nStageNo();

public:
	cTaskConsMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo);   // DOL 0x800be39c

	virtual ~cTaskConsMenuMap();   // DOL 0x800be950 (thunk 0x800be9a8)

	virtual void ChangeData(uint nWorldNo, uint nStageNo);   // DOL 0x800bd978 (vtable +0x98)

protected:
	virtual int nGetMapAnm(uint nState);   // DOL 0x800be49c (vtable +0x94)

public:
	bool IsDpdHit(float x, float y) override;   // DOL 0x800be500

protected:
	void WaitExec() override;     // DOL 0x800be638

	bool AppearExec() override;   // DOL 0x800be69c

	void PointExec() override;    // DOL 0x800be764

	bool DecideExec() override;   // DOL 0x800be7c8

	bool DeleteExec() override;   // DOL 0x800be874
};

}
