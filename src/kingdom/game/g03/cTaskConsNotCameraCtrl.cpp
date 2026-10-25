#include "kingdom/game/g03/cTaskConsNotCameraCtrl.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"

namespace kingdom::game::g03
{

cTaskConsNotCameraCtrl::cTaskConsNotCameraCtrl()
{
	SetName("cTaskConsNotCameraCtrl");
	Init();
}

cTaskConsNotCameraCtrl::~cTaskConsNotCameraCtrl()
{
	Term();
}

void cTaskConsNotCameraCtrl::Init()
{
	RegistNotSetPos(0u, "BIN_ANT_CAMERA");
	SetState(0u);
	SetAnim(0u, 0);
}

void cTaskConsNotCameraCtrl::Term()
{
}

void cTaskConsNotCameraCtrl::Start()
{
	Visible(0u);
}

void cTaskConsNotCameraCtrl::End()
{
	Invisible(0u);
}

void cTaskConsNotCameraCtrl::Update(bool go)
{
	if (go)
	{
		Start();
	}
	else
	{
		End();
	}
}

bool cTaskConsNotCameraCtrl::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
