#include "kingdom/game/g03/cTaskCharGhost.hpp"

namespace kingdom::game::g03
{

const Vector4 cTaskCharGhost::CHOCO_POS[2] = {Vector4(-13.8f, 2.5f, -131.398f), Vector4(-26.923f, 1.3824f, -306.0363f)};

void cTaskCharGhost::Revive()
{
	m_bRevival = true;
}

int& cTaskCharGhost::nStat()
{
	return m_nStatus;
}

int& cTaskCharGhost::nOptionItem()
{
	return m_nOptionItem;
}

bool& cTaskCharGhost::bRevival()
{
	return m_bRevival;
}

Vector4& cTaskCharGhost::vStartPos()
{
	return m_vStartPos;
}

bool cTaskCharGhost::IsDead()
{
	return false;
}

cTaskCharGhost::cTaskCharGhost()
{
	Init();
}

void cTaskCharGhost::Init()
{
	m_nStatus = 0;
	m_nOptionItem = 0;
	m_bRevival = false;
	m_vStartPos = Vector4();
	m_bSndEngineStop = false;
}

void cTaskCharGhost::Term()
{
	cTaskCharEnemy::Term();
}

}
