#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomAnita.hpp"

namespace kingdom { class cTaskCursor; }

// The shared "Play again?" console: kingdom::game::cTaskConsRetry (DOL vtable 0x80445148, a cTaskAnita; Galaga's cTaskRetry 0x8044a938 and
// the Pac-N-Roll / Pac-Motos retries derive from it). Ported from main.dol (the converted C# drove a Unity UI instead):
//   Init 0x80148c64 (anitas 2 = text board anim 0, 0 = YES anim 3, 1 = NO anim 8, all from one layout), CreateCursor 0x80148ea4 (a cTaskCursor
//   per player: idle anim 0 / 2 / 4 / 6, A / B / Z (0x860) plays 1 / 3 / 5 / 7, priority 0x8ffffffe), SetInvisible 0x80149018, Execute
//   0x80149110, Start 0x80149b88.
// Anims: YES 3 hidden, 4 shown, 5 focused, 6 decided, 7 out; NO 8 hidden, 9 shown, 10 focused, 11 decided, 12 out; board 0 / 1 / 2.
namespace kingdom::game
{

class cTaskConsRetry : public cTaskAnita
{
public:
	enum eSelect
	{
		eSELECT_NONE = 0,
		eSELECT_YES = 1,
		eSELECT_NO = 2
	};

protected:
	enum eStep
	{
		eSTEP_INVISIBLE = 0,
		eSTEP_IN = 1,
		eSTEP_SELECT = 2,
		eSTEP_DECIDE = 3,
		eSTEP_OUT = 4
	};

private:
	enum EAnita
	{
		eANITA_YES = 0,
		eANITA_NO = 1,
		eANITA_TEXT_BOARD = 2
	};

	enum ECursor
	{
		eCURSOR_MAX = 4
	};

	enum EThrough
	{
		eTHROUGH_WAIT = 20
	};

	cTaskConsRetry::eStep m_nStep{};          // +0x1c8
	cTaskConsRetry::eSelect m_nSelectBuf{};   // +0x1cc the focused button
	cTaskConsRetry::eSelect m_nSelect{};      // +0x1d0 the decision
	uint m_nCursorMax{};                      // +0x1d4
	kingdom::cTaskCursor* m_pCursor[eCURSOR_MAX]{};   // +0x1d8
	uint m_nThroughCnt{};                     // +0x1e8

protected:
	cTaskConsRetry::eStep& nStep();
	cTaskConsRetry::eSelect& nSelectBuf();
	cTaskConsRetry::eSelect& nSelect();
	uint& nCursorMax();

public:
	cTaskConsRetry(string retry_key);
	cTaskConsRetry(string retry_key, string cursor_key, uint cursor_max);
	virtual ~cTaskConsRetry();

	bool IsYes();
	bool IsNo();
	bool IsNone();
	bool IsVisible();
	kingdom::cTaskCursor* pCursor(uint n) { return n < m_nCursorMax ? m_pCursor[n] : nullptr; }

private:
	void Init(string retry_key);                   // DOL 0x80148c64

public:
	void CreateCursor(string cursor_key, uint cursor_max);   // DOL 0x80148ea4

private:
	void SetInvisible();                           // DOL 0x80149018
	bool IsAnim();                                 // the three anitas' animations ended (state bit 0x10000)
	void Focus(eSelect sel);                       // the focus anims of Execute (5 / 9 or 4 / 10), only when they change
	cKingdomAnita* Setup(uint n, string key, int ano = 0, float x = 0.0f, float y = 0.0f, float z = 0.0f);

protected:
	void SetCursorInvisible();
	void SetCursorVisible();

public:
	bool Execute(cJetTask* pParam) override;       // DOL 0x80149110
	void Start();                                  // DOL 0x80149b88
};

}
