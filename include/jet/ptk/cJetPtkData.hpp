#pragma once

#include "common.hpp"
#include "jet/ptk/datEmi.hpp"
#include "jet/ptk/datGen.hpp"
#include "jet/ptk/datScn.hpp"
#include "jet/ptk/PtkTex.hpp"
#include "jet/ptk/cJetPtkTex.hpp"

namespace jet::ptk
{

class cJetPtkData
{
private:
	std::vector<byte> m_pPtk;

	std::vector<PtkTex*> m_pTexs;

	int m_nScnMax{};

	int m_nEmiMax{};

	int m_nGenMax{};

	std::vector<datScn> m_datScnList;

	std::vector<datGen> m_datGenList;

	void Init();

	void Term();

public:
	cJetPtkData();

	cJetPtkData(const std::vector<byte>& pPtk, cJetPtkTex* pTex);

	virtual ~cJetPtkData();

	void SetData(const std::vector<byte>& pPtk, cJetPtkTex* pTex);

	std::vector<byte>& pDatPtk();

	PtkTex* pTex(uint n);

	datScn* pDatScn(uint nScnNo = 0u);

	datEmi* pDatEmi(uint nEmiNo = 0u);

	datGen* pDatGen(uint nGenNo);

	datScn& pDatScnCopy(uint nScnNo = 0u);

	datGen& pDatGenCopy(uint nGenNo);

	int& nScnMax();

	int& nEmiMax();

	int& nGenMax();
};

}
