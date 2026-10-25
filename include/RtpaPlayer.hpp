#pragma once

#include "common.hpp"
#include "RtpaParam.hpp"

class RtpaPlayer : public MonoBehaviour
{
public:
	RtpaParam* m_targetFile{};

	std::vector<Renderer*> m_rendererList;

private:
	RtpaParam* m_rtpaParam{};

	Texture* m_prevTex{};

	std::vector<Material*> m_tmpMaterialList;

public:
	float AnimTime{};

	float AnimSpeed = 1.0f;

private:
	void Awake();

	void OnDestroy();

public:
	void RegistRtpa(RtpaParam* param, bool force);

	void SetFrame(float time);

	void UpdateFrame();

private:
	void UpdateTime();

	void ClampTime();

	void SetTexture();

	// [ContextMenu("Rendererを取得")]
	void SetTargetRenerer();
};
