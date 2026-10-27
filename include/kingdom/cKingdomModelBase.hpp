#pragma once

#include <memory>

#include "common.hpp"

namespace wii { class ChrAnim; struct BoneKey; }
namespace kingdom
{
class cKingdomModel;
}

namespace kingdom
{

class cKingdomModelBase
{
private:
	class sGarbageModel
	{
	public:
		cKingdomModelBase* model{};

		bool isGarbage{};
	};

public:
	class sLodObj
	{
	public:
		cKingdomModel* model{};

		float distSq{};
	};

	static constexpr int EXPAND_MAX = 8;

	static constexpr int BLEND_MAX = 4;

	static constexpr int VIEW_MAX = 4;

	static constexpr int VIS_BLEND_MAX = 0;

	static constexpr uint DEFAULT_BUFFER_OPTION = 0u;

	// Native model state (gfx::ModelDraw, animation slots): see cKingdomModelBase.cpp. The Unity GameObject / Animation of the C# port are scrapped.
	struct Native;

private:
	Vector4 m_vTrans{};

	Vector4 m_vRotate{};

	Vector4 m_vScale = Vector4::one;

	std::vector<cKingdomModelBase::sLodObj*> m_pLodObjs;

	uint m_nLod{};

	std::vector<cKingdomModelBase::sGarbageModel*> m_vecGarbage;

	bool m_bVisible{};
	bool m_bShadowOnly{};

protected:
	bool m_bUpdateExpand{};

private:
	int m_nLightSetIdx{};

	int m_nFogIdx{};

	std::vector<GameObject*> m_tmpChildObjList;

public:
	// model-space matrix (3x4 row major) of a node in the current pose (0x800e0254-like node queries); false for an unknown node
	bool NodeMatrix(const std::string& node, float m3x4[12]);
	// the per node skin matrices (3x4 row major, gfx::ModelDraw::Skin) of this instance's current CHR pose; false without a native model.
	// `out` is left empty when no CHR slot poses the model (bind pose). HasChrPose: some slot poses it (weight > 0, a clip with frames)
	bool PosedSkin(std::vector<float>& out);
	bool HasChrPose() const;

protected:
	Native* m_native{};
	float m_world[16]{};
	bool m_useMtx{};
	float m_mtxCM[16]{};      // rotation + translation, column major
	float m_mtxScale[3]{1, 1, 1};

private:

	std::unordered_map<int, std::string> m_dicAnimationList;

	bool m_bDrawDepth{};

public:
	// Draws this model (and its expanded children) into the open 3D pass: called by NativeScene::Flush. `eye` = camera position (sky models follow it).
	virtual void DrawNative(const float eye[3]) {}
	virtual void DebugPrint() {}
	// drops the cached models / animations (the attached archives changed: a new stage)
	static void ClearAssetCache();
	// world matrix (column major) from vTrans / vRotate (Z*Y*X, radians) / vScale: what UpdateMatrix computes
	void CalcWorld(float out[16]) const;
	// the game places models with a full matrix (position + rotation; Unity's pScnMdl().SetMtx) and a scale (SetScale) instead of vTrans / vRotate:
	// row-major game matrix, column vectors; the rotation columns are normalised (the scale comes from SetNativeScale)
	void SetNativeMtx(const Matrix4x4& m);
	void SetNativeScale(float x, float y, float z);
	void ClearNativeMtx() { m_useMtx = false; }   // back to vTrans / vRotate / vScale placement
	// a full world matrix (column major) kept as it is: no normalisation, so it may squash (projected shadows); scale 1
	void SetNativeMtxRaw(const float cm[16]);
	void GetNativeMtx(Matrix4x4& out) const;

	string AnimationDic(int index);

	cKingdomModelBase(uint lodSize = 1u);

	virtual ~cKingdomModelBase();

	virtual bool IsDrawShadowDepth();

	virtual void IsDrawShadowDepth(bool b);

	Vector4& vTrans();

	Vector4& vRotate();

	Vector4& vScale();

	float vTrans(uint n);

	float vRotate(uint n);

	float vScale(uint n);

	uint nLod();

	cKingdomModelBase::sLodObj* pLodObj(uint n = 0u);

	bool IsVisible();
	// A model that only casts the depth shadow (cTaskModelEx::RegistShadowModel): Visible() may be called on it, but the colour pass never draws it.
	void SetShadowOnly(bool on) { m_bShadowOnly = on; }
	bool IsShadowOnly() const { return m_bShadowOnly; }

	bool IsUpdateExpand();

protected:
	virtual void Init();

public:
	virtual void Term();

	void Visible();

	void Invisible();

protected:
	Object* SearchData(string key);

public:
	// The CHR0 bone clip behind an archive entry handle (ResourceDatabase::Get / SearchData result), cached; nullptr if it has none.
	// The DOL samples clips directly (nw4r::g3d::ResAnmChr::GetAnmResult) where the Unity code used a hidden dummy GameObject + Animation.Sample.
	static std::shared_ptr<wii::ChrAnim> ChrAnimOf(const AnimationClip* clip);
	// the CHR0 clip bound to blend slot `idx` of this model (RegistAnm), nullptr when none; GetFrameAnmChr(idx) is its current frame
	std::shared_ptr<wii::ChrAnim> AnmChrOf(uint idx);
	// ChrAnmResult.rt of a bone as a 4x4: Rz * Ry * Rx rotation (the bone convention of gfx::ModelDraw), plus the bone translation in the 4th column when `keepTranslation`
	// (nw4r's FUN_8013e884 copies the 3x4 rt; the miss animation and the hit objects zero the translation afterwards).
	static Matrix4x4 BoneRt(const wii::BoneKey& key, bool keepTranslation);

	void RegistAnm(uint blend_idx, string key, string node = string());

	void RegistAnm(uint blend_idx, Object* data, string node = string());

	int RegistAnmNoIdx(string key, string node = string());

	int RegistAnmNoIdx(Object* data, string node = string());

private:
	int _RegistAnm(int blend_idx, Object* data, string node);

public:
	static void ReleaseAnm(AnimationClip* anime);

	void ReleaseAnmChr(uint blend_idx);

	void ReleaseAnmVis(uint blend_idx);

	void ReleaseAnmClr(uint blend_idx);

	void ReleaseAnmTexPat(uint blend_idx);

	void ReleaseAnmTexSrt(uint blend_idx);

	void ReleaseAnmShp(uint blend_idx);

private:
	int AnmBlendAttach(cKingdomModel* model, Animation* anmBlend, int blend_idx, uint blend_max, AnimationClip* anmRes, string node);

public:
	void SetFrameAnmChr(uint blend_idx, float frame);

	void SetFrameAnmVis(uint blend_idx, float frame);

	void SetFrameMatClr(uint blend_idx, float frame);

	void SetFrameAnmTexPat(uint blend_idx, float frame);

	void SetFrameAnmTexSrt(uint blend_idx, float frame);

	void SetFrameAnmShp(uint blend_idx, float frame);

	void SetFrameAnmAll(uint blend_idx, float frame);

	float GetFrameAnmChr(uint blend_idx);

	float GetFrameAnmVis(uint blend_idx);

	float GetFrameMatClr(uint blend_idx);

	float GetFrameAnmTexPat(uint blend_idx);

	float GetFrameAnmTexSrt(uint blend_idx);

	float GetFrameAnmShp(uint blend_idx);

	float GetFrameMaxAnmChr(uint blend_idx);

	float GetFrameMaxAnmVis(uint blend_idx);

	float GetFrameMaxMatClr(uint blend_idx);

	float GetFrameMaxAnmTexPat(uint blend_idx);

	float GetFrameMaxAnmTexSrt(uint blend_idx);

	float GetFrameMaxAnmShp(uint blend_idx);

	void SetUpdateRateAnmChr(uint blend_idx, float rate);

	void SetUpdateRateAnmVis(uint blend_idx, float rate);

	void SetUpdateRateMatClr(uint blend_idx, float rate);

	void SetUpdateRateAnmTexPat(uint blend_idx, float rate);

	void SetUpdateRateAnmTexSrt(uint blend_idx, float rate);

	void SetUpdateRateAnmShp(uint blend_idx, float rate);

	void SetUpdateRateAll(uint blend_idx, float rate);

	float GetUpdateRateAnmChr(uint blend_idx);

	float GetUpdateRateAnmVis(uint blend_idx);

	float GetUpdateRateMatClr(uint blend_idx);

	float GetUpdateRateAnmTexPat(uint blend_idx);

	float GetUpdateRateAnmTexSrt(uint blend_idx);

	float GetUpdateRateAnmShp(uint blend_idx);

	void SetWeightAnmChr(uint blend_idx, float weight);

	float GetWeightAnmChr(uint blend_idx);

	void SetWeightAnmShp(uint blend_idx, float weight);

	float GetWeightAnmShp(uint blend_idx);

	void GetLocalNodeScale(Vector4& v, string node);

	void GetLocalNodeScale(Vector4 v, uint node);

	void GetLocalNodeRotate(Vector4& v, string node);

	void GetLocalNodeRotate(Vector4 v, uint node);

	void GetLocalNodeTrans(Vector4& v, string node);

	void GetLocalNodeTrans(Vector4 v, uint node);

	Transform* GetLocalNode(string node);

	void UpdateFrame();

	void UpdateMatrix();
	bool m_bExpandChild{};   // native: attached to a parent with RegistExpand (the parent draws it; not submitted on its own)

	void AttachShadow();

	void SetLightSetIdx(int idx);

	int GetLightSetIdx();

	void SetFogIdx(int idx);

	int GetFogIdx();

	void GarbageModel(cKingdomModelBase* p);

	void EnableGarbageModel();

	void ClearGarbageModel();

	void ForceClearGarbageModel();
};

}
