#include "kingdom/menu/TitleScene.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "wii/Archive.hpp"
#include "kingdom/NativeScene.hpp"
#include "wii/Brres.hpp"

namespace kingdom::menu
{

namespace
{
constexpr float kCameraFovDeg = 40.0f;      // TITLE_CSV_CAMERA "eCSV_FAR,40.0" -> cTaskCamera+0x1bc
constexpr float kCameraYOffset = 0.1f;      // FUN_8011fff8: position.y += 0.1
constexpr float kPacBlendStep = 0.05f;      // 0x80121280
constexpr float kPacIdleFrames = 60.0f;     // mode 0 waits until the counter reaches this (0x80120978 vs the constant at r2-0x5c84)

// DOL mode table (FUN_80121308 case bodies): TPA / CHA entry numbers (1 based) per mode
struct PacMode { int tpa; int cha; };
constexpr PacMode kPacModes[4] = {{1, 1}, {1, 4}, {5, 1}, {5, 7}};

const std::uint8_t* FindEntry(const std::string& name, std::size_t& size)
{
	return wii::ArchiveStore::Find(name, size);
}

bool LoadBrres(const std::string& name, wii::Brres& b)
{
	std::size_t size = 0;
	const std::uint8_t* d = FindEntry(name, size);
	if (d == nullptr || !b.Load(d, size))
	{
		std::fprintf(stderr, "TitleScene: cannot load %s\n", name.c_str());
		return false;
	}
	return true;
}

template <class Anim>
bool LoadAnimEntry(const std::string& name, Anim& a)
{
	wii::Brres b;
	if (!LoadBrres(name, b)) return false;
	if (!a.Load(b))
	{
		std::fprintf(stderr, "TitleScene: %s has no animation block\n", name.c_str());
		return false;
	}
	return true;
}

std::string Num2(int n)
{
	char buf[8];
	std::snprintf(buf, sizeof(buf), "%02d", n);
	return buf;
}

// column-major translation matrix
void Translation(const float p[3], float m[16])
{
	for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	m[12] = p[0]; m[13] = p[1]; m[14] = p[2];
}

float Norm(float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0) for (int i = 0; i < 3; i++) v[i] /= l;
	return l;
}
}

bool TitleScene::Load()
{
	m_loaded = false;
	wii::Brres brres;

	// node translations of the placement model: the position of each scene object (FUN_8001dd54)
	wii::Mdl0 set;
	if (!LoadBrres("TITLE_SET_MDL", brres) || brres.ModelNames().empty() || !set.Load(brres, brres.ModelNames().front())) return false;
	auto nodePos = [&](const char* name, float out[3]) {
		for (const auto& n : set.nodes)
			if (n.name == name) { for (int i = 0; i < 3; i++) out[i] = n.translate[i]; return true; }
		std::fprintf(stderr, "TitleScene: node %s not in TITLE_SET_MDL\n", name);
		return false;
	};
	auto model = [&](const char* entry, Holder& h) {
		wii::Brres b;
		return LoadBrres(entry, b) && !b.ModelNames().empty() && h.model.Load(b, b.ModelNames().front());
	};
	if (!model("TITLE_SKY_MDL", m_sky) || !model("TITLE_GATE_MDL", m_gate) || !model("TITLE_TAPE_MDL", m_tape) || !model("TITLE_PACMAN_MDL", m_pac)) return false;
	if (!nodePos("g10b_sky01", m_sky.pos) || !nodePos("g10b_tit_gate01", m_gate.pos) || !nodePos("g10b_tit_tape01", m_tape.pos)) return false;
	// Pac-Man's position is a constant of cTaskPacman::Init (0x80120618: (0, 0.4, 0, 1) stored at +0x1f0 and copied to the holder)
	m_pac.pos[0] = 0.0f; m_pac.pos[1] = 0.4f; m_pac.pos[2] = 0.0f;

	bool ok = LoadAnimEntry("TITLE_CLOUD_TSA", m_cloud) && LoadAnimEntry("TITLE_A01_CHA", m_gateAnim) && LoadAnimEntry("TITLE_TAPE_CHA", m_tapeAnim);
	for (int i = 0; i < 10 && ok; i++) ok = LoadAnimEntry("TITLE_PACMAN_" + Num2(i + 1) + "_CHA", m_pacCha[i]);
	for (int i = 0; i < 6 && ok; i++) ok = LoadAnimEntry("TITLE_PACMAN_" + Num2(i + 1) + "_TPA", m_pacTpa[i]);
	if (!ok) return false;

	// the camera: FUN_8011fe14 reads the first camera of TITLE_CAMERA_GATE01_SCA at frame 0
	wii::ScnAnim scn;
	if (!LoadAnimEntry("TITLE_CAMERA_GATE01_SCA", scn) || !scn.SampleCamera("", 0.0f, m_cam)) return false;

	// scene lights: light set 0 = TITLE_BG_SCA for the background objects (cTaskBg sets it on slots 0-2), set 1 = TITLE_MDL_SCA for
	// the tape and Pac-Man (their Init calls FUN_8001e1f8(model, 1)). Ambient colour of the materials is their own (UNVERIFIED).
	{
		wii::ScnAnim bg, mdl;
		if (!LoadAnimEntry("TITLE_BG_SCA", bg) || !bg.SampleLight(0, m_lightBg) || !LoadAnimEntry("TITLE_MDL_SCA", mdl) || !mdl.SampleLight(0, m_lightMdl)) return false;
		auto apply = [](Holder& h, const wii::LightResult& l) { h.model.SetLight(l.pos, l.aim, l.color, l.specColor, l.shininess); };
		apply(m_sky, m_lightBg); apply(m_gate, m_lightBg); apply(m_tape, m_lightMdl); apply(m_pac, m_lightMdl);
	}
	m_sky.model.SetTexSrt(&m_cloud, 0);
	m_tape.model.SetPose({{&m_tapeAnim, 0, 1.0f}});

	// cTaskPacman::Init ends in mode 0: both layers hold clip 01 / pattern 01, counter and weight 0 (0x80120858..0x8012089c)
	m_pacMode = 0;
	m_pacCur = m_pacPrev = Layer{1, 1, 0, 0};
	m_pacCounter = 0;
	m_pacWeight = 0;
	m_rng.seed(0x20061206u);   // DOL 0x801412b4 [audited: init_genrand = std::mt19937 seed] = init_genrand, 0x8014146c = genrand: std::mt19937 is the same generator
	m_gateStarted = false;
	m_wind.reset();
	{
		size_t sz = 0;
		const std::uint8_t* d = wii::ArchiveStore::Find("TITLE_WIND_PTO", sz);
		wii::PtkFile f;
		float wp[3];
		if (d && f.Parse(d, sz) && nodePos("ef_t_wind01", wp))
		{
			const float mtx[12] = {1, 0, 0, wp[0], 0, 1, 0, wp[1], 0, 0, 1, wp[2]};
			m_wind = std::make_unique<gfx::PtkSim>(std::move(f));
			m_wind->Spawn(mtx);
		}
	}
	// DOL 0x80202f48: the depth shadow light (scene root +0x1f8 = 1): mode 0, target 0, rotation (pi/4, 0, 0), ortho 20 x 20, near 20, far 60, radius 30.
	// Pac-Man is the only caster (0x8012056c, vtable +0x20 (1)); the castle's materials receive it (0x8001e16c)
	m_shadow = kingdom::cKingdomShadow();
	m_shadow.SetMode(0);
	m_shadow.SetTarget(0.0f, 0.0f, 0.0f);
	m_shadow.SetAngles(0.7853982f, 0.0f, 0.0f);
	m_shadow.SetOrtho(20.0f, 20.0f, 20.0f, 60.0f);
	m_shadow.SetRadius(30.0f);
	m_skyFrame = m_gateFrame = m_tapeFrame = 0;
	m_loaded = true;
	return true;
}

// DOL 0x80121308 [audited]: new mode: the current layer's clips go to slot 1, restarted at frame 0 (0x8001a564 attaches at frame 0), the new mode's clips become current
void TitleScene::PacSetMode(int mode)
{
	m_pacPrev = Layer{m_pacCur.tpa, m_pacCur.cha, 0, 0};
	m_pacCur = Layer{kPacModes[mode].tpa, kPacModes[mode].cha, 0, 0};
	m_pacMode = mode;
	m_pacCounter = 0;
	m_pacWeight = 0;
}

void TitleScene::StartPressed()
{
	if (!m_loaded) return;
	m_gateStarted = true;  // FUN_8011fad0: TITLE_A01_CHA on the gate from frame 0
	m_gateFrame = 0;
	PacSetMode(1);         // FUN_80121308(pacman, 1)
}

// DOL 0x80120914 (cTaskPacman::Execute) [audited]; Term DOL 0x801208bc [audited: Term frees the generator only] only frees the generator
void TitleScene::PacExecute()
{
	if (m_pacMode == 0)
	{
		if (m_pacCounter == kPacIdleFrames) PacSetMode((m_rng() & 1) ? 3 : 2); // FUN_8014146c = MT19937 genrand, bit 0
	}
	else if (m_pacMode == 2 || m_pacMode == 3)
	{
		const wii::ChrAnim& a = m_pacCha[m_pacCur.cha - 1];
		if (m_pacCur.chaFrame >= (float)a.numFrame - 1.0f) PacSetMode(0); // clip played out
	}
	m_pacWeight = std::min(1.0f, m_pacWeight + kPacBlendStep);
	m_pacCounter += 1.0f;
}

void TitleScene::Update(bool frozen)
{
	if (!m_loaded || frozen) return;
	m_skyFrame += 1.0f;
	m_tapeFrame += 1.0f;
	if (m_gateStarted) m_gateFrame += 1.0f;
	PacExecute();
	if (m_wind) m_wind->Step(1.0f);
	for (Layer* l : {&m_pacCur, &m_pacPrev})
	{
		l->chaFrame += 1.0f;
		l->tpaFrame += 1.0f;
	}
}

void TitleScene::Draw()
{
	if (!m_loaded || std::getenv("MEGAMIX_NOSCENE")) return;

	// camera (FUN_8011fff8): aim camera, position y + 0.1, twist around the line of sight
	float eye[3] = {m_cam.pos[0], m_cam.pos[1] + kCameraYOffset, m_cam.pos[2]};
	float f[3] = {m_cam.aimPos[0] - eye[0], m_cam.aimPos[1] - eye[1], m_cam.aimPos[2] - eye[2]};
	Norm(f);
	float up[3] = {0, 1, 0};
	const float tw = m_cam.twist * 3.14159265f / 180.0f;
	if (tw != 0.0f) // rotate the up vector around f
	{
		const float c = std::cos(tw), s = std::sin(tw);
		const float r[3] = {f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0]};
		for (int i = 0; i < 3; i++) up[i] = up[i] * c + r[i] * s;
	}
	float sx[3] = {f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0]};
	Norm(sx);
	const float u[3] = {sx[1] * f[2] - sx[2] * f[1], sx[2] * f[0] - sx[0] * f[2], sx[0] * f[1] - sx[1] * f[0]};
	const float view[16] = {sx[0], u[0], -f[0], 0, sx[1], u[1], -f[1], 0, sx[2], u[2], -f[2], 0,
	                        -(sx[0] * eye[0] + sx[1] * eye[1] + sx[2] * eye[2]), -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]),
	                        f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2], 1};
	const float t = 1.0f / std::tan(0.5f * kCameraFovDeg * 3.14159265f / 180.0f);
	const float zn = m_cam.nearZ, zf = m_cam.farZ, asp = m_cam.aspect;
	const float proj[16] = {t / asp, 0, 0, 0, 0, t, 0, 0, 0, 0, (zf + zn) / (zn - zf), -1, 0, 0, 2 * zf * zn / (zn - zf), 0};

	// animation state -> models
	m_sky.model.SetTexSrt(&m_cloud, m_skyFrame);
	m_tape.model.SetPose({{&m_tapeAnim, wii::ClipFrame(m_tapeFrame, m_tapeAnim.numFrame, m_tapeAnim.loops), 1.0f}});
	if (m_gateStarted) m_gate.model.SetPose({{&m_gateAnim, wii::ClipFrame(m_gateFrame, m_gateAnim.numFrame, m_gateAnim.loops), 1.0f}});
	{
		const wii::ChrAnim& cur = m_pacCha[m_pacCur.cha - 1];
		const wii::ChrAnim& prev = m_pacCha[m_pacPrev.cha - 1];
		m_pac.model.SetPose({{&cur, wii::ClipFrame(m_pacCur.chaFrame, cur.numFrame, cur.loops), m_pacWeight},
		                     {&prev, wii::ClipFrame(m_pacPrev.chaFrame, prev.numFrame, prev.loops), 1.0f - m_pacWeight}});
		const wii::PatAnim& tpa = m_pacTpa[m_pacCur.tpa - 1];
		m_pac.model.SetTexPat(&tpa, wii::ClipFrame(m_pacCur.tpaFrame, tpa.numFrame, tpa.loops));
	}

	float world[16];
	float lightView[16], lightProj[16];
	if (kingdom::NativeScene::ShadowMatrices(m_shadow, lightView, lightProj) && gfx::ShadowBeginPass(lightView, lightProj, 512, 512))
	{
		gfx::ModelDraw::SetPass(0);
		Translation(m_pac.pos, world);
		m_pac.model.Draw(world);
		gfx::ShadowEndPass();
		gfx::SetShadowReceiver(lightView, lightProj);
	}
	gfx::Begin3D(view, proj, true);
	for (Holder* h : {&m_sky, &m_gate, &m_tape, &m_pac})
	{
		Translation(h->pos, world);
		h->model.Draw(world);
	}
	if (m_wind && !std::getenv("MEGAMIX_NOPTK")) m_wind->Draw();
	gfx::End3D();
	gfx::SetShadowReceiver(nullptr, nullptr);
}

}
