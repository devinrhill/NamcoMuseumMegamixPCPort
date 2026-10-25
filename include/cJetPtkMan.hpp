#pragma once

#include "common.hpp"
#include "cJetPtkHnd.hpp"
#include "jet/ptk/cJetPtkData.hpp"
#include "jet/ptk/cJetPtkDrawList.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/cJetPtkEmiAccess.hpp"
#include "jet/ptk/cJetPtkGen.hpp"
#include "jet/ptk/cJetPtkGenAccess.hpp"
#include "jet/ptk/cJetPtkPtc.hpp"
#include "jet/ptk/cJetPtkPtcAccess.hpp"
#include "jet/ptk/cJetPtkTex.hpp"
#include "jet/ptk/cJetPtkWork.hpp"

class cJetPtkMan
{
private:
	enum eEndianCheck
	{
		eEndian_Little = 0,
		eEndian_Error = 1
	};

	static constexpr float JET_FACTOR_MAX = 1.0f;

	static constexpr float JET_FACTOR_MIN = 0.8f;

	bool m_bFactorFlag{};

	float m_fFactorMax{};

	float m_fFactorMin{};

	uint m_nSlotMax{};

	std::vector<jet::ptk::cJetPtkData*> m_pPtkData;

	jet::ptk::cJetPtkWork* m_pPtkWork{};

	jet::ptk::cJetPtkTex* m_Texture;

	std::vector<jet::ptk::cJetPtkEmi*> m_EmiList;

	std::vector<jet::ptk::cJetPtkGen*> m_GenList;

	std::vector<jet::ptk::cJetPtkPtc*> m_PtcList;

	jet::ptk::cJetPtkDrawList* m_DrawList;

	jet::ptk::cJetPtkEmiAccess* m_EmiAces;

	jet::ptk::cJetPtkGenAccess* m_GenAces;

	jet::ptk::cJetPtkPtcAccess* m_PtcAces;

	std::vector<int> m_RemoveList;

public:
	float _debug_life_time{};

private:
	void Init();

public:
	virtual void Term();

private:
	float _LoadFactor(float fLoadFactor);

public:
	cJetPtkMan();

	cJetPtkMan(uint nSlot, uint nEmi, uint nGen, uint nPtc);

	virtual ~cJetPtkMan();

	virtual void Create(uint nSlot, uint nEmi, uint nGen, uint nPtc);

	void Release();

	void SetData(uint nSlotNo, std::vector<byte>& pPtkBin);

	void SetOverrideAlphaTest(uint nSlotNo);

	bool Generate(cJetPtkHnd* pHnd, uint nSlotNo, bool bOneTime, bool bDepend, bool bClipping, float fDelay, int nLightSetIdx, int nFogIdx, int drawLayer, float loadFactor = -1.0f);

	void Genocide();

	void AnimUpdate(float fStep = 1.0f);

	virtual void Draw(Camera* camera, Vector4 vPos, Vector4 vTarget, float fFovy = 0.0f);

	bool IsFactor();

	void OffFactor();

	void OnFactor();

	void Factor(float fMin, float fMax);

	float fFactorMin();

	float fFactorMax();

	uint nEmi();

	uint nGen();

	uint nPtc();

	uint nUseEmi();

	uint nUseGen();

	uint nUsePtc();
};
