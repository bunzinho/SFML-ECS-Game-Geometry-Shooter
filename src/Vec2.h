#pragma once

class Vec2
{
public:

	float x = 0;
	float y = 0;

	Vec2();
	Vec2(float xin, float yin);
	Vec2(float radians);

	bool operator == (const Vec2& rhs) const;
	bool operator != (const Vec2& rhs) const;

	Vec2 operator + (const Vec2& rhs) const;
	Vec2 operator - (const Vec2& rhs) const;
	Vec2 operator / (const float val) const;
	Vec2 operator * (const float val) const;

	void operator += (const Vec2& rhs);
	void operator -= (const Vec2& rhs);
	void operator *= (const float val);
	void operator /= (const float val);

	float distance(const Vec2& rhs) const;
	Vec2 normalized() const;
	float length() const;
	float length_squared() const;

	static Vec2 Lerp(const Vec2& a, const Vec2& b, const float t);
};
