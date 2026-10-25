#include "kingdom/game/g03/cTaskMenuPacman.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g03
{

namespace
{
constexpr float PI = 3.14159265f;

bool LoadBrres(const char* name, wii::Brres& b)
{
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(name, size);
	if (d == nullptr || !b.Load(d, size))
	{
		std::fprintf(stderr, "cTaskMenuPacman: cannot load %s\n", name);
		return false;
	}
	return true;
}

// column-major 4x4 multiply: out = a * b
void Mul(const float* a, const float* b, float* out)
{
	float r[16];
	for (int c = 0; c < 4; c++)
		for (int row = 0; row < 4; row++)
		{
			float s = 0;
			for (int k = 0; k < 4; k++) s += a[k * 4 + row] * b[c * 4 + k];
			r[c * 4 + row] = s;
		}
	for (int i = 0; i < 16; i++) out[i] = r[i];
}
}

int& cTaskMenuPacman::nState()
{
	return m_nState;
}

int& cTaskMenuPacman::nStep()
{
	return m_nStep;
}

float& cTaskMenuPacman::fScale()
{
	return m_fScale;
}

cTaskMenuPacman::cTaskMenuPacman()
{
	nDrawPriority(65541u);
	nState() = 0;
	nStep() = 0;
	fScale() = 0.0f;
}

void cTaskMenuPacman::Term()
{
}

void cTaskMenuPacman::AppearState()
{
	nState() = 1;
	nStep() = 0;
}

bool cTaskMenuPacman::IsAppearEndState()
{
	return nState() == 2;
}

void cTaskMenuPacman::WaitState()
{
	nState() = 0;
	nStep() = 0;
}

void cTaskMenuPacman::RegistData()
{
	m_bLoaded = false;
	wii::Brres set, mdl, scn, cha;
	if (!LoadBrres("BIN_MENU_SET_MDL", set) || set.ModelNames().empty()) return;
	wii::Mdl0 placement;
	if (!placement.Load(set, set.ModelNames().front())) return;
	bool found = false;
	for (const auto& n : placement.nodes)
	{
		if (n.name != "set_pac") continue;
		for (int i = 0; i < 3; i++) m_nodePos[i] = n.translate[i];
		m_nodeRotY = n.rotate[1];
		m_nodeScale = n.scale[0];
		found = true;
	}
	if (!found) return;
	if (!LoadBrres("BIN_PAC_NORMAL_MDL", mdl) || mdl.ModelNames().empty() || !m_pac.Load(mdl, mdl.ModelNames().front())) return;
	if (!LoadBrres("BIN_PAC_N_MOTION_01_CHA", cha) || !m_motion.Load(cha)) return;
	wii::ScnAnim sa;
	wii::LightResult light;
	if (!LoadBrres("BIN_MENU_SCA", scn) || !sa.Load(scn) || !sa.SampleCamera("", 0.0f, m_cam) || !sa.SampleLight(0, light)) return;
	m_pac.SetLight(light.pos, light.aim, light.color, light.specColor, light.shininess);
	m_fFrame = 0.0f;
	m_bLoaded = true;
}

bool cTaskMenuPacman::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	switch (nState())
	{
	case 1:
		if (nStep() == 0)
		{
			fScale() = 0.0f;
			nStep()++;
		}
		else if (nStep() == 1)
		{
			fScale() += SCALE_ADD_VAL;
			if (fScale() >= 1.0f)
			{
				fScale() = 1.0f;
				nState() = 2;
				nStep() = 0;
			}
		}
		break;
	}
	if (m_bLoaded) m_fFrame += 1.0f; // UpdateFrame: one clip frame per game frame (UNVERIFIED rate, as in the title scene)
	return true;
}

void cTaskMenuPacman::Draw(int nStatus, cJetTask* pParam)
{
	if (std::getenv("MEGAMIX_MENU_DEBUG")) std::fprintf(stderr, "menuPac draw loaded=%d state=%d scale=%.2f\n", (int)m_bLoaded, nState(), fScale());
	if (!m_bLoaded || fScale() <= 0.0f) return; // the state only drives the appear scale; the model stays once it has grown

	// rotate camera (SCN0 flag bit 0 clear): world = T * Rz * Ry * Rx, the view is its inverse
	const float rx = m_cam.aimPos[0] * PI / 180.0f, ry = m_cam.aimPos[1] * PI / 180.0f, rz = m_cam.aimPos[2] * PI / 180.0f;
	const float cx = std::cos(rx), sx = std::sin(rx), cy = std::cos(ry), sy = std::sin(ry), cz = std::cos(rz), sz = std::sin(rz);
	// columns of the camera's rotation R = Rz * Ry * Rx
	const float r[3][3] = {
		{cz * cy, sz * cy, -sy},
		{cz * sy * sx - sz * cx, sz * sy * sx + cz * cx, cy * sx},
		{cz * sy * cx + sz * sx, sz * sy * cx - cz * sx, cy * cx}};
	// view = R^T, then translate by -R^T * pos
	float view[16] = {r[0][0], r[1][0], r[2][0], 0, r[0][1], r[1][1], r[2][1], 0, r[0][2], r[1][2], r[2][2], 0, 0, 0, 0, 1};
	for (int row = 0; row < 3; row++)
		view[12 + row] = -(view[0 * 4 + row] * m_cam.pos[0] + view[1 * 4 + row] * m_cam.pos[1] + view[2 * 4 + row] * m_cam.pos[2]);

	const float t = 1.0f / std::tan(0.5f * m_cam.fovy * PI / 180.0f);
	const float zn = m_cam.nearZ, zf = m_cam.farZ, asp = 4.0f / 3.0f;
	const float proj[16] = {t / asp, 0, 0, 0, 0, t, 0, 0, 0, 0, (zf + zn) / (zn - zf), -1, 0, 0, 2 * zf * zn / (zn - zf), 0};

	m_pac.SetPose({{&m_motion, wii::ClipFrame(m_fFrame, m_motion.numFrame, m_motion.loops), 1.0f}});

	// model matrix: node translation, rotY, scale (node scale * appear scale)
	const float s = m_nodeScale * fScale();
	const float a = m_nodeRotY * PI / 180.0f;
	const float world[16] = {std::cos(a) * s, 0, -std::sin(a) * s, 0, 0, s, 0, 0, std::sin(a) * s, 0, std::cos(a) * s, 0,
	                         m_nodePos[0], m_nodePos[1], m_nodePos[2], 1};
	gfx::Begin3D(view, proj, true);
	m_pac.Draw(world);
	gfx::End3D();
}

}
