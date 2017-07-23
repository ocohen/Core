#ifndef OC_VECTOR4
#define OC_VECTOR4

#include "Vector.h"

struct Vector4 : public Vector<4>
{
	float& x = NFloat<4>::data[0];
	float& y = NFloat<4>::data[1];
	float& z = NFloat<4>::data[2];
	float& w = NFloat<4>::data[3];

	Vector4()
	{
	}

	Vector4(float inX, float inY, float inZ, float inW)
	: TVector {inX, inY, inZ, inW}
	{
	}

	Vector4(const Vector4& other)
	: TVector {other.x, other.y, other.z, other.w}
	{
	}

	explicit Vector4(float k)
	: TVector(k)
	{
	}

	explicit Vector4(const NFloat<4>& rhs)
	: TVector(rhs)
	{
	}

	Vector4& operator=(const Vector4& other)
	{
		TVector::operator=(other);
		return *this;
	}

	Vector4 operator*(float k) const { return Vector4(TVector::operator*(k)); }
	Vector4 operator+(const Vector4& rhs) const { return Vector4(TVector::operator+(rhs)); }
	Vector4 operator-(const Vector4& rhs) const { return Vector4(TVector::operator-(rhs)); }
	Vector4 operator/(float k) const { return Vector4(TVector::operator/(k)); }

	Vector4& operator*=(float k) { TVector::operator*=(k); return *this; }
	Vector4& operator/=(float k) { TVector::operator/=(k); return *this; }
	Vector4& operator+=(const Vector4& rhs) { TVector::operator+=(rhs); return *this; }
	Vector4& operator-=(const Vector4& rhs) { TVector::operator+=(rhs); return *this; }

	Vector4& operator-() { return Vector4(TVector::operator-()); }

	Vector4 getNormal() const { return Vector4(TVector::getNormal()); }
	Vector4& normalize() { TVector::normalize(); return *this; }
};

inline Vector4 operator*(float lhs, const Vector4& rhs)
{
	return rhs * lhs;
}

#endif