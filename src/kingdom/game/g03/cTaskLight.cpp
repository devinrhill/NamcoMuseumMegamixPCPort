#include "kingdom/game/g03/cTaskLight.hpp"

#include "cJetTask.hpp"

namespace kingdom::game::g03
{

cTaskLight::~cTaskLight()
{
	Term();
}

void cTaskLight::Init()
{
	m_nType = 0u;
}

void cTaskLight::Term()
{
}

bool cTaskLight::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskLight::Draw(int nStatus, cJetTask* pParam)
{
}

}
