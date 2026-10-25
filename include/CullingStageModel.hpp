#pragma once

#include "common.hpp"

class CullingStageModel : public MonoBehaviour
{
public:
	std::vector<Renderer*> m_meshList;

	std::vector<int> m_targetMeshIdxList;

	void SetCulling(bool on, bool changeActive = false);
};
