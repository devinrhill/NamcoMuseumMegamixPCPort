#pragma once

#include "common.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/menu/TitleScene.hpp"

namespace kingdom::menu
{

// Job 137 (0x08900000): the title screen. Ported from main.dol:
//   RTTI 0x80478910, vtable 0x8044fcc0, ctor inlined in the job init 0x8019a98c, setup 0x80202f48,
//   Execute 0x802039b4, Draw 0x80203ee4, anim helper 0x80203fb8, dtor 0x80204220.
// Not in the Unity port. Member offsets are the DOL's, kept as comments.
//
// The original scene children (cTaskBg, cTaskPacman, cTaskTape, cTaskEffect, cTaskCamera, ...) are 3D g3d
// models; here they exist as named placeholder tasks so the task tree matches, and render nothing yet.
class cAppTitle : public cAppKingdom
{
public:
	cAppTitle();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void Init();                  // DOL 0x80202f48
	void SetTitleAnim(int mode);  // DOL 0x80203fb8

	cKingdomAnitaVector m_Anita;          // +0x280 (layers 2 and 3 hold the title layout)
	cTaskFade* m_pFade{};                 // +0x294
	cJetTask* m_pBg{};                    // +0x298  kingdom::menu::cTaskBg
	cJetTask* m_pPacman{};                // +0x29c  kingdom::menu::cTaskPacman
	bool m_bAttract{};                    // +0x2a0  chooses the exit job (set nowhere in Execute)
	WiiSnd::Handle m_Snd;                 // +0x2a4
	TitleScene m_Scene;                   // the cTaskBg / cTaskPacman / cTaskTape / cTaskCamera models (PC port: one object)
};

}
