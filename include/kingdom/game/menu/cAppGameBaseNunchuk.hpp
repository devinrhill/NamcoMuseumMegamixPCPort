#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/cTaskFade.hpp"
#include "cJetTask.hpp"

// "Connect the Nunchuk" card shown between a game's menu and its stage (no Unity counterpart: ported from the DOL).
// kingdom::game::menu::cAppGameBaseNunchuk, vtable 0x80444e48: ctor part 0x80143aec, Execute 0x80143dac, Draw 0x80144138,
// slot 18 IsSkip 0x80144130 (base: true), slot 19 RegistLayout 0x8014420c.
namespace kingdom::game::menu
{

class cAppGameBaseNunchuk : public cAppKingdom
{
public:
	cAppGameBaseNunchuk();
	bool Execute(cJetTask* pParam) override;

protected:
	// slot 18 (+0x48): true = do not show the card (the subclass checks the Nunchuk)
	virtual bool IsSkip();
	// slot 19 (+0x4c): the layout (archive entry name; the DOL passes its CRC32), loaded hidden
	void RegistLayout(const string& name);
	uint& nNextJob() { return m_nNextJob; }

private:
	uint m_nNextJob = 0xffffffffu;   // +0x294
	cTaskAnita* m_pAnita{};          // +0x280 (an Anita vector drawn by the app's own Draw in the DOL)
	cTaskFade* m_pFade{};            // +0x298
};

}
