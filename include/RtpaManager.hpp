#pragma once

#include "common.hpp"
#include "RtpaParam.hpp"
#include "RtpaPlayer.hpp"

class RtpaManager : public MonoBehaviour
{
public:
	std::vector<RtpaPlayer*> m_rtpaPlayerList;

	void RegistRtpa(RtpaParam* param);

	void SetFrame(float time);

	void SetSpeed(float speed);

	void UpdateFrame();

private:
	// [ContextMenu("子のRtpaPlayerを取得")]
	void SetChildRtpaPlayer();
};
