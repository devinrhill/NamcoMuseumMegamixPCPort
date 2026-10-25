#pragma once

#include "common.hpp"

class KingdomAnimManager : public MonoBehaviour
{
private:
	// [SerializeField]
	std::vector<Animation*> m_childAnimList;

	bool m_bPrevPause{};

	void Update();

	void OnPause();

	void OnResume();

	// [ContextMenu("子のAnimationを取得")]
	void SetChildAnim();

public:
	void SetChildAnim(bool self);

	void PlayAnim(float time);
};
