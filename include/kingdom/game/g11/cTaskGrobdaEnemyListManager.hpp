#pragma once

#include <string>

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskKingdom.hpp"

// Grobda Remix (g11) the enemy waves: kingdom::game::g11::cTaskGrobdaEnemyListManager (0x1fc bytes, vtable 0x8038c... ; ctor 0x800365fc, Execute 0x800366e8, name 0x8003684c,
// advance 0x8003688c). m11s_mapNN_ene_table.csv (NN = stage + 1): row 0 a comment, then {score threshold, wave list csv}; the level row is the highest whose threshold the best player
// score has reached; m11s_mapNN_eneMM.csv lists the .bobj enemy placements of the level (five waves); one is picked at random, then the list is walked in order (wrapping).
namespace kingdom::game::g11
{
class cTaskGrobdaEnemyListManager : public kingdom::cTaskKingdom
{
public:
	cTaskGrobdaEnemyListManager();                    // DOL 0x800365fc
	bool Execute(cJetTask* pParam) override;          // DOL 0x800366e8
	std::string Current();                            // DOL 0x8003684c: the name of the next wave file
	void Advance();                                   // DOL 0x8003688c

private:
	void LoadLevel(int row);
	kingdom::cKingdomCSV m_Table;                     // +0x1b4
	kingdom::cKingdomCSV m_Waves;                     // +0x1d4
	int m_nLevel{1};                                  // +0x1f4
	int m_nWave{0};                                   // +0x1f8
};
}
