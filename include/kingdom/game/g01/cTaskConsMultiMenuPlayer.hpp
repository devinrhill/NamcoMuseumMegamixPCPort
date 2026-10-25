#pragma once

#include "common.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Galaga Remix (g01) multiplayer stage select: one panel per player (name, Mii face, the player's best score of the chosen stage).
// kingdom::game::g01::cTaskConsMultiMenuPlayer, DOL vtable 0x80438070, 0x210 bytes, derived from kingdom::game::cTaskConsMenuBase.
// Built by the console manager ctor 0x800bc114 (multiplayer only): new cTaskConsMultiMenuPlayer(man, player, GalagaSys->pName(player)),
// then SetRecord(world man +0x218, man +0x210).
// Vtable: Execute 0x800c1368 (slot 6), Draw = cTaskAnita::Draw 0x80039bfc, WaitExec 0x800c0d84 (slot 18), AppearExec 0x800c0e18 (19),
// PointExec / DecideExec = base 0x801456d0 / 0x801456d4, DeleteExec 0x800c1268 (22).
namespace kingdom::game::g01
{

class cTaskConsMultiMenuPlayer : public cTaskConsMenuBase
{
public:
	cTaskConsMultiMenuPlayer(cTaskConsMenuManBase* const& pConsMan, int nPlayer, char* pName);   // DOL 0x800c0044

	virtual ~cTaskConsMultiMenuPlayer();   // DOL 0x800c13dc

	// DOL 0x800c0b88: the record of (world, stage) when it changed: score = rec[10 + player], mark = rec[0] bit player
	void SetRecord(int nWorld, int nStage);

	bool Execute(cJetTask* pParam) override;   // DOL 0x800c1368
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskAnita::Draw 0x80039bfc, then the cTaskFontWriter child's queue (the name)

protected:
	void WaitExec() override;     // DOL 0x800c0d84
	bool AppearExec() override;   // DOL 0x800c0e18
	bool DeleteExec() override;   // DOL 0x800c1268

private:
	// DOL 0x800c0c14: the six score digits (anitas 4..9 = 10^0 .. 10^5) in anim kind nKind (0 appear, 1 wait, 2 delete)
	void SetScoreAnm(int nKind);

	char* m_pName{};           // +0x1d8 GalagaSys->pName(player): text of the name; the sGalagaPlayer (+0x44 / +0x48 = face icon arguments)
	int m_nPlayer{};           // +0x1dc
	int m_nScore{-1};          // +0x1e0 shown score (record word 10 + player)
	// +0x1e4 cTaskFaceIcon* (child task, priority 0x80000003): see NEEDS in the .cpp. Its icon 0 state the DOL writes:
	bool m_bFaceVisible{};     //   icon(0) +0x7c
	Vector4 m_vFacePos{};      //   icon(0) +0x40: anita 11 layer 9 (pattern 2) + panel origin
	Vector4 m_vFaceScale{};    //   icon(0) +0x60..+0x6c: |(layer 11 x - layer 10 x) / 128| for x and y, 0, w 1
	// +0x1e8 cTaskFontWriter* (child task, priority 0x80000002, buffer 0x400 x 0x200, font KingSys +0xa68, colour -1, size 18)
	bool m_bFontVisible{};     //   writer +0x268
	Vector4 m_vFontPos{};      // +0x1f0 name position: layout (y up) + (320, -240) -> Execute prints at ((int)x, (int)-y) = top-left screen pixels
	bool m_bMark{};            // +0x200 record bit of this player (anita 3 shown)
	int m_nWorld{-1};          // +0x204
	int m_nStage{-1};          // +0x208
};

}
