#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Rally-X Remix (g02) stage select: the 3D world map of the pointed stage. Ported from main.dol (no C# exists for Rally-X).
// kingdom::game::g02::cTaskMenuMap (DOL vtable 0x8043e6e0, 0x230 bytes, ctor 0x80111d84), parent kingdom::game::cTaskMenuBgBase (a
// cTaskConsMenuBase with its own state +0x1d8 / step +0x1dc, a scale factor +0x1e0, the shown model +0x1e4 and a cTaskModel child +0x1e8;
// its ctor part is inlined in 0x80111d84, dtor 0x80111e70, Draw 0x80112814 is EMPTY: the anita (ANITA_MENU_SELECT_01) is only the pointer
// hit area, never drawn; 0x80112818 sets +0x1e4). cTaskMenuBgBase is folded into this class natively. The multiplayer subclass
// cTaskMultiMenuMap (0x8043e898, its RegistData 0x80112898) is NOT PORTED.
namespace kingdom::game::g02
{

class cTaskMenuMap : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskMenuMap(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo, int nStageNo);   // DOL 0x80111d84
	~cTaskMenuMap() override;            // DOL 0x801127b8

	virtual void ChangeData(int nIdx);   // DOL 0x801089f4 (vtable +0x94): hide the shown world model, show model nIdx (stage + world * 6)
	virtual void RegistData();           // DOL 0x80111ec8 (vtable +0x98)
	int nCurIdx() const { return m_nCurIdx; }   // +0x1e4 (the man compares it)
	void SetCurIdx(int n) { m_nCurIdx = n; }    // DOL 0x80112818

	bool IsDpdHit(float x, float y) override;   // DOL 0x80112114 (pattern 0x24)
	bool Execute(cJetTask* pParam) override;    // DOL 0x8011274c
	void Draw(int nStatus, cJetTask* pParam) override {}   // DOL 0x80112814 (cTaskMenuBgBase: empty)

protected:
	void WaitExec() override;            // DOL 0x8011224c
	bool AppearExec() override;          // DOL 0x8011230c
	void PointExec() override;           // DOL 0x80112430
	bool DecideExec() override;          // DOL 0x8011252c
	bool DeleteExec() override;          // DOL 0x8011262c

private:
	void ApplyScale();                   // the inlined "model[+0x1e4] scale = node scale * +0x1e0" of every *Exec

	int m_nBgState{};                    // +0x1d8 (cTaskMenuBgBase: 2 after appearing, 4 after deleting; nothing reads it)
	int m_nBgStep{};                     // +0x1dc
	float m_fScale{};                    // +0x1e0
	int m_nCurIdx{};                     // +0x1e4
	kingdom::cTaskModel* m_pModel{};     // +0x1e8: models 0..23 = MENU_WORLD_*, 24 = MENU_NULL_MODEL
	Vector4 m_vTrans{0.0f, 0.0f, 0.0f, 1.0f};   // +0x1f0
	Vector4 m_vRotate{0.0f, 0.0f, 0.0f, 0.0f};  // +0x200
	Vector4 m_vScale{1.0f, 1.0f, 1.0f, 1.0f};   // +0x210
	int m_nWorldNo{};                    // +0x220
	int m_nStageNo{};                    // +0x224
	float m_fSpeed{};                    // +0x228 the pulse step
};

}
