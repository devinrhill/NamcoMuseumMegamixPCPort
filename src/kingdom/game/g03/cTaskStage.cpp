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

bool cTaskStage::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskStage::Draw(int nStatus, cJetTask* pParam)
{
}

}
