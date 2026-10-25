#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "kingdom/sMatchingData.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::menu
{

class cTaskConsStart : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_START = 0
	};

protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_DELETE = 4,
		eANM_DECIDE_DELETE = 5,
		eANM_MAX = 6
	};

private:
	static constexpr int FONT_BUF_SIZE = 1024;

	static constexpr uint FONT_SIZE = 18u;

	static constexpr uint MIN_FONT_SIZE = 16u;

	static constexpr int SET_PAT = 2;

	uint SET_LAY[4] = {0u, 1u, 2u, 3u};

	int HIT_PAT[4] = {17, 18, 19, 20};

	static constexpr int STR_SET_PAT = 31;

	uint STR_SET_LAY[2] = {0u, 2u};

	static inline const string NODE_NAME_BASE = "set_1%d";

	uint m_nPlayerNo{};

	uint m_nCtrlNo{};

	int m_nHitPat{};

	bool m_bSelectFlag{};

	sMatchingData::InfoData* m_pInfo{};

	Vector4 m_vTrans{};

	Vector4 m_vMiiNamePos{};

	float m_fMaxStringWidth{};

protected:
	uint& nPlayerNo();

	uint& nCtrlNo();

	int& nHitPat();

	bool& bSelectFlag();

	sMatchingData::InfoData*& pInfo();

	Vector4& vTrans();

	Vector4& vMiiNamePos();

	float& fMaxStringWidth();

public:
	virtual ~cTaskConsStart();

	bool IsSelectFlag();

	uint nGetPlayerNo();

	void nSetCtrlNo(uint nNo);

	uint nGetCtrlNo();

	cTaskConsStart(int nId, string AnitaKey, string SetKey, uint nPlayerNo);

	void RegistMii(sMatchingData::InfoData* pInfo);

protected:
	virtual int nGetStartAnmNo(uint nState);

public:
	bool IsDpdHit(float x, float y) override;

	virtual void Cancel();

protected:
	bool AppearProc() override;

	void WaitProc() override;

	void PointProc() override;

	bool DecideProc() override;

	bool DeleteProc() override;

public:
	bool Execute(cJetTask* pParam) override;
};

}
