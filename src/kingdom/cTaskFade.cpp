#include "kingdom/cTaskFade.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomFade.hpp"

namespace kingdom
{

cTaskFade::cTaskFade()
	: fade(new cKingdomFade())
{
	nDrawPriority(2415919103u);
	SetName("cTaskFade");
}

bool cTaskFade::Execute(cJetTask* pParam)
{
	if (!fade->IsEnd())
	{
		fade->UpdateFrame();
	}
	return true;
}

void cTaskFade::Draw(int nStatus, cJetTask* pParam)
{
	fade->Draw();
}

bool cTaskFade::Fade(EFadeKind type, int msec)
{
	return Fade((uint)type, msec);
}

bool cTaskFade::Fade(uint type, int msec)
{
	return fade->Fade(type, msec);
}

bool cTaskFade::IsEnd()
{
	return fade->IsEnd();
}

void cTaskFade::UpdateFrame()
{
	fade->UpdateFrame();
}

}
