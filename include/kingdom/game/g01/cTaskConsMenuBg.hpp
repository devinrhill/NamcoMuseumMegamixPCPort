#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Galaga (g01) stage select: background console (DOL vtable 0x804379b0). Ported from main.dol only.
namespace kingdom::game::g01
{

class cTaskConsMenuBg : public cTaskConsMenuBase
{
protected:
	// DOL 0x80370120
	static const int s_AnmTbl[3];

private:
	int m_nWorldNo{};   // +0x1d8 (ctor arg; the manager passes its world, man+0x218)

public:
	// DOL 0x800bbf38
	cTaskConsMenuBg(cTaskConsMenuManBase* const& pConsMan, int nWorldNo);

	// DOL 0x800bc010 (thunk 0x800bc10c)
	virtual ~cTaskConsMenuBg();

	// the manager writes +0x1d8 directly when the world changes (cTaskConsMenuMan::Execute states 7 / 8)
	int& nWorldNo() { return m_nWorldNo; }

protected:
	// DOL 0x800bc068 (new vtable slot 37, +0x94)
	virtual int nGetBgAnmNo(int nAnmState);

	// DOL 0x800bc0a8
	void WaitExec() override;
};

}
