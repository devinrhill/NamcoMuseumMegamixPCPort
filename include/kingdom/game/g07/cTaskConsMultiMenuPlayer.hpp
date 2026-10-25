#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/sMatchingData.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuPlayer : public cTaskConsMenuBase
{
private:
	enum EMiiWnd
	{
		eMii_Wnd_CenterMiddle = 0,
		eMii_Wnd_LeftTop = 1,
		eMii_Wnd_RightBottom = 2
	};

protected:
	enum EAnita
	{
		eANITA_WND = 0,
		eANITA_MII_WND = 1,
		eANITA_MAX = 2
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

public:
	static constexpr int FACE_ICON_TEX_SIZE = 128;

private:
	static const int FONT_BUF_SIZE;

	static const float FONT_SIZE;

	static const int SET_FONT_POS_PAT;

	static const uint SET_FONT_POS_LAY[4];

	static const int SET_MII_WND_PAT[4];

	static const uint SET_MII_WND_LAY[3];

	uint m_nPlayerNo{};

	sMatchingData::InfoData* m_pInfo{};

protected:
	uint& nPlayerNo();

	sMatchingData::InfoData*& pInfo();

public:
	virtual ~cTaskConsMultiMenuPlayer();

	cTaskConsMultiMenuPlayer(cTaskConsMenuManBase* const& pConsMan, const uint& nPlayerNo, sMatchingData::InfoData* pInfo);

	void SetData();

protected:
	virtual int nGetWndAnmNo(int nAnmState);

	bool AppearExec() override;

	void WaitExec() override;

public:
	void Draw(int nStatus, cJetTask* pParam) override;

	bool Execute(cJetTask* pParam) override;
};

}
