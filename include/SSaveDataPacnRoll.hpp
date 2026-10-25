#pragma once

#include "common.hpp"

struct SSaveDataPacnRoll
{
public:
	struct sStageRecord
	{
	public:
		uint nTime{};

		uint nGhost{};

		uint nCookie{};

		uint nCookieMax{};

		bool bEatPizza{};

		bool bTimeNew{};

		bool bGhostNew{};

		bool bCookieNew{};

		bool bFree{};

		sStageRecord Clone();
	};

	uint nValidate{};

	uint nDefaultLife{};

	uint nDefaultHP{};

	uint nLife{};

	uint nMaxHP{};

	bool bAllClear{};

	std::vector<sStageRecord> StageRecord{};

	void Init();
};
