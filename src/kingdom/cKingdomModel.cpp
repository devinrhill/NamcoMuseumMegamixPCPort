#include "kingdom/cKingdomModel.hpp"

#include <cmath>
#include <cstring>
#include <iterator>

#include "CustomKingdomUtil.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include "RtpaManager.hpp"
#include "kingdom/NativeModel.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/ResHandle.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "kingdom/cKingdomModel.hpp"

// Native version of the Wii model object (the Unity original is in old/BACKUP_unity_headers/): the model is a gfx::ModelDraw, "pScnMdl" is a GameObject
// that only points back at this model (ResMdl / ResNode read the node tree through it), expanded models are placed on a node of their parent.
namespace kingdom
{
namespace
{
// column-major 4x4 product a * b
void Mul(const float* a, const float* b, float* out)
{
	float r[16];
	for (int c = 0; c < 4; c++)
		for (int row = 0; row < 4; row++)
		{
			float s = 0;
			for (int k = 0; k < 4; k++) s += a[k * 4 + row] * b[c * 4 + k];
			r[c * 4 + row] = s;
		}
	std::memcpy(out, r, sizeof r);
}
}

GameObject* LoadModelObject(const string& key)
{
	if (ResourceDatabase::Get<Object>(key) == nullptr) return nullptr;
	cKingdomModel* m = new cKingdomModel(key);   // kept alive: ResMdl / ResNode point into it
	m->Invisible();                              // data only (hit meshes, placement nodes): never drawn
	NativeScene::Remove(m);
	return m->pScnMdl();
}

GameObject* cKingdomModel::pScnMdl() { return m_pScnMdl; }
const wii::Mdl0* cKingdomModel::pMdl0() { return (m_native != nullptr && m_native->draw) ? &m_native->draw->Model() : nullptr; }
Animation* cKingdomModel::pAnim() { return m_pAnim; }
int cKingdomModel::pScnMdlExpandCnt() { return (int)std::size(m_ExpandObjs); }

cKingdomModel* cKingdomModel::pExpandMdl(uint exp_idx)
{
	CustomKingdomUtil::Assert(8 > exp_idx, "error");
	return ((m_ExpandObjs[exp_idx] != nullptr) ? m_ExpandObjs[exp_idx]->model : nullptr);
}

RtpaManager* cKingdomModel::RtpaManager() { return m_rtpaManager; }
AnimationClip*& cKingdomModel::PrevAnimClip() { return m_prevAnimClip; }
string& cKingdomModel::AnimClipName() { return m_animClipName; }
Transform* cKingdomModel::pScnMdlTransform() { return m_transform; }

cKingdomModel::cKingdomModel()
{
	Init();
	pLodObj()->model = this;
}

cKingdomModel::cKingdomModel(Object* data, uint option)
{
	Init();
	RegistModel(data, option);
	pLodObj()->model = this;
}

cKingdomModel::cKingdomModel(string key, uint option)
{
	Init();
	RegistModel(SearchData(key), option);
	pLodObj()->model = this;
}

cKingdomModel::~cKingdomModel()
{
	Term();
}

void cKingdomModel::Init()
{
	m_pScnMdl = nullptr;
	for (int i = 0; i < (int)std::size(m_ExpandObjs); i++) m_ExpandObjs[i] = new cKingdomModel::sExpandObj();
}

void cKingdomModel::Term()
{
	for (int i = 0; i < (int)std::size(m_ExpandObjs); i++)
		if (m_ExpandObjs[i] != nullptr && m_ExpandObjs[i]->isInnerCreate) { delete m_ExpandObjs[i]->model; m_ExpandObjs[i]->model = nullptr; }
	delete m_pScnMdl;
	m_pScnMdl = nullptr;
	m_Materials.clear();
	cKingdomModelBase::Term();
}

void cKingdomModel::CopyMaterial() {}

void cKingdomModel::RegistModel(Object* data, uint option)
{
	CustomKingdomUtil::Assert(nullptr == m_pScnMdl, "error");
	const ResHandle* h = dynamic_cast<const ResHandle*>(data);
	ResHandle viaObject;   // Instantiate(gameObject): a new model of the same entry as the data-only object LoadModelObject made
	if (h == nullptr)
	{
		if (const GameObject* go = dynamic_cast<const GameObject*>(data))
			if (go->native != nullptr) { viaObject.entry = static_cast<cKingdomModel*>(go->native)->m_native->key; h = &viaObject; }
	}
	if (h == nullptr || h->entry.empty())
	{
		std::fprintf(stderr, "cKingdomModel::RegistModel: model not found (%s)\n", data != nullptr ? data->name.c_str() : "null (key not in the attached archives)");
		return;
	}
	m_native = new Native();
	m_native->draw = LoadNativeModel(h->entry);
	m_native->key = h->entry;
	if (!m_native->draw) std::fprintf(stderr, "cKingdomModel::RegistModel: %s has no model\n", h->entry.c_str());
	m_pScnMdl = new GameObject();
	m_pScnMdl->name = h->entry;
	m_pScnMdl->native = this;
	NativeScene::Add(this);
}

void cKingdomModel::RegistModel(string key, uint option)
{
	Object* data = SearchData(key);
	if (data == nullptr) std::fprintf(stderr, "cKingdomModel::RegistModel: '%s' is not in the attached archives\n", key.c_str());
	RegistModel(data, option);
}

void cKingdomModel::RegistExpand(uint exp_idx, string node, cKingdomModel* exp_model)
{
	CustomKingdomUtil::Assert(8 > exp_idx, "error");
	m_ExpandObjs[exp_idx]->model = exp_model;
	m_ExpandObjs[exp_idx]->isInnerCreate = false;
	m_ExpandObjs[exp_idx]->node = node;
	m_bUpdateExpand = true;
	if (exp_model != nullptr)
	{
		exp_model->m_bExpandChild = true;   // drawn by this model
		NativeScene::Remove(exp_model);
	}
}

void cKingdomModel::RegistExpand(uint exp_idx, string node, Object* data, uint option)
{
	CustomKingdomUtil::Assert(8 > exp_idx, "error");
	cKingdomModel* child = new cKingdomModel();
	child->RegistModel(data, option);
	child->SetLightSetIdx(GetLightSetIdx());
	RegistExpand(exp_idx, node, child);
	m_ExpandObjs[exp_idx]->isInnerCreate = true;
}

void cKingdomModel::RegistExpand(uint exp_idx, string node) {}

void cKingdomModel::RegistExpand(uint exp_idx, string node, string key, uint option)
{
	RegistExpand(exp_idx, node, SearchData(key), option);
}

void cKingdomModel::ReleaseExpand(uint exp_idx)
{
	CustomKingdomUtil::Assert(8 > exp_idx, "error");
	if (m_ExpandObjs[exp_idx]->isInnerCreate) delete m_ExpandObjs[exp_idx]->model;
	m_ExpandObjs[exp_idx]->model = nullptr;
	m_ExpandObjs[exp_idx]->isInnerCreate = false;
}

void cKingdomModel::DebugPrint()
{
	float w[16];
	CalcWorld(w);
	std::fprintf(stderr, "  %s model %-28s at (%.2f %.2f %.2f) scale (%.2f %.2f %.2f) %s\n", IsVisible() ? "vis" : "HID", m_native ? m_native->key.c_str() : "?", w[12], w[13], w[14], std::sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]), std::sqrt(w[4] * w[4] + w[5] * w[5] + w[6] * w[6]), std::sqrt(w[8] * w[8] + w[9] * w[9] + w[10] * w[10]), m_useMtx ? "mtx" : "trs");
}

void cKingdomModel::UpdateFrame()
{
	cKingdomModelBase::UpdateFrame();   // ScnMdlSimple::UpdateFrame
	if (!m_bUpdateExpand) return;       // +0x45
	for (sExpandObj* e : m_ExpandObjs)
		if (e != nullptr && e->model != nullptr && e->model != this) e->model->UpdateFrame();
}

void cKingdomModel::DrawNative(const float eye[3])
{
	float w[16];
	CalcWorld(w);
	// sky domes follow the camera (the title / stage sky models sit at the eye)
	if (m_native != nullptr && m_native->key.find("sky") != std::string::npos) { w[12] = eye[0]; w[13] = eye[1]; w[14] = eye[2]; }
	DrawNativeAt(w, eye);
}

void cKingdomModel::DrawNativeAt(const float world[16], const float eye[3])
{
	DrawNativeModel(m_native, world, GetLightSetIdx());
	if (!m_bUpdateExpand) return;
	for (int i = 0; i < (int)std::size(m_ExpandObjs); i++)
	{
		cKingdomModel* child = m_ExpandObjs[i] != nullptr ? m_ExpandObjs[i]->model : nullptr;
		if (child == nullptr || !child->IsVisible()) continue;
		float node[12] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
		if (!m_ExpandObjs[i]->node.empty()) NativeNodeMatrix(m_native, m_ExpandObjs[i]->node, node);
		// node matrix: 3x4 row major -> column major 4x4
		const float nodeCM[16] = {node[0], node[4], node[8], 0, node[1], node[5], node[9], 0, node[2], node[6], node[10], 0, node[3], node[7], node[11], 1};
		float local[16], onNode[16], full[16];
		child->CalcWorld(local);
		Mul(world, nodeCM, onNode);
		Mul(onNode, local, full);
		child->DrawNativeAt(full, eye);
	}
}

}
