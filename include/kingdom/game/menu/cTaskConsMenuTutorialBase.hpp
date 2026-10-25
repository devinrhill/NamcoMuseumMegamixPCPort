#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "MotosUIRuleButtonSet.hpp"
#include "MotosUIRuleImage.hpp"
#include "MotosUIRuleText.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/MsgTable.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

namespace kingdom::game::menu
{

class cTaskConsMenuTutorialBase : public cTaskAnita
{
private:
	enum EState
	{
		eSTATE_APPEAR = 0,
		eSTATE_WAIT = 1,
		eSTATE_NEXT = 2,
		eSTATE_PREV = 3,
		eSTATE_CLOSE = 4,
		eSTATE_DELETE = 5,
		eSTATE_DELETE_END = 6,
		eSTATE_MAX = 7
	};

	enum EPoint
	{
		ePOINT_NONE = 0,
		ePOINT_NEXT = 1,
		ePOINT_PREV = 2,
		ePOINT_CLOSE = 3,
		ePOINT_MAX = 4
	};

	enum EAnitaID
	{
		eANT_BG = 0,
		eANT_PROCESS = 1,
		eANT_PAGE_NOW = 2,
		eANT_PAGE_MAX = 3,
		eANT_SLASH = 4,
		eANT_ORIGINAL = 5,
		eANT_COVER = 15,
		eANT_CLOSE = 16,
		eANT_PREV = 17,
		eANT_NEXT = 18,
		eANT_MAX = 19
	};

public:
	enum EAnimKind
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_DELETE = 4,
		eANM_MAX = 5
	};

	static constexpr uint ORIGINAL_DATA_NUM = 10u;

private:
	static constexpr int BG_ANM_CHANGE_NEXT = 42;

	static constexpr int BG_ANM_CHANGE_PREV = 42;

	int BG_ANM_LIST[5] = {37, -1, -1, -1, 43};

	int PROCESS_ANM_LIST[5] = {35, -1, -1, -1, 36};

	static constexpr int COVER_ANM_END = 41;

	int COVER_ANM_LIST[5] = {39, -1, -1, -1, 40};

	int PREV_ANM_LIST[5] = {25, 26, 27, 28, 29};

	int NEXT_ANM_LIST[5] = {30, 31, 32, 33, 34};

	int CLOSE_ANM_LIST[5] = {20, 21, 22, 23, 24};

	int SLASH_ANM_LIST[5] = {18, -1, -1, -1, 19};

	int NUMBER_ANM_LIST[9][5]{};

	static constexpr int CLOSE_HIT_PAT = 3;

	static constexpr int PREV_HIT_PAT = 5;

	static constexpr int NEXT_HIT_PAT = 4;

	static constexpr int PAGE_SET_PAT = 0;

	uint PAGE_SET_LAY[2] = {0u, 2u};

	static constexpr int SLASH_SET_PAT = 0;

	static constexpr uint SLASH_SET_LAY = 1u;

	static constexpr int FONT1_SET_PAT = 1;

	static constexpr int FONT2_SET_PAT = 2;

	static constexpr int FONT_BUF_SIZE = 6144;

	int m_nState{};

	int m_nStep{};

	int m_nPointObjOld{};

	uint m_nMaxPageNo{};

	uint m_nNowPageNo{};

	sTutorialData* m_pTutorialData{};

	Vector4 m_vFontPos[2]{};

	int m_nCsvRow{};

	cKingdomCSV* m_CsvText{};
	kingdom::cTaskCursor* m_pCursor{};   // +0x1e0: the pointer sprite (also the pointer source of DpdSelect)
	int m_nPointSel{};                   // +0x1d0 in the DOL's DpdSelect: 0 none, 1 next, 2 previous, 3 close
	kingdom::MsgTable m_Text;            // pData()->TextDataName (BIN_MENU_TEXT_TUTORIAL): row 0 = size, row n = page text

public:
	void SetCursor(kingdom::cTaskCursor* pCursor) { m_pCursor = pCursor; }
	MotosUIRuleButtonSet* UnityUIButton{};

	MotosUIRuleImage* UnityUIImage{};

	MotosUIRuleText* UnityUIText{};

private:
	uint _nGetOrgAntNo(uint nNo = std::numeric_limits<uint>::max());

protected:
	sTutorialPage* pGetPageData(uint nPageNo);

	uint& nMaxPageNo();

	uint& nNowPageNo();

	sTutorialData*& pData();

	Vector4& vFontPos(int n);

	int& nCsvRow();

	cKingdomCSV*& CsvText();

public:
	virtual ~cTaskConsMenuTutorialBase();

	virtual bool IsDeleteEnd();

	cTaskConsMenuTutorialBase();

	cTaskConsMenuTutorialBase(string CommonKey, sTutorialData* pTutorialData);

protected:
	void _cTaskConsMenuTutorialBase(string CommonKey, sTutorialData* pTutorialData);

public:
	virtual void RegistData(string CommonKey, sTutorialData* pTutorialData);

	void Start();

protected:
	virtual void AllWaitState();

	virtual bool DpdSelect();

	virtual void KeySelect();

public:
	virtual void Select();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	bool IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID anita);

	bool IsButtonAnimEnd(cTaskConsMenuTutorialBase::EAnitaID anita);

	bool ButtonVisibleValue(bool visible);

	bool IsVisibleImage(uint pageNo);

	bool ImageVisibleValue(bool visible);

	bool TextVisibleValue(bool visible);
};

}
