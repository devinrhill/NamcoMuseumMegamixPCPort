#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "KingdomAnimManager.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskPanel : public cTaskModel
{
private:
	enum eMODEL
	{
		eMODEL_UNION = 0,
		eMODEL_CRACK_1 = 1,
		eMODEL_CRACK_2 = 2,
		eMODEL_CRACK_3 = 3,
		eMODEL_BREAK = 4,
		eMODEL_MAX = 5
	};

	enum ECrackType
	{
		eCrack1 = 0,
		eCrack2 = 1,
		eCrack3 = 2,
		eMax = 3
	};

	Vector4 m_vTrans{};

	Vector4 m_vRotate{};

	Constants::PANEL_INFO* m_Panel{};

	int m_nMapPos[2]{};

	uint m_nID{};

	uint m_nFlag{};

	uint m_nCurModelIdx{};

	Constants::ePANEL_STEP m_nStep{};

	Constants::ePANEL m_nType{};

	cTaskPanel::eMODEL m_nModel{};

	float m_fAnimFrame{};

	bool m_bUpdateMtx{};

	KingdomAnimManager* m_animMgr{};

	int m_crackType{};

	static const int rot[4];

public:
	bool CHECK_PANEL_WARNING(string xxx);

	bool CHECK_PANEL_ASSERT(string xxx);

	cTaskPanel();

	cTaskPanel(Constants::ePANEL type);

	uint& nID();

	uint& nFlag();

	Constants::ePANEL nType();

	void fFrame(float fFrame);

	float fFrame();

	int& nMapPos(uint n);

	Constants::PANEL_INFO*& pPanelInfo();

	virtual bool AnimateModel(uint model);

private:
	void Init();

public:
	void Term() override;

	bool Create(Constants::ePANEL type);

private:
	bool CreatePanelN(Constants::ePANEL type);

	bool CreatePanelX(Constants::ePANEL type);

	bool CreatePanelG(Constants::ePANEL type);

	bool CreatePanelW(Constants::ePANEL type);

public:
	bool Release();

	uint StrikePanel();

	void StepChange(Constants::ePANEL_STEP step);

	void ExecPanel();

	void ExplodePanel();

	void BigExplodePanel();

private:
	cTaskPanel::eMODEL StepModelIdx(Constants::ePANEL type, Constants::ePANEL_STEP step);

	bool ModelChange(cTaskPanel::eMODEL idx);

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void UpdateMatrix();

public:
	Vector4& vTrans();

	float& vTrans(int n);

	Vector4& vRotate();

	float& vRotate(int n);

protected:
	void ReleaseTerm() override { cTaskPanel::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
