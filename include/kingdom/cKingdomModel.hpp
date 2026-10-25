#pragma once

#include "common.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "RtpaManager.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom
{

// native: the "GameObject" of a model entry (what dynamic_cast<GameObject*>(ResourceDatabase.Get<Object>(key)) gave in Unity); not drawn. null when missing.
GameObject* LoadModelObject(const string& key);

class cKingdomModel : public cKingdomModelBase
{
private:
	class sExpandObj
	{
	public:
		cKingdomModel* model{};

		bool isInnerCreate{};

		string node{};   // node of the parent the child is attached to
	};

	GameObject* m_pScnMdl{};

	Animation* m_pAnim{};

	cKingdomModel::sExpandObj* m_ExpandObjs[8]{};

	RtpaManager* m_rtpaManager{};

	AnimationClip* m_prevAnimClip{};

	string m_animClipName{};

	Transform* m_transform{};

public:
	std::vector<Material*> m_Materials;

	GameObject* pScnMdl();
	// parsed MDL0 of the loaded model (node tree, shapes); null before RegistModel / when the entry has no model
	const wii::Mdl0* pMdl0();

	Animation* pAnim();

	int pScnMdlExpandCnt();

	cKingdomModel* pExpandMdl(uint exp_idx);

	RtpaManager* RtpaManager();

	AnimationClip*& PrevAnimClip();

	string& AnimClipName();

	Transform* pScnMdlTransform();

	cKingdomModel();

	cKingdomModel(Object* data, uint option = 0u);

	cKingdomModel(string key, uint option = 0u);

	virtual ~cKingdomModel();

protected:
	void Init() override;

public:
	void DrawNative(const float eye[3]) override;
	// DOL 0x8001ded4: the model's clips advance, and so do those of its expanded models (Pac-Man's run clip on the orbit skeleton)
	void UpdateFrame();
	void DebugPrint() override;
	// draws with an explicit world matrix (column major) - the expanded children are placed on the node of their parent
	void DrawNativeAt(const float world[16], const float eye[3]);

protected:

public:
	void Term() override;

	void CopyMaterial();

	void RegistModel(Object* data, uint option = 0u);

	void RegistModel(string key, uint option = 0u);

	void RegistExpand(uint exp_idx, string node, cKingdomModel* exp_model);

	void RegistExpand(uint exp_idx, string node, Object* data, uint option);

	void RegistExpand(uint exp_idx, string node);

	void RegistExpand(uint exp_idx, string node, string key, uint option = 0u);

	void ReleaseExpand(uint exp_idx);
};

}
