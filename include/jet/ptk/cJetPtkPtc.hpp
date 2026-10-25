#pragma once

#include "common.hpp"
#include "jet/ptk/EPtkGen.hpp"
#include "jet/ptk/datScn.hpp"
#include "jet/ptk/PtkTex.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/cJetPtkGen.hpp"
#include "jet/ptk/wrkPtc.hpp"

namespace jet::ptk
{

class cJetPtkPtc
{
private:
	enum EFlags
	{
		ePTC_USE = 1,
		ePTC_ACTIVE = 2,
		ePTC_VISIBLE = 4,
		ePTC_PARENT_MTX = 8
	};

	class cCache
	{
	public:
		int m_nBlend{};

		int m_nAlpha{};

		int m_nFilter{};

		int m_nFog{};

		int m_nLightSet{};

		bool m_bFirst{};

		bool m_bTex{};

		bool m_bLight{};

		PtkTex* m_pTex{};

		cCache();

	private:
		void Init();

	public:
		void Reset();
	};

	static const int scZTestModeID;

	static const int scZWriteParamID;

	static const int scMainTexID;

	datScn m_pDatScn{};

	cJetPtkGen* m_pGen{};

	cJetPtkEmi* m_pEmi{};

	PtkTex* m_pTex{};

	wrkPtc* m_WrkPtc;

	uint m_nFlags{};

	static cCache* m_Cache;

	Matrix4x4 m_mParentMtx{};

	Matrix4x4 m_mParentBillBoardMtx{};

	cJetPtkPtc* m_pNext{};

	cJetPtkPtc* m_pPrev{};

	cJetPtkPtc* m_pDrawNext{};

	cJetPtkPtc* m_pDrawPrev{};

	float m_fCamLength{};

	static std::unordered_map<int, Shader*> s_ShaderCache;

protected:
	Matrix4x4 m_mWorldMtx{};

	Material* m_Material{};

	std::unordered_map<Shader*, Material*> m_materialCache;

	Mesh* m_Mesh{};

	int m_iDrawLayer{};

	Vector3 m_Vertices[4]{};

	Vector2 m_uv[2]{};

	Vector2 m_UVs[4]{};

	Color32 m_Colors[4]{};

	int m_prevZTest = -1;

	int m_prevBlendMode = -1;

	Texture* m_prevTex{};

	bool m_isPacnRoll{};

private:
	static Matrix4x4 _mtx_all;

public:
	int get_DrawLayer();

	void set_DrawLayer(int value);

private:
	void _InitShaderCache();

protected:
	void _InitShader(EPtkGen blendMode);

private:
	void _InitVertexColor(Color32 col);

	void _InitUV(float _u0, float _v0, float _u1, float _v1);

	void _InitMesh(float hx, float hy);

	void _InitMesh(float v0x, float v0y, float v1x, float v1y, float v2x, float v2y, float v3x, float v3y);

public:
	void Init();

	void Free();

private:
	void _BlendSetting();

	void _AlphaSetting();

	void _DepthSetting();

	void _VertexSetting();

	void _TextureSetting();

	void _RenderSetting();

	void _CalcMtxBillBoard(const Matrix4x4& BillBoardMtx);

	void _CalcMtxBillBoardNon();

	void _RenderBox();

	void _RenderBoxUpper();

	void _RenderBoxLower();

	void _RenderBoxParaPoint();

	void _Render();

	float _Curve(const int& type, const float& st, const float& ed, float rate);

	bool _Ageing(float fStep, float fLoadFactor);

	void _UpdatePos();

	void _UpdateState(float fStep);

	bool IsSkipUpdateState();

public:
	void AnimUpdate(float fStep, float fLoadFactor);

	void Draw(Vector4 vCam);

	void Draw(const Matrix4x4& mMtx);

	Matrix4x4& pGetParentMtx();

	Matrix4x4& pGetParentBillBoardMtx();

	void bUse(bool b);

	bool IsUse();

	bool IsParentUse();

	bool IsActive();

	bool IsDelay();

	bool IsVisible();

	void bVisible(bool b);

private:
	void IsGetParentMtx(bool b);

	bool IsGetParentMtx();

public:
	bool IsClippingVisible();

	void pGen(cJetPtkGen* pGen);

	cJetPtkGen* pGen();

	void pEmi(cJetPtkEmi* pEmi);

	void pParentMtx(const Matrix4x4& p);

	Matrix4x4& pParentMtx();

	void pParentBillBoardMtx(Matrix4x4& p);

	Matrix4x4& pParentBillBoardMtx();

	cJetPtkEmi* pEmi();

	void pTex(PtkTex* pTex);

	PtkTex* pTex();

	wrkPtc* pWrkPtc();

	void pDatScn(const datScn& pDatScn);

	datScn& pDatScn();

	void pNext(cJetPtkPtc* pNext);

	cJetPtkPtc* pNext();

	void pPrev(cJetPtkPtc* pPrev);

	cJetPtkPtc* pPrev();

	void pDrawNext(cJetPtkPtc* pNext);

	cJetPtkPtc* pDrawNext();

	void pDrawPrev(cJetPtkPtc* pPrev);

	cJetPtkPtc* pDrawPrev();

	void fCamLength(float fLength);

	float fCamLength();

	cJetPtkPtc();

	virtual ~cJetPtkPtc();

	static void InvalidCache();
};

}
