#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom { class cKingdomAnita; }

// Rally-X Remix (g02): kingdom::game::g02::cTaskRallyAnitaBase (vtable 0x804410a8, parent kingdom::cTaskAnita; dtor 0x80061914, Execute
// 0x80122f3c, Draw 0x80123244). The base of every single-player console part (cTaskConsFuel / Radar / Info / Stage / Time / Signal / Message /
// Result / Lose / Bang / Warning). The constructor is inlined into each part: priority 0x80000001, byte +0x1c8 = 1; the anita vector is the
// cTaskAnita one (+0x1b4). Helpers (all take the anita index of this task):
//   0x80123034 Regist(idx, name, anim, x, y, z): RegistData + SetAnimNo(anim, reset) + vTrans (anita +0x400); returns the anita
//   0x80122ff4 SetAnim(idx, anim): SetAnimNo(anim, reset = false)
//   0x80123138 SetPosLayer(idx, pat, lay): vTrans.xy = the position of layer `lay` of pattern `pat` (0x80014408)
//   0x801231e8 GetLayerPos(out, idx, pat, lay)
// The DOL's secondary-vtable slots listed for this class by vtable_audit (0x80125c18 ..) belong to the next vtable (the app's) and are not
// cTaskRallyAnitaBase methods.
namespace kingdom::game::g02
{
class cTaskConsole;

class cTaskRallyAnitaBase : public kingdom::cTaskAnita
{
public:
	explicit cTaskRallyAnitaBase(cTaskConsole* pConsole);
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80122f3c
	void Draw(int nStatus, cJetTask* pParam) override;         // DOL 0x80123244

	cTaskConsole* pConsole() const { return m_pConsole; }

protected:
	kingdom::cKingdomAnita* Regist(uint idx, const char* name, int anim, float x = 0.0f, float y = 0.0f, float z = 0.0f);   // DOL 0x80123034
	void SetAnim(uint idx, int anim);                          // DOL 0x80122ff4
	bool SetPosLayer(uint idx, int pat, uint lay);             // DOL 0x80123138
	bool GetLayerPos(Vector4& out, uint idx, int pat, uint lay);   // DOL 0x801231e8
	bool IsEnd(uint idx);                                      // anita state & 0x10000 (JET_ANITA_STATE_ANIM_END)
	void Show(uint idx, bool b);                               // anita state bit 1 (visible)
	void Animate(uint idx, bool b);                            // anita state bit 2 (animate)

	bool m_b1c8{true};                                         // +0x1c8 (set by the constructor; no reader found)
	cTaskConsole* m_pConsole{};                                // the owning console (the DOL keeps it per class: Fuel +0x1e4, Info +0x1dc, ...)
};
}
