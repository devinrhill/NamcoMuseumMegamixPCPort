#pragma once

#include "common.hpp"
#include "kingdom/cKingdomAnita.hpp"

namespace kingdom
{

class cKingdomAnitaVector
{
private:
	std::vector<cKingdomAnita*> m_vecAnita;

	bool m_bDrawReverse{};

	void Init();

	void Term();

public:
	cKingdomAnitaVector();

	virtual ~cKingdomAnitaVector();

	cKingdomAnita* pAnita(uint n);

	int nAnita();

	bool& bDrawReverse();

	bool SetState(uint flag, bool bit = true);

	void UpdateFrame();

	void Draw();
};

}
