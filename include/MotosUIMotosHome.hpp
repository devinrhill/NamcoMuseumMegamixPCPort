#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"

class MotosUIMotosHome : public MotosUIBase
{
public:
	enum class EType
	{
		Single = 0,
		Multi = 1
	};

	enum class EButtonType
	{
		Rule = 0,
		Multi = 1,
		Back = 2
	};

	enum EAnim
	{
		Highlight = 0,
		Push = 1
	};

	enum EBaseAnim
	{
		In = 0
	};

private:
	Animation* m_baseAnim{};

	std::vector<Animation*> m_buttonAnim{};

	Text* m_mulitiButtonText{};

	static std::vector<string> scAnimName;

	static std::vector<string> scBaseAnimName;

public:
	void Init(EType type);

	void PlayAnim(EButtonType type, EAnim anim);

	void PlayBaseAnim(EBaseAnim anim);

	bool IsPlayAnim(EButtonType type, EAnim anim);

	bool IsPlayBaseAnim(EBaseAnim anim);

	void ResetAnim(EButtonType type, bool force = false);

	void ResetBaseAnim(bool force = false);

protected:
	void OnPause() override;

	void OnResume() override;
};
