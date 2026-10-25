#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

namespace kingdom::game::g03
{

// The Museum Megamix main menu hub (job 138 = 0x08a00000). DOL kingdom::game::g03::cAppMenu (vtable 0x80389d30, 0xca0 bytes): a cAppG03PacnRollGame
// that runs the castle garden stage g03wXXs04 (round 27) with the sys in top menu mode: Pac-Man rolls up to one of the arcade doors (floor
// face kinds MENU_G00..G07) and the job of that game starts. Constructor body 0x8001014c, Execute 0x80010eb4, selection 0x80012a1c,
// camera focus 0x80012204, node lookup 0x80012d80. Only the hub (current job != 0x08c00000) is ported; the classic menu (job 140) is NOT.
class cAppMenu : public cAppG03PacnRollGame
{
public:
	cAppMenu();

	bool Execute(cJetTask* pParam) override;

private:
	enum { MODE_START = 0, MODE_RUN = 1, MODE_SELECTED = 2, MODE_FELL = 4, MODE_WAIT_JOB = 5 };

	int Select();                    // DOL 0x80012a1c
	void Focus(int idx);             // DOL 0x80012204
	int NearestPoint() const;        // the 7-point distance loop of Execute
	void LoadNodes();                // DOL 0x80012d80
	void ToMode(uint mode);
	float Csv(int row, int col) { return m_csv.F32(row, col, 0.0f); }

	cKingdomCSV m_csv;               // +0xc58: CSV_MENU_PARAM
	cTaskFade* m_pFade{};            // +0xc9c
	uint m_nStep{};                  // +0x70
	int m_nSel{}, m_nSelOld{};       // +0x3e4 / +0x3e8
	bool m_bLanding{};               // +0x3ec: Pac-Man falls in from above and bounces once when he lands (first arrival from the title)
	bool m_bGoalSound{};             // +0x3ed
	float m_vSaved[3]{};             // +0x3f0: where Pac-Man (re)starts
	float m_fSavedAngle{};           // +0x400
	float m_vDoor[9][3]{};           // +0x410: door node translations (waniwani, rallyx, pacnroll, galaga, motos, arcade, grobda)
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
