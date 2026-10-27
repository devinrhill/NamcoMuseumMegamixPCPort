#pragma once

#include "common.hpp"
#include "cJetTask.hpp"
#include "gfx/ModelDraw.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "wii/Anim.hpp"

namespace kingdom::game::g03
{

// The rotating Pac-Man of the Pac-N-Roll stage select (DOL vtable 0x80390e7c; the DOL class is a cTaskModelBase). The C# port places him
// from constants of Unity's Front3D camera; here he stands on node `set_pac` of BIN_MENU_SET_MDL (translation 0.02/0.045/0, rotY 7.5,
// scale 0.86) and is seen through the camera / light of BIN_MENU_SCA. The model task layer (cTaskModel / cKingdomModel) is not ported, so
// this task owns its gfx::ModelDraw like kingdom::menu::TitleScene does.
// UNVERIFIED: that placement (DOL not traced), camera field of view and light handling.
class cTaskMenuPacman : public cTaskKingdom
{
protected:
	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_APPEAR = 1,
		eSTATE_APPEAR_END = 2,
		eSTATE_MAX = 3
	};

public:
	float SCALE_ADD_VAL = 0.075f;

private:
	int m_nState{};
	int m_nStep{};
	float m_fScale{};
	float m_fFrame{};
	bool m_bLoaded{};

	gfx::ModelDraw m_pac;
	wii::ChrAnim m_motion;
	wii::CameraResult m_cam;
	float m_nodePos[3]{}, m_nodeRotY{}, m_nodeScale{1.0f};

protected:
	int& nState();
	int& nStep();
	float& fScale();

public:
	cTaskMenuPacman();

	void Term();

	virtual void AppearState();
	virtual bool IsAppearEndState();
	virtual void WaitState();
	virtual void RegistData();

	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskMenuPacman::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
