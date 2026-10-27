#pragma once

#include "common.hpp"
#include "cSysTaskUser.hpp"

namespace kingdom
{

class cTaskKingdom : public cSysTaskUser
{
public:
	class sFlow
	{
	public:
		uint m_nModeState{};

		uint m_nNewState{};

		uint m_nOldState{};

		uint m_nModeStep{};

		uint m_nModeProc[8]{};
	};

private:
	cTaskKingdom::sFlow* m_Flow[8]{};

	bool m_bTermDone{};

	void Init();

	void Term();

public:
	cTaskKingdom();

	cTaskKingdom::sFlow*& pFlow(uint n);

	uint& nModeState(uint n = 0u);

	uint& nOldState(uint n);

	uint& nNewState(uint n);

	uint& nModeStep(uint n);

	uint& nModeProc(uint n, int work);

	void NextMode(uint n, uint mode);

	void nDrawPriority(uint n) override;

	uint nDrawPriority() override;

	// The DOL destroys a released task (cJetNode::Release 0x8008ff14 -> vtable +0x10, default 0x80009dc8 = delete this): the destructor
	// chain runs every class's Term, most derived first. The port never deletes tasks: DeleteExecute runs TermOnce instead, and every task
	// class with a Term overrides ReleaseTerm as `X::Term(); Base::ReleaseTerm();` (the dtor chain). Code that terminated a task by hand
	// (the C# stand-in for `delete`) calls TermOnce, so a released task never runs its Term twice. cAppKingdom keeps its own DeleteExecute.
	void TermOnce();

protected:
	virtual void ReleaseTerm() {}

	void DeleteExecute() override;
};

}
