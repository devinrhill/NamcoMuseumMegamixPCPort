#pragma once

#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace wii { struct Mdl0Node; }

// Galaga Remix (g01) stage scenery placed by the stage skeleton m01b_s%x_bg_s (cTaskStage 0x800fa80c):
//   cTaskGimmick (0x330 bytes, vtable 0x8043b018): 68 kinds (big comets m01g_l_co*, the sun, cliffs, trees, ivy ...), kind 9, priority 0x10002,
//     a 3D sound actor (+0x328, not ported). Its clips start at a random frame and play at 0.1 (1.0 in world 3; pattern clips 1.0).
//   cTaskGimmickMComet (vtable 0x8043aef8): 12 middle comets, collidable (+0x14): a shot that hits one plays PTK_M_COMET01 + m01se_03hit1
//     at the hit point and flies on (kind 9 does not use up the shot, 0x800d7344).
namespace kingdom::game::g01
{
class cTaskGimmick : public cObjectAABB, public kingdom::cTaskModel
{
public:
	cTaskGimmick();                                          // DOL 0x800ec9a0 (vtable +0xa4)
	void Setup(const wii::Mdl0Node& node, int nIndex, int nFogIdx);   // DOL 0x800eca58
	bool Execute(cJetTask* pParam) override;                 // DOL 0x800ed000
	void Draw(int nStatus, cJetTask* pParam) override;       // DOL 0x800ed088: only while active (+8) and shown (+9)
	static int Find(const char* nodeName);                   // the index of the first name of 0x803759ac the node name contains, or -1

protected:
	void Place(const wii::Mdl0Node& node);                   // the node's local translate / rotate (deg * FLOAT_8047b5fc) / scale
};

class cTaskGimmickMComet : public cTaskGimmick
{
public:
	cTaskGimmickMComet();                                    // DOL 0x800ed0ac (vtable +0xa4: +0x32c = 0)
	void Setup(const wii::Mdl0Node& node, int nIndex, int nFogIdx);   // DOL 0x800ed128
	bool Execute(cJetTask* pParam) override;                 // DOL 0x800ed5dc
	void HitCallback(cObject* other) override;               // DOL 0x800ed66c
	static int Find(const char* nodeName);                   // table 0x80375548

private:
	int m_nIndex{};                                          // +0x32c
};

// cTaskGimmickBreakComet (vtable 0x8043add8; init 0x800ed7a0 kind 10, setup 0x800ed7ac, exec 0x800edae8, HitCallback 0x800edb78): the
// shootable comets of CometGimmick_s%x.csv (cTaskStage 0x800f99fc), worth GimmickParam_s%x.csv BLUE / RED / GREEN_SCORE by type
class cTaskGimmickBreakComet : public cTaskGimmick
{
public:
	cTaskGimmickBreakComet();
	void Setup(int nType, float fRate, const Vector4& pos, const Vector4& rot, const Vector4& scale, int nScore, int nFogIdx);
	bool Execute(cJetTask* pParam) override;
	void HitCallback(cObject* other) override;

private:
	int m_nType{};                                           // +0x32c
	int m_nScore{};                                          // +0x84
};
}
