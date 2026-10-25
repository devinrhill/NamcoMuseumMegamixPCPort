#include "kingdom/game/g01/cTaskBossPacman.hpp"

#include <cstdio>

#include "kingdom/game/g01/cTaskScene.hpp"

namespace kingdom::game::g01
{

// DOL 0x800d0cec: stage, segments (+0x468), scene / enemy manager / life (+0x3a0 / +0x3a4 / +0x3a8), collision registration, then vtable +0x78
// = 0x800d0bf8 (cDemoPacmanBoss for stage 10, registered by cTaskPacman's ctor) -> the course clip (+0xd4)
cTaskBossPacman::cTaskBossPacman(int nStage, cTaskScene* pScene) : cTaskPacman(nStage, pScene)
{
	SetName("g01::cTaskBossPacman");
	SetCourseClip();
}

// DOL 0x800d0f7c: the model again (+0xac), the orbit clip of the segment, CHAR_PACMAN_ANM_CHA_01 (0x0c454022) and CHAR_PACMAN_ANM_TPA_06, sync
void cTaskBossPacman::SetCourseClip()
{
	DeleteModel(0);
	LoadModel();
	cKingdomModel* orbit = pModel(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "m01m_s%x_pac_ob_cha.brres", m_nStage + m_nSeg);
	orbit->RegistAnm(0u, name);
	if (cKingdomModel* pac = orbit->pExpandMdl(0))
	{
		pac->RegistAnm(0u, "CHAR_PACMAN_ANM_CHA_01");
		pac->RegistAnm(0u, "CHAR_PACMAN_ANM_TPA_06");
	}
	SyncCourse();   // +0xd8
}

// DOL 0x800d0dd0: the scene started the next course segment (+700): the segment's orbit clip (+0x108 = 0x800d0c88), sync (+0xd8)
void cTaskBossPacman::PreGameStep()
{
	if (m_pScene == nullptr || !m_pScene->IsSegmentChanged()) return;
	m_nSeg++;
	char name[0x80];
	std::snprintf(name, sizeof name, "m01m_s%x_pac_ob_cha.brres", m_nStage + m_nSeg);
	pModel(0)->RegistAnm(0u, name);
	SyncCourse();
}


void cTaskBossPacman::HitEnd()
{
	cKingdomModel* orbit = pModel(0);
	if (cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr)
	{
		pac->ReleaseAnmVis(0u);   // FUN_8001ad60
		pac->RegistAnm(0u, "CHAR_PACMAN_ANM_TPA_06");
	}
}

}
