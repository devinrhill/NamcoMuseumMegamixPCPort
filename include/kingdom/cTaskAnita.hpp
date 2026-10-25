#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"

namespace kingdom
{

class cTaskAnita : public cTaskKingdom
{
public:
	cKingdomAnitaVector* AnitaVector{};

	cTaskAnita();

	virtual ~cTaskAnita();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	cKingdomAnita* pAnita(uint n);

	int nAnita();

	bool& bDrawReverse();

	bool SetState(uint flag, bool bit = true);
};

}
