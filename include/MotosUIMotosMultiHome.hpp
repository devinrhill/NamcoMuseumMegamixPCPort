#pragma once

#include "common.hpp"
#include "MotosUIBase.hpp"

class MotosUIMotosMultiHome : public MotosUIBase
{
public:
	enum EButtonType
	{
		Back = 0,
		Player2 = 1,
		Player3 = 2,
		Player4 = 3
	};

	enum EAnim
	{
		Highlight = 0,
		Push = 1
	};

	enum EBaseAnim
	{
		In = 0,
		Out = 1
	};

private:
	Animation* m_baseAnim{};

	std::vector<Animation*> m_buttonAnim{};

	Image* m_backButtonImage{};

	Text* m_backButtonText{};

	Color m_backButtonDefaultColor{};

	Color m_backButtonTextDefaultColor{};

	static std::vector<string> scAnimName;

	static std::vector<string> scBaseAnimName;

	void Awake();

public:
	void PlayAnim(EButtonType type, EAnim anim);

	void PlayBaseAnim(EBaseAnim anim);

	bool IsPlayAnim(EButtonType type, EAnim anim);

	bool IsPlayBaseAnim(EBaseAnim anim);

	void ResetAnim(EButtonType type, bool force = false);

protected:
	void OnPause() override;

	void OnResume() override;
};
