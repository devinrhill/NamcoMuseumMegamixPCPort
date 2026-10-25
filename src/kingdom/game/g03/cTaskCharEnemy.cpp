#include "kingdom/game/g03/cTaskCharEnemy.hpp"

#include "cJetTask.hpp"

namespace kingdom::game::g03
{

void cTaskCharEnemy::Init()
{
	SetName("cTaskCharEnemy");
	nPushOutLevel() = 1u;
}

void cTaskCharEnemy::Term()
{
	cTaskChar::Term();
}

void cTaskCharEnemy::SetModel()
{
}

cTaskCharEnemy::cTaskCharEnemy()
{
	Init();
}

bool cTaskCharEnemy::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskCharEnemy::Draw(int nStatus, cJetTask* pParam)
{
}

}
