#include "platform/devtools.hpp"
#include "platform/devtools_g01edit.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

namespace kingdom::game::g01
{

namespace
{
u32 Be32(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
float BeF(const u8* p) { u32 v = Be32(p); float f; std::memcpy(&f, &v, 4); return f; }
constexpr float SPEED_MIN = 0.01f;    // FLOAT_8047b6f0: a ramp speed in [-1, 0.01) is raised to this (FLOAT_8047b6ec = -1)
}

// DOL cScnData::SetData (reached from LoadScnData, see reversing/dol/g01_ghidra/cScnData.cpp): headerless records
bool cScnData::SetData(const u8* be, std::size_t size)
{
	records.clear();
	nCommand = nSpawn = 0;
	if (be == nullptr) return false;
	for (std::size_t o = 0; o + 0x30 <= size; o += 0x30)
	{
		sScnRecord r{};
		for (int i = 0; i < 3; i++) r.vParent[i] = BeF(be + o + 4 * i);
		r.n0c = Be32(be + o + 0x0c);
		r.nIndex = Be32(be + o + 0x10);
		r.fCourseTime = BeF(be + o + 0x14);
		r.fCourseSpeed = BeF(be + o + 0x18);
		r.fPacmanSpeed = BeF(be + o + 0x1c);
		r.fDelay = BeF(be + o + 0x20);
		r.nWord = Be32(be + o + 0x24);
		r.nParam1 = Be32(be + o + 0x28);
		r.nParam2 = Be32(be + o + 0x2c);
		if (r.nClass() == 0) nCommand++;
		else if (r.nClass() == 2) nSpawn++;
		records.push_back(r);
	}
	return !records.empty();
}

// DOL 0x800f6200 [audited: the light manager, animation controller and the ScnRoot's AnmScn are the native scene's]. The DOL also creates the light manager (+0x1d0) and the animation controller (+0x1d4 / +0x1d8) on the g3d ScnRoot,
// loads the scn data (0x800f618c: "scn_s%xp%d.scn", stage, number of players) and builds the demo object (vtable +0x70 = 0x800f6354:
// cDemoSceneW1S1 / W1S3 / W2S3 / W3S3 for stages 1 / 3 / 6 / 9: their logic runs in the scene's states 0 / 1).
cTaskScene::cTaskScene(int nStage, void* pEnemyMan, float fSpeed)
{
	(void)pEnemyMan;
	SetName("cTaskScene");
	m_nStage = nStage;
	m_nWorld = KingdomGlobal::GalagaSys->nWorld();
	m_fSpeed = fSpeed;
	char name[0x80];
	std::snprintf(name, sizeof name, "scn_s%xp%d.scn", nStage, KingdomGlobal::GalagaSys->nPlayer());
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	devtools::G01EditOverride(name, d, size);   // devtools: the dev window's Galaga editor swaps in its applied scn (not 1:1)
	if (!m_Scn.SetData(d, size)) std::fprintf(stderr, "g01::cTaskScene: %s not found\n", name);
	// DOL 0x800f6b40 [audited: field reset; native members] (scene reset): need masks 0, cleared mask word 0 = 1 (group 0 counts as cleared from the start). Group 1 is the
	// enemy gate's (GateGalaga_s%xp%d.csv column 3, cTaskEnemyGate 0x800e4814): it clears when the gate leaves (shot down or its clip ran out).
	m_nClearGroups[0] = 0x1;
	LoadLights();
	LoadCourse();
	ResetCourse();
	SetStartCamera();
	m_nState = 2;
	m_nStep = 0;
	// 0x800f6354: demo objects for stage 1 (cDemoSceneW1S1, the opening demo) and 3 / 6 / 9 (cDemoSceneW1S3 / W2S3 / W3S3, the goal demos)
	if ((nStage == 1 || nStage == 3 || nStage == 6 || nStage == 9) && KingdomGlobal::GalagaSys->pDemoMan() != nullptr)
		KingdomGlobal::GalagaSys->pDemoMan()->Regist(this);
	ApplyCamera();
	ApplyLights();
}

cTaskScene::cTaskScene(int nStage, float fSpeed, NoSetup)
{
	SetName("cTaskScene");
	m_nStage = nStage;
	m_nWorld = KingdomGlobal::GalagaSys->nWorld();
	m_fSpeed = fSpeed;
	m_nClearGroups[0] = 0x1;
}

cTaskScene::~cTaskScene() = default;

// DOL 0x800f653c [audited] (vtable +0x74)
void cTaskScene::GameStep()
{
	CourseStep();
	UpdateFrame();
	if (m_bEnd)   // the course ended (+0x1f8): state 2, the scene stands still
	{
		m_nState = 2;
		m_nStep = 0;
	}
}

bool cTaskScene::LoadScn0(const std::string& name)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	if (d == nullptr) return false;
	m_AnmData.emplace_back(d, d + size);
	wii::Brres b;
	wii::ScnAnim a;
	if (!b.Load(m_AnmData.back().data(), size) || !a.Load(b)) return false;
	m_Anm.push_back(std::move(a));
	return true;
}

// DOL 0x800f6fbc [audited]
void cTaskScene::LoadCourse()
{
	LoadCourse(m_nStage);
}

void cTaskScene::LoadCourse(int nCode)
{
	m_Anm.clear();
	m_AnmData.clear();
	char name[0x80];
	std::snprintf(name, sizeof name, "m01s_s%x_cam_01_sca.brres", nCode);
	if (!LoadScn0(name)) std::fprintf(stderr, "g01::cTaskScene: %s not found\n", name);
	m_fLength = 0.0f;
	for (const auto& a : m_Anm)
		if (m_fLength < (float)a.numFrame) m_fLength = (float)a.numFrame;
}

// DOL 0x800f7490 [audited: fog SCN0 bound natively with the light]
void cTaskScene::LoadLights()
{
	const char* fmt[2] = {"m01s_s%x_lig_01_sca.brres", "m01s_s%x_lig_c01_sca.brres"};
	for (int i = 0; i < 2; i++)
	{
		char name[0x80];
		std::snprintf(name, sizeof name, fmt[i], m_nStage);
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(name, size);
		wii::Brres b;
		wii::ScnAnim& a = m_LightAnm[i];
		wii::LightResult& light = m_LightFog[i];
		light = wii::LightResult{};
		m_bLight[i] = false;
		if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleLight(0, light))
		{
			m_bLight[i] = true;
			// the stage fog is its own SCN0 (m01s_s%x_fog_01_sca); UNVERIFIED which code binds it (not found yet), applied with the stage light
			char fog[0x80];
			std::snprintf(fog, sizeof fog, "m01s_s%x_fog_01_sca.brres", m_nStage);
			std::size_t fsize = 0;
			const u8* fd = wii::ArchiveStore::Find(fog, fsize);
			wii::Brres fb;
			wii::ScnAnim fa;
			if (fd != nullptr && fb.Load(fd, fsize) && fa.Load(fb)) fa.SampleFog(light);
			else a.SampleFog(light);
		}
	}
	ApplyLights();
}

// the whole light set (ambient + every enabled light) at the controller's frame, with the set's fog
void cTaskScene::ApplyLights()
{
	for (int i = 0; i < 2; i++)
	{
		if (!m_bLight[i]) continue;
		wii::LightResult l;
		const float f = std::min(m_fFrame, (float)m_LightAnm[i].numFrame);
		if (!m_LightAnm[i].SampleLightSet(f, l)) continue;
		const wii::LightResult& fog = m_LightFog[i];
		l.fogType = fog.fogType; l.fogStart = fog.fogStart; l.fogEnd = fog.fogEnd;
		for (int k = 0; k < 3; k++) l.fogColor[k] = fog.fogColor[k];
		kingdom::NativeScene::SetLight(i, l);
		static int dbg = 0;
		if (std::getenv("MEGAMIX_G01DBG") && dbg < 2 && ++dbg)
		{
			std::fprintf(stderr, "g01 light set %d: ambient %.2f %.2f %.2f, %d lights\n", i, l.ambient[0], l.ambient[1], l.ambient[2], l.numRig);
			for (int k = 0; k < l.numRig; k++)
				std::fprintf(stderr, "  dir %.2f %.2f %.2f col %.2f %.2f %.2f spec %d %.2f %.2f %.2f sh %.1f\n", l.rig[k].dir[0], l.rig[k].dir[1], l.rig[k].dir[2], l.rig[k].color[0], l.rig[k].color[1], l.rig[k].color[2], l.rig[k].hasSpec, l.rig[k].spec[0], l.rig[k].spec[1], l.rig[k].spec[2], l.rig[k].shininess);
		}
	}
}

// DOL 0x800f663c [audited]
void cTaskScene::SetStartCamera()
{
	m_Anm.clear();
	m_AnmData.clear();
	LoadLights();
	char name[0x100];
	std::snprintf(name, sizeof name, "m01s_%xstart_cam_sca.brres", m_nWorld);
	if (!LoadScn0(name)) std::fprintf(stderr, "g01::cTaskScene: %s not found\n", name);
	m_fFrame = 0.0f;   // 0x800f7da4(0.0)
	m_fRate = 1.0f * m_fSpeed;   // 0x800f7dd4(1.0)
	m_fLength = 0.0f;   // 0x800f76a4
	for (const auto& a : m_Anm)
		if (m_fLength < (float)a.numFrame) m_fLength = (float)a.numFrame;
}

// DOL 0x800f671c [audited: light SCN0 sampled to light set 0 natively] (the light SCN0 is bound to the scene like the camera; natively its first light at frame 0 goes to light set 0, UNVERIFIED)
void cTaskScene::SetDemoCamera()
{
	m_Anm.clear();
	m_AnmData.clear();
	if (!LoadScn0("m01s_s1_cam_10_sca.brres")) std::fprintf(stderr, "g01::cTaskScene: m01s_s1_cam_10_sca.brres not found\n");
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find("m01s_s1_lig_10_sca.brres", size);
	wii::Brres b;
	wii::ScnAnim& a = m_LightAnm[0];
	wii::LightResult& light = m_LightFog[0];
	light = wii::LightResult{};
	m_bLight[0] = false;
	if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleLight(0, light))
	{
		m_bLight[0] = true;
		a.SampleFog(light);
	}
	m_fFrame = 0.0f;
	m_fRate = 1.0f * m_fSpeed;
	m_fLength = 0.0f;
	for (const auto& s : m_Anm)
		if (m_fLength < (float)s.numFrame) m_fLength = (float)s.numFrame;
}

void cTaskScene::DemoFinish()
{
	m_bDemoDone = true;
	m_bGoalDone = true;
	if (m_nStage == 1) SetStartCamera();   // cDemoSceneW1S1::Term 0x801c067c; the goal demos' Term (0x800c961c) does nothing
}

// DOL 0x800f67a0 [audited] (vtable +0x88): the goal camera m01s_s%x_cam_%x0_sca (20 / 30 / 40 for stages 3 / 6 / 9) and its light SCN0s
// m01s_s%x_lig_%x0_sca / m01s_s%x_lig_c%x0_sca when they exist; frame 0, rate 1. Stage 10 (cTaskBossScene's +0x88 = 0x800d1b2c, called
// by cDemoSceneBoss::Start 0x801c08f0): cam_50 / lig_50 / lig_c50, and the fog SCN0 again (0x800f7524: m01s_s%x_fog_01_sca; natively the fog
// stays as LoadLights set it)
void cTaskScene::SetGoalCamera()
{
	const int n = m_nStage == 3 ? 2 : m_nStage == 6 ? 3 : m_nStage == 10 ? 5 : 4;
	m_Anm.clear();
	m_AnmData.clear();
	char name[0x80];
	std::snprintf(name, sizeof name, "m01s_s%x_cam_%x0_sca.brres", m_nStage, n);
	if (!LoadScn0(name)) std::fprintf(stderr, "g01::cTaskScene: %s not found\n", name);
	const char* fmt[2] = {"m01s_s%x_lig_%x0_sca.brres", "m01s_s%x_lig_c%x0_sca.brres"};
	for (int i = 0; i < 2; i++)
	{
		std::snprintf(name, sizeof name, fmt[i], m_nStage, n);
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(name, size);
		wii::Brres b;
		if (d != nullptr && b.Load(d, size) && m_LightAnm[i].Load(b)) m_bLight[i] = true;
	}
	m_fFrame = 0.0f;
	m_fRate = 1.0f * m_fSpeed;
	m_fLength = 0.0f;
	for (const auto& a : m_Anm)
		if (m_fLength < (float)a.numFrame) m_fLength = (float)a.numFrame;
}

// DOL 0x800f66d0 [audited]
void cTaskScene::SetCourseCamera()
{
	LoadCourse();
	LoadLights();
	ResetCourse();
}

// DOL 0x800f6bc4 [audited]
void cTaskScene::ResetCourse()
{
	m_nCmdNo = 0;
	m_fTimer = 0.0f;
	m_nCmdSeen = 0;
	m_nCmdCursor = 0;
	m_bEnd = false;
	m_pCur = NextCommand();
	m_nCmdNo++;
	m_pNext = NextCommand();
	m_nCmdNo++;
	m_fFrame = 0.0f;
	if (m_pCur != nullptr)
	{
		m_fRate = m_pCur->fCourseSpeed * m_fSpeed;
		m_fPacmanSpeed = m_pCur->fPacmanSpeed * m_fSpeed;
		// 0x800f6bc4: the first command's need mask (groups 0..param1 must be gone before a gate opens) and the jump flag
		for (u32& g : m_nNeedGroups) g = 0;
		for (u32 i = 0; i < m_pCur->nParam1 + 1u && i < 128u; i++) m_nNeedGroups[i >> 5] |= 1u << (i & 31);
		m_b240 = (m_pCur->nParam2 & 4) != 0;
	}
}

// DOL 0x800f7784 [audited]: the next command record (class 0) after the last one returned, nullptr when there is none
const sScnRecord* cTaskScene::NextCommand()
{
	const u32 n = (u32)m_Scn.records.size();
	for (u32 i = 0; i < n; i++)
	{
		const u32 idx = (m_nCmdCursor + i) % n;
		if (m_Scn.records[idx].nClass() == 0)
		{
			if (m_nCmdSeen == m_nCmdNo)
			{
				m_nCmdCursor = idx + 1;
				m_nCmdSeen = (m_nCmdSeen + 1) % (m_Scn.nCommand ? m_Scn.nCommand : 1);
				return &m_Scn.records[idx];
			}
			m_nCmdSeen = (m_nCmdSeen + 1) % (m_Scn.nCommand ? m_Scn.nCommand : 1);
		}
	}
	return nullptr;
}

// DOL 0x800f7e3c [audited: the null cur / next guard is the port's (the DOL reads low memory); the course end flag matches in the data (last command time 720.2 > length 720)]
void cTaskScene::CourseStep()
{
	if (m_pCur == nullptr || m_pNext == nullptr)
	{
		m_bEnd = m_fLength < m_fFrame;
		return;
	}
	const float frame = m_fFrame;
	const float rate = m_fRate;
	bool done = false;
	switch (m_pCur->nCommand())
	{
	case 3:   // gate
		if (rate == 0.0f)
			done = m_pCur->fDelay <= m_fTimer;
		else if (0.0f <= m_pNext->fCourseTime - m_pCur->fCourseTime)
			done = m_pNext->fCourseTime <= frame;
		else
			done = frame <= m_pNext->fCourseTime;
		if (done)
		{
			bool cleared = true;
			for (int i = 0; i < 4; i++)
				if ((m_nClearGroups[i] & m_nNeedGroups[i]) != m_nNeedGroups[i]) cleared = false;
			if (!cleared)
			{
				done = false;
				if (rate != 0.0f) m_fFrame = m_pCur->fCourseTime + (frame - m_pNext->fCourseTime);   // back to the gate's start (+0x1fa)
				m_fTimer = 0.0f;
			}
		}
		break;
	case 2:   // wait
		done = m_pCur->fDelay <= m_fTimer;
		break;
	case 1:   // ramp
	{
		const float t = (frame - m_pCur->fCourseTime) / (m_pNext->fCourseTime - m_pCur->fCourseTime);
		float speed = t * (m_pNext->fCourseSpeed - m_pCur->fCourseSpeed) + m_pCur->fCourseSpeed;
		if (-1.0f <= speed && speed < SPEED_MIN) speed = SPEED_MIN;
		m_fRate = speed * m_fSpeed;
		float pac = t * (m_pNext->fPacmanSpeed - m_pCur->fPacmanSpeed) + m_pCur->fPacmanSpeed;
		if (-1.0f <= pac && pac < SPEED_MIN) pac = SPEED_MIN;
		m_fPacmanSpeed = pac * m_fSpeed;
		done = m_pNext->fCourseTime <= frame;
		break;
	}
	case 0:
		done = m_pNext->fCourseTime <= frame;
		break;
	default:
		break;
	}
	if (done)
	{
		m_fTimer = 0.0f;
		m_pCur = m_pNext;
		m_pNext = NextCommand();
		m_nCmdNo++;
		m_fRate = m_pCur->fCourseSpeed * m_fSpeed;
		m_fPacmanSpeed = m_pCur->fPacmanSpeed * m_fSpeed;
		for (u32& g : m_nNeedGroups) g = 0;
		for (u32 i = 0; i < m_pCur->nParam1 + 1u && i < 128u; i++) m_nNeedGroups[i >> 5] |= 1u << (i & 31);
		// p2 bit 0: PTK_MAP_MIST01 (0x21), else bit 1: PTK_MAP_STONE01 (0x22) on the scene's handle +0x250 (FUN_80021a9c: one-time, dependent,
		// not clipped, no translate)
		if ((m_pCur->nParam2 & 1) != 0) m_CourseFx.Generate(0x21u, true, true, false, 0.0f);
		else if ((m_pCur->nParam2 & 2) != 0) m_CourseFx.Generate(0x22u, true, true, false, 0.0f);
		if ((m_pCur->nParam2 & 3) != 0 && std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 course effect %s (frame %d)\n", (m_pCur->nParam2 & 1) ? "MAP_MIST01" : "MAP_STONE01", devtools::FrameNo()); // devtools: frame number
		m_b240 = (m_pCur->nParam2 & 4) != 0;
	}
	else
	{
		m_fTimer += 1.0f;
	}
	if (m_pNext == nullptr) m_bEnd = true;
	// spawns between the two commands: by local delay while the course stands, by course time otherwise (FUN_800f783c walks the spawn records)
	if (m_pCur != nullptr && m_pNext != nullptr && m_pEnemyMan != nullptr)
	{
		for (const sScnRecord& r : m_Scn.records)
		{
			if (r.nClass() != 2 || !(m_pCur->nIndex < r.nIndex && r.nIndex < m_pNext->nIndex)) continue;
			const bool fire = (m_fRate == 0.0f) ? (m_fPrevTimer < r.fDelay && r.fDelay <= m_fTimer)
			                                    : (m_fPrevFrame < r.fCourseTime && r.fCourseTime <= m_fFrame);
			if (fire) m_pEnemyMan->Spawn(&r, (int)(r.nWord >> 10 & 0x3ff), (int)(r.nWord & 0x3ff), r.nParam2);
		}
		m_fPrevTimer = m_fTimer;
		m_fPrevFrame = m_fFrame;
	}
	m_bEnd = m_bEnd || m_fLength < m_fFrame;   // vtable +0x48 = 0x800f7e14
}

// DOL 0x800f7e28 [audited: frame controller update; not clamped (UNVERIFIED)] (the frame controller's UpdateFrame). UNVERIFIED: not clamped at the length. The course end test (vtable +0x48 =
// 0x800f7e14) is length < frame, which a frame clamped to the length never passes; the SCN0 samples clamp by themselves.
void cTaskScene::UpdateFrame()
{
	m_fFrame += m_fRate;
	if (m_fFrame < 0.0f) m_fFrame = 0.0f;
}

void cTaskScene::ApplyCamera()
{
	wii::CameraResult cam;
	for (const auto& a : m_Anm)
	{
		if (!a.SampleCamera("", m_fFrame < m_fLength ? m_fFrame : m_fLength, cam)) continue;
		float target[3] = {cam.aimPos[0], cam.aimPos[1], cam.aimPos[2]};
		kingdom::NativeScene::SetCamera(cam.pos, target, cam.fovy, cam.nearZ, cam.farZ);
		return;
	}
}

// DOL 0x800f6900 [audited: state 0 / 1 / 3 / 5 / 6 / 7 compared; the demo objects' ticks 0x801bce00 / 0x801bce94 are inlined]. Nothing while the game is paused (DAT_80479118).
bool cTaskScene::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	(void)pParam;
	switch (m_nState)
	{
	case 0:   // opening demo: 0x801bce00 on the demo object (Init +0x14 on the first frame: cDemoSceneW1S1 0x801c061c -> +0x84; then while
	          // IsWaitEnd +8 holds: W1S1 0x801c0630 frame <= length, the others 0x800f6aec = 0); the frame advances (0x800f7e28) while not done
		if (m_nDemoStep == 0)
		{
			if (m_nStage == 1) SetDemoCamera();
			m_nDemoStep = 1;
		}
		else if (!m_bDemoDone)
		{
			if (m_nStage != 1 || !(m_fFrame <= m_fLength)) m_bDemoDone = true;
		}
		if (!m_bDemoDone) UpdateFrame();
		break;
	case 1:   // goal demo: 0x801bce94 on the demo object: Start (+0x1c: W1S3 / W2S3 / W3S3 0x801c0698 / 0x801c0760 / 0x801c0828, cDemoSceneBoss
	          // 0x801c08f0 -> vtable +0x88), then while Execute +0xc holds (0x801c06ac / 0x801c0774 / 0x801c083c / 0x801c0904: frame <= +0x1ec;
	          // the base 0x800f6ae4 = 0); the frame advances while not done
		if (m_nGoalStep == 0)
		{
			if (m_nStage == 3 || m_nStage == 6 || m_nStage == 9 || m_nStage == 10) SetGoalCamera();
			m_nGoalStep = 1;
		}
		else if (!m_bGoalDone)
		{
			const bool has = m_nStage == 3 || m_nStage == 6 || m_nStage == 9 || m_nStage == 10;
			if (!has || !(m_fFrame <= m_fLength)) m_bGoalDone = true;
		}
		if (!m_bGoalDone) UpdateFrame();
		break;
	case 3:   // ready: the world's start camera, then the course camera
		if (m_fFrame < m_fLength)
		{
			UpdateFrame();
		}
		else
		{
			SetCourseCamera();
			m_nState = 4;
		}
		break;
	case 6:   // miss: the course stops (0x800f7dd4(0.0) once)
		if (m_nStep == 0)
		{
			m_fRate = 0.0f;
			m_nStep++;
		}
		break;
	case 5:   // game (vtable +0x74)
		GameStep();
		break;
	case 7:   // goal (vtable +0x78 = 0x800f6598): the stage's goal camera
		GoalCameraStep();
		break;
	default:
		break;
	}
	ApplyCamera();
	ApplyLights();
	return true;
}


// DOL 0x800f6598 [audited: port also recomputes the length]: step 0 the animations cleared (FUN_800f6f38), the stage lights (0x800f7490), m01s_s%x_cam_03_sca.brres (s_m01s_s_x_cam_03_sca_brres_8037510c)
// at frame 0 (FLOAT_8047b6d0), rate 1 (FLOAT_8047b6d4); every frame the scene's frame advances
void cTaskScene::GoalCameraStep()
{
	if (m_nStep == 0)
	{
		m_Anm.clear();
		m_AnmData.clear();
		LoadLights();
		char name[0x80];
		std::snprintf(name, sizeof name, "m01s_s%x_cam_03_sca.brres", m_nStage);
		if (!LoadScn0(name)) std::fprintf(stderr, "g01::cTaskScene: %s not found\n", name);
		m_fFrame = 0.0f;
		m_fRate = 1.0f * m_fSpeed;
		m_fLength = 0.0f;
		for (const auto& a : m_Anm)
			if (m_fLength < (float)a.numFrame) m_fLength = (float)a.numFrame;
		m_nStep++;
	}
	UpdateFrame();
}

}
