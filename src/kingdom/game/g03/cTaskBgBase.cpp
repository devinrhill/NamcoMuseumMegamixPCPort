#include "kingdom/game/g03/cTaskBgBase.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "ResUserDataItem.hpp"
#include "RtpaParam.hpp"
#include <Constants.hpp>
#include <ResourceCache.hpp>

namespace kingdom::game::g03
{

cTaskBgBase::cTaskBgBase()
{
	SetName("cTaskBgBase");
	Init();
}

cTaskBgBase::cTaskBgBase(ResNode* const& node0, ResNode* const& node1)
{
	Init();
	Regist(node0, node1);
}

void cTaskBgBase::Init()
{
}

void cTaskBgBase::Term()
{
	cTaskPBModel::Term();
}

void cTaskBgBase::Regist(ResNode* const& node0, ResNode* const& node1)
{
	CustomKingdomUtil::Assert(node0 != nullptr, "nullpo");
	CustomKingdomUtil::Assert(node1 != nullptr, "nullpo");
	string outStr;
	CustomKingdomUtil::snprintf(outStr, -1, "%s.brres", node0->GetName());
	Object* data = ResourceDatabase::Get<Object>(outStr, ResourceCache::EType::Temporary);
	this->Model->RegistModel(data);
	Vector3 vector = node1->GetTranslate();
	::Constants::jetGeoVecSet(this->Model->vTrans(), 0.0f - vector.x, vector.y, vector.z, 1.0f);
	vector = node1->GetRotate();
	::Constants::jetGeoVecSet(this->Model->vRotate(), ::Constants::JET_DEG2RAD(vector.x), 0.0f - ::Constants::JET_DEG2RAD(vector.y), 0.0f - ::Constants::JET_DEG2RAD(vector.z), 1.0f);
	vector = node1->GetScale();
	::Constants::jetGeoVecSet(this->Model->vScale(), vector.x, vector.y, vector.z, 1.0f);
	this->Model->SetLightSetIdx(0);
	this->Model->SetFogIdx(0);
	SetUserDataAnm(this->Model, node0);
	if (node1 != node0)
	{
		SetUserDataAnm(this->Model, node1);
	}
}

// the C# coroutine only spread Regist over several frames; native: synchronous
IEnumerator* cTaskBgBase::coRegist(ResNode* node0, ResNode* node1)
{
	Regist(node0, node1);
	return nullptr;
}

void cTaskBgBase::SetUserDataAnm(cKingdomModel* model, ResNode* const& node)
{
	CustomKingdomUtil::Assert(model != nullptr, "nullpo");
	CustomKingdomUtil::Assert(node != nullptr, "nullpo");
	ResUserData* resUserData = node->GetResUserData();
	float num = 0.0f;
	bool flag = false;
	string text = "";
	// FIXME(cs2cpp): unresolved identifier Regex
	// TODO(unity): Match match = Regex.Match(model.pScnMdl().name, "_bg[0-9]\\(Clone\\)$");
	// FIXME(cs2cpp): member Success on <unknown Match>
	// TODO(unity): if (match.Success)
	// 		{
	// 			text = match.Value.Replace("(Clone)", "");
	// 			Debug.LogWarning(text);
	// 		}
	if (!resUserData->IsValid())
	{
		return;
	}
	for (int i = 0; i < resUserData->GetItemNumEntries(); i++)
	{
		ResUserDataItem* item = resUserData->GetItem(i);
		if (item->IsValid() && (CustomKingdomUtil::strstr(item->GetName(), "frame") || CustomKingdomUtil::strstr(item->GetName(), "Frame")) && !CustomKingdomUtil::strstr(item->GetName(), "_rcha") && !CustomKingdomUtil::strstr(item->GetName(), "_rvia") && !CustomKingdomUtil::strstr(item->GetName(), "_rcla") && !CustomKingdomUtil::strstr(item->GetName(), "_rtpa") && !CustomKingdomUtil::strstr(item->GetName(), "_rtsa"))
		{
			num = item->GetF32();
			flag = true;
			break;
		}
	}
	for (int j = 0; j < resUserData->GetItemNumEntries(); j++)
	{
		ResUserDataItem* item = resUserData->GetItem(j);
		if (!item->IsValid())
		{
			continue;
		}
		string outStr;
		CustomKingdomUtil::snprintf(outStr, -1, "%s", item->GetName());
		string text2 = CustomKingdomUtil::strrchr(outStr, '_');
		if (text2.empty() || (!CustomKingdomUtil::strstr(text2, "_rcha") && !CustomKingdomUtil::strstr(text2, "_rvia") && !CustomKingdomUtil::strstr(text2, "_rcla") && !CustomKingdomUtil::strstr(text2, "_rtpa") && !CustomKingdomUtil::strstr(text2, "_rtsa")))
		{
			continue;
		}
		// "<anim>_rcha" -> "<anim>_cha" (the user data names the animation of this node, the value is its speed); regex "_r(.{3})$" -> "_$1"
		if (outStr.size() >= 5 && outStr[outStr.size() - 5] == '_' && outStr[outStr.size() - 4] == 'r') outStr.erase(outStr.size() - 4, 1);
		CustomKingdomUtil::snprintf(outStr, -1, "%s.brres", outStr);
		string text3 = ResourceDatabase::GetPath(outStr);
		Object* obj = ResourceDatabase::Get<Object>(text3);
		if (obj == nullptr)
		{
			continue;   // C#: "warning! not found bg anme"
		}
		const int num2 = model->RegistAnmNoIdx(obj);
		const float f = item->GetF32();
		model->SetUpdateRateAll((uint)num2, f);
		if (flag)
		{
			model->SetFrameAnmAll((uint)num2, num);   // the Unity clip time was seconds (num / 60); native frames
		}
	}
}

void cTaskBgBase::SetMaterialColor(Color color)
{
}

void cTaskBgBase::SetMaterialAlpha(byte alpha)
{
}

}
