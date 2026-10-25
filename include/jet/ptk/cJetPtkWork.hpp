#pragma once

#include "common.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/cJetPtkGen.hpp"
#include "jet/ptk/cJetPtkPtc.hpp"

namespace jet::ptk
{

class cJetPtkWork
{
private:
	struct EmitterBuffer
	{
		std::vector<cJetPtkEmi*> pArray;

		uint nSize;

		uint nLastIdx;
	};

	struct GeneratorBuffer
	{
		std::vector<cJetPtkGen*> pArray;

		uint nSize;

		uint nLastIdx;
	};

	struct ParticleBuffer
	{
		std::vector<cJetPtkPtc*> pArray;

		uint nSize;

		uint nLastIdx;
	};

	EmitterBuffer m_Emis{};

	cJetPtkEmi* m_rollPowCookieEmitter{};

	cJetPtkEmi* m_rollDashEmitter{};

	GeneratorBuffer m_Gens{};

	ParticleBuffer m_Ptcs{};

public:
	void Init();

private:
	void Term();

public:
	void Create(uint nEmi, uint nGen, uint nPtc);

	void Release();

	uint nEmi();

	uint nGen();

	uint nPtc();

	cJetPtkEmi* GetFreeEmi();

	cJetPtkEmi* GetRollPowCookieEmi();

	cJetPtkEmi* GetRollDashEmi();

	cJetPtkGen* GetFreeGen();

	cJetPtkPtc* GetFreePtc();

	cJetPtkWork();

	cJetPtkWork(uint nEmi, uint nGen, uint nPtc);

	virtual ~cJetPtkWork();
};

}
