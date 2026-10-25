#pragma once

#include <vector>

#include "kingdom/game/g01/cTaskScene.hpp"

// Galaga Remix (g01) stage 10 scene: kingdom::game::g01::cTaskBossScene (vtable 0x80439040, 0x2c0 bytes; setup 0x800d1638). The course is a
// list of segments (stage 10 has two: "a" and "b": scn_s%xp%d.scn / m01s_s%x_cam_01_sca with x = stage + segment): when one ends the next
// starts (0x800d16a0, the frame overflow carried, +700 for one frame); after the last it restarts (0x800d1644, +0x2bd for one frame), so the
// course never ends by itself (vtable +0x48 = 0x800d1630 returns 0): the stage ends with the boss. No start camera (0x800d1a58).
namespace kingdom::game::g01
{
class cTaskBossScene : public cTaskScene
{
public:
	cTaskBossScene(int nStage, int nSegments, float fSpeed);   // DOL 0x800d1638 + 0x800f6200 (via vtable +0x70 / +0x8c / +0x80)

protected:
	void GameStep() override;          // DOL 0x800d1918 (vtable +0x74)

private:
	void SetCourseCamera();            // DOL 0x800d1a5c (vtable +0x80)
	void LastSegmentEnd();             // DOL 0x800d1644 (vtable +0x98)
	void NextSegment(int nSeg);        // DOL 0x800d16a0 (vtable +0x9c)
	int m_nSegments{};                 // +0x2b0
	std::vector<cScnData> m_SegScn;    // +0x2a8 (0x18 each)
	float m_fPrevFrame{}, m_fLastFrame{};   // +0x2b4 / +0x2b8
};
}
