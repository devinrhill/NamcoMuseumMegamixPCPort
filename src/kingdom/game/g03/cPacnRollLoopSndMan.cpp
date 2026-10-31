#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"

#include <algorithm>
#include <cstdint>

#include "SoundManager.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "snd/Mixer.hpp"

namespace kingdom::game::g03
{

namespace
{
// BasicSound::Pause(flag, 10) (0x8002647c / 0x800264e4): the fade counts game frames (nw4r updates once per frame)
constexpr int PAUSE_FADE_MS = 10 * 1000 / 60;

int VoiceOf(GameObject* handle)
{
	return (int)(reinterpret_cast<std::uintptr_t>(handle) - 1u);
}
}

cPacnRollLoopSndMan* cPacnRollLoopSndMan::s_pInstance = nullptr;

// DOL 0x800b3a00 [audited: described by the 43rd-session pass] (inline in cAppG03PacnRollGame::Init)
cPacnRollLoopSndMan::cPacnRollLoopSndMan()
{
	SetName("cPacnRollLoopSndMan");
	s_pInstance = this;
}

// DOL 0x800ba60c [audited: resume with fade on pause end, drop stopped handles vs decomp]
cPacnRollLoopSndMan::~cPacnRollLoopSndMan()
{
	Term();
}

void cPacnRollLoopSndMan::Term()
{
	if (s_pInstance == this) s_pInstance = nullptr;
}

bool cPacnRollLoopSndMan::CanPlay()
{
	return s_pInstance == nullptr || !s_pInstance->m_bStop;
}

GameObject* cPacnRollLoopSndMan::Track(GameObject* handle)
{
	if (s_pInstance != nullptr && handle != nullptr)
	{
		std::vector<GameObject*>& list = s_pInstance->m_list;
		if (std::find(list.begin(), list.end(), handle) == list.end()) list.push_back(handle);
	}
	return handle;
}

int cPacnRollLoopSndMan::Track(int voice)
{
	if (voice >= 0) Track(reinterpret_cast<GameObject*>((std::uintptr_t)voice + 1u));
	return voice;
}

void cPacnRollLoopSndMan::PauseAll()
{
	if (s_pInstance == nullptr) return;
	for (GameObject* handle : s_pInstance->m_list) snd::Pause(VoiceOf(handle), true, PAUSE_FADE_MS);
	s_pInstance->m_bPause = true;
}

// the handles stop with 10 ms = 0 frames (0x800ba364 -> 0x80026318, cKingdomSys::nMSecToFrame): at once
void cPacnRollLoopSndMan::StopAll()
{
	if (s_pInstance == nullptr) return;
	for (GameObject* handle : s_pInstance->m_list) SoundManager::get_SE()->Stop(handle);
	s_pInstance->m_list.clear();
	s_pInstance->m_bStop = true;
}

// DOL 0x800ba518 [audited: resume with fade on pause end, drop stopped handles vs decomp]
bool cPacnRollLoopSndMan::Execute(cJetTask* pParam)
{
	if (m_bPause && !Constants::IS_PAC_PAUSE())
	{
		for (GameObject* handle : m_list) snd::Pause(VoiceOf(handle), false, PAUSE_FADE_MS);
		m_bPause = false;
	}
	m_list.erase(std::remove_if(m_list.begin(), m_list.end(), [](GameObject* handle) { return !SoundManager::get_SE()->IsPlaying(handle); }), m_list.end());
	return true;
}

// DOL 0x800902b8 [audited] (blr)
void cPacnRollLoopSndMan::Draw(int nStatus, cJetTask* pParam)
{
}

}
