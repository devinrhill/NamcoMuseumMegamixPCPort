#include "kingdom/game/g01/cTaskConsoleMultiPlayer.hpp"

#include "kingdom/game/g01/cTaskConsoleResultMulti.hpp"

namespace kingdom::game::g01
{

// DOL 0x800d740c [audited: +0x26c object child +0x7c = 0 not ported, as the base]: step 0 waits for the result console's state 5 (0x800eb6e0), then the panel (anita 0, wait + 2) and the digits (0x800d5aa0 kind 2) go
// out; then done when anitas 0..0xc are hidden or at their end (the base's ResultExec 0x800d6444 does the same work). The face icon (+0x26c) is not ported.
bool cTaskConsoleMultiPlayer::ResultExec()
{
	SetEnable(false);   // +0x24c = 0
	if (m_nResultStep == 0 && (m_pResult == nullptr || !m_pResult->IsEnd())) return false;
	return cTaskConsolePlayer::ResultExec();
}

}
