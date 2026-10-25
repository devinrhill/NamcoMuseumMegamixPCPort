#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/cTaskFade.hpp"

namespace kingdom::game
{

class cAppGameBaseGameOver : public cAppKingdom
{
private:
	enum EMode
	{
		eMODE_INIT = 0,
		eMODE_EXEC = 1,
		eMODE_TERM = 2
	};

	uint m_nAnita{};

	uint m_nNextJob{};

	cTaskFade* m_pTaskFade{};

public:
	cKingdomAnitaVector* AnitaVector{};

	cKingdomAnita* pAnita(uint n);

	cAppGameBaseGameOver();

	void Free() override;

	uint& nNextJob();

	uint& nAnita();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
