#include "kingdom/game/g04/cTaskEnemyManager.hpp"

#include <bitset>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/game/g04/cTaskBG.hpp"

namespace kingdom::game::g04
{

// DOL 0x8019be50 (PATTERN blocks) + 0x8019c3a8 (the commands after START)
bool cParamEnemyPattern::Load(const char* key)
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData(key)) return false;
	const int rows = (int)csv.pCSV().size();
	auto cell = [&](int r, int c) -> std::string { return csv.Str(r, c, std::string()); };
	// 0x8019be50: every row whose column 0 is "PATTERN" (0x80139928 = find row) starts a block: column 1 = id, then 7 rows "name,value"
	for (int r = 0; r < rows; r++)
	{
		if (cell(r, 0) != "PATTERN") continue;
		sEnemyPattern p{};
		p.nId = csv.S32(r, 1, 0);
		for (int k = 1; k <= 7 && r + k < rows; k++)
		{
			const std::string n = cell(r + k, 1);
			const float v = csv.F32(r + k, 2, 0.0f);
			if (n == "wait") p.fWait = v;                    // DAT_8037c320
			else if (n == "move") p.fMove = v;               // DAT_8037c325
			else if (n == "return") p.fReturn = v;
			else if (n == "allframe") p.fAllFrame = v;
			else if (n == "attack_before_wait") p.fAttackBeforeWait = v;
			else if (n == "attack_after_wait") p.fAttackAfterWait = v;
			else if (n == "attack") p.fAttack = v;
		}
		patterns.push_back(p);
	}
	// 0x8019c3a8: from the START row to END. The pattern pointer is &patterns[column 3] (0x8019c684: by row order, not by id). The command
	// vectors keep pointers into `patterns`, which no longer grows here.
	int r = 0;
	while (r < rows && cell(r, 0) != "START") r++;
	auto pat = [&](int i) -> const sEnemyPattern* { return (i >= 0 && i < (int)patterns.size()) ? &patterns[(std::size_t)i] : nullptr; };
	for (; r < rows; r++)
	{
		const std::string c0 = cell(r, 0);
		if (c0 == "SET_PATTERN")
		{
			sEnemyCommand c{};
			c.nType = 0;
			c.nKind = csv.S32(r, 1, 0);
			const std::string places = cell(r, 2);
			int last = -1;
			for (char ch : places)
				if (ch >= 'A' && ch <= 'D')
				{
					c.nMask |= 1u << (ch - 'A');
					last = ch - 'A';
				}
			c.pPattern = pat(csv.S32(r, 3, 0));
			c.nRed = csv.S32(r, 4, 0);
			c.nRedHP = csv.S32(r, 5, 0);
			c.nColor = csv.S32(r, 6, 0);
			if (last >= 0 && c.pPattern != nullptr) cmds[last].push_back(c);   // the DOL pushes to the place of the LAST letter
		}
		else if (c0 == "TIME_SYNC")
		{
			sEnemyCommand c{};
			c.nType = 1;
			for (auto& v : cmds) v.push_back(c);
		}
		else if (c0 == "END")
			break;
		else if (c0 == "SET_RAND")
		{
			sEnemyCommand c{};
			c.nType = 2;
			c.nKind = csv.S32(r, 1, 0);
			const std::string places = cell(r, 2);
			for (char ch : places)
				if (ch >= 'A' && ch <= 'D') c.nMask |= 1u << (ch - 'A');
			c.pPattern = pat(csv.S32(r, 3, 0));
			c.nRed = csv.S32(r, 4, 0);
			c.nRedHP = csv.S32(r, 5, 0);
			c.nColor = csv.S32(r, 6, 0);
			if (c.pPattern != nullptr)
				for (char ch : places)
					if (ch >= 'A' && ch <= 'D') cmds[ch - 'A'].push_back(c);
		}
	}
	return true;
}

// DOL 0x801ae84c
cTaskEnemyManager::cTaskEnemyManager(cTaskBG* pBG) : m_pBG(pBG), m_Rand((u32)std::time(nullptr))
{
	SetName("g04::cTaskEnemyManager");
	for (uint i = 0; i < 4; i++)
	{
		m_pEnemy[i] = new cTaskEnemy(this, i);
		m_pEnemy[i]->nDrawPriority(0x10002u);
	}
	PlaceEnemies();
	for (uint i = 0; i < 4; i++) ConnectChild(m_pEnemy[i]);
	LoadPatterns(KingdomGlobal::WaniWaniSys->nPlayers());
	// cParamEnemyLevel (0x8019bc48): the level CSV of the player count (0x8037d0d8)
	static const char* const LEVEL[4] = {"CSV_ENEMY_LEVEL", "CSV_MULTI2_LEVEL", "CSV_MULTI3_LEVEL", "CSV_MULTI4_LEVEL"};
	kingdom::cKingdomCSV csv;
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	if (csv.RegistData(LEVEL[(n >= 1 && n <= 4) ? n - 1 : 0]))
	{
		static const int ROW[4] = {0, 1, 2, 4};
		for (int i = 0; i < 4; i++)
		{
			m_nLevelScore[i] = csv.S32(ROW[i], 1, 0);
			m_nLevelPattern[i] = csv.S32(ROW[i], 2, 0);
		}
		m_nAfterEvent = csv.S32(3, 2, 0);
	}
	m_nFrame = 0;
	m_nMode = 0;
	m_bAngry = false;
	m_bAfterEvent = false;
	m_RandGroup.clear();
	LoadPattern(0);
}

// the gators on the BG nodes wani_A..D (0x8019d02c); B / D turned by +-30 degrees (FLOAT_8047ccd0 / ccd4); hide_X (0x8019d10c) -> +0x600,
// show_X (0x8019d180) -> +0x610
void cTaskEnemyManager::PlaceEnemies()
{
	for (uint i = 0; i < 4; i++)
	{
		cTaskEnemy* e = m_pEnemy[i];
		const Vector4 pos = m_pBG->WaniNode(i);
		for (uint k = 0; k < 2; k++)
		{
			kingdom::cKingdomModel* m = e->pModel(k);
			m->vTrans() = pos;
			if (i == 1) m->vRotate()[1] = 0.5235988f;
			else if (i == 3) m->vRotate()[1] = -0.5235988f;
		}
		e->SetHomePos(pos);
		e->m_vHide = m_pBG->HideNode(i);
		e->m_vShow = m_pBG->ShowNode(i);
	}
}

// DOL 0x801af97c: CSV_ENEMY00..20 (0x8037d0e8), CSV_MULTI2_ENEMYnn (0x8037d13c), _MULTI3_ (0x8037d190), _MULTI4_ (0x8037d1e4)
void cTaskEnemyManager::LoadPatterns(int nPlayer)
{
	for (int i = 0; i < 21; i++)
	{
		char key[32];
		if (nPlayer <= 1) std::snprintf(key, sizeof key, "CSV_ENEMY%02d", i);
		else std::snprintf(key, sizeof key, "CSV_MULTI%d_ENEMY%02d", nPlayer, i);
		m_pPattern[i] = std::make_unique<cParamEnemyPattern>();
		m_pPattern[i]->Load(key);
	}
}

// DOL 0x801af470
void cTaskEnemyManager::LoadPattern(int idx)
{
	for (cTaskEnemy* e : m_pEnemy) e->ClearCommands();
	if (idx >= 0 && idx < 21 && m_pPattern[idx] != nullptr)
		for (uint i = 0; i < 4; i++)
			for (const sEnemyCommand& c : m_pPattern[idx]->cmds[i]) m_pEnemy[i]->AddCommand(&c);
	for (cTaskEnemy* e : m_pEnemy)
	{
		e->Rewind();
		e->nSync() = 1;
	}
	for (cTaskEnemy* e : m_pEnemy)
	{
		e->bSynced() = false;
		e->SetState(5);
	}
	m_RandGroup.clear();
}

// DOL 0x801af250
void cTaskEnemyManager::SelectLevel(uint score)
{
	auto rnd = [](int lo, int hi) { return (int)((uint)std::rand() % (uint)(hi - lo)) + lo; };   // libc rand 0x80211048
	if (score <= (uint)m_nLevelScore[0]) LoadPattern((int)((uint)std::rand() % (uint)m_nLevelPattern[0]));
	else if (score <= (uint)m_nLevelScore[1]) LoadPattern(rnd(m_nLevelPattern[0], m_nLevelPattern[1]));
	else if (!m_bAngry && score <= (uint)m_nLevelScore[2]) LoadPattern(rnd(m_nLevelPattern[1], m_nLevelPattern[2]));
	else if (!m_bAngry || m_bAfterEvent)
	{
		if (!m_bAngry) LoadPattern(rnd(m_nLevelPattern[1], m_nLevelPattern[2]));
		else
		{
			uint p = (uint)rnd(m_nAfterEvent, m_nLevelPattern[3]) + 1u;
			if (0x14u < p) p = 0x14u;
			LoadPattern((int)p);
		}
	}
	else
	{
		LoadPattern(m_nAfterEvent);
		m_bAfterEvent = true;
	}
}

// DOL 0x801af5d8
void cTaskEnemyManager::SetAllState(int state)
{
	for (cTaskEnemy* e : m_pEnemy) e->SetState(state);
}

// DOL 0x801af8d4
void cTaskEnemyManager::SetMode(int mode)
{
	m_nMode = mode;
	if (mode == 2) SetAllState(0x11);
	else if (mode == 3)
	{
		SetAllState(0x13);
		m_bAngry = true;
		m_bAfterEvent = false;
	}
}

// DOL 0x801afed8
void cTaskEnemyManager::EndAngry()
{
	m_bAngry = true;
	m_bAfterEvent = false;
	m_nMode = 1;
}

// DOL 0x801aec9c
void cTaskEnemyManager::Reset()
{
	m_nFrame = 0;
	m_bAngry = false;
	m_bAfterEvent = false;
	m_nMode = 0;
	m_RandGroup.clear();
	PlaceEnemies();
	for (cTaskEnemy* e : m_pEnemy) e->Reset();
	LoadPattern(0);
}

// DOL 0x801afdec: single = kind table 0x8037d238 by the result rank, multiplayer = the winner's colour
void cTaskEnemyManager::SetResultLook(int idx)
{
	static const s32 KIND[5] = {3, 0, 4, 2, 1};   // DAT_8037d238
	const int k = KingdomGlobal::WaniWaniSys->nPlayers() == 1 ? KIND[(idx >= 0 && idx < 5) ? idx : 0] : idx;
	for (cTaskEnemy* e : m_pEnemy) e->SetResultLook(k);
}

// DOL 0x801af7dc
void cTaskEnemyManager::AddRandGroup(u32 mask)
{
	for (u32 m : m_RandGroup)
		if (m == mask) return;
	m_RandGroup.push_back(mask);
}

// DOL 0x801af634: a SET_RAND group starts when all its gators wait (state 0xe): one of them, MT() % count, comes out (2), the others skip (4)
void cTaskEnemyManager::RandGroups()
{
	for (auto it = m_RandGroup.begin(); it != m_RandGroup.end();)
	{
		const u32 mask = *it;
		const uint n = (uint)std::bitset<4>(mask & 0xfu).count();
		uint waiting = 0;
		for (uint i = 0; i < 4; i++)
			if (m_pEnemy[i]->nState() == 0xe && (mask & (1u << i)) != 0) waiting++;
		if (waiting == n && n != 0)
		{
			const uint pick = m_Rand() % n;
			uint k = 0;
			for (uint i = 0; i < 4; i++)
			{
				cTaskEnemy* e = m_pEnemy[i];
				if (e->nState() == 0xe && (mask & (1u << i)) != 0)
				{
					e->SetState(k == pick ? 2 : 4);
					k++;
				}
			}
			it = m_RandGroup.erase(it);
		}
		else
			++it;
	}
}

// DOL 0x801af048
bool cTaskEnemyManager::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app->IsPause() || app->IsHome()) return true;
	m_nFrame = app->TimerFrames();   // the timer's +0x1d0
	if (m_nMode == 1)
	{
		if (m_nFrame % 900u == 0) SelectLevel(app->Score());   // 0x801b1a1c
		bool all = true;
		for (cTaskEnemy* e : m_pEnemy) all = all && e->bSynced();
		if (all)
		{
			for (cTaskEnemy* e : m_pEnemy) e->bSynced() = false;
			SetAllState(5);
		}
		all = true;
		for (cTaskEnemy* e : m_pEnemy) all = all && e->nState() == 0xd;
		if (all) SetAllState(5);
		RandGroups();
		m_nFrame++;
	}
	return cTaskKingdom::Execute(pParam);
}

}
