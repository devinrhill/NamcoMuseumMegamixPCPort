#include "kingdom/game/g03/cTaskStage.hpp"

#include "cJetTask.hpp"

namespace kingdom::game::g03
{

cTaskStage::cTaskStage()
{
	SetName("cTaskStage");
	Init();
}

cTaskStage::~cTaskStage()
{
	Term();
}

void cTaskStage::Init()
{
}

void cTaskStage::Term()
{
}

// DOL 0x801888f0 [audited: constant getter vs disassembly] (li r3, 1; blr)
bool cTaskStage::Execute(cJetTask* pParam)
{
	return true;
}

// DOL 0x801888f8 [audited: described by the 43rd-session pass] (blr); dtor 0x80188864
void cTaskStage::Draw(int nStatus, cJetTask* pParam)
{
}

}
