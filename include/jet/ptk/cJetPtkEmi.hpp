#pragma once

#include "common.hpp"
#include "jet/ptk/wrkEmi.hpp"
#include "cJetPtkHnd.hpp"
#include "jet/ptk/cJetPtkData.hpp"

namespace jet::ptk
{

class cJetPtkEmi
{
private:
	enum EFlag
	{
		eEMI_ACTIVE = 1,
		eEMI_VISIBLE = 2,
		eEMI_CLIPPING = 4,
		eEMI_ONETIME = 8,
		eEMI_WAIT = 16,
		eEMI_USE = 32,
		eEMI_DEAD_RESERVE = 64,
		eEMI_CLIPPING_VISIBLE = 128,
		FLAG_REPORT_HAVE_NO_DEAD_GENERATOR = 1073741824,
		FLAG_REPORT_HAVE_NO_DEAD_PARTICLE = std::numeric_limits<int>::min()
	};

	cJetPtkHnd* m_pHnd{};

	cJetPtkData* m_pPtkData{};

	wrkEmi m_WrkEmi{};

	float m_fTime{};

	bool m_bDepend{};

	float m_fLifeTime{};

	int m_nActivePtcCnt{};

	uint m_nFlags{};

	float m_fDelay{};

	Vector4 m_vTrans{};

	Vector4 m_vRotate{};

	Vector4 m_vScale{};

	Matrix4x4 m_mWorldMtx{};

	Matrix4x4 m_mBillBoardMtx{};

	cJetPtkEmi* m_pNext{};

	cJetPtkEmi* m_pPrev{};

	int m_nFogIdx{};

	int m_nLightSetIdx{};

	int m_iDrawLayer{};

	static const Rect scClippingRect;

	static const float scClippingDist;

	bool m_checkClippingDist{};

	bool IsClippingDist();

	float GetClippingDist();

	void Term();

public:
	cJetPtkEmi();

	virtual ~cJetPtkEmi();

	void Init();

	void UpdateMatrix();

	void AnimUpdate(float fStep = 1.0f);

	void AddActivePtc(uint nPtcCnt);

	virtual bool ClippingExec(Camera* camera, Vector4 vCamPos, Vector4 vCamPos2Tgt, float fFovy);

	void bWait(bool b);

	bool IsWait();

	void bUse(bool b);

	bool IsUse();

	void bDeadReserve(bool b);

	bool IsDeadReserve();

	void bActive(bool b);

	bool IsActive();

	void bVisible(bool b);

	bool IsVisible();

	void bOneTime(bool b);

	bool IsOneTime();

	void bClippingVisible(bool b);

	bool IsClippingVisible();

	void bClipping(bool b);

	bool IsClipping();

	void bDepend(bool b);

	bool IsDepend();

	float fTime();

	void fLifeTime(float time);

	float fLifeTime();

	void pPtkData(cJetPtkData* pDat);

	cJetPtkData* pPtkData();

	void fDelay(float fDelay);

	float fDelay();

	bool IsDelay();

	void pNext(cJetPtkEmi* pNext);

	cJetPtkEmi* pNext();

	void pPrev(cJetPtkEmi* pPrev);

	cJetPtkEmi* pPrev();

	void DrawLayer(int layer);

	int DrawLayer();

	void nFogIdx(int idx);

	int nFogIdx();

	void nLightSetIdx(int idx);

	int nLightSetIdx();

	Matrix4x4& pWorldMtx();

	Matrix4x4& pBillBoardMtx();

	Vector4 vTrans();

	Vector4 vRotate();

	Vector4 vScale();

	void AttachHnd(cJetPtkHnd* pHnd);

	void DetachHnd();
};

}
