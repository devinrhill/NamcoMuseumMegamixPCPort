#include "kingdom/game/g03/cTaskEffect.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/MslRand.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "ResUserDataItem.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskEffect::cTaskEffect()
{
	SetName("cTaskEffect");
	Init();
}

cTaskEffect::cTaskEffect(ResNode* nodeEff)
{
	Init();
	if (nodeEff != nullptr)
	{
		Regist(nodeEff);
		return;
	}
	this->get_KingdomPtkMan()->Create(37u, 64u, 512u, 4096u);
	RegistPreset();
}

cTaskEffect::~cTaskEffect()
{
	Term();
}

void cTaskEffect::Init()
{
	m_vecPtkHnd.reserve(64);
}

void cTaskEffect::Term()
{
}

bool cTaskEffect::Execute(cJetTask* pParam)
{
	Constants::PACN_APP()->pTaskCamera();
	if (Constants::IS_PAC_PAUSE())
	{
		this->get_KingdomPtkMan()->UpdateFrame(0.0f);
	}
	else
	{
		this->get_KingdomPtkMan()->UpdateFrame();
	}
	return true;
}

void cTaskEffect::Regist()
{
	this->get_KingdomPtkMan()->Create(37u, 64u, 512u, 4096u);
	RegistPreset();
}

void cTaskEffect::Regist(ResNode* nodeEff)
{
	CustomKingdomUtil::Assert(nodeEff != nullptr, "nullpo");
	uint num = 0u;
	int num2 = 0;
	ResNode* childNode = nodeEff->GetChildNode(num2);
	while (childNode->IsValid())
	{
		if (childNode->IsActive())
		{
			num++;
		}
		num2++;
		childNode = nodeEff->GetChildNode(num2);
	}
	this->get_KingdomPtkMan()->Create(37 + num, 64u, 512u, 4096u);
	RegistPreset();
	uint num3 = 37u;
	Vector4 v = Vector4::zero;
	Vector4 v2 = Vector4::zero;
	Vector4 v3 = Vector4::zero;
	uint num4 = 0u;
	uint num5 = 0u;
	int num6 = 0;
	childNode = nodeEff->GetChildNode(num6);
	while (childNode->IsValid())
	{
		if (!childNode->IsActive())
		{
			num3++;
			num6++;
			childNode = nodeEff->GetChildNode(num6);
			continue;
		}
		int num7 = 0;
		ResNode* childNode2 = childNode->GetChildNode(num7);
		num4 = GetDelay(childNode);
		this->get_KingdomPtkMan()->RegistData(num3, childNode->GetName());
		while (childNode2->IsValid())
		{
			if (!childNode2->IsActive())
			{
				num7++;
				childNode2 = childNode->GetChildNode(num7);
				continue;
			}
			Vector3 vector = childNode2->GetTranslate();
			::Constants::jetGeoVecSet(v, vector.x, vector.y, vector.z, 1.0f);
			vector = CustomKingdomUtil::ConvertEuler((Vector3)childNode2->GetRotate());
			::Constants::jetGeoVecSet(v2, ::Constants::JET_DEG2RAD(vector.x), ::Constants::JET_DEG2RAD(vector.y), ::Constants::JET_DEG2RAD(vector.z), 1.0f);
			vector = childNode2->GetScale();
			::Constants::jetGeoVecSet(v3, vector.x, vector.y, vector.z, 1.0f);
			v.x *= -1.0f;
			num5 = GetDelay(childNode2);
			// DOL 0x801786e0 [audited: described by the 43rd-session pass]: the node's own Delay, else rand() % the group's Delay (an integer in frames; the converted code used Unity's Random.Range(0f, delay))
			float fDelay = ((num5 != 0) ? ((float)num5) : ((num4 == 0) ? 0.0f : (float)kingdom::MslRandRange(0, (int)num4)));
			bool flag = false;
			if (CustomKingdomUtil::strstr(childNode->GetName(), "ef_magma01"))
			{
				flag = true;
			}
			cKingdomPtkHnd* item = new cKingdomPtkHnd(num3, false, false, !flag, fDelay, v, v2, v3, 0, 0);
			if (flag)
			{
				m_vecPtkHnd.push_back(item);
			}
			num7++;
			childNode2 = childNode->GetChildNode(num7);
		}
		num3++;
		num6++;
		childNode = nodeEff->GetChildNode(num6);
	}
}

uint cTaskEffect::GetDelay(ResNode* node)
{
	CustomKingdomUtil::Assert(node != nullptr, "nullpo");
	ResUserData* resUserData = node->GetResUserData();
	if (resUserData->IsValid())
	{
		for (int i = 0; i < resUserData->GetItemNumEntries(); i++)
		{
			ResUserDataItem* item = resUserData->GetItem(i);
			if (item->IsValid() && CustomKingdomUtil::strstr(item->GetName(), "Delay"))
			{
				return (uint)::Constants::fabs(item->GetS32());
			}
		}
	}
	return 0u;
}

void cTaskEffect::RegistPreset()
{
	this->get_KingdomPtkMan()->RegistData(0u, "BIN_EFF_GATE");
	this->get_KingdomPtkMan()->RegistData(1u, "BIN_EFF_DASH");
	this->get_KingdomPtkMan()->RegistData(2u, "BIN_EFF_STAR");
	this->get_KingdomPtkMan()->RegistData(3u, "BIN_EFF_DUST");
	this->get_KingdomPtkMan()->RegistData(4u, "BIN_EFF_CHANGE");
	this->get_KingdomPtkMan()->RegistData(5u, "BIN_EFF_HIT_S");
	this->get_KingdomPtkMan()->RegistData(6u, "BIN_EFF_HIT_L");
	this->get_KingdomPtkMan()->RegistData(7u, "BIN_EFF_SHOCK");
	this->get_KingdomPtkMan()->RegistData(8u, "BIN_EFF_WING");
	this->get_KingdomPtkMan()->RegistData(9u, "BIN_EFF_RIPPLE");
	this->get_KingdomPtkMan()->RegistData(10u, "BIN_EFF_SPLASH");
	this->get_KingdomPtkMan()->RegistData(11u, "BIN_EFF_SPLASH_POISON");
	this->get_KingdomPtkMan()->RegistData(12u, "BIN_EFF_BUBBLE");
	this->get_KingdomPtkMan()->RegistData(13u, "BIN_EFF_POW_COOKIE");
	this->get_KingdomPtkMan()->RegistData(14u, "BIN_EFF_SWEAT");
	this->get_KingdomPtkMan()->RegistData(15u, "BIN_EFF_FLAME");
	this->get_KingdomPtkMan()->RegistData(16u, "BIN_EFF_WIND");
	this->get_KingdomPtkMan()->RegistData(17u, "BIN_EFF_STORM");
	this->get_KingdomPtkMan()->RegistData(18u, "BIN_EFF_LEAF");
	this->get_KingdomPtkMan()->RegistData(19u, "BIN_EFF_CHECKPOINT");
	this->get_KingdomPtkMan()->RegistData(20u, "BIN_EFF_GOAL");
	this->get_KingdomPtkMan()->RegistData(21u, "BIN_EFF_EXPLOSION");
	this->get_KingdomPtkMan()->RegistData(22u, "BIN_EFF_BOSS_STORM");
	this->get_KingdomPtkMan()->RegistData(23u, "BIN_EFF_PIYO");
	this->get_KingdomPtkMan()->RegistData(24u, "BIN_EFF_SCREEN_SMOKE");
	this->get_KingdomPtkMan()->RegistData(25u, "BIN_EFF_CAR_SMOKE");
	this->get_KingdomPtkMan()->RegistData(26u, "BIN_EFF_BOSS_UFO");
	this->get_KingdomPtkMan()->RegistData(27u, "BIN_EFF_HOVER");
	this->get_KingdomPtkMan()->RegistData(28u, "BIN_EFF_BOAT_SPLASH");
	this->get_KingdomPtkMan()->RegistData(29u, "BIN_EFF_PLAIN_SMOKE");
	this->get_KingdomPtkMan()->RegistData(30u, "BIN_EFF_LOG_SMOKE");
	this->get_KingdomPtkMan()->RegistData(31u, "BIN_EFF_SWITCH");
	this->get_KingdomPtkMan()->RegistData(32u, "BIN_EFF_JET");
	this->get_KingdomPtkMan()->RegistData(33u, "BIN_EFF_WALL");
	this->get_KingdomPtkMan()->RegistData(34u, "BIN_EFF_FLOW");
	this->get_KingdomPtkMan()->RegistData(35u, "BIN_EFF_WORLD5WIND");
	this->get_KingdomPtkMan()->RegistData(36u, "BIN_EFF_BOSS_DEMO_00");
}

}
