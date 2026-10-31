#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g11/cTaskGrobdaModel.hpp"
#include <string>
#include <vector>
#include "WiiSnd.hpp"

// Grobda Remix (g11) arena. kingdom::game::g11::cTaskGrobdaStage (0x224 bytes; ctor FUN_800373cc, Init FUN_800376a4, Execute FUN_80037fcc).
// Stage archive entries (plain names): BIN_SCA_STAGE_CAMERA (SCN0 camera), BIN_SCA_LIGHT_BG / BIN_SCA_LIGHT_CHARA (light sets 0 / 1),
// BIN_MDL_STAGE (+ BIN_TSA_STAGE), BIN_MDL_STAGE_HIT, m11s_mapNN.bobj (MAPOO OBJ maps), m11s_mapNN_ene*.bobj / .csv (enemy waves).
// MAPOO OBJ objects (cGrobdaMapoo FUN_8019374c..): type 0x1000000N = start of player N, 0x30000001 / 0x30000002 = structure 01 / 02;
// world x = 2 * x / 16 - 31, z = 2 * z / 16 - 32 (FUN_8019376c / FUN_80193854; structures snap x / z to multiples of 8 first).
namespace kingdom { class cTaskFade; }
namespace kingdom::game::g11
{
class cTaskGrobdaConsoleManager;
class cTaskGrobdaEnemy;
class cTaskGrobdaCombo;
class cTaskGrobdaEnemyListManager;
class cTaskGrobdaPlayer;

// kingdom::game::g11::cTaskGrobdaBG (0xb50 bytes; Init FUN_8002f49c, Execute FUN_8002fafc): BIN_MDL_BG (+ BIN_CHA_BG, BIN_CLA_BG) in slot 0 and
// BIN_MDL_STAGE (+ BIN_TSA_STAGE) in slot 1, light set 0 / fog 0. The "root" children of BIN_MDL_STAGE_HIT go to FUN_80192f24 (collision, not ported).
class cTaskGrobdaBG : public cTaskGrobdaModel
{
public:
	cTaskGrobdaBG();
	bool Execute(cJetTask* pParam) override;
};

// kingdom::game::g11::cTaskGrobdaStructure01 / 02 (0xb50 bytes; Init FUN_800acaf0 / FUN_800acd58): BIN_MDL_WALL_01, or BIN_MDL_WALL_02 with
// BIN_CHA_WALL_02 + BIN_CLA_WALL_02; light set 0 / fog 0. Collision boxes (FLOAT_80477308.. / 80477320..) not ported.
class cTaskGrobdaStructure : public cTaskGrobdaModel
{
public:
	cTaskGrobdaStructure(int kind, float x, float z);
	bool Execute(cJetTask* pParam) override;
};

class cTaskGrobdaStage : public kingdom::cTaskKingdom
{
public:
	cTaskGrobdaStage();
	bool Execute(cJetTask* pParam) override;   // DOL 0x80037fcc (camera only)

	bool PlayerStart(int n, float& x, float& z) const;

	static cTaskGrobdaStage* Instance() { return s_pInstance; }
	int nPlayers() const { return m_nPlayers; }
	cTaskGrobdaConsoleManager* Console() { return m_pConsole; }
	cTaskGrobdaModel* Player(int i);                 // the player tanks (+0x204 + 4 i)
	int Score(int i) const;                          // the app's score of player i (app +0x280 + 4 i)
	void AddScore(int player, int pts);              // DOL 0x8009ffcc
	cTaskGrobdaCombo* AddCombo(uint type);           // DOL 0x80034224 + 0x800397b8
	bool IsPlaying() const { return m_nState == 2; }   // DOL 0x80039300
	bool IsFinishing() const { return m_nState >= 3; } // DOL 0x80039314
	bool IsEnded() const { return m_nState == 5; }   // DOL 0x80039330
	int  State() const { return m_nState; }
	void SetPaused(bool b) { m_bPause = b; }         // app +0x1b4 != 0

private:
	void LoadMap(const char* name);    // DOL 0x8003822c (player starts) + 0x800384b4 (structures)
	void PlayState();                  // DOL 0x800391c8
	void UpdateWaves();                // DOL 0x80039344
	void UpdateCombos();               // DOL 0x80039aac
	bool LoadWave(const std::string& name);   // DOL 0x800384b4 (the enemy half)
	static cTaskGrobdaStage* s_pInstance;
	int m_nPlayers = 1;
	cTaskGrobdaPlayer* m_pPlayer[4]{};
	cTaskGrobdaEnemyListManager* m_pList = nullptr;   // +0x220
	std::vector<cTaskGrobdaEnemy*> m_vPending;        // +0x1c0 the wave in play
	std::vector<cTaskGrobdaEnemy*> m_vLive;           // +0x1cc the wrecks leaving
	std::vector<cTaskGrobdaCombo*> m_vCombo;          // +0x1d8
	int m_nComboMax = 0;                              // +0x1bc
	WiiSnd::Handle m_Bgm;              // +0x1e4 (cGrobdaSndHnd)
	cTaskGrobdaConsoleManager* m_pConsole = nullptr;   // +0x21c
	kingdom::cTaskFade* m_pFade = nullptr;             // +0x218
	int m_nState = 0;                  // +0x1b4
	bool m_bEnter = true;              // +0x1b8 (state entered)
	bool m_bPause = false;
	bool m_bPlay0 = false;             // 'on enter' of the states 0 / 1 (the DOL's +0x1b8 byte)
	int m_nFrame = 0;                  // +0x1ec
	int m_nTimeMs = 120000;            // +0x1e8 
	float m_vStart[4][2]{};
	float m_fStartAngle[4]{};
	bool m_bStart[4]{};
};
}
