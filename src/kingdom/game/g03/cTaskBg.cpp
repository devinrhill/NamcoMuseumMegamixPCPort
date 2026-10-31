#include "kingdom/game/g03/cTaskBg.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

// DOL 0x8014badc [audited: per AUDIT_G03_G07 row] (ctor 0x8014bc60: name "cTaskBg", scene matrix = translate * RotZ * RotY * RotX (each when non-zero), scale)
void cTaskBg::Initialize()
{
	SetName("cTaskBg");
	Init();
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, this->Model->vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, this->Model->vRotate());
	Matrix4x4 _discard1;
	::Constants::jetGeoMtx44to34(_discard1, m);
	DummyNW4R::SetMtx(this->Model->pScnMdl(), m);
	DummyNW4R::SetScale(this->Model->pScnMdl(), this->Model->vScale()[0], this->Model->vScale()[1], this->Model->vScale()[2]);
}

void cTaskBg::Init()
{
}

void cTaskBg::Term()
{
	cTaskBgBase::Term();
}

// DOL 0x8014bce4 [audited: per AUDIT_G03_G07 row] (model UpdateFrame unless paused / HOME held; reached through the +0xd8 sub-vtable thunk 0x8014bd70)
bool cTaskBg::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE())
	{
		this->Model->UpdateFrame();
	}
	return true;
}

void cTaskBg::Draw(int nStatus, cJetTask* pParam)
{
	if (this->Model->IsVisible())
	{
		PushBack();
	}
}

}
