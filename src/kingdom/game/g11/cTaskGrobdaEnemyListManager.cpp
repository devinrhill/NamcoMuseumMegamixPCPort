#include "kingdom/MslRand.hpp"
#include "kingdom/game/g11/cTaskGrobdaEnemyListManager.hpp"

#include <cstdio>
#include <cstdlib>

#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cGrobdaCollision.hpp"
#include "kingdom/game/g11/cTaskGrobdaStage.hpp"

namespace kingdom::game::g11
{

// the csv file name of table row `row` (MsgTable(csv, row, 1)), loaded into m_Waves
void cTaskGrobdaEnemyListManager::LoadLevel(int row)
{
	std::string name = m_Table.Str(row, 1, "");
	if (name.empty()) return;
	m_Waves.RegistData(name);
	m_nLevel = row;
	const int n = m_Waves.nRow();
	m_nWave = n > 0 ? kingdom::MslRand() % n : 0;
}

// DOL 0x800365fc [audited]
cTaskGrobdaEnemyListManager::cTaskGrobdaEnemyListManager()
{
	SetName("cTaskGrobdaEnemyListManager");
	char name[0x100];
	std::snprintf(name, sizeof(name), "m11s_map%02d_ene_table.csv", cG11GrobdaSys::Instance()->nStage() + 1);
	m_Table.RegistData(name);
	m_nLevel = 1;
	m_nWave = 0;
	LoadLevel(1);
}

// DOL 0x800366e8 [audited]: the level follows the best score
bool cTaskGrobdaEnemyListManager::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (!cTaskGrobdaCollision::Running()) return true;
	int best = 0;
	cTaskGrobdaStage* st = cTaskGrobdaStage::Instance();
	if (st != nullptr)
		for (int i = 0; i < st->nPlayers(); i++)
			if (best < st->Score(i)) best = st->Score(i);
	const int rows = m_Table.nRow();
	int level = 1;
	while (level + 1 < rows && m_Table.S32(level + 1, 0, 0xffff) <= best) level++;
	if (level != m_nLevel) LoadLevel(level);
	return true;
}

// DOL 0x8003684c [audited]
std::string cTaskGrobdaEnemyListManager::Current()
{
	return m_Waves.Str(m_nWave, 0, "");
}

// DOL 0x8003688c [audited]
void cTaskGrobdaEnemyListManager::Advance()
{
	m_nWave++;
	if (m_nWave >= m_Waves.nRow()) m_nWave = 0;
}

}
