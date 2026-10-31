#include "kingdom/game/g03/cTaskCharEnemy.hpp"

#include "cJetTask.hpp"

namespace kingdom::game::g03
{

// DOL 0x8015b2a0 [audited: described by the 43rd-session pass] (name "cTaskCharEnemy", +0x1e0 push-out level = 1)
void cTaskCharEnemy::Init()
{
	SetName("cTaskCharEnemy");
	nPushOutLevel() = 1u;
}

// DOL 0x8015b368 [audited: blr] (blr)
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

// DOL 0x8015b36c [audited: described by the 43rd-session pass] (return 1); GetAnmState 0x8015b248 returns 0, SetModel 0x8015b378 / IjikeStart 0x8015b244 are blr
bool cTaskCharEnemy::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskCharEnemy::Draw(int nStatus, cJetTask* pParam)
{
}

}
