#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/NET_RF_PACKET.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"

namespace kingdom::game::g07
{

class cTaskHitMan : public cTaskKingdom
{
private:
	NET_RF_PACKET* m_nRefInfos[32]{};

	int m_nRefIndex{};

	byte m_nRefID{};

public:
	uint GetRefID();

	NET_RF_PACKET* RefInfo(int n);

	cTaskHitMan();

	virtual ~cTaskHitMan();

protected:
	virtual void Init();

public:
	virtual void Free();

	bool Execute(cJetTask* pParam) override;

	int HitCheck(cTaskChar* pChar1, cTaskChar* pChar2, Nullable<Vector4> pSpeed1 = nullptr, Nullable<Vector4> pSpeed2 = nullptr);

	bool HitCheckSimple(cTaskChar* pChar1, cTaskChar* pChar2, Vector4 pSpeed1, Vector4 pSpeed2);

private:
	int m_HitCheckExSub(int& pAxis, cTaskChar* pChar1, cTaskChar* pChar2, Vector4 pPos1, Vector4 pPos2, float fSize, int nHigh);

public:
	bool HitCheckThrough(cTaskChar* pChar1, cTaskChar* pChar2);

	bool ExecHitGimmick();

	bool RegistReflect(int nGrobalID1, int nGrobalID2, int axis);

	NET_RF_PACKET* SearchReflect(int nGrobalID1, int nGrobalID2);

	void DeleteReflect(int nGrobalID1, int nGrobalID2);

	void RegistPacket();

	void SyncPacket();
};

}
