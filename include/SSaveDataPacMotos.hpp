#pragma once

#include "common.hpp"

struct SSaveDataPacMotos
{
public:
	struct ScoreData
	{
	public:
		uint nHiScore{};

		uint nRecordTime{};

		bool bHiScoreNew{};

		bool bRecordTimeNew{};

		bool bClear{};

		bool bOpen{};
	};

	int nSaveVersion{};

	std::vector<ScoreData> sScoreDatas{};

	std::vector<uint> nItemData{};

	bool bAllClear{};

	uint nPlayerStock{};

	void Init();
};
