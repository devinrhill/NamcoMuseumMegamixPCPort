#include "kingdom/game/g02/cTaskMap.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"
#include "wii/Archive.hpp"

namespace kingdom::game::g02
{

cTaskMap::sHooks cTaskMap::Hooks;

namespace
{
u32 BE32(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
s16 BE16(const u8* p) { return (s16)((p[0] << 8) | p[1]); }

const u8* Find(const char* name, const char* magic)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	return d != nullptr && size >= 0x20 && std::memcmp(d, magic, 8) == 0 ? d : nullptr;
}

int Round(float v) { return (int)(v + (0.0f <= v ? 0.5f : -0.5f)); }   // FLOAT_8047a998 / 8047a9a8 / 8047a9ac

// the drop-flag search rings of FUN_80078574: 8 cells two steps away (0x8036cb38) and 12 cells three steps away (0x8036cb78), clockwise from -z
const int RING1[8][2] = {{0, -2}, {1, -1}, {2, 0}, {1, 1}, {0, 2}, {-1, 1}, {-2, 0}, {-1, -1}};
const int RING2[12][2] = {{0, -3}, {1, -2}, {2, -1}, {3, 0}, {2, 1}, {1, 2}, {0, 3}, {-1, 2}, {-2, 1}, {-3, 0}, {-2, -1}, {-1, -2}};
}

// DOL 0x8012490c (ctor) + 0x80076cb0 (Init; priority 0x10002)
cTaskMap::cTaskMap()
{
	SetName("g02::cTaskMap");
	nDrawPriority(0x10002u);
	m_nPlayer = 0;
	m_nEnemy = 0;
	m_pObj = Find("MAPOO_OBJ", "MAPOOOBJ");
	m_pMap = Find("MAPOO_MAP", "MAPOOMAP");
	m_pPnl = Find("MAPOO_PNL", "MAPOOPNL");
	m_pGimmickMan = new cTaskGimmickMan();   // +0x1dc (0x1c0 bytes)
	ConnectChild(m_pGimmickMan);
	// INTEGRATION POINT (W2): the flag manager +0x1e0 (new 0x1ec, FUN_80070040) and the smoke manager +0x1e4 (new 0x1c0), both children
	if (Hooks.NewFlagMan != nullptr) m_pFlagMan = Hooks.NewFlagMan(this);
	if (Hooks.NewSmokeMan != nullptr) m_pSmokeMan = Hooks.NewSmokeMan(this);
	for (int i = 0; i < 0x20; i++) AddSmoke();   // the smoke pool
	if (m_pMap != nullptr) m_Attr.assign((std::size_t)nWidth() * nHeight(), 0u);   // +0x1e8 (FUN_8013b9f4 + construct)
	BuildCells();
	PlaceObjects();
	// +0x1f8 cKingdomRand(0x20061206) (FUN_801412b4): m_Rand
	// the radar's "taken flag is back" test (the console calls the gimmick's +0x1dc getter FUN_800b1d6c)
	if (cTaskConsole::Hooks.GimmickRespawn == nullptr) cTaskConsole::Hooks.GimmickRespawn = &GimmickActive;
}

bool cTaskMap::Execute(cJetTask* pParam)
{
	(void)pParam;
	return true;
}

int cTaskMap::nWidth() const { return m_pMap != nullptr ? (int)BE32(m_pMap + 0x10) : 0; }
int cTaskMap::nHeight() const { return m_pMap != nullptr ? (int)BE32(m_pMap + 0x14) : 0; }

int cTaskMap::Cell(int x, int z, int layer) const
{
	if (m_pMap == nullptr || x < 0 || z < 0 || x >= nWidth() || z >= nHeight()) return 0;
	return BE16(m_pMap + 0x20 + ((std::size_t)layer * nWidth() * nHeight() + (std::size_t)z * nWidth() + x) * 2);
}

u32 cTaskMap::Attr(int x, int z) const
{
	if (x < 0 || z < 0 || x >= nWidth() || z >= nHeight() || m_Attr.empty()) return 0;
	return m_Attr[(std::size_t)z * nWidth() + x];
}

// DOL 0x80077190 (the cell loop) + 0x80077b54
void cTaskMap::BuildCells()
{
	if (m_pMap == nullptr) return;
	const int w = nWidth(), h = nHeight(), layers = (int)BE32(m_pMap + 0x0c);
	auto mud = [](int v) { return (unsigned)(v - 6) < 2u; };
	for (int l = 0; l < layers; l++)
		for (int z = 0; z < h; z++)
			for (int x = 0; x < w; x++)
			{
				u32& attr = m_Attr[(std::size_t)z * w + x];
				const int v = Cell(x, z, l);
				if (mud(v))
				{
					attr |= 0x8001u;
					const bool big = v == 6;
					// the neighbours are read from layer 0 (the DOL indexes the cells from +0x20 without the layer offset)
					if (mud(Cell(x - 1, z)))
					{
						if (mud(Cell(x + 1, z))) m_pGimmickMan->Create(big ? 7 : 8, x, z, 3);
						else m_pGimmickMan->Create(big ? 5 : 6, x, z, 3);
					}
					else if (mud(Cell(x + 1, z))) m_pGimmickMan->Create(big ? 9 : 10, x, z, 3);
					else if (mud(Cell(x, z - 1)))
					{
						if (mud(Cell(x, z + 1))) m_pGimmickMan->Create(7, x, z, 0);
						else m_pGimmickMan->Create(big ? 5 : 6, x, z, 0);
					}
					else if (mud(Cell(x, z + 1))) m_pGimmickMan->Create(big ? 9 : 10, x, z, 0);
					else m_pGimmickMan->Create(4, x, z, 0);
				}
				else if (v == 2) attr |= 1u;
				else if (v == 4) { attr |= 1u; m_pGimmickMan->Create(1, x, z, 0); }
				else if (v == 5) { attr |= 1u; m_pGimmickMan->Create(2, x, z, 0); }
				else if (v == 8) { attr |= 1u; m_pGimmickMan->Create(3, x, z, 0); }
			}
}

void cTaskMap::ObjCell(const u8* rec, int& x, int& z)
{
	int ox = (int)BE32(rec + 4), oz = (int)BE32(rec + 8);
	if (ox > 7) ox -= 8;
	if (oz > 15) oz -= 16;
	x = ox >> 4;   // the DOL rounds toward zero; the values are never negative here
	z = oz >> 4;
}

bool cTaskMap::PlayerStart(int n, int& x, int& z) const
{
	if (m_pObj == nullptr) return false;
	const u32 count = BE32(m_pObj + 0x20), off = BE32(m_pObj + 0x24);
	for (u32 i = 0; i < count; i++)
	{
		const u8* rec = m_pObj + off + i * 0x40;
		if (BE32(rec) == 0 && n-- == 0)
		{
			ObjCell(rec, x, z);
			return true;
		}
	}
	return false;
}

// DOL 0x80079ad0
int cTaskMap::nFlagTotal() const
{
	if (Hooks.FlagCount != nullptr && m_pFlagMan != nullptr) return Hooks.FlagCount(m_pFlagMan);   // FUN_800705ec(+0x1e0)
	return (int)m_vFlags.size();   // fallback: the flags the map gave the manager
}

void cTaskMap::AddFlag(cTaskGimmick* g)
{
	cTaskItemFlag* f = dynamic_cast<cTaskItemFlag*>(g);   // (FUN_8007013c dynamic_casts to cTaskItemFlag)
	if (f == nullptr) return;
	m_vFlags.push_back(f);
	if (Hooks.FlagAdd != nullptr && m_pFlagMan != nullptr) Hooks.FlagAdd(m_pFlagMan, f);   // FUN_8007013c(+0x1e0, g)
	if (m_pConsole != nullptr) m_pConsole->RegistFlag(f, f->nType());   // FUN_800662c8(+0x1ec, g, type)
}

void cTaskMap::AddSmoke()
{
	cTaskGimmickSmoke* s = static_cast<cTaskGimmickSmoke*>(m_pGimmickMan->Create(0, nullptr, 0));   // FUN_800736cc(+0x1dc, 0, 0, 0)
	if (s == nullptr) return;
	m_vSmokes.push_back(s);
	if (Hooks.SmokeAdd != nullptr && m_pSmokeMan != nullptr) Hooks.SmokeAdd(m_pSmokeMan, s);   // FUN_8007c0e8(+0x1e4, g)
}

// the DOL registers each flag while it places it (FUN_80077e48 / FUN_800780f4 with the console the ctor got, +0x1ec); natively the app hands
// the console over after the ctor, so the flags made so far are registered here and every later one in AddFlag
void cTaskMap::RegistFlags(cTaskConsole* pConsole)
{
	m_pConsole = pConsole;
	if (pConsole == nullptr) return;
	for (cTaskItemFlag* f : m_vFlags)
		if (f != nullptr) pConsole->RegistFlag(f, f->nType());
}

// DOL 0x80079fbc
void cTaskMap::ResetGimmicks()
{
	if (Hooks.SmokeReset != nullptr && m_pSmokeMan != nullptr)
	{
		Hooks.SmokeReset(m_pSmokeMan);   // FUN_8007c400(+0x1e4)
		return;
	}
	for (cTaskGimmickSmoke* s : m_vSmokes)   // fallback (the same loop): every active smoke +0x54
		if (s != nullptr && s->IsActive()) s->Disable();
}

// DOL 0x80079b50
void cTaskMap::Contact(cTaskUnit* pUnit, int x, int z)
{
	if (m_pGimmickMan != nullptr) m_pGimmickMan->Contact(pUnit, x, z, true);   // FUN_80073a2c(+0x1dc, unit, x, z, 1)
}

// DOL 0x80079b5c
bool cTaskMap::DropFlag(int n, const float pPos[4], int nDir)
{
	for (int i = 0; i < n; i++)
	{
		float to[4];
		std::memcpy(to, pPos, sizeof to);
		if (!FindDropCell(to, pPos, nDir)) return false;   // FUN_80078574
		bool ok = false;
		if (Hooks.FlagDrop != nullptr && m_pFlagMan != nullptr) ok = Hooks.FlagDrop(m_pFlagMan, 1, to, pPos);   // FUN_800704c0(+0x1e0, 1, to, pos)
		else
		{
			for (cTaskItemFlag* f : m_vFlags)   // fallback (the same loop): the first flag whose +0x5c takes it
				if (f != nullptr && f->Drop(to, pPos))
				{
					ok = true;
					break;
				}
		}
		if (!ok) return false;
	}
	return true;
}

// DOL 0x80078574: a free road cell for a dropped flag around the car's cell (cx, cz). Ring 1 (two steps, 8 cells) / ring 2 (three steps, 12
// cells) are marked reachable through the road cells next to the car, then dropped when a gimmick is on them, they are no road, or a flag is
// already flying there. rand % 1000 < 750 prefers ring 2. The search walks the sides {-z 0, +z 1, +x 2, -x 3, behind the car} in a random
// order (the side behind = the car direction 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, so that side is twice as likely), each side a few cells from a
// random start. out (a copy of pPos) gets the cell's world x / z; true when it moved.
bool cTaskMap::FindDropCell(float out[4], const float pPos[4], int nDir)
{
	out[0] = pPos[0];
	out[2] = pPos[2];
	const int cx = Round(pPos[0]) + 4, cz = Round(pPos[2]) + 8;
	int r1[8][2], r2[12][2];
	for (int i = 0; i < 8; i++) { r1[i][0] = RING1[i][0] + cx; r1[i][1] = RING1[i][1] + cz; }
	for (int i = 0; i < 12; i++) { r2[i][0] = RING2[i][0] + cx; r2[i][1] = RING2[i][1] + cz; }
	int a1[8] = {}, a2[12] = {};
	auto set = [](int* a, std::initializer_list<int> idx) { for (int i : idx) a[i] = 1; };
	if (IsRoad(cx, cz - 1))
	{
		set(a1, {0, 1, 7});
		if (IsRoad(cx, cz - 2)) set(a2, {0, 1, 11});
		if (IsRoad(cx + 1, cz - 1)) set(a2, {1, 2});
		if (IsRoad(cx - 1, cz - 1)) set(a2, {10, 11});
	}
	if (IsRoad(cx + 1, cz))
	{
		set(a1, {1, 2, 3});
		if (IsRoad(cx + 2, cz)) set(a2, {2, 3, 4});
		if (IsRoad(cx + 1, cz - 1)) set(a2, {1, 2});
		if (IsRoad(cx + 1, cz + 1)) set(a2, {4, 5});
	}
	if (IsRoad(cx, cz + 1))
	{
		set(a1, {3, 4, 5});
		if (IsRoad(cx, cz + 2)) set(a2, {5, 6, 7});
		if (IsRoad(cx + 1, cz + 1)) set(a2, {4, 5});
		if (IsRoad(cx - 1, cz + 1)) set(a2, {7, 8});
	}
	if (IsRoad(cx - 1, cz))
	{
		set(a1, {5, 6, 7});
		if (IsRoad(cx - 2, cz)) set(a2, {8, 9, 10});
		if (IsRoad(cx - 1, cz + 1)) set(a2, {7, 8});
		if (IsRoad(cx - 1, cz - 1)) set(a2, {10, 11});
	}
	auto flagAt = [this](int x, int z) {
		const float p[4] = {(float)(x - 4), 0.0f, (float)(z - 8), 0.0f};
		if (Hooks.FlagAt != nullptr && m_pFlagMan != nullptr) return Hooks.FlagAt(m_pFlagMan, p);   // FUN_80070554(+0x1e0, p)
		for (const cTaskItemFlag* f : m_vFlags)   // fallback (the same test)
			if (f != nullptr && f->IsFlying() && f->vTo()[0] == p[0] && f->vTo()[1] == p[1] && f->vTo()[2] == p[2]) return true;
		return false;
	};
	auto filter = [&](int* a, int (*r)[2], int n) {
		for (int i = 0; i < n; i++)
		{
			if (a[i] == 0) continue;
			if (!m_pGimmickMan->IsCellFree(r[i][0], r[i][1])) a[i] = 0;   // FUN_80073af4(+0x1dc, x, z)
			if (!IsRoad(r[i][0], r[i][1])) a[i] = 0;
			if (flagAt(r[i][0], r[i][1])) a[i] = 0;
		}
	};
	filter(a1, r1, 8);
	filter(a2, r2, 12);
	const bool any1 = std::any_of(a1, a1 + 8, [](int v) { return v != 0; });
	const bool any2 = std::any_of(a2, a2 + 12, [](int v) { return v != 0; });
	int mode;   // 0 ring 1, 1 ring 2, 2 none
	if ((u32)m_Rand() % 1000u < 750u) mode = any2 ? 1 : (any1 ? 0 : 2);   // genrand FUN_8014146c(+0x1f8)
	else mode = any1 ? 0 : (any2 ? 1 : 2);
	int back = nDir;
	if (nDir == 0) back = 1;
	else if (nDir == 1) back = 0;
	else if (nDir == 2) back = 3;
	else if (nDir == 3) back = 2;
	// {side, rand} insertion-sorted by rand (table 0x8036cbd8 = 0, 1, 2, 3, 0 with the last side replaced by `back`)
	std::pair<int, u32> order[5] = {{0, 0u}, {1, 0u}, {2, 0u}, {3, 0u}, {back, 0u}};
	for (int i = 0; i < 5; i++)
	{
		order[i].second = (u32)m_Rand();
		for (int k = i; k > 0 && order[k].second < order[k - 1].second; k--) std::swap(order[k], order[k - 1]);
	}
	int pick = -1;
	if (mode == 0)
	{
		u32 v = (u32)m_Rand() % 9u;
		for (int k = 0; k < 5 && pick < 0; k++)
		{
			const int d = order[k].first;
			if ((unsigned)d > 3u) continue;
			for (int t = 0; t < 3; t++)
			{
				int i;
				if (d == 2) i = (int)(v % 3u) + 1;          // 1..3 (+x)
				else if (d == 0) i = ((int)(v % 3u) + 7) & 7;   // 7, 0, 1 (-z)
				else if (d == 1) i = (int)(v % 3u) + 3;     // 3..5 (+z)
				else i = (int)(v % 3u) + 5;                 // 5..7 (-x)
				if (a1[i] != 0) { pick = i; break; }
				v++;
			}
		}
		if (pick >= 0)
		{
			out[0] = (float)(r1[pick][0] - 4);
			out[2] = (float)(r1[pick][1] - 8);
		}
	}
	else if (mode == 1)
	{
		u32 v = (u32)m_Rand() % 15u;
		for (int k = 0; k < 5 && pick < 0; k++)
		{
			const int d = order[k].first;
			if ((unsigned)d > 3u) continue;
			for (int t = 0; t < 5; t++)
			{
				int i;
				if (d == 2) i = (int)(v % 5u) + 1;            // 1..5
				else if (d == 0) i = ((int)(v % 5u) + 10) % 12;   // 10, 11, 0, 1, 2
				else if (d == 1) i = (int)(v % 5u) + 4;       // 4..8
				else i = (int)(v % 5u) + 7;                   // 7..11
				if (a2[i] != 0) { pick = i; break; }
				v++;
			}
		}
		if (pick >= 0)
		{
			out[0] = (float)(r2[pick][0] - 4);
			out[2] = (float)(r2[pick][1] - 8);
		}
	}
	else return false;
	return !(out[0] == pPos[0] && out[2] == pPos[2]);
}

// DOL 0x80079c1c (the DOL unrolls the retry four times and recurses through 0x80079e3c)
bool cTaskMap::PutSmoke(int nOwner, int nMax, const float pPos[4], int nIndex)
{
	const int cell[2] = {Round(pPos[0]) + 4, Round(pPos[2]) + 8};
	auto put = [&]() -> int {
		if (Hooks.SmokePut != nullptr && m_pSmokeMan != nullptr) return Hooks.SmokePut(m_pSmokeMan, nOwner, nMax, cell, nIndex);   // FUN_8007c47c
		// fallback (the same logic): one smoke per cell and owner, at most nMax alive, else the first inactive one
		int alive = 0;
		for (const cTaskGimmickSmoke* s : m_vSmokes)
			if (s != nullptr && s->IsActive() && s->nOwner() == nOwner)
			{
				alive++;
				if (s->m_nX == cell[0] && s->m_nZ == cell[1]) return 0;
			}
		if (alive < nMax)
			for (cTaskGimmickSmoke* s : m_vSmokes)
				if (s != nullptr && !s->IsActive())
				{
					s->Put(nOwner, cell, nIndex);
					return 1;
				}
		return 0;
	};
	for (;;)
	{
		const int r = put();
		if (r >= 0) return r != 0;
		AddSmoke();   // no free smoke: one more (FUN_800736cc(+0x1dc, 0, 0, 0) + FUN_8007c0e8)
	}
}

// DOL 0x80079fc4
bool cTaskMap::SmokeFree(int nOwner)
{
	if (Hooks.SmokeFree != nullptr && m_pSmokeMan != nullptr) return Hooks.SmokeFree(m_pSmokeMan, nOwner);   // FUN_8007c59c(+0x1e4, id)
	for (const cTaskGimmickSmoke* s : m_vSmokes)   // fallback (the same test)
		if (s != nullptr && s->IsActive() && s->nOwner() == nOwner) return false;
	return true;
}

// DOL 0x80079fcc
bool cTaskMap::NextFlag(int nIndex, float out[4]) const
{
	const cTaskItemFlag* f = nullptr;
	if (Hooks.FlagGet != nullptr && m_pFlagMan != nullptr) f = Hooks.FlagGet(m_pFlagMan, nIndex);   // FUN_800705f4(+0x1e0, index)
	else if (nIndex >= 0 && (std::size_t)nIndex < m_vFlags.size()) f = m_vFlags[(std::size_t)nIndex];   // fallback (list order)
	kingdom::cKingdomModel* m = f != nullptr ? const_cast<cTaskItemFlag*>(f)->pModel(0) : nullptr;   // the DOL returns pModel(flag, 0)
	if (m == nullptr) return false;   // (the DOL would read through the null)
	const Vector4 t = m->vTrans();
	out[0] = t.x;
	out[1] = t.y;
	out[2] = t.z;
	out[3] = t.w;
	return true;
}

// DOL 0x800780f4 (+ the unit part 0x80078260)
bool cTaskMap::PlaceCar(cTaskUnit* pCar, int nType, int nIndex, bool bReset)
{
	if (m_pObj == nullptr || pCar == nullptr) return false;
	const u32 count = BE32(m_pObj + 0x20), off = BE32(m_pObj + 0x24);
	const u32 want = nType != 0 ? 1u : 0u;
	int running = 0;
	for (u32 i = 0; i < count; i++)
	{
		const u8* rec = m_pObj + off + i * 0x40;
		if (BE32(rec) != want) continue;
		int idx;
		if (want == 0) idx = (int)BE32(rec + 0x10);   // players: the record's own number
		else idx = running++;                         // enemies: the running count
		if (idx != nIndex) continue;
		int x = 0, z = 0;
		ObjCell(rec, x, z);
		if (want == 0)
		{
			pCar->SetStart(x, z, (int)BE32(rec + 0x14));
		}
		else
		{
			pCar->SetStart(x, z, (int)BE32(rec + 0x10));
			pCar->SetAiType((int)BE32(rec + 0xc));        // +0x1e0
			pCar->SetSpeedLevel((int)BE32(rec + 0x14));
		}
		if (bReset || nType != 1) return true;
		// the first placement of an enemy: the flags it carries (rec +0x18; the boss stage's red car): +0x2d0 = max(n, 0), and that many
		// plain flags (type 0xb, no cell) for the flag manager and the radar, disabled (+0x54: hidden) until the car drops them
		const int n = (int)BE32(rec + 0x18);
		pCar->nFlags(n);
		for (int k = n; k != 0 && k > 0; k--)
		{
			cTaskGimmick* g = m_pGimmickMan->Create(0xb, nullptr, 0);
			AddFlag(g);
			if (g != nullptr) g->Disable();
		}
		return true;
	}
	return false;
}

// DOL 0x80077190 (flags) + 0x80077530 + 0x80077e48
void cTaskMap::PlaceObjects()
{
	if (m_pObj == nullptr || m_pMap == nullptr) return;
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	// CSV_FLAG (cParamFlagData): column 1 of row (stage + (world - 1) * 7) = how many of the type 6 candidates get a flag
	u32 nFlag = 0;
	kingdom::cKingdomCSV csv;
	if (sys != nullptr && csv.RegistData("CSV_FLAG"))
	{
		const int row = sys->nStageInWorld() + (sys->nWorld() - 1) * 7;
		if (row >= 0 && row < 0x1c) nFlag = (u32)std::max(0, csv.S32(row, 1, 0));
	}
	const u32 count = BE32(m_pObj + 0x20), off = BE32(m_pObj + 0x24);
	u32 nCand = 0;
	for (u32 i = 0; i < count; i++)
		if (BE32(m_pObj + off + i * 0x40) == 6) nCand++;
	// multiplayer: the count comes from the sys (FUN_800c5950) and FUN_8007765c places them; not ported
	// a random order of the candidates ({index, rand} insertion-sorted by rand), then the first nFlag + 1 sorted by index (the DOL's loop
	// reaches one entry past nFlag)
	std::vector<std::pair<u32, u32>> order;
	for (u32 i = 0; i < nCand; i++)
	{
		order.push_back({i, (u32)std::rand()});
		for (std::size_t k = order.size() - 1; k > 0 && order[k].second < order[k - 1].second; k--) std::swap(order[k], order[k - 1]);
	}
	const std::size_t sorted = std::min<std::size_t>(order.size(), (std::size_t)nFlag + 1);
	std::sort(order.begin(), order.begin() + (std::ptrdiff_t)sorted, [](const auto& a, const auto& b) { return a.first < b.first; });
	u32 cand = 0, placed = 0;
	std::size_t next = 0;
	for (u32 i = 0; i < count; i++)
	{
		const u8* rec = m_pObj + off + i * 0x40;
		const u32 type = BE32(rec);
		bool use = true;
		if (type == 6)
		{
			use = false;
			if (placed < nFlag)
			{
				if (next < order.size() && cand == order[next].first)
				{
					use = true;
					next++;
					placed++;
				}
				cand++;
			}
		}
		if (!use) continue;
		int x = 0, z = 0;
		ObjCell(rec, x, z);
		if (x < 0 || z < 0 || x >= nWidth() || z >= nHeight()) continue;
		// FUN_80077e48
		switch (type)
		{
		case 0: m_nPlayer++; break;
		case 1: m_nEnemy++; break;
		case 2: m_pGimmickMan->Create(0x22, x, z, 0); break;
		case 3: m_pGimmickMan->Create(0x23, x, z, 0); break;
		case 4: m_pGimmickMan->Create(0x25, x, z, 0); break;
		case 5: m_pGimmickMan->Create(0x24, x, z, 0); break;
		case 6: AddFlag(m_pGimmickMan->Create(0x0b, x, z, 0)); break;
		case 7: AddFlag(m_pGimmickMan->Create(0x21, x, z, 0)); break;
		case 8: AddFlag(m_pGimmickMan->Create(0x20, x, z, 0)); break;
		case 9: AddFlag(m_pGimmickMan->Create((int)BE32(rec + 0x10) + 0x0b, x, z, 0)); break;
		default: break;
		}
	}
}

}
