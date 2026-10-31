#include "kingdom/game/g01/cTaskBossScene.hpp"

#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "wii/Archive.hpp"

namespace kingdom::game::g01
{

// DOL 0x800d1638 [audited]: the segment count (+0x2b0), then the base setup 0x800f6200 with the boss overrides: scn files (+0x70 = 0x800d173c:
// cDemoSceneBase::LoadScnData per segment), lights, course "a", ResetCourse, demo object (+0x8c = 0x800d1888: cDemoSceneBoss for stage 10,
// registered with the demo manager; its goal part is cTaskScene's state 1 with cam_50) and the course camera (+0x80)
cTaskBossScene::cTaskBossScene(int nStage, int nSegments, float fSpeed) : cTaskScene(nStage, fSpeed, NoSetup{})
{
	SetName("cTaskBossScene");
	m_nSegments = nSegments < 1 ? 1 : nSegments;
	m_SegScn.resize((std::size_t)m_nSegments);
	for (int i = 0; i < m_nSegments; i++)
	{
		char name[0x80];
		std::snprintf(name, sizeof name, "scn_s%xp%d.scn", nStage + i, KingdomGlobal::GalagaSys->nPlayer());
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(name, size);
		if (!m_SegScn[(std::size_t)i].SetData(d, size)) std::fprintf(stderr, "g01::cTaskBossScene: %s not found\n", name);
	}
	m_Scn = m_SegScn[0];   // +0x234 = the first segment's data
	if (nStage == 10)
		if (cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan()) dm->Regist(this);   // 0x800d1888
	SetCourseCamera();
	m_nState = 2;
	m_nStep = 0;
	ApplyCamera();
	ApplyLights();
}

// DOL 0x800d1a5c [audited]: lights, course of the stage (segment "a"), ResetCourse
void cTaskBossScene::SetCourseCamera()
{
	LoadLights();
	LoadCourse(m_nStage);
	ResetCourse();
}

// DOL 0x800d1644 [audited]: the last segment again from its start, the frame overflow kept, +0x2bd
void cTaskBossScene::LastSegmentEnd()
{
	const float over = m_fLastFrame - m_fLength;
	ResetCourse();
	m_fFrame = over;
	m_bLooped = true;
	if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 boss scene: segment %d loops (overflow %.1f)\n", m_nSeg, over);
}

// DOL 0x800d16a0 [audited]: lights, the course of stage + segment, +700, the segment's scn data, ResetCourse, the frame overflow
void cTaskBossScene::NextSegment(int nSeg)
{
	const float over = m_fLastFrame - m_fLength;
	LoadLights();
	LoadCourse(m_nStage + nSeg);
	m_bSegChanged = true;
	m_Scn = m_SegScn[(std::size_t)nSeg];
	ResetCourse();
	m_fFrame = over;
	if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 boss scene: segment %d starts (length %.1f)\n", nSeg, m_fLength);
}

// The other overrides: +0x78 0x800d19fc (scene state 7: the current camera from frame 0 at rate 1, then it runs) is reached only from
// cTaskScene::GoalStart 0x800f6ad0, which only Pac-Man's goal run calls (stage 10 has none); +0x84 0x800d1aa8 (cam_10 / lig_10 of stage 1) is
// only called by cDemoSceneW1S1's Init (stage 10 has cDemoSceneBoss). Both are unreachable on stage 10.
// DOL 0x800d1918 [audited: the +0x1fa / +0x1f9 flags are not kept, cTaskPacman pins itself to the scene every tick]
void cTaskBossScene::GameStep()
{
	m_bSegChanged = false;
	m_bLooped = false;
	m_fPrevFrame = m_fFrame;
	CourseStep();   // vtable +0x4c
	UpdateFrame();
	m_fLastFrame = m_fFrame;
	if (m_fLength < m_fLastFrame) m_bEnd = true;
	if (m_bEnd)
	{
		if (m_nSegments - 1 == m_nSeg) LastSegmentEnd();   // +0x98
		else
		{
			m_nSeg++;
			NextSegment(m_nSeg);   // +0x9c
		}
		m_bEnd = false;
	}
}

}
