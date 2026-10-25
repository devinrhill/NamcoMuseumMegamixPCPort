#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) camera helper: kingdom::game::g01::cTaskCamera (0x270 bytes, vtable 0x80439588; KingdomGlobal::GalagaSys->pTaskCamera()).
// It does not move the camera (cTaskScene binds the course SCN0 to the g3d camera); every frame it reads camera 0 and keeps its frame:
// +0x210 position, +0x220 aim point, +0x230 right, +0x240 up, +0x250 forward. Enemy orbits live in that frame (LocalToWorld: origin at the aim
// point, local x to the camera's LEFT, y up, z forward). The sound listener part (PlayerParam.csv: INTERIOR_SIZE / DISTANCE / DISTANCE_MAX,
// 0x800d3064 / 0x800d3194) is not ported.
namespace kingdom::game::g01
{
class cTaskCamera : public cObject, public kingdom::cTaskKingdom
{
public:
	cTaskCamera();                             // DOL 0x800d3194 (Init after new 0x270)
	bool Execute(cJetTask* pParam) override;   // DOL 0x800d322c

	void Update();                             // the body of Execute (also called right after the scene moved the camera)
	void LocalToWorld(float out[3], const float local[3]) const;   // DOL 0x800d3520
	void LocalDirToWorld(float out[3], const float local[3]) const;   // DOL 0x800d3628
	void WorldToLocal(float out[3], const float world[3]) const;   // DOL 0x800d3714
	void WorldDirToLocal(float out[3], const float world[3]) const;   // DOL 0x800d37c8
	const float* vPos() const { return m_vPos; }
	const float* vAim() const { return m_vAim; }

private:
	float m_vPos[3]{};       // +0x210
	float m_vAim[3]{};       // +0x220
	float m_vRight[3]{1, 0, 0};    // +0x230
	float m_vUp[3]{0, 1, 0};       // +0x240
	float m_vForward[3]{0, 0, -1}; // +0x250
};
}
