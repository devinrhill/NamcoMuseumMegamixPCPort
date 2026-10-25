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
};

}
