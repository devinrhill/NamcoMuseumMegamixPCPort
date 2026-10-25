#pragma once

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::uint8_t u8;
typedef std::int8_t s8;
typedef std::uint16_t u16;
typedef std::int16_t s16;
typedef std::uint32_t u32;
typedef std::int32_t s32;
typedef std::uint32_t u32;
typedef std::int32_t s32;


typedef std::uint8_t byte;
typedef std::int8_t sbyte;
typedef std::uint32_t uint;
typedef std::int32_t sint;
typedef std::string string;

struct Vector4 {
	static const Vector4 zero;
	static const Vector4 unit;
	static const Vector4 one;

	float x;
	float y;
	float z;
	float w;

	Vector4() {}

	Vector4(float x, float y, float z): x{x}, y{y}, z{z}, w{1.0f} {}
	Vector4(const struct Vector3& v);

	Vector4(float x, float y, float z, float w):
		x{x},
		y{y},
		z{z},
		w{w} {}

	float& operator[](int index) {
		switch(index) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		default:
			return w;
		}
	}

		float sqrMagnitude() const { return x * x + y * y + z * z + w * w; }
		float magnitude() const { return std::sqrt(x * x + y * y + z * z + w * w); }

		Vector4 operator+(const Vector4& v) const {
		return {x + v.x, y + v.y, z + v.z, w + v.w};
	}

	Vector4 operator-(const Vector4& v) const {
		return {x - v.x, y - v.y, z - v.z, w - v.w};
	}

	Vector4 operator*(const Vector4& v) const {
		return {x * v.x, y * v.y, z * v.z, w * v.w};
	}

	Vector4 operator/(const Vector4& v) const {
		return {x / v.x, y / v.y, z / v.z, w / v.w};
	}

	Vector4 operator*(float s) const {
		return {x * s, y * s, z * s, w * s};
	}

	Vector4 operator/(float s) const {
		return {x / s, y / s, z / s, w / s};
	}

	Vector4& operator+=(const Vector4& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	Vector4& operator-=(const Vector4& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	Vector4& operator*=(const Vector4& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	Vector4& operator/=(const Vector4& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	Vector4& operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	Vector4& operator/=(float s) {
		x /= s;
		y /= s;
		z /= s;
		w /= s;
		return *this;
	}
};

inline Vector4 operator*(float s, const Vector4& v) {
	return v * s;
}

struct Vector3 {
	static const Vector3 zero;
	static const Vector3 one;
	static const Vector3 right;
	static const Vector3 up;
	static const Vector3 forward;

	float x;
	float y;
	float z;

	Vector3() {}

	Vector3(const Vector4& v) : x{v.x}, y{v.y}, z{v.z} {}

	Vector3(float x, float y, float z):
		x{x},
		y{y},
		z{z} {}

	float& operator[](int index) {
		switch(index) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return x;
		}
	}

	Vector3 operator+(const Vector3& v) const {
		return {x + v.x, y + v.y, z + v.z};
	}

	Vector3 operator-(const Vector3& v) const {
		return {x - v.x, y - v.y, z - v.z};
	}

	Vector3 operator*(const Vector3& v) const {
		return {x * v.x, y * v.y, z * v.z};
	}

	Vector3 operator/(const Vector3& v) const {
		return {x / v.x, y / v.y, z / v.z};
	}

	Vector3 operator*(float s) const {
		return {x * s, y * s, z * s};
	}

	Vector3 operator/(float s) const {
		return {x / s, y / s, z / s};
	}

	Vector3& operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3& operator-=(const Vector3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& operator*=(const Vector3& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vector3& operator/=(const Vector3& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	Vector3& operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	Vector3& operator/=(float s) {
		x /= s;
		y /= s;
		z /= s;
		return *this;
	}

	Vector3 operator-() const { return {-x, -y, -z}; }
	float sqrMagnitude() const { return x * x + y * y + z * z; }
	float magnitude() const { return std::sqrt(x * x + y * y + z * z); }
	Vector3 normalized() const
	{
		const float m = magnitude();
		return m > 1e-5f ? Vector3(x / m, y / m, z / m) : Vector3(0.0f, 0.0f, 0.0f);
	}
	static float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
	static Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
};

inline Vector3 operator*(float s, const Vector3& v) {
	return v * s;
}

inline Vector4::Vector4(const Vector3& v): x{v.x}, y{v.y}, z{v.z}, w{1.0f} {}
inline const Vector4 Vector4::zero{0.0f, 0.0f, 0.0f, 0.0f};
inline const Vector4 Vector4::unit{1.0f, 1.0f, 1.0f, 0.0f};
inline const Vector4 Vector4::one{1.0f, 1.0f, 1.0f, 1.0f};
inline const Vector3 Vector3::zero{0.0f, 0.0f, 0.0f};
inline const Vector3 Vector3::one{1.0f, 1.0f, 1.0f};
inline const Vector3 Vector3::right{1.0f, 0.0f, 0.0f};
inline const Vector3 Vector3::up{0.0f, 1.0f, 0.0f};
inline const Vector3 Vector3::forward{0.0f, 0.0f, 1.0f};

inline bool operator==(const Vector4& a, const Vector4& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
inline bool operator!=(const Vector4& a, const Vector4& b) { return !(a == b); }

struct Quaternion;

struct Matrix4x4 {
	union {
		float m[16];

		struct {
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};
	};

	Matrix4x4() {}

	// C# jagged style m[row][col] (the converted code indexes matrices this way)
	float* operator[](int row) { return &m[row * 4]; }
	const float* operator[](int row) const { return &m[row * 4]; }

	Matrix4x4(
		float m0, float m1, float m2, float m3,
		float m4, float m5, float m6, float m7,
		float m8, float m9, float m10, float m11,
		float m12, float m13, float m14, float m15
	) : m{
		m0, m1, m2, m3,
		m4, m5, m6, m7,
		m8, m9, m10, m11,
		m12, m13, m14, m15
	} {}

	static Matrix4x4 Translate(const struct Vector3& v);
	static Matrix4x4 Translate(const struct Vector4& v);   // xyz
	bool ValidTRS() const { return m30 == 0.0f && m31 == 0.0f && m32 == 0.0f && m33 == 1.0f; }  // Unity: a valid translation / rotation / scale matrix
	static Matrix4x4 Rotate(const struct Quaternion& q); // defined in UnityShim.hpp
	static Matrix4x4 Scale(const struct Vector3& v);
	void SetTRS(const struct Vector3& pos, const struct Quaternion& rot, const struct Vector3& scale);
	static Matrix4x4 identity() {
		return Matrix4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}

		Matrix4x4 operator+(const Matrix4x4& v) const {
		Matrix4x4 out;
		for (int i = 0; i < 16; ++i)
			out.m[i] = m[i] + v.m[i];
		return out;
	}

	Matrix4x4 operator-(const Matrix4x4& v) const {
		Matrix4x4 out;
		for (int i = 0; i < 16; ++i)
			out.m[i] = m[i] - v.m[i];
		return out;
	}

	// Matrix product (Unity's Matrix4x4 * Matrix4x4), row-major storage, column vectors: (A * B) applies B first.
	// (The converted code originally multiplied element by element, which broke every composed transform.)
	Matrix4x4 operator*(const Matrix4x4& v) const {
		Matrix4x4 out;
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 4; ++c)
				out.m[r * 4 + c] = m[r * 4 + 0] * v.m[0 * 4 + c] + m[r * 4 + 1] * v.m[1 * 4 + c] +
				                   m[r * 4 + 2] * v.m[2 * 4 + c] + m[r * 4 + 3] * v.m[3 * 4 + c];
		return out;
	}

	Matrix4x4 operator/(const Matrix4x4& v) const {
		Matrix4x4 out;
		for (int i = 0; i < 16; ++i)
			out.m[i] = m[i] / v.m[i];
		return out;
	}

	Matrix4x4 operator*(float s) const {
		Matrix4x4 out;
		for (int i = 0; i < 16; ++i)
			out.m[i] = m[i] * s;
		return out;
	}

	Matrix4x4 operator/(float s) const {
		Matrix4x4 out;
		for (int i = 0; i < 16; ++i)
			out.m[i] = m[i] / s;
		return out;
	}

	Matrix4x4& operator+=(const Matrix4x4& v) {
		for (int i = 0; i < 16; ++i)
			m[i] += v.m[i];
		return *this;
	}

	Matrix4x4& operator-=(const Matrix4x4& v) {
		for (int i = 0; i < 16; ++i)
			m[i] -= v.m[i];
		return *this;
	}

	Matrix4x4& operator*=(const Matrix4x4& v) {
		*this = *this * v;
		return *this;
	}

	Matrix4x4& operator/=(const Matrix4x4& v) {
		for (int i = 0; i < 16; ++i)
			m[i] /= v.m[i];
		return *this;
	}

	Matrix4x4& operator*=(float s) {
		for (int i = 0; i < 16; ++i)
			m[i] *= s;
		return *this;
	}

	Matrix4x4& operator/=(float s) {
		for (int i = 0; i < 16; ++i)
			m[i] /= s;
		return *this;
	}
};

inline Matrix4x4 operator*(float s, const Matrix4x4& m) {
	return m * s;
}

struct Rect {};

struct Camera {};

static Vector4 normalize3(const Vector4& v)
{
    float length = std::sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );

    if (length == 0.0f)
        return Vector4(0.0f, 0.0f, 0.0f, v.w);

    float invLength = 1.0f / length;

    return Vector4(
        v.x * invLength,
        v.y * invLength,
        v.z * invLength,
        v.w
    );
}

static Matrix4x4 transpose(const Matrix4x4& m)
{
    Matrix4x4 out;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            out.m[r * 4 + c] = m.m[c * 4 + r];
        }
    }

    return out;
}

static Matrix4x4 inverse(const Matrix4x4& m)
{
    // Gauss-Jordan elimination with partial pivoting.
    float a[4][8];

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c)
            a[r][c] = m.m[r * 4 + c];

        for (int c = 0; c < 4; ++c)
            a[r][c + 4] = (r == c) ? 1.0f : 0.0f;
    }

    for (int c = 0; c < 4; ++c) {
        int pivot = c;

        for (int r = c + 1; r < 4; ++r) {
            if (std::fabs(a[r][c]) > std::fabs(a[pivot][c]))
                pivot = r;
        }

        if (std::fabs(a[pivot][c]) < 1e-8f)
            return Matrix4x4::identity(); // Singular matrix.

        if (pivot != c) {
            for (int k = 0; k < 8; ++k) {
                float temp = a[c][k];
                a[c][k] = a[pivot][k];
                a[pivot][k] = temp;
            }
        }

        float invPivot = 1.0f / a[c][c];

        for (int k = 0; k < 8; ++k)
            a[c][k] *= invPivot;

        for (int r = 0; r < 4; ++r) {
            if (r == c)
                continue;

            float factor = a[r][c];

            for (int k = 0; k < 8; ++k)
                a[r][k] -= factor * a[c][k];
        }
    }

    Matrix4x4 out;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c)
            out.m[r * 4 + c] = a[r][c + 4];
    }

    return out;
}

static Matrix4x4 translateMatrix(const Vector4& v)
{
    Matrix4x4 out = Matrix4x4::identity();

    out.m[3]  = v.x;
    out.m[7]  = v.y;
    out.m[11] = v.z;

    return out;
}

static Matrix4x4 scaleMatrix(const Vector4& v)
{
    Matrix4x4 out = Matrix4x4::identity();

    out.m[0]  = v.x;
    out.m[5]  = v.y;
    out.m[10] = v.z;

    return out;
}

static Matrix4x4 rotationMatrix(const Vector4& axis, float radians)
{
    const float lenSq =
        axis.x * axis.x +
        axis.y * axis.y +
        axis.z * axis.z;

    if (lenSq < 1e-12f)
        return Matrix4x4::identity();

    const float invLen = 1.0f / std::sqrt(lenSq);
    const float x = axis.x * invLen;
    const float y = axis.y * invLen;
    const float z = axis.z * invLen;

    const float c = std::cos(radians);
    const float s = std::sin(radians);
    const float t = 1.0f - c;

    return Matrix4x4(
        t*x*x + c,     t*x*y - s*z,   t*x*z + s*y,   0.0f,
        t*x*y + s*z,   t*y*y + c,     t*y*z - s*x,   0.0f,
        t*x*z - s*y,   t*y*z + s*x,   t*z*z + c,     0.0f,
        0.0f,          0.0f,          0.0f,          1.0f
    );
}

static Matrix4x4 perspectiveMatrix(
    float fovy, float aspect, float r_near, float r_far)
{
    // fovy is in radians. OpenGL-style depth range: [-1, 1].
    if (aspect == 0.0f || r_near <= 0.0f ||
        r_far <= r_near)
        return Matrix4x4::identity();

    const float f = 1.0f / std::tan(fovy * 0.5f);

    return Matrix4x4(
        f / aspect, 0.0f,  0.0f,                                  0.0f,
        0.0f,       f,     0.0f,                                  0.0f,
        0.0f,       0.0f, -(r_far + r_near) / (r_far - r_near),
                              -(2.0f * r_far * r_near) / (r_far - r_near),
        0.0f,       0.0f, -1.0f,                                  0.0f
    );
}

static Matrix4x4 frustumMatrix(
    float left, float right, float bottom, float top,
    float r_near, float r_far)
{
    if (right == left || top == bottom ||
        r_near <= 0.0f || r_far <= r_near)
        return Matrix4x4::identity();

    const float rl = right - left;
    const float tb = top - bottom;
    const float fn = r_far - r_near;

    return Matrix4x4(
        (2.0f * r_near) / rl, 0.0f, (right + left) / rl,  0.0f,
        0.0f, (2.0f * r_near) / tb, (top + bottom) / tb,  0.0f,
        0.0f, 0.0f, -(r_far + r_near) / fn,
                       -(2.0f * r_far * r_near) / fn,
        0.0f, 0.0f, -1.0f, 0.0f
    );
}

static Matrix4x4 orthoMatrix(
    float left, float right, float bottom, float top,
    float r_near, float r_far)
{
    if (right == left || top == bottom || r_far == r_near)
        return Matrix4x4::identity();

    return Matrix4x4(
        2.0f / (right - left), 0.0f, 0.0f,
            -(right + left) / (right - left),
        0.0f, 2.0f / (top - bottom), 0.0f,
            -(top + bottom) / (top - bottom),
        0.0f, 0.0f, -2.0f / (r_far - r_near),
            -(r_far + r_near) / (r_far - r_near),
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

static Matrix4x4 lookAtMatrix(
    const Vector4& eye,
    const Vector4& center,
    const Vector4& up)
{
    auto normalize = [](float x, float y, float z) {
        const float len = std::sqrt(x*x + y*y + z*z);

        if (len < 1e-8f)
            return Vector4(0.0f, 0.0f, 0.0f, 0.0f);

        return Vector4(x / len, y / len, z / len, 0.0f);
    };

    auto cross = [](const Vector4& a, const Vector4& b) {
        return Vector4(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x,
            0.0f
        );
    };

    const Vector4 f = normalize(
        center.x - eye.x,
        center.y - eye.y,
        center.z - eye.z
    );

    const Vector4 s = normalize3(cross(f, up));
    const Vector4 u = cross(s, f);

    return Matrix4x4(
        s.x,  s.y,  s.z,  -(s.x * eye.x + s.y * eye.y + s.z * eye.z),
        u.x,  u.y,  u.z,  -(u.x * eye.x + u.y * eye.y + u.z * eye.z),
        -f.x, -f.y, -f.z,  (f.x * eye.x + f.y * eye.y + f.z * eye.z),
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

static Vector4 transformPoint(const Matrix4x4& m, const Vector4& v)
{
	return Vector4(
		m.m00 * v.x + m.m01 * v.y + m.m02 * v.z + m.m03,
		m.m10 * v.x + m.m11 * v.y + m.m12 * v.z + m.m13,
		m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m23,
		v.w
	);
}

// perspective
/*
static Vector4 transformPoint(const Matrix4x4& m, const Vector4& v)
{
	const float x = m.m00 * v.x + m.m01 * v.y + m.m02 * v.z + m.m03 * v.w;
	const float y = m.m10 * v.x + m.m11 * v.y + m.m12 * v.z + m.m13 * v.w;
	const float z = m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m23 * v.w;
	const float w = m.m30 * v.x + m.m31 * v.y + m.m32 * v.z + m.m33 * v.w;

	if (w != 0.0f && w != 1.0f)
		return Vector4(x / w, y / w, z / w, 1.0f);

	return Vector4(x, y, z, w);
}
*/

namespace Debug {

inline void LogFormat(const char* format, ...) {
	std::va_list args;
	va_start(args, format);

	std::vprintf(format, args);

	va_end(args);
}

inline void LogError(const char* format, ...) {
	std::va_list args;
	va_start(args, format);

	std::vfprintf(stderr, format, args);

	va_end(args);
}

}

#include "UnityShim.hpp"
