#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

class GameObject;

namespace kingdom::game::g03
{

// DOL vtable 0x80437690, KingSys +0x60 (created in cAppG03PacnRollGame::Init): the list of the sounds started on a cPacnRollLoopSndHnd (vtable 0x804376f0). Natively
// there is no handle class: a looping cue's start site asks CanPlay() and passes the started sound to Track() (= 0x800ba1f0, the handle's Play). A list entry
// is the SE bank handle (mixer voice id + 1).
class cPacnRollLoopSndMan : public cTaskKingdom
{
public:
	cPacnRollLoopSndMan();
	virtual ~cPacnRollLoopSndMan();

	// DOL 0x800ba1f0: false after StopAll (no new loops until the next Init); true when no game runs (the hub's Pac-Man)
	static bool CanPlay();
	static GameObject* Track(GameObject* handle);
	static int Track(int voice);
	static void PauseAll();     // DOL 0x800ba3f0
	static void StopAll();      // DOL 0x800ba454

	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void Term();

	static cPacnRollLoopSndMan* s_pInstance;
	std::vector<GameObject*> m_list;   // +0x1b4
	bool m_bPause{};                   // +0x1c0
	bool m_bStop{};                    // +0x1c1

protected:
	void ReleaseTerm() override { cPacnRollLoopSndMan::Term(); cTaskKingdom::ReleaseTerm(); }
};

}
