#pragma once

#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) the READY sign: kingdom::game::g01::cTaskConsoleReadyString (0x230 bytes, vtable 0x80439a08, a cTaskGameConsoleBase), the
// first console the console manager builds (0x800ebab0: ctor 0x800d7908 priority 0x80000002, Init 0x800d7924). One ANT_SCORE anita: anim 0
// played once when the consoles get their READY call (state 1, 0x800eb664), then hidden.
namespace kingdom::game::g01
{
class cTaskConsoleReadyString : public cObject, public kingdom::cTaskAnita
{
public:
	cTaskConsoleReadyString();                 // DOL 0x800d7908 + Init 0x800d7924 + the manager's start 0x800eb3b0
	bool Execute(cJetTask* pParam) override;   // cTaskGameConsoleBase::Execute 0x800eb400 (state 1 part)
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskGameConsoleBase::Draw 0x800eb548
	void Ready() { m_nState = 1; m_nStep = 0; m_nSub = 0; }   // DOL 0x800eb664 (console vtable +0x78)
	bool IsReadyEnd() const { return m_nState == 2; }        // DOL 0x800eb680

private:
	bool ReadyStep();                          // DOL 0x800d79ec (vtable +0x5c)
	int m_nSub{};                              // +0x220
};
}
