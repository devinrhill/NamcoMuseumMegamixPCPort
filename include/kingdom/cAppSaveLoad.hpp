#pragma once

#include "common.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/cKingdomSave.hpp"
#include "kingdom/MsgTable.hpp"
#include "kingdom/cTaskFade.hpp"

namespace kingdom
{

// Jobs 133 / 134 (0x08500000 / 0x08600000): the save-data / transition app. Ported from main.dol:
//   RTTI 0x80476720, vtable 0x80389338, ctor inlined in the job inits 0x8000e420 / 0x8000e5dc calling the setup FUN_80006fa0,
//   Execute 0x8000786c (0x1b34 bytes, jump table 0x803892a0), Draw 0x8000970c, dtor 0x80009954.
// Not in the Unity port. Job 133 (fade) is created with force = true, job 134 with force = false (`stb ... 0x760`).
//
// What it does: checks / loads / saves / creates the save data through cKingdomSave while showing messages on a framed
// dialog (layers 0-3 of RESIDENT_ANITA_FRAME), and finally jumps to the job stored in cKingdomSys + 0xa88.
// States: 0 start, 1 fade in, 2 check, 3 load, 4-5 save, 6-7 create (space check), 8 "delete broken data?", 9/10 not enough
// room, 11 "continue without saving?", 12 created, 13 deleted, 14 "continue without saving" screen, 15-18 fatal NAND errors,
// 19 fade out, 20 jump to the stored job.
class cAppSaveLoad : public cAppKingdom
{
public:
	explicit cAppSaveLoad(bool force);

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void Init();                              // DOL 0x80006fa0
	void ShowMessage(int index);              // DOL 0x800097ec: text for message `index` on layer 3
	bool RouteError(uint status);             // DOL 0x80009414: save status -> next state; false = retry the operation
	void MenuInput();                         // DOL 0x80009598: up/down cursor, decide

	cKingdomAnitaVector m_Anita;              // +0x280, layers 0-3 = RESIDENT_ANITA_FRAME
	cTaskFade* m_pFade{};                     // +0x348
	uint m_nErrState{0x15};                   // +0x34c: state to return to after the "continue without saving" screen (leave steps of 8 / 9 / 10 / 11 / 14 set it)
	uint m_nRetry{};                          // +0x350
	uint m_nChoice{};                         // +0x354: 0 = none yet, 1 / 2 = first / second option
	uint m_nCursor{};                         // +0x358: 0 = none, 1 = up (first option), 2 = down (second option)
	bool m_bForce{};                          // +0x760: a save is required (job 133)
	std::string m_msgText;                    // text of the current message (table row index + 1, English)
	float m_msgX{}, m_msgY{};                 // centre of the text on the 640 x 480 screen (y down)
	bool m_msgPos{};
	MsgTable m_msgs;                          // RESIDENT_MSG_NAND_ERROR
	bool m_msgsLoaded{};
	bool m_bOk{};                             // +0x761: result of the last operation
};

}
