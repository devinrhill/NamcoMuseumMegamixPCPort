#include "ResNode.hpp"

#include <cmath>

#include "RMDLUserDataParam.hpp"
#include "ResUserData.hpp"

// Native ResNode: a node of a parsed MDL0 (the Unity version walked the GameObject hierarchy of the imported model; original in old/BACKUP_unity_headers/).
// Hierarchy: the Wii model's node list is the tree; "root" is node 0. User data (per node parameters of the gimmicks) is not parsed yet: always empty.
namespace
{
Quaternion EulerDeg(const float r[3])
{
	const float d = 3.14159265f / 180.0f;
	const float cx = std::cos(r[0] * d * 0.5f), sx = std::sin(r[0] * d * 0.5f), cy = std::cos(r[1] * d * 0.5f), sy = std::sin(r[1] * d * 0.5f), cz = std::cos(r[2] * d * 0.5f), sz = std::sin(r[2] * d * 0.5f);
	// R = Rz * Ry * Rx
	return Quaternion(sx * cy * cz - cx * sy * sz, cx * sy * cz + sx * cy * sz, cx * cy * sz - sx * sy * cz, cx * cy * cz + sx * sy * sz);
}
}

ResNode::ResNode()
{
}

ResNode::ResNode(Transform* baseNode)
{
	This = baseNode;
}

ResNode::ResNode(const wii::Mdl0* mdl, int idx) : Mdl(mdl), Idx(idx)
{
	if (mdl != nullptr && idx >= 0 && idx < (int)mdl->nodes.size())
	{
		const wii::Mdl0Node& n = mdl->nodes[idx];
		This = new Transform();
		This->name = n.name;
		This->position = Vector3(n.translate[0], n.translate[1], n.translate[2]);
		This->localScale = Vector3(n.scale[0], n.scale[1], n.scale[2]);
		This->rotation = EulerDeg(n.rotate);
	}
}

bool ResNode::IsValid()
{
	return Idx >= 0 || nullptr != This;
}

bool ResNode::IsActive()
{
	return IsValid();
}

string ResNode::GetName()
{
	return (Mdl != nullptr && Idx >= 0) ? Mdl->nodes[Idx].name : string();
}

ResNode* ResNode::GetChildNode(int idx)
{
	if (Mdl == nullptr || Idx < 0) return new ResNode();
	int n = 0;
	for (const auto& c : Mdl->nodes)
		if (c.parent == Idx && n++ == idx) return new ResNode(Mdl, c.id);
	return new ResNode();
}

ResUserData* ResNode::GetResUserData()
{
	if (UserData == nullptr)
	{
		UserData = (Mdl != nullptr && Idx >= 0) ? new ResUserData(&Mdl->nodes[Idx].userData) : new ResUserData();
	}
	return UserData;
}

// The converted game code was written against the Unity import of the models, which mirrored X: positions arrive as (-x, y, z) and euler angles as
// (x, -y, -z), and the code flips them back (cTaskBgBase::Regist: vTrans = (-p.x, p.y, p.z)). The native node data is the Wii's own, so these two
// return the mirrored (Unity) convention to keep that code correct; GetTransRotMatrix(convert = true) is the Wii convention again.
Vector4 ResNode::GetTranslate(bool isLocal)
{
	if (Mdl == nullptr || Idx < 0) return Vector4();
	const wii::Mdl0Node& n = Mdl->nodes[Idx];
	if (isLocal) return Vector4(-n.translate[0], n.translate[1], n.translate[2], 1.0f);
	return Vector4(-n.model[3], n.model[7], n.model[11], 1.0f);
}

Vector4 ResNode::GetRotate(bool isLocal)
{
	if (Mdl == nullptr || Idx < 0) return Vector4();
	const wii::Mdl0Node& n = Mdl->nodes[Idx];
	if (isLocal) return Vector4(n.rotate[0], -n.rotate[1], -n.rotate[2], 1.0f);   // degrees (localEulerAngles)
	// world: euler of the bind pose rotation (R = Rz * Ry * Rx), degrees
	const float* m = n.model;
	const float sx = std::sqrt(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]);
	const float sy = sx > 1e-6f ? -m[8] / sx : 0.0f;
	const float r2d = 180.0f / 3.14159265f;
	const float ex = std::atan2(m[9], m[10]) * r2d, ey = std::asin(sy < -1 ? -1 : (sy > 1 ? 1 : sy)) * r2d, ez = std::atan2(m[4], m[0]) * r2d;
	return Vector4(ex, -ey, -ez, 1.0f);
}

Vector4 ResNode::GetScale()
{
	if (Mdl == nullptr || Idx < 0) return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	const wii::Mdl0Node& n = Mdl->nodes[Idx];
	return Vector4(n.scale[0], n.scale[1], n.scale[2], 1.0f);
}

// translation + rotation of the local pose (scale 1)
Matrix4x4 ResNode::GetTransRotMatrix(bool convert)
{
	Matrix4x4 m = Matrix4x4::identity();
	if (Mdl == nullptr || Idx < 0) return m;
	const wii::Mdl0Node& n = Mdl->nodes[Idx];
	if (convert) m.SetTRS(Vector3(n.translate[0], n.translate[1], n.translate[2]), EulerDeg(n.rotate), Vector3(1.0f, 1.0f, 1.0f));
	else
	{
		const float r[3] = {n.rotate[0], -n.rotate[1], -n.rotate[2]};
		m.SetTRS(Vector3(-n.translate[0], n.translate[1], n.translate[2]), EulerDeg(r), Vector3(1.0f, 1.0f, 1.0f));
	}
	return m;
}

void ResNode::SetNodeUserData(ResNode* node)
{
}
