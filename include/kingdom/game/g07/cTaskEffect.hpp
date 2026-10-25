#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBPtkMan.hpp"
#include <ESEKind.hpp>
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/MO_EFFECT_INFO.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"

namespace kingdom::game::g07
{

class cTaskEffect : public cTaskPBPtkMan
{
private:
	static const string ptk_tbl[46];

public:
	static constexpr int EMITTER_MAX = 256;

	static constexpr int GENERATOR_MAX = 512;

	static constexpr int PARTICLE_MAX = 2048;

	static constexpr int BOSS_EMITTER_MAX = 512;

	static constexpr int BOSS_GENERATOR_MAX = 512;

	static constexpr int BOSS_PARTICLE_MAX = 4096;

	cKingdomPtkHnd* m_hPtk{};

	cKingdomPtkHnd* m_ahPtk[128]{};

	MO_EFFECT_INFO* m_aEffInfo[128]{};

private:
	float m_fFrameRate{};

	bool m_bPause{};

	uint m_nInfoIdx{};

	Vector4 m_vColPos{};

	bool m_nowFramePlaySEExpl01{};

	bool m_nowFramePlaySEExplPanel{};

	bool m_nowFramePlaySEExpl03{};

	bool m_nowFramePlaySEHitZako03{};

public:
	cTaskEffect();

	virtual ~cTaskEffect();

	float GetFrameRate();

	void SetFrameRate(float rate);

	bool IsPause();

	void Pause();

	void Resume();

private:
	void Init();

	void Term();

public:
	void SetData();

	void AllDetach();

	bool Execute(cJetTask* pParam) override;

	cKingdomPtkHnd* SpawnEffect(int nKind, Vector4 pMPos, cJetTask* pParam, int delay = 0, bool bDead = true, float loadFactor = -1.0f);

	ESEKind nGetColSE(cTaskChar* pCharA, cTaskChar* pCharB);

	void SpawnColEffect(cTaskChar* pChar0, cTaskChar* pChar1);

	Vector4 ColPos(cTaskChar* pChar0, cTaskChar* pChar1, bool Ad = false);

	static void CustumFuncCharge(MO_EFFECT_INFO* pInfo);

	static void CustumFuncPiyori(MO_EFFECT_INFO* pInfo);
};

}
