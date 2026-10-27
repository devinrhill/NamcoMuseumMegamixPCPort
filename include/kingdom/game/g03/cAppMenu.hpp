#pragma once

#include "common.hpp"
#include "kingdom/MsgTable.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

namespace kingdom::game::g03
{

// The Museum Megamix main menu hub (job 138 = 0x08a00000). DOL kingdom::game::g03::cAppMenu (vtable 0x80389d30, 0xca0 bytes): a cAppG03PacnRollGame
// that runs the castle garden stage g03wXXs04 (round 27) with the sys in top menu mode: Pac-Man rolls up to one of the arcade doors (floor
// face kinds MENU_G00..G07) and the job of that game starts. Constructor body 0x8001014c, Execute 0x80010eb4, selection 0x80012a1c,
// camera focus 0x80012204, node lookup 0x80012d80. The same class is the classic menu (job 140 = MENU_CLASSIC, round 26 = g03wXXs03, the arcade
// hall): the current job selects the branches; there Pac-Man rolls up to one of the 18 cabinets, a panel shows the game's name + hi score and
// its description (0x80011c9c / 0x80011ac8), A starts it (job 141 with the game index), the exit area goes back to the hub.
class cAppMenu : public cAppG03PacnRollGame
{
public:
	cAppMenu();

	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x8001292c

private:
	enum { MODE_START = 0, MODE_RUN = 1, MODE_SELECTED = 2, MODE_FELL = 4, MODE_WAIT_JOB = 5 };

	int Select();                    // DOL 0x80012a1c
	void Focus(int idx);             // DOL 0x80012204
	int NearestPoint() const;        // the 7-point distance loop of Execute (8 = none; always 8 in the classic menu)
	void ClassicInit();              // the classic branch of the ctor 0x8001014c (arrival, panel anitas, description CSV, hi scores)
	void ClassicPanel();             // DOL 0x80011c9c
	void ClassicText();              // DOL 0x80011ac8
	void ClassicDecide();            // the classic A press of Execute mode 1
	void LoadHiScores();             // the save block 6 tail of the ctor
	void LoadNodes();                // DOL 0x80012d80
	void ToMode(uint mode);
	float Csv(int row, int col) { return m_csv.F32(row, col, 0.0f); }
	float CsvOld(int row, int col) { return m_csvOld.F32(row, col, 0.0f); }

	bool m_bClassic{};               // current job == MENU_CLASSIC (the DOL asks the job each time)
	cKingdomCSV m_csv;               // +0xc58: CSV_MENU_PARAM
	cKingdomCSV m_csvOld;            // +0xc7c: menu_old_param.csv
	cKingdomAnitaVector m_anita;     // +0x3d0: MAIN_MENU_ANITA_OLDGAME x 11 (0 the game, 1 "HI-SCORE", 2..10 the digits)
	MsgTable m_msg;                  // +0x7e0: CSV_MSG_OLD_INFO (the descriptions)
	std::string m_text;              // +0x7e4
	float m_vText[2]{};              // +0x7d0: description origin
	std::uint8_t m_nTextAlpha{};     // +0xbe4
	bool m_bPanel{};                 // +0xbe5: the classic panel / text are drawn this frame
	bool m_bDecided{};               // +0x3ee
	uint m_nPanelGame{};             // +0x6c0
	uint m_nPanelWait{};             // +0x6c4: frames on the same cabinet (the panel appears at 21)
	uint m_nPanelScore{};            // +0x6c8
	uint m_nHiScore[18]{};           // +0x6cc
	cTaskFade* m_pFade{};            // +0xc9c
	uint m_nStep{};                  // +0x70
	int m_nSel{}, m_nSelOld{};       // +0x3e4 / +0x3e8
	bool m_bLanding{};               // +0x3ec: Pac-Man falls in from above and bounces once when he lands (first arrival from the title)
	bool m_bGoalSound{};             // +0x3ed
	float m_vSaved[3]{};             // +0x3f0: where Pac-Man (re)starts
	float m_fSavedAngle{};           // +0x400
	float m_vDoor[18][3]{};          // +0x410: door node translations (waniwani, rallyx, pacnroll, galaga, motos, arcade, grobda) / the 18 cabinets
	float m_vDoorRot[18][3]{};       // +0x5a0: their rotations (radians)
	float m_vPoint[7][3]{};          // +0x530: the "_center" nodes
	float m_vTarget[3]{};            // +0xbf0: camera look-at point (smoothed)
	float m_vMenuCam[3]{};           // +0xc00: camera offset (smoothed)
	float m_vFocus[3]{};             // +0xc10: Pac-Man follow point (smoothed)
	float m_fFocusProgress{1.0f};    // +0xc20
	int m_nFocusIdx{8};              // +0xc24
	float m_vFocusOffset[3]{};       // +0xc30: camera offset of the focused door
	int m_nBgmVoice{-1};             // +0xc40
};

}
