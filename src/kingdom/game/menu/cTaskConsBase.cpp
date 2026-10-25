#include "kingdom/game/menu/cTaskConsBase.hpp"

namespace kingdom::game::menu
{

int& cTaskConsBase::nId()
{
	return m_nId;
}

int& cTaskConsBase::nState()
{
	return m_nState;
}

int& cTaskConsBase::nStep()
{
	return m_nStep;
}

cTaskConsBase::cTaskConsBase(int nId)
{
	nState() = 0;
	nStep() = 0;
	this->nId() = nId;
}

cTaskConsBase::~cTaskConsBase()
{
}

bool cTaskConsBase::IsDpdHit(float x, float y)
{
	return false;
}

int cTaskConsBase::IsDpdHitId(float x, float y)
{
	if (IsDpdHit(x, y))
	{
		return nId();
	}
	return -1;
}

int cTaskConsBase::nGetId()
{
	return m_nId;
}

}
