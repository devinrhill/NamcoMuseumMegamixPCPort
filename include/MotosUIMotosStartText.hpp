#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"

class MotosUIMotosStartText : public MotosUIBase
{
public:
	enum EType
	{
		Single = 0,
		Multi = 1
	};

private:
	GameObject* m_multiItemObj{};

	GameObject* m_multiSettingObj{};

	Text* m_settingPlayerText{};

	string m_settingPlayerStr{};

public:
	void Init(EType type);

	void SetPlayerNo(int playerNo);
};
