#pragma once

#include "common.hpp"

class Extentions
{
public:
	static void SampleFast(Animation* anim, float time);

	static void ProgressFast(Animation* anim, float progress);

	static void Sample(Animation* anim, float time);

	static void Sample(Animation* anim, const string& animName, float time);

	static void Progress(Animation* anim, float progress);

	static void Progress(Animation* anim, const string& animName, float progress);

	static Material* GetMaterial(GameObject* obj, int index);

	static std::vector<Material*> GetAllMaterial(GameObject* obj);

	static void SetRendererEnable(GameObject* obj, bool sw);

	static Vector3 GetBoneVector(GameObject* obj, int id);

	static Transform* GetBone(GameObject* obj, string name);

	static Transform* GetBone(GameObject* obj, int id);

	static Matrix4x4 GetBoneMatrix(GameObject* obj, int id);

private:
	static std::vector<Transform*> GetChildList(Transform* root);

public:
	static Vector3 GetUnityPos(Vector3* originPos);

	static Vector4 GetUnityPos(Vector4* originPos);
	// the C# `this` extension methods are called on values: reference forms
	static Vector3 GetUnityPos(Vector3& v) { return GetUnityPos(&v); }
	static Vector4 GetUnityPos(Vector4& v) { return GetUnityPos(&v); }
	static Vector3 GetUnityPos(const Vector3& v) { Vector3 t = v; return GetUnityPos(&t); }
	static Vector4 GetUnityPos(const Vector4& v) { Vector4 t = v; return GetUnityPos(&t); }
	static Vector3 GetUnityRot(Vector3& v) { return GetUnityRot(&v); }
	static Vector4 GetUnityRot(Vector4& v) { return GetUnityRot(&v); }

	static Vector3 GetUnityRot(Vector3* originRot);

	static Vector4 GetUnityRot(Vector4* originRot);

	static Vector3 GetUnityRot(Quaternion* originRot);

	static Quaternion GetUnityQtn(Quaternion* originRot);

	static void SetLayerRecursively(GameObject* self, int layer);

	static Vector3 GetPosition(const Matrix4x4& self);

	static Matrix4x4 ConvertCoordinateSystem(const Matrix4x4& self);
};
