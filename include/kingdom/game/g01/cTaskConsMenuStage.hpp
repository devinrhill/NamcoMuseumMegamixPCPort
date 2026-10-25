#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g01
{

// Galaga stage select: one of the three stage buttons of a world (layout ANT_MENU, crc 0xd3a909fe). vtable 0x80437de8.
// Created by the console manager ctor (0x800bc114) as Stage(man, world man+0x218, index 0..2, bMulti man+0x20c),
// followed by LoadStageData(world) (0x800bf520).
class cTaskConsMenuStage : public cTaskConsMenuBase
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

	static constexpr int HIT_PAT = 11;              // IsDpdHit 0x800bf57c: pattern 0xb
	static constexpr int SET_STAGE_PAT = 1;         // ctor 0x800bf388: layer position of pattern 1, layer = stage index

	static const int ANM_LIST[2][3][3][eANM_MAX];   // DOL 0x80370438 (90 words): [!bActive][world][stage][anm], -1 = none

private:
	uint m_nWorldNo{};    // +0x1d8
	uint m_nStageNo{};    // +0x1dc  index in the world 0..2
	bool m_bActive{};     // +0x1e0  byte: stage record bOpen == 1
	uint m_bMulti{};      // +0x1e4  ctor argument, not read by the functions of this class (UNVERIFIED use)
	int m_n1e8{};         // +0x1e8  0 in the ctor, not read by the functions of this class (UNVERIFIED meaning)

protected:
	uint& nWorldNo();

	uint& nStageNo();

	bool& bActive();

	int nGetStageAnm(int nAnm);   // inlined table read (0x80370438) of every *Exec

public:
	cTaskConsMenuStage(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo, uint bMulti);   // DOL 0x800bf388

	// +0x1e0 read directly by cTaskConsMenuMan (KeySelect 0x800bdac8, Execute state 3): the stage can be chosen
	bool IsOpen() const { return m_bActive; }

	virtual ~cTaskConsMenuStage();   // DOL 0x800bf4c8 (thunk 0x800bfb94)

	void LoadStageData(uint nWorldNo);   // DOL 0x800bf520 (unnamed in the maps; name from the g07 equivalent)

	bool IsDpdHit(float x, float y) override;   // DOL 0x800bf57c

	void WaitState() override;    // DOL 0x800bfaf4

	void PointState() override;   // DOL 0x800bfb44

protected:
	void WaitExec() override;     // DOL 0x800bf6c8

	bool AppearExec() override;   // DOL 0x800bf76c

	void PointExec() override;    // DOL 0x800bf870

	bool DecideExec() override;   // DOL 0x800bf91c

	bool DeleteExec() override;   // DOL 0x800bfa08
};

}
