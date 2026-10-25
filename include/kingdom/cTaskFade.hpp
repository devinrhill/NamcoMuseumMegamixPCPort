#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/EFadeKind.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomFade.hpp"

namespace kingdom
{

class cTaskFade : public cTaskKingdom
{
private:
	cKingdomFade* fade{};

public:
	cTaskFade();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool Fade(EFadeKind type, int msec = 500);

	bool Fade(uint type, int msec = 500);

	bool IsEnd();

	void UpdateFrame();
};

}
