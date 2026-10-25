#include "kingdom/game/g11/cTaskGrobdaModel.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g11
{

namespace
{
float s_Dummy[4] = {};   // a model that does not exist yet
}

// DOL 0x80036a90
cTaskGrobdaModel::cTaskGrobdaModel()
{
	SetName("cTaskGrobdaModel");
}

float* cTaskGrobdaModel::GetPos()
{
	cKingdomModel* m = pModel((uint)m_nModelIdx);
	return m != nullptr ? &m->vTrans().x : s_Dummy;
}

float* cTaskGrobdaModel::GetRot()
{
	cKingdomModel* m = pModel((uint)m_nModelIdx);
	return m != nullptr ? &m->vRotate().x : s_Dummy;
}

void cTaskGrobdaModel::SetPos(float x, float y, float z)
{
	float* p = GetPos();
	p[0] = x;
	p[1] = y;
	p[2] = z;
}

// DOL 0x80036b00: FUN_80193b34 (the timers freeze while the app is paused or HOME is held), then cTaskModel::Execute
bool cTaskGrobdaModel::Execute(cJetTask* pParam)
{
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys->pAppMain();
	UpdateTimers((app != nullptr && app->IsPause()) || (GamePad::Get(0).hold & 0x400u) != 0);
	return cTaskModel::Execute(pParam);
}

}
