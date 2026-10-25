#pragma once
// Stand-ins for the UnityEngine types the C# port still refers to. Behaviour is filled in
// as the platform layer (raylib / OpenGL) grows; these exist so the game code compiles.

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include "common.hpp"

typedef float f32;
typedef double f64;

struct Vector2 {
	float x{};
	float y{};
	Vector2() {}
	Vector2(float x, float y) : x{x}, y{y} {}
	Vector2 operator+(const Vector2& v) const { return {x + v.x, y + v.y}; }
	Vector2 operator-(const Vector2& v) const { return {x - v.x, y - v.y}; }
	Vector2 operator*(float s) const { return {x * s, y * s}; }
	static const Vector2 zero;
	float& operator[](int i) { return i == 0 ? x : y; }
	float operator[](int i) const { return i == 0 ? x : y; }
};

inline const Vector2 Vector2::zero{0.0f, 0.0f};

struct Quaternion {
	float x{}, y{}, z{}, w{1.0f};
	Quaternion() {}
	Quaternion(float x, float y, float z, float w) : x{x}, y{y}, z{z}, w{w} {}
	static Quaternion identity() { return {0, 0, 0, 1}; }

	// Rotation of `degrees` around `axis` (Unity Quaternion.AngleAxis).
	static Quaternion AngleAxis(float degrees, const Vector3& axis)
	{
		float len = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
		if (len < 1e-8f) return identity();
		float half = degrees * 3.14159265358979323846f / 360.0f, s = std::sin(half) / len;
		return {axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
	}

	Quaternion operator*(const Quaternion& o) const
	{
		return {w * o.x + x * o.w + y * o.z - z * o.y,
		        w * o.y + y * o.w + z * o.x - x * o.z,
		        w * o.z + z * o.w + x * o.y - y * o.x,
		        w * o.w - x * o.x - y * o.y - z * o.z};
	}

	// Unity Quaternion.Euler: degrees, applied Z then X then Y.
	static Quaternion Euler(float xDeg, float yDeg, float zDeg)
	{
		const float h = 3.14159265358979323846f / 360.0f;
		Quaternion qx(std::sin(xDeg * h), 0, 0, std::cos(xDeg * h));
		Quaternion qy(0, std::sin(yDeg * h), 0, std::cos(yDeg * h));
		Quaternion qz(0, 0, std::sin(zDeg * h), std::cos(zDeg * h));
		return qy * qx * qz;
	}

	// Degrees, Unity order (Z, then X, then Y), each in [0, 360).
	Vector3 eulerAngles() const
	{
		const float rad2deg = 180.0f / 3.14159265358979323846f;
		float sx = 2.0f * (w * x - y * z);
		sx = sx > 1.0f ? 1.0f : (sx < -1.0f ? -1.0f : sx);
		Vector3 e;
		e.x = std::asin(sx);
		e.y = std::atan2(2.0f * (w * y + z * x), 1.0f - 2.0f * (x * x + y * y));
		e.z = std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (x * x + z * z));
		e.x *= rad2deg; e.y *= rad2deg; e.z *= rad2deg;
		auto wrap = [](float a) { a = std::fmod(a, 360.0f); return a < 0.0f ? a + 360.0f : a; };
		e.x = wrap(e.x); e.y = wrap(e.y); e.z = wrap(e.z);
		return e;
	}
};

struct Color {
	float r{1}, g{1}, b{1}, a{1};
	Color() {}
	Color(float r, float g, float b, float a = 1.0f) : r{r}, g{g}, b{b}, a{a} {}
	static Color Lerp(const Color& a, const Color& b, float t)
	{
		t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
		return Color(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t);
	}
};

struct StringBuilder
{
	std::string s;
	StringBuilder& append(const std::string& v) { s += v; return *this; }
	StringBuilder& append(const char* v) { s += v; return *this; }
	StringBuilder& clear() { s.clear(); return *this; }
	std::string ToString() const { return s; }
	std::string toString() const { return s; }
	operator std::string() const { return s; }
};

struct Ray;
struct Bounds {
	Vector3 center{};
	Vector3 size{};
	Bounds() {}
	Bounds(const Vector3& c, const Vector3& s) : center{c}, size{s} {}
	bool Intersects(const Bounds& o) const;
	bool IntersectRay(const Ray& ray, float& distance) const;
	float SqrDistance(const Vector3& p) const;
	void Expand(float amount) { size = Vector3(size.x + amount, size.y + amount, size.z + amount); }
	void Expand(const Vector3& amount) { size = Vector3(size.x + amount.x, size.y + amount.y, size.z + amount.z); }
	Vector3 extents() const { return Vector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f); }
	Vector3 min() const { return Vector3(center.x - size.x * 0.5f, center.y - size.y * 0.5f, center.z - size.z * 0.5f); }
	Vector3 max() const { return Vector3(center.x + size.x * 0.5f, center.y + size.y * 0.5f, center.z + size.z * 0.5f); }
	void SetMinMax(const Vector3& lo, const Vector3& hi) { size = Vector3(hi.x - lo.x, hi.y - lo.y, hi.z - lo.z); center = Vector3(lo.x + size.x * 0.5f, lo.y + size.y * 0.5f, lo.z + size.z * 0.5f); }
};

// UnityEngine.Plane: normal . p + distance = 0
struct Plane {
	Vector3 normal{};
	float distance{};
	Plane() {}
	Plane(const Vector3& n, float d) : normal{n}, distance{d} {}
	Plane(const Vector3& n, const Vector3& p) : normal{n}, distance{-(n.x * p.x + n.y * p.y + n.z * p.z)} {}
	float GetDistanceToPoint(const Vector3& p) const { return normal.x * p.x + normal.y * p.y + normal.z * p.z + distance; }
	bool Raycast(const Ray& ray, float& enter) const;
};

enum class ShadowCastingMode { Off, On, TwoSided, ShadowsOnly };

namespace Mathf {
constexpr float PI = 3.14159265358979323846f;
inline float Abs(float v) { return std::fabs(v); }
inline int Abs(int v) { return std::abs(v); }
inline float Sin(float v) { return std::sin(v); }
inline float Cos(float v) { return std::cos(v); }
inline float Acos(float v) { return std::acos(v); }
inline float Atan(float v) { return std::atan(v); }
inline float Atan2(float y, float x) { return std::atan2(y, x); }
inline float Sqrt(float v) { return std::sqrt(v); }
inline float Floor(float v) { return std::floor(v); }
inline int FloorToInt(float v) { return (int)std::floor(v); }
inline int RoundToInt(float v) { return (int)std::lround(v); }
inline float Max(float a, float b) { return a > b ? a : b; }
inline float Max(float a, float b, float c) { return Max(Max(a, b), c); }
inline int Max(int a, int b) { return a > b ? a : b; }
inline float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline int Clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float Min(float a, float b) { return a < b ? a : b; }
inline float Min(float a, float b, float c) { return Min(Min(a, b), c); }
inline int Min(int a, int b) { return a < b ? a : b; }
inline bool Approximately(float a, float b) { return std::fabs(a - b) < 1e-6f; }
}

namespace Random {
inline int Range(int lo, int hi) { return hi <= lo ? lo : lo + std::rand() % (hi - lo); }
inline float Range(float lo, float hi) { return lo + (hi - lo) * (std::rand() / (float)RAND_MAX); }
}

namespace Time {
inline float deltaTime = 1.0f / 60.0f;
}

class Object {
public:
	virtual ~Object() {}
	std::string name;
};

class Component : public Object {};
class Transform : public Component {
public:
	Vector3 position{};
	Vector3 localScale{1, 1, 1};
	Quaternion rotation{};
};
class GameObject : public Object {
public:
	Transform* transform{};
	void* native{};          // native build: the kingdom::cKingdomModel this stands for (see cKingdomModel::pScnMdl)
	bool activeSelf{true};
	void SetActive(bool b) { activeSelf = b; }
};
class MonoBehaviour : public Component {};
class Renderer : public Component {};
class Mesh;
class SkinnedMeshRenderer : public Renderer {
public:
	Mesh* sharedMesh{};
};
class MeshRenderer : public Renderer {};
class MeshFilter : public Component {};
class Mesh : public Object {};
class Material : public Object {};
class Texture : public Object {
public:
	int gfxId{0};      // gfx::TexId of the uploaded pixels (0 = none)
	int width{0};
	int height{0};
};
class Texture2D : public Texture {};
class Sprite : public Object {};
class AnimationClip : public Object {};
class Animation : public Component {};

class SoundHandle {
public:
	static void DetachSound(SoundHandle*&) {}
};

// ---- more Unity stand-ins (boot path) ---------------------------------------------------------
#include <optional>
#include "Ref.hpp"

// C# Nullable<T> / T?: assignable from nullptr, compared with HasValue.
template<class T>
struct Nullable {
	T Value{};
	bool HasValue{false};
	bool has_value() const { return HasValue; }
	const T& value() const { return Value; }
	T& value() { return Value; }
	Nullable() {}
	Nullable(std::nullptr_t) {}
	Nullable(const T& v) : Value{v}, HasValue{true} {}
	bool operator==(std::nullptr_t) const { return !HasValue; }
	bool operator!=(std::nullptr_t) const { return HasValue; }
};

// Coroutines (IEnumerator/yield) are not ported yet; their bodies are TODO(unity).
class IEnumerator {};

struct Color32 {
	byte r{}, g{}, b{}, a{255};
	Color32() {}
	Color32(byte r, byte g, byte b, byte a = 255) : r{r}, g{g}, b{b}, a{a} {}
};

struct Ray {
	Vector3 origin{};
	Vector3 direction{};
	Ray() {}
	Ray(const Vector3& o, const Vector3& d) : origin{o}, direction{d.normalized()} {}
	Vector3 GetPoint(float t) const { return origin + direction * t; }
};

inline bool Bounds::Intersects(const Bounds& o) const
{
	const Vector3 a0 = min(), a1 = max(), b0 = o.min(), b1 = o.max();
	return a0.x <= b1.x && a1.x >= b0.x && a0.y <= b1.y && a1.y >= b0.y && a0.z <= b1.z && a1.z >= b0.z;
}

// slab test; distance = entry distance (0 when the origin is inside)
inline bool Bounds::IntersectRay(const Ray& ray, float& distance) const
{
	const Vector3 lo = min(), hi = max();
	float tmin = 0.0f, tmax = 1e30f;
	const float o[3] = {ray.origin.x, ray.origin.y, ray.origin.z}, d[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
	const float l[3] = {lo.x, lo.y, lo.z}, h[3] = {hi.x, hi.y, hi.z};
	for (int i = 0; i < 3; i++)
	{
		if (std::fabs(d[i]) < 1e-9f)
		{
			if (o[i] < l[i] || o[i] > h[i]) { distance = 0.0f; return false; }
			continue;
		}
		float t0 = (l[i] - o[i]) / d[i], t1 = (h[i] - o[i]) / d[i];
		if (t0 > t1) { const float t = t0; t0 = t1; t1 = t; }
		tmin = t0 > tmin ? t0 : tmin;
		tmax = t1 < tmax ? t1 : tmax;
		if (tmin > tmax) { distance = 0.0f; return false; }
	}
	distance = tmin;
	return true;
}

inline float Bounds::SqrDistance(const Vector3& p) const
{
	const Vector3 lo = min(), hi = max();
	const float dx = p.x < lo.x ? lo.x - p.x : (p.x > hi.x ? p.x - hi.x : 0.0f);
	const float dy = p.y < lo.y ? lo.y - p.y : (p.y > hi.y ? p.y - hi.y : 0.0f);
	const float dz = p.z < lo.z ? lo.z - p.z : (p.z > hi.z ? p.z - hi.z : 0.0f);
	return dx * dx + dy * dy + dz * dz;
}

namespace GeometryUtility
{
// bounds of the 8 corners (or any points) transformed by the matrix (row-major, column vectors)
inline Bounds CalculateBounds(const Vector3* pts, const Matrix4x4& m)
{
	Vector3 lo(1e30f, 1e30f, 1e30f), hi(-1e30f, -1e30f, -1e30f);
	for (int i = 0; i < 8; i++)
	{
		const Vector3& p = pts[i];
		const Vector3 q(m.m[0] * p.x + m.m[1] * p.y + m.m[2] * p.z + m.m[3], m.m[4] * p.x + m.m[5] * p.y + m.m[6] * p.z + m.m[7], m.m[8] * p.x + m.m[9] * p.y + m.m[10] * p.z + m.m[11]);
		lo = Vector3(q.x < lo.x ? q.x : lo.x, q.y < lo.y ? q.y : lo.y, q.z < lo.z ? q.z : lo.z);
		hi = Vector3(q.x > hi.x ? q.x : hi.x, q.y > hi.y ? q.y : hi.y, q.z > hi.z ? q.z : hi.z);
	}
	Bounds b;
	b.SetMinMax(lo, hi);
	return b;
}
}

inline bool Plane_Raycast(const struct Plane& pl, const Ray& ray, float& enter);

enum AmbientMode { Skybox, Trilight, Flat, Custom };

class Shader : public Object {};
class Light : public Component {};
class Text : public Component {};
class TMP_Text : public Component {};
class Image : public Component {};
class RawImage : public Component {};
class PostProcessLayer : public Component {};
class TextAsset : public Object {
public:
	std::vector<byte> bytes;
};

class ScriptableObject : public Object {};

template<class... A> using Action = std::function<void(A...)>;
template<class R, class... A> using Func = std::function<R(A...)>;

struct Vector2Int {
	int x{};
	int y{};
	Vector2Int() {}
	Vector2Int(int x, int y) : x{x}, y{y} {}
};




// ---- Matrix4x4 members that need Vector3 / Quaternion ---------------------------------------------
inline Matrix4x4 Matrix4x4::Translate(const Vector3& v)
{
	Matrix4x4 m = Matrix4x4::identity();
	m.m03 = v.x; m.m13 = v.y; m.m23 = v.z;
	return m;
}

inline Matrix4x4 Matrix4x4::Translate(const Vector4& v)
{
	Matrix4x4 m = Matrix4x4::identity();
	m.m03 = v.x; m.m13 = v.y; m.m23 = v.z;
	return m;
}

inline Matrix4x4 Matrix4x4::Rotate(const Quaternion& q)
{
	Matrix4x4 m = Matrix4x4::identity();
	const float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
	const float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
	const float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
	m.m00 = 1 - 2 * (yy + zz); m.m01 = 2 * (xy - wz);     m.m02 = 2 * (xz + wy);
	m.m10 = 2 * (xy + wz);     m.m11 = 1 - 2 * (xx + zz); m.m12 = 2 * (yz - wx);
	m.m20 = 2 * (xz - wy);     m.m21 = 2 * (yz + wx);     m.m22 = 1 - 2 * (xx + yy);
	return m;
}

inline Matrix4x4 Matrix4x4::Scale(const Vector3& v)
{
	Matrix4x4 m = Matrix4x4::identity();
	m.m00 = v.x; m.m11 = v.y; m.m22 = v.z;
	return m;
}

// Unity Matrix4x4.SetTRS: translate * rotate * scale
inline void Matrix4x4::SetTRS(const Vector3& pos, const Quaternion& rot, const Vector3& scale)
{
	*this = Matrix4x4::Translate(pos) * Matrix4x4::Rotate(rot) * Matrix4x4::Scale(scale);
}

inline Vector4 operator*(const Matrix4x4& m, const Vector4& v)
{
	return Vector4(m.m00 * v.x + m.m01 * v.y + m.m02 * v.z + m.m03 * v.w,
	               m.m10 * v.x + m.m11 * v.y + m.m12 * v.z + m.m13 * v.w,
	               m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m23 * v.w,
	               m.m30 * v.x + m.m31 * v.y + m.m32 * v.z + m.m33 * v.w);
}

// UnityEngine.Plane.Raycast: enter = distance along the ray, true when the ray (forward only) hits the plane
inline bool Plane::Raycast(const Ray& ray, float& enter) const
{
	const float denom = Vector3::Dot(ray.direction, normal);
	const float num = -Vector3::Dot(ray.origin, normal) - distance;
	if (std::fabs(denom) < 1e-9f) { enter = 0.0f; return false; }
	enter = num / denom;
	return enter > 0.0f;
}
