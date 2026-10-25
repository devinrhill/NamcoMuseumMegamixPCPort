#pragma once

#include "common.hpp"

// [Serializable]
class RtpaParam : public ScriptableObject
{
public:
	// [Serializable]
	class AnimParam
	{
	public:
		int m_frame{};

		int m_texIdx{};
	};

	bool m_isLoop{};

	int m_maxFrame{};

	string m_materialName{};

	std::vector<Texture*> m_texureList;

	std::vector<AnimParam*> m_animParamList;
};
