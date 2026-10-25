#pragma once

#include "kingdom/game/g01/cTaskPacman.hpp"

// Galaga Remix (g01) stage 10 Pac-Man: kingdom::game::g01::cTaskBossPacman (vtable 0x80438e80, 0x470 bytes; setup 0x800d0cec). No start clip
// (vtable +0xd0 = 0x800d1270 does nothing: the boss app starts the course at once); the course clip is m01m_s%x_pac_ob_cha of the current
// course segment (x = stage + segment, 0x800d0c88), with the run clip and the face pattern CHAR_PACMAN_ANM_TPA_06 (0x800d0f7c). The boss
// goal demo (cDemoPacmanBoss: the a50 clips 0x800d1054) runs on cTaskPacman's state 1.
namespace kingdom::game::g01
{
class cTaskBossPacman : public cTaskPacman
{
public:
	cTaskBossPacman(int nStage, cTaskScene* pScene);   // DOL 0x800d0cec (vtable +0x78 = 0x800d0bf8)

protected:
	void SetCourseClip() override;   // DOL 0x800d0f7c (vtable +0xd4)
	void HitEnd() override;          // DOL 0x800d0f20: vis released, CHAR_PACMAN_ANM_TPA_06 (0xdc623be1) again
	void PreGameStep() override;     // DOL 0x800d0dd0 (the segment part)

private:
	int m_nSeg{};                    // +0x464
};
}
