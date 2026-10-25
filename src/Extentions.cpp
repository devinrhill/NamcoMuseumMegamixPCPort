#include "Extentions.hpp"

// Unity extension methods of the C# port. The native build has no Unity scene graph: the GameObject / Animation helpers do nothing, but the
// coordinate conversions are real: the converted code was written for the X-mirrored Unity import of the models (see ResNode.cpp) and converts back to
// the Wii convention with these (all of them are involutions).
void Extentions::SampleFast(Animation*, float) {}
void Extentions::ProgressFast(Animation*, float) {}
void Extentions::Sample(Animation*, float) {}
void Extentions::Sample(Animation*, const string&, float) {}
void Extentions::Progress(Animation*, float) {}
void Extentions::Progress(Animation*, const string&, float) {}
Material* Extentions::GetMaterial(GameObject*, int) { return nullptr; }
std::vector<Material*> Extentions::GetAllMaterial(GameObject*) { return {}; }
void Extentions::SetRendererEnable(GameObject*, bool) {}
Vector3 Extentions::GetBoneVector(GameObject*, int) { return Vector3(0.0f, 0.0f, 0.0f); }
Transform* Extentions::GetBone(GameObject*, string) { return nullptr; }
Transform* Extentions::GetBone(GameObject*, int) { return nullptr; }
Matrix4x4 Extentions::GetBoneMatrix(GameObject*, int) { return Matrix4x4::identity(); }
std::vector<Transform*> Extentions::GetChildList(Transform*) { return {}; }
Vector3 Extentions::GetUnityPos(Vector3* p) { return Vector3(-p->x, p->y, p->z); }
Vector4 Extentions::GetUnityPos(Vector4* p) { return Vector4(-p->x, p->y, p->z, p->w); }
Vector3 Extentions::GetUnityRot(Vector3* r) { return Vector3(r->x, -r->y, r->z); }
Vector4 Extentions::GetUnityRot(Vector4* r) { return Vector4(r->x, -r->y, r->z, r->w); }
Vector3 Extentions::GetUnityRot(Quaternion* q) { Vector3 e = q->eulerAngles(); return Vector3(e.x, -e.y, -e.z); }
Quaternion Extentions::GetUnityQtn(Quaternion* q) { return Quaternion(-q->x, q->y, q->z, -q->w); }
void Extentions::SetLayerRecursively(GameObject*, int) {}
Vector3 Extentions::GetPosition(const Matrix4x4& self) { return Vector3(self.m03, self.m13, self.m23); }
// mirror in X: M * A * M with M = diag(-1, 1, 1, 1) (position x flips, the rotation is mirrored, scale unchanged)
Matrix4x4 Extentions::ConvertCoordinateSystem(const Matrix4x4& self)
{
	Matrix4x4 r = self;
	for (int i = 0; i < 4; i++)
	{
		if (i != 0) { r.m[i * 4 + 0] = -r.m[i * 4 + 0]; }
	}
	for (int j = 0; j < 4; j++)
	{
		if (j != 0) { r.m[0 * 4 + j] = -r.m[0 * 4 + j]; }
	}
	return r;
}
