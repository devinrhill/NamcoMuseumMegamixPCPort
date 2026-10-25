#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "MotosUIPause.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game
{

class cTaskGameBasePause : public cTaskKingdom
{
protected:
	enum EAnita
	{
		eANITA_BG = 0,
		eANITA_ICON_MENU = 1,
		eANITA_ICON_GAME = 2,
		eANITA_PAUSE = 3,
		eANITA_MAX = 4
	};

	enum ESelect
	{
		eSELECT_GAME = 0,
		eSELECT_MENU = 1,
		eSELECT_NONE = 2
	};

	enum EMode
	{
		eMODE_WAIT = 0,
		eMODE_FADE_IN = 1,
		eMODE_EXEC = 2,
		eMODE_DECIDE = 3,
		eMODE_GAME_OUT = 4,
		eMODE_MENU_OUT = 5
	};

	uint m_nMenuJob{};

	uint m_nSelect{};

	cTaskFade* m_pTaskFade{};

	bool m_bPause{};

	sMatchingData* m_pMatchingData{};

	uint m_nCtrlNum{};

	// DOL 0x80147a64: one pointer cursor per controller (+0x1d8, 4 entries), shown while the pause is open
	kingdom::cTaskCursor* m_pCursor[4]{};

	void ShowCursors(bool on);

public:
	cKingdomAnitaVector* AnitaVector{};

	MotosUIPause* UnityUI{};

	cKingdomAnita* pAnita(uint n);

	cTaskGameBasePause();

	cTaskGameBasePause(string key, string cursor, sMatchingData* pMatchingData = nullptr);

	virtual ~cTaskGameBasePause();

	virtual void Free();

	uint& nMenuJob();

	bool IsPause();

private:
	void Init();

	void Term();

public:
	bool RegistData(string key, string cursor, sMatchingData* pMatchingData = nullptr);

	bool Start();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
