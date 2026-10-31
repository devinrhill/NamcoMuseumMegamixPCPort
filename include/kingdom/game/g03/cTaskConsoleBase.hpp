#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsoleBase : public cTaskAnita
{
private:
	Vector4 m_vTrans{};

	uint m_nState{};

	uint m_nStateNew{};

	uint m_nStep{};

	uint m_nStepNew{};

public:
	cTaskConsoleBase();

	virtual ~cTaskConsoleBase();

	uint nGetState();

	uint nGetStep();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool Regist(uint anita, string key);

	bool Regist(uint anita, string key, int pat, int lay = 0);

	bool RegistNotSetPos(uint anita, string key);

	bool SetPosition(uint anita, int pat, int lay = 0);

	bool SetAnim(uint anita, int ano);

	bool ChangeAnim(uint anita, int ano);

	bool IsAnimEnd(uint anita);

	bool IsVisible(uint anita);

	void Visible(uint anita);

	void Invisible(uint anita);

	void SetState(uint num);

	void SetStep(uint num);

	void UpdateState();

	bool IsManagerVisible();

	// DOL 0x8018a200 (the cursors' IsInScreen): a world position through the game camera's view / projection and the 640 x 480 viewport; false when it is
	// behind the camera, beyond the far plane or outside the screen. out = the HUD (Anita) coordinates: x right, y up, origin at the screen centre
	static bool WorldToHud(const Vector4& world, Vector4& out);

protected:
	void ReleaseTerm() override { cTaskConsoleBase::Term(); cTaskAnita::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
