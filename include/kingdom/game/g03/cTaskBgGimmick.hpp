#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmick : public cTaskBgHit
{
public:
	enum EAct : uint
	{
		eACT_NONE = 1u,
		eACT_ROTATE_L = 2u,
		eACT_ROTATE_R = 3u,
		eACT_ANM_CHR = 4u,
		eACT_MASK = 255u,
		eACT_FLAG_FULLCTRL = 1073741824u,
		eACT_FLAG_ANMPAUSE = eACT_FLAG_FULLCTRL,
		eACT_FLAG_ANMSTOP = 2147483648u
	};

	enum EGimmick
	{
		eGIMMICK_TYPE_UNKNOWN = 0,
		eGIMMICK_TYPE_TRAIN = 1,
		eGIMMICK_MAX = 2
	};

private:
	uint m_nActFlag{};

	uint m_nRotFrameMax{};

	uint m_nRotFrame{};

	uint m_nGimmickType{};

	Vector4 m_vDefaultRotate{};

public:
	uint& nActFlag();

	virtual uint GetRotFrameMax();

	cTaskBgGimmick(ResNode* const& node0, ResNode* const& node1);

	// length of the half extents of this gimmick's model volume (DOL: nw4r ScnObj::GetBoundingVolume 0x802f6400 of the scene model, then
	// half the size per axis and PSVECDotProduct + sqrt, inlined in cTaskBgGimmickAddinSnd::Init 0x8014e128 and PistonMan::RegistGimmick
	// 0x801527c8). Natively the MDL0 header's volume (UNVERIFIED that the nw4r volume is the unskinned header box for these models).
	float ModelHalfDiagonal();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	bool BaseExecute(cJetTask* pParam);

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void GetUserData(ResNode* const& node);

protected:
	void ReleaseTerm() override { cTaskBgGimmick::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
