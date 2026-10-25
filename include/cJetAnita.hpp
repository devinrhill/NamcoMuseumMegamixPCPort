#pragma once

#include "common.hpp"
#include "jet/EAnitaLay.hpp"
#include "jet/sDatLayP.hpp"

class cJetAnita
{
protected:
	class Seq
	{
	public:
		int anm_no{};

		float frm_cnt{};

		float frm_end{};

		float frm_rate{};

		float frm_noA{};

		float frm_noB{};

		int pat_noA{};

		int pat_noB{};
	};

	class AnitaTex
	{
	public:
		Texture* pBrres{};

		bool IsClut{};

		uint Width{};

		uint Height{};
	};

	struct Cache
	{
		int blend;

		int tex;

		int filter;
	};

public:
	class MaterialInfo
	{
	public:
		Material* mat{};

		jet::EAnitaLay blendMode{};
	};

	class MeshInfo
	{
	public:
		Mesh* mesh{};

		float w{};

		float h{};

		uint color{};

		Vector4 UV{};
	};

	static constexpr int JET_ANITA_TEX_MAX = 16;

	static constexpr int JET_ANITA_STATE_VISIBLE = 1;

	static constexpr int JET_ANITA_STATE_ANIMATE = 2;

	static constexpr int JET_ANITA_STATE_FORCE_LOOP = 4;

	static constexpr uint JET_ANITA_STATE_ANIM_END = 65536u;

	static constexpr uint JET_ANITA_STATE_ANIM_REVERSE = 131072u;

	static constexpr uint JET_ANITA_STATE_ANIM_IS_LOOP = 262144u;

protected:
	std::vector<byte> m_pAnita;

	uint m_nState{};

	Seq* m_AnimSeq{};

	AnitaTex* m_Textures[16]{};

	Vector4 m_vTrans{};

	Vector4 m_vRot{};

	Vector4 m_vScale{};

	uint m_nCompare{};

	uint m_nFunc{};

	uint m_nUpdate{};

private:
	static constexpr float cfDEPTH_Z = 0.05f;

	static constexpr int ciLAYER_3D = 0;

	static constexpr int ciLAYER_UI = 5;

	static constexpr int ciLAYER_EFFECT = 9;

	static const int caTRIANGLES[6];

protected:
	static Shader* g_aShaders[3];

	static int g_iDrawCount;

	int m_iDrawLayer = 5;

	Matrix4x4 m_mtx{};

	Matrix4x4 m_userAffineMtx{};

	int m_iLayerIndex{};

	std::vector<MaterialInfo*> m_vMaterials;

	std::vector<MeshInfo*> m_vMeshs;

	static const int scMainTexPropertyID;

	static Vector3 m_tmpVertices[4];

	static Vector2 m_tmpUVs[4];

	static Color32 m_tmpColors[4];

	MeshInfo* GetMesh(int index, float w, float h);

	Material* GetMaterial(int index, jet::EAnitaLay blendMode);

public:
	static void ShaderInit();

	void SetDrawLayer(int layer);

	static void DrawInit();

protected:
	virtual bool GXSetAnitaEnv();

	virtual bool DrawPat(int pat);

	virtual bool DrawPatIntepolate();

	virtual bool DrawLayer(jet::sDatLayP* pLay);

	virtual bool Interpolate(jet::sDatLayP* pLay0, jet::sDatLayP* pLay1, jet::sDatLayP* pLay2, float r);

	virtual float Interpolate(float s, float e, float r);

	virtual float InterpolateRot(float s, float e, float r);

	virtual bool LayerAffine(jet::sDatLayP* pLay);

	virtual bool LayerAffineUser();

public:
	cJetAnita();

	cJetAnita(const std::vector<byte>& anita);

	virtual ~cJetAnita();

	virtual void Init();

	virtual void Term();

	virtual bool SetData(const std::vector<byte>& anita, bool checkLanguage = false);

	std::vector<byte>& GetData();

	bool SetTexture(bool overwrite = true, bool checkLanguage = false);

	bool SetTexture(int idx, Texture* brres);

	Texture* GetTexture(int idx);

	virtual bool AnimUpdate(float step);

	virtual bool AnimUpdate();

	bool Draw(int pat = -1);

	void SetZMode(uint compare, uint func, uint update);

	bool SetAnimNo(int no, bool reset = true, float frm = 0.0f);

	int GetAnimNo();

	bool IsState(uint flag);

	uint GetState();

	bool SetState(uint flag, bool bit = true);

	float GetFrameRate();

	void SetFrameRate(float rate);

	float GetFrame();

	void SetFrame(float frm);

	float GetFrameNum(int no = -1);

	uint GetPatNum();

	uint GetAnimNum();

	Vector4& vTrans();

	Vector4& vRot();

	Vector4& vScale();
};
