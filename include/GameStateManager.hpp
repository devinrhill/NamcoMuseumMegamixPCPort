#pragma once

#include "common.hpp"
#include "GameDefine.hpp"
#include "SGameRecordInfo.hpp"
#include "SSaveDataPacMotos.hpp"
#include <vector>

// Native stand-in for the Unity GameStateManager (the original header, kept in old/BACKUP_unity_headers/, never compiled). Only what the g03 game
// code uses: the per-game record and the result screen's score list.
class GameStateManager
{
public:
	struct ResultInfo
	{
		void AddScore(int, int, int) {}
		std::vector<bool> m_use5Item = std::vector<bool>(32, false);   // Pac-Motos: items used five times in a run (unlock data)
	};
	static SGameRecordInfo& GetRecord(GameDefine::EGameTitle) { static SGameRecordInfo r; return r; }
	static inline SSaveDataPacMotos m_pacMotosSave{};   // the save the Unity port kept between scenes
	static ResultInfo* GetResult() { static ResultInfo r; return &r; }
};
