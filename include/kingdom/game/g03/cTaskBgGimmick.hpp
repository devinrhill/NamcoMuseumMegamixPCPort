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

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	bool BaseExecute(cJetTask* pParam);

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void GetUserData(ResNode* const& node);
};

}
