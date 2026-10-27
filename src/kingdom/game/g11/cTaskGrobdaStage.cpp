#include "kingdom/game/g11/cTaskGrobdaStage.hpp"
#include "kingdom/game/g11/cTaskGrobdaConsole.hpp"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "GamePad.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cGrobdaCollision.hpp"
#include "wii/Mdl0.hpp"
#include "kingdom/game/g11/cAppG11GrobdaGame.hpp"
#include "kingdom/game/g11/cTaskGrobdaCombo.hpp"
#include "kingdom/game/g11/cTaskGrobdaEnemy.hpp"
#include "kingdom/game/g11/cTaskGrobdaEnemyBig.hpp"
#include "kingdom/game/g11/cTaskGrobdaEnemyListManager.hpp"
#include "kingdom/game/g11/cTaskGrobdaPlayer.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

namespace kingdom::game::g11
{

cTaskGrobdaStage* cTaskGrobdaStage::s_pInstance = nullptr;

namespace
{
u32 BE32(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }

bool Exists(const char* name)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(name, size) != nullptr;
}

// FUN_8019376c / FUN_80193854 with snapping: the nearest multiple of 8 (ties go up)
int Snap(int v)
{
	if (v % 8 == 0) return v;
	const int lo = (v >> 3) * 8, hi = lo + 8;
	return std::abs(lo - v) < std::abs(hi - v) ? lo : hi;
}
}

// DOL 0x8002f49c
cTaskGrobdaBG::cTaskGrobdaBG()
{
	SetName("cTaskGrobdaBG");
	cKingdomModel* m = CreateModel(0);
	m->RegistModel("BIN_MDL_BG", 0x36f);
	if (Exists("BIN_CHA_BG")) m->RegistAnm(0u, "BIN_CHA_BG");
	if (Exists("BIN_CLA_BG")) m->RegistAnm(0u, "BIN_CLA_BG");
	m->SetLightSetIdx(0);
	m->SetFogIdx(0);
	m = CreateModel(1);
	m->RegistModel("BIN_MDL_STAGE", 0x36f);
	if (Exists("BIN_TSA_STAGE")) m->RegistAnm(0u, "BIN_TSA_STAGE");
	m->SetLightSetIdx(0);
	m->SetFogIdx(0);
	// the arena border: the child nodes (c_1 .. c_8) of BIN_MDL_STAGE_HIT (crc 0xe2d57afc) become the polygon of this object, in reverse order
	// (FUN_80192f24); the DOL reads them as the children of the node "root"
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find("BIN_MDL_STAGE_HIT", size);
	wii::Brres brres;
	wii::Mdl0 hit;
	if (d != nullptr && brres.Load(d, size) && !brres.ModelNames().empty() && hit.Load(brres, brres.ModelNames().front()))
	{
		float v[64][4] = {};
		uint n = 0;
		for (const auto& nd : hit.nodes)
			if (nd.name.size() > 2 && nd.name[0] == 'c' && nd.name[1] == '_' && n < 64)
			{
				v[n][0] = nd.translate[0];
				v[n][1] = nd.translate[1];
				v[n][2] = nd.translate[2];
				n++;
			}
		if (n > 1) SetShape(v, n, true);
	}
}

// DOL 0x8002fafc (the pause test and the effect hook 0x8009ffa4 not ported)
bool cTaskGrobdaBG::Execute(cJetTask* pParam)
{
	if (cTaskGrobdaCollision::Running())
	{
		UpdateFrame();
		if (cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(2, this, 0);   // FUN_8009ffa4(app, 2, this, 0)
	}
	return cTaskGrobdaModel::Execute(pParam);
}

// DOL 0x800acaf0 / 0x800acd58
cTaskGrobdaStructure::cTaskGrobdaStructure(int kind, float x, float z)
{
	SetName(kind == 2 ? "cTaskGrobdaStructure02" : "cTaskGrobdaStructure01");
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(kind == 2 ? "BIN_MDL_WALL_02" : "BIN_MDL_WALL_01", 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(0);
	if (kind == 2)
	{
		if (Exists("BIN_CHA_WALL_02")) m->RegistAnm(0u, "BIN_CHA_WALL_02");
		if (Exists("BIN_CLA_WALL_02")) m->RegistAnm(0u, "BIN_CLA_WALL_02");
	}
	m->vTrans() = Vector4(x, 0.0f, z, 0.0f);   // FUN_800384b4: (x, FLOAT_8047a030 = 0, z, 0)
	m_fRadius = 1.0f;   // +0x24 (FLOAT_80477310 / 80477328)
	const float hw = 2.0f * 0.5f, hd = 2.0f * 0.5f;   // +0x1c / +0x20 = 2 x 2 (FLOAT_80477308 / 8047730c): FUN_8019334c(w, d)
	const float box[4][4] = {{hw, 0.0f, hd, 0.0f}, {hw, 0.0f, -hd, 0.0f}, {-hw, 0.0f, -hd, 0.0f}, {-hw, 0.0f, hd, 0.0f}};
	SetShape(box, 4, false);
}

// DOL 0x800acbc0 (cTaskGrobdaStructure::Execute and the effect hook not ported)
bool cTaskGrobdaStructure::Execute(cJetTask* pParam)
{
	if (cTaskGrobdaCollision::Running())
	{
		UpdateFrame();
		UpdateShape();   // FUN_801933dc
		if (cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(2, this, 0);
	}
	return cTaskGrobdaModel::Execute(pParam);
}

// DOL 0x800373cc + Init 0x800376a4 (partial: the enemy list manager +0x220, the player tanks (0x80194d0c, 0xc80 bytes, one per player,
// priority 0x10002), collision +0x214, console manager +0x21c, fade +0x218, sound handle +0x1e4 are not ported)
cTaskGrobdaStage::cTaskGrobdaStage()
{
	SetName("cTaskGrobdaStage");
	// light manager +0x1f4 / fog manager +0x1f8: BIN_SCA_LIGHT_BG -> set 0, BIN_SCA_LIGHT_CHARA -> set 1
	kingdom::NativeScene::Reset();
	static const char* const LIGHT[2] = {"BIN_SCA_LIGHT_BG", "BIN_SCA_LIGHT_CHARA"};
	for (int i = 0; i < 2; i++)
	{
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(LIGHT[i], size);
		wii::Brres b;
		wii::ScnAnim a;
		wii::LightResult light;
		if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleLight(0, light))
		{
			a.SampleFog(light);
			kingdom::NativeScene::SetLight(i, light);
		}
	}
	ConnectChild(new kingdom::cTaskNativeScene());
	m_pConsole = new cTaskGrobdaConsoleManager(KingdomGlobal::GrobdaSys->nPlayer());   // +0x21c (builder 0x8012f7c8)
	ConnectChild(m_pConsole);
	m_pFade = new kingdom::cTaskFade();   // +0x218
	m_pFade->nDrawPriority(0x8fffffefu);
	ConnectChild(m_pFade);
	ConnectChild(new cTaskGrobdaBG());   // +0x200 (priority 0x10000)
	char name[0x200];
	std::snprintf(name, sizeof(name), "m11s_map%02d.bobj", (std::rand() & 3) + 1);   // the DOL picks one of the four maps at random
	LoadMap(name);
	// one tank per player (+0x204..; priority 0x10002); the DOL creates them before loading the map and then moves them to the starts
	s_pInstance = this;
	m_nPlayers = KingdomGlobal::GrobdaSys->nPlayer();
	for (int i = 0; i < m_nPlayers && i < 4; i++)
	{
		float x = 0.0f, z = 0.0f;
		PlayerStart(i, x, z);
		m_pPlayer[i] = new cTaskGrobdaPlayer(i, x, z);
		ConnectChild(m_pPlayer[i]);
	}
	m_pList = new cTaskGrobdaEnemyListManager();   // +0x220 (0x8003...: the waves)
	ConnectChild(m_pList);
	ConnectChild(new cTaskGrobdaCollision());   // +0x214 (0x220 bytes): after the objects, so the lists of this frame are resolved
}

void cTaskGrobdaStage::LoadMap(const char* name)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	if (d == nullptr || size < 0x28 || BE32(d) != 0x4d41504fu || BE32(d + 4) != 0x4f4f424au) return;
	const u32 layers = BE32(d + 0x0c);
	struct sBlock { cTaskGrobdaStructure* p; int x, z; };
	std::vector<sBlock> blocks;   // the structures with their snapped map cells (FUN_800384b4 joins neighbouring blocks)
	for (u32 l = 0; l < layers && l < 4; l++)
	{
		const u32 count = BE32(d + 0x20 + l * 8), off = BE32(d + 0x24 + l * 8);
		for (u32 i = 0; i < count; i++)
		{
			const u8* rec = d + off + i * 0x40;
			if (off + (i + 1) * 0x40 > size) break;
			const u32 type = BE32(rec);
			const int ox = (int)BE32(rec + 4), oz = (int)BE32(rec + 8);
			if ((type & 0xff000000u) == 0x10000000u && type != 0x10000000u)   // FUN_8003822c: player starts, no snapping
			{
				const u32 n = type - 0x10000001u;
				if (n < 4)
				{
					m_vStart[n][0] = 2.0f * (float)ox * 0.0625f - 31.0f;
					m_vStart[n][1] = 2.0f * (float)oz * 0.0625f - 32.0f;
					m_bStart[n] = true;
				}
			}
			else if ((type & 0xff000000u) == 0x30000000u && type != 0x30000000u)   // FUN_800384b4: structures
			{
				const int kind = type == 0x30000002u ? 2 : (type == 0x30000001u ? 1 : 0);
				if (kind == 0) continue;
				const float x = 2.0f * (float)Snap(ox) * 0.0625f - 31.0f;
				const float z = 2.0f * (float)Snap(oz) * 0.0625f - 32.0f;
				cTaskGrobdaStructure* st = new cTaskGrobdaStructure(kind, x, z);
				ConnectChild(st);
				blocks.push_back({st, Snap(ox), Snap(oz)});
			}
		}
	}
	// the edges two neighbouring blocks (16 map cells apart) share are left out of the sweeps: edge bits +x 1, -z 2, -x 4, +z 8 of the 4-vertex box
	for (size_t i = 0; i < blocks.size(); i++)
		for (size_t j = i + 1; j < blocks.size(); j++)
		{
			sBlock &a = blocks[i], &b = blocks[j];
			if (a.x == b.x)
			{
				if (a.z + 0x10 == b.z) { a.p->m_nMask |= 8; b.p->m_nMask |= 2; }
				else if (a.z - 0x10 == b.z) { a.p->m_nMask |= 2; b.p->m_nMask |= 8; }
			}
			else if (a.z == b.z)
			{
				if (a.x + 0x10 == b.x) { a.p->m_nMask |= 1; b.p->m_nMask |= 4; }
				else if (a.x - 0x10 == b.x) { a.p->m_nMask |= 4; b.p->m_nMask |= 1; }
			}
		}
}

bool cTaskGrobdaStage::PlayerStart(int n, float& x, float& z) const
{
	if (n < 0 || n >= 4 || !m_bStart[n]) return false;
	x = m_vStart[n][0];
	z = m_vStart[n][1];
	return true;
}

// DOL 0x800391c8 (play state): frame counter, time left = total - ms(frames) (0x8000d80c: frames * 100000 / 6000), expiry -> state 3
void cTaskGrobdaStage::PlayState()
{
	if (m_bEnter)
	{
		m_bEnter = false;
		static_cast<cAppG11GrobdaGame*>(KingdomGlobal::KingSys->pAppMain())->StartPlayWatch();   // FUN_8009ff68
	}
	m_nFrame++;
	const int ms = (int)((100000.0f * (float)m_nFrame) / 6000.0f);
	if (m_nTimeMs - ms < 1)
	{
		m_pConsole->SetTime(0);
		WiiSnd::FadeOut(m_Bgm, 500);   // FUN_80026318(+0x1e4, 500)
		WiiSnd::Handle se;
		WiiSnd::Play(se, "m11se_result");   // DAT_8047a02c
		WiiSnd::Detach(se);
		m_nState = 3;
		m_bEnter = true;
		return;
	}
	m_pConsole->SetTime(m_nTimeMs - ms);   // 0x80130cdc
	UpdateWaves();    // 0x80039344
	UpdateCombos();   // 0x80039aac
}

cTaskGrobdaModel* cTaskGrobdaStage::Player(int i)
{
	return i >= 0 && i < 4 ? static_cast<cTaskGrobdaModel*>(m_pPlayer[i]) : nullptr;
}

int cTaskGrobdaStage::Score(int i) const
{
	return static_cast<cAppG11GrobdaGame*>(KingdomGlobal::KingSys->pAppMain())->nScore(i);
}

// DOL 0x8009ffcc: the app's score of the player (0 .. 999999, 99999 in multiplayer) and the console
void cTaskGrobdaStage::AddScore(int player, int pts)
{
	static_cast<cAppG11GrobdaGame*>(KingdomGlobal::KingSys->pAppMain())->AddScore(player, pts, m_pConsole);
}

// DOL 0x800397b8: the combo joins the stage's list (it is active from then on)
cTaskGrobdaCombo* cTaskGrobdaStage::AddCombo(uint type)
{
	cTaskGrobdaCombo* c = new cTaskGrobdaCombo(type);
	c->SetMode(1);
	ConnectChild(c);
	m_vCombo.push_back(c);
	return c;
}

// DOL 0x80039344: the wrecks that are gone are released; when the wave in play is all wrecks, the wrecks leave (and join the live list), the next
// wave file is loaded and its enemies drop in
void cTaskGrobdaStage::UpdateWaves()
{
	for (size_t i = m_vLive.size(); i-- > 0;)
		if (m_vLive[i]->IsGone())
		{
			m_vLive[i]->Release();   // cJetNode::Release(+0x97c)
			m_vLive.erase(m_vLive.begin() + (long)i);
		}
	bool ready = true;
	for (cTaskGrobdaEnemy* e : m_vPending)
		if (!e->IsReady()) { ready = false; break; }
	if (!ready) return;
	const std::string name = m_pList->Current();
	m_pList->Advance();
	for (cTaskGrobdaEnemy* e : m_vPending)
	{
		e->Leave();   // 0x80027474
		m_vLive.push_back(e);
	}
	m_vPending.clear();
	LoadWave(name);
	for (cTaskGrobdaEnemy* e : m_vPending) e->Begin();   // 0x80027464
}

// DOL 0x80039aac: finished combos (mode 2) are released; the best running combo is shown on the console
void cTaskGrobdaStage::UpdateCombos()
{
	int best = 0;
	for (size_t i = m_vCombo.size(); i-- > 0;)
	{
		best = std::max(best, m_vCombo[i]->nCount());
		if (m_vCombo[i]->nMode() == 2)
		{
			m_vCombo[i]->Release();
			m_vCombo.erase(m_vCombo.begin() + (long)i);
		}
	}
	if (m_nComboMax < best)
	{
		m_nComboMax = best;
		if (best > 1 && m_pConsole != nullptr) m_pConsole->SetCombo(best);   // 0x80130cf4
	}
	else if (best != m_nComboMax)
		m_nComboMax = 0;
}

// DOL 0x800384b4 (the enemy half): every record 0x2000000N makes an enemy of the kind N at (x, 0, z); its three words start it
bool cTaskGrobdaStage::LoadWave(const std::string& name)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	if (d == nullptr || size < 0x28 || BE32(d) != 0x4d41504fu || BE32(d + 4) != 0x4f4f424au) return false;
	const u32 layers = BE32(d + 0x0c);
	for (u32 l = 0; l < layers && l < 4; l++)
	{
		const u32 count = BE32(d + 0x20 + l * 8), off = BE32(d + 0x24 + l * 8);
		for (u32 i = 0; i < count; i++)
		{
			if (off + (i + 1) * 0x40 > size) break;
			const u8* rec = d + off + i * 0x40;
			const u32 type = BE32(rec);
			if ((type & 0xff000000u) != 0x20000000u || type == 0x20000000u) continue;
			cTaskGrobdaEnemy* e = nullptr;
			if (type >= 0x20000001u && type <= 0x20000003u)
				e = new cTaskGrobdaEnemyTank(type, (int)(type - 0x20000001u));   // 1 Purple, 2 Orange, 3 Yellow
			else if (type == 0x20000004u || type == 0x20000005u)
				e = new cTaskGrobdaEnemyFrossor(type, type == 0x20000005u);      // 4 Green, 5 Brown
			else if (type == 0x20000006u)
				e = new cTaskGrobdaEnemyFortres();                               // 6
			if (e == nullptr) continue;
			const float x = 2.0f * (float)(int)BE32(rec + 4) * 0.0625f - 31.0f;
			const float z = 2.0f * (float)(int)BE32(rec + 8) * 0.0625f - 32.0f;
			float* p = e->GetPos();
			p[0] = x; p[1] = 0.0f; p[2] = z; p[3] = 0.0f;
			std::memcpy(e->GetOldPos(), p, 16);
			e->SetStart((int)BE32(rec + 0x10), (int)BE32(rec + 0x14), (int)BE32(rec + 0x0c));
			ConnectChild(e);
			m_vPending.push_back(e);
		}
	}
	return true;
}

// DOL 0x80037fcc (camera + state machine): cTaskGrobdaCamera (0x8003027c) takes ScnRoot camera 0 and 0x80191f88 sets it from frame 0 of
// BIN_SCA_STAGE_CAMERA (aim posture); its Execute (0x80030444, below) runs natively at the end of this Execute
bool cTaskGrobdaStage::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (!m_bPause && (GamePad::Get(0).hold & 0x400u) == 0)   // DOL 0x80037fcc: app +0x1b4 == 0 and HOME not held
	{
		const bool enter = m_bEnter;
		switch (m_nState)
		{
		case 0:
			if (!m_bPlay0)
			{
				m_bPlay0 = true;
				m_pFade->Fade(2u, 500);
				m_pConsole->Start();                 // vtable +0x48
				m_pConsole->SetTime(m_nTimeMs);      // 0x80130cdc
			}
			if (m_pFade->IsEnd())
			{
				WiiSnd::Play(m_Bgm, "m11bgm_game");   // FUN_80194190(DAT_8047a028)
				m_nState = 1;
				m_bPlay0 = false;
			}
			break;
		case 1:
			if (!m_bPlay0) { m_bPlay0 = true; m_pConsole->StartReadyGo(); }   // 0x80130c94
			if (m_pConsole->IsReadyDone()) { m_nState = 2; m_bPlay0 = false; m_bEnter = true; }   // 0x80130d08
			break;
		case 2:
			PlayState();
			break;
		case 3:
			if (enter) { m_bEnter = false; m_pConsole->Finish(); }   // 0x80130ca8
			if (m_pConsole->IsEnd()) { m_nState = 4; m_bEnter = true; }   // 0x80130d38
			break;
		case 4:
			if (enter)
			{
				m_bEnter = false;
				m_pConsole->End();               // vtable +0x4c
				m_pFade->Fade(0x10008u, 500);
			}
			if (m_pFade->IsEnd()) m_nState = 5;
			break;
		default: break;
		}
	}
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find("BIN_SCA_STAGE_CAMERA", size);
	wii::Brres b;
	wii::ScnAnim a;
	wii::CameraResult cam;
	if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleCamera("", 0.0f, cam))
	{
		float target[3] = {cam.aimPos[0], cam.aimPos[1], cam.aimPos[2]};
		// DOL 0x80030444 cTaskGrobdaCamera::Execute -> FUN_80191d2c (cGrobdaCamera update, every frame): position = Trans(aim +0x20) x RotZ(+0x38)
		// x RotY(+0x34) x RotX(+0x30) x Trans(pos +0x10) applied to the origin, aim posture unchanged. 0x80191f88 stored the file's pos / aim, the
		// rotations stay 0 (only the ctor 0x80191c0c writes them), so the drawn camera is aim + pos (w1 stage: y 38.5 / z 31.64, not 38 / 30.84)
		float pos[3] = {cam.aimPos[0] + cam.pos[0], cam.aimPos[1] + cam.pos[1], cam.aimPos[2] + cam.pos[2]};
		kingdom::NativeScene::SetCamera(pos, target, cam.fovy, cam.nearZ, cam.farZ);
	}
	return true;
}

}
