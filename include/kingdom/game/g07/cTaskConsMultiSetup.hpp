#pragma once

#include "common.hpp"
#include <string>
#include <vector>
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "MotosUIMotosStartText.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/game/g07/cTaskConsPacIconSingleSetting.hpp"
#include "kingdom/game/g07/cTaskConsPlayerNumber.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiSetup : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_SEL_BAR = 0,
		eANM_ALMIGHTY = 1,
		eANM_MESSAGE = 2,
		eANM_ALMIGHTY_UP = 3,
		eANM_ALMIGHTY_LECTURE = 4,
		eANM_SETTING = 5,
		eANM_PACMAN_ICON_L = 6,
		eANM_ROUND = 7,
		eANM_1P = 8,
		eANM_2P = 9,
		eANM_3P = 10,
		eANM_4P = 11,
		eANM_WINNER_PLAYER = 12,
		eANM_MAX = 13
	};

	static constexpr int MAX_CONSOLE_SETUP_NUMBER = 3;

	static constexpr int MAX_CONSOLE_SETUP_PLNO = 2;

	cTaskConsPacIconSingleSetting* m_pPacIcon{};

	cTaskConsPlayerNumber* m_pPlayerNo[2]{};

	cTaskConsItemNumber* m_pItemNumber{};

	cTaskConsItemNumber* m_pRoundNumber{};

	Ref<int>* m_aExecNumber = new Ref<int>();

	Ref<int>* m_aExecRoundNumber = new Ref<int>();

	static const uint FONT_SIZE;

	static const float HINT_POS[2];

	static const float PLAYER_NO_POS1[2];

	static const float PLAYER_NO_POS2[2];

	static const float FONT_POS[2];

	static const float FONT_POS_WIN[2];

	// the font writer +0x21c (Init 0x80100e0c: font KingSys +0xa68, size 28): Execute queues the names (0x80016e88, mode 2 = baseline, centred), Draw
	// 0x80102190 draws the queue. Natively Execute collects them here and Draw prints them.
	struct sName
	{
		float x, y;
		std::string text;
		uint color;
	};
	std::vector<sName> m_names;
	void QueueName(float x, float y, const std::string& text, uint color, uint shadow);

public:
	MotosUIMotosStartText* UnityUIStart{};

	cTaskConsMultiSetup();

	static cTaskAnita* Construct();

private:
	void Init();

public:
	void Term();

	void Setup();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsMultiSetup::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
