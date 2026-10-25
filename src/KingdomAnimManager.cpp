#include "KingdomAnimManager.hpp"

#include "InGamePadManager.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

using namespace kingdom::game::g03;
using namespace kingdom::game::g07;

void KingdomAnimManager::Update()
{
	if (KingdomGlobal::KingSys == nullptr)
	{
		return;
	}
	bool flag = false;
	flag = ((kingdom::game::g03::Constants::PACN_APP() != nullptr) ? kingdom::game::g03::Constants::IS_PAC_PAUSE() : ((KingdomGlobal::g_pMotosGame == nullptr) ? InGamePadManager::get_Disable() : (kingdom::game::g07::Constants::SYSTEM_IS_PAUSE() || KingdomGlobal::g_pMotosGame->nStep(0) == 1)));
	if (flag != m_bPrevPause)
	{
		if (flag)
		{
			OnPause();
		}
		else
		{
			OnResume();
		}
		m_bPrevPause = flag;
	}
}

void KingdomAnimManager::OnPause()
{
	for (Animation* childAnim : m_childAnimList)
	{
		if (childAnim != nullptr && false) // TODO(unity): childAnim.clip != null
		{
			// TODO(unity): childAnim[childAnim.clip.name].speed = 0f;
		}
	}
}

void KingdomAnimManager::OnResume()
{
	for (Animation* childAnim : m_childAnimList)
	{
		if (childAnim != nullptr && false) // TODO(unity): childAnim.clip != null
		{
			// TODO(unity): childAnim[childAnim.clip.name].speed = 1f;
		}
	}
}

void KingdomAnimManager::SetChildAnim()
{
	SetChildAnim(false);
}

void KingdomAnimManager::SetChildAnim(bool self)
{
	// TODO(unity): GetComponentsInChildren(m_childAnimList);
	if (!self)
	{
		// TODO(unity): m_childAnimList.RemoveAll((Animation v) => v.gameObject == base.gameObject);
	}
}

void KingdomAnimManager::PlayAnim(float time)
{
	for (Animation* childAnim : m_childAnimList)
	{
		if (childAnim != nullptr && false) // TODO(unity): childAnim.clip != null
		{
			// TODO(unity): childAnim[childAnim.clip.name].time = time;
			// TODO(unity): childAnim.Play();
		}
	}
}
