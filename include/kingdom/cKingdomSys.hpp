#pragma once

#include "common.hpp"
#include "cJetTask.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/sKingdomData.hpp"
#include "kingdom/cKingdomSave.hpp"

namespace kingdom
{

class cKingdomSys : public cJetTask
{
public:
	static constexpr int KINGDOM_FPS = 6000;

	static constexpr uint KINGDOM_SND_HEAP_SIZE = 8388608u;

	static constexpr uint KINGDOM_LEAVE_MSEC = 30000u;

	static constexpr uint KINGDOM_LEAVE_DVD_MSEC = 300000u;

private:
	cAppKingdom* m_pAppMain{};

	// Target job stored by Kingdom_SetJobNo() while a transition app (jobs 0x08500000 / 0x08600000) runs.
	// DOL: cKingdomSys + 0xa88.
	uint m_nNextJob{};

	// Read by cAppTitle; meaning not yet known. DOL: cKingdomSys + 0xa44 / + 0xa91.
	uint m_nUnkA44{};
	bool m_bUnkA91{};
	bool m_bObjA50Flag0{};
	bool m_bUnkA95{};            // DOL cKingdomSys + 0xa95, read by cAppSaveLoad
	void* m_pUnkA90{};           // DOL cKingdomSys + 0xa90, cAppSaveLoad stores the save manager here on its error screens
	cKingdomSave* m_pSave{};     // DOL cKingdomSys + 0xa8c (created by the cKingdomSys init; creation site not traced yet)
	bool m_bObjA50Flag1{};

public:
	cKingdomSys(); // DOL 0x8000a344: names the task "cKingdomSys" (string 0x803684e0)

	cAppKingdom*& pAppMain();

	uint& nNextJob() { return m_nNextJob; }
	// DOL 0x8009dc08: FUN_8009dc08(cKingdomSys[0xa50], a, b) stores two bytes at +0x24c / +0x24d of that object. Unknown class
	// (likely a system overlay such as the HOME menu); recorded here so the calls stay visible.
	void SetObjA50Flags(bool a, bool b) { m_bObjA50Flag0 = a; m_bObjA50Flag1 = b; }

	cKingdomSave*& pSave() { return m_pSave; }
	// DOL 0x8000d8cc / 0x8000d8d4: the save manager's block copies (cKingdomSave::ReadBlock / WriteBlock); false without a manager
	bool LoadGameData(uint block, void* dst, uint size);
	bool SaveGameData(uint block, const void* src, uint size);
	bool& bUnkA95() { return m_bUnkA95; }
	void*& pUnkA90() { return m_pUnkA90; }
	bool bObjA50Flag1() const { return m_bObjA50Flag1; }

	uint& nUnkA44() { return m_nUnkA44; }
	bool& bUnkA91() { return m_bUnkA91; }

	bool AttachArcData(const string& pArcName, bool visible = false);

	bool RegistArcDataAS(const string& pArcName);

	bool DetachArcData(const string& pArcName);

	sKingdomData* SearchDataEx(const string& pKey);

	const std::vector<byte>& SearchData(const string& pKey);

	int nMSecToFrame(int nMSec);
};

}
