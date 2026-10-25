#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::menu
{

class cTaskConsBase : public cTaskAnita
{
private:
	int m_nId{};

	int m_nState{};

	int m_nStep{};

protected:
	int& nId();

	int& nState();

	int& nStep();

public:
	cTaskConsBase(int nId = -1);

	virtual ~cTaskConsBase();

	virtual bool IsDpdHit(float x, float y);

	int IsDpdHitId(float x, float y);

	int nGetId();
};

}
