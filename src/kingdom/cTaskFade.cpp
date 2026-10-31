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

// DOL 0x80009e5c [audited: the scene root's screen capture flag (cleared by Fade 0x800093a0, set here at the end) is not ported]
bool cTaskFade::Execute(cJetTask* pParam)
{
	if (!fade->IsEnd())
	{
		fade->UpdateFrame();
	}
	return true;
}

// DOL 0x80009e0c [audited: FUN_80016404 hands the scene root's screen capture to the clip as texture 1, not ported: the G01.. fades may use it]
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
