#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/StringUtils.hpp"

Vec4::Vec4(Vec4 const& copyFrom):x(copyFrom.x),y(copyFrom.y),z(copyFrom.z),w(copyFrom.w)
{
}

Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW):
	x(initialX), y(initialY), z(initialZ), w(initialW)
{
}

void Vec4::SetFromText(char const* text)
{
	Strings result = SplitStringOnDelimiterIgnoreSpace(text, ',');
	x = (float)atof(result[0].c_str());
	y = (float)atof(result[1].c_str());
	z = (float)atof(result[2].c_str());
	w = (float)atof(result[3].c_str());
}

bool Vec4::operator==(Vec4 const& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z && compare.w == w)
	{
		return true;
	}
	else
		return false;
}

bool Vec4::operator!=(Vec4 const& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z && compare.w == w)
	{
		return false;
	}
	else
		return true;
}

Vec4 const Vec4::operator+(Vec4 const& vecToAdd) const
{
	return Vec4(x+vecToAdd.x,y+vecToAdd.y,z+vecToAdd.z,w+vecToAdd.w);
}

Vec4 const Vec4::operator-(Vec4 const& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}

Vec4 const Vec4::operator-() const
{
	return Vec4(-x,-y,-z,-w);
}

Vec4 const Vec4::operator*(float uniformScale) const
{
	return Vec4(uniformScale * x, uniformScale * y, uniformScale * z, uniformScale * w);
}

Vec4 const Vec4::operator/(float inverseScale) const
{
	return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale);
}

void Vec4::operator+=(Vec4 const& vecToAdd)
{
	x = x + vecToAdd.x;
	y = y + vecToAdd.y;
	z = z + vecToAdd.z;
	w = w + vecToAdd.w;
}

void Vec4::operator-=(Vec4 const& vecToSubtract)
{
	x = x - vecToSubtract.x;
	y = y - vecToSubtract.y;
	z = z - vecToSubtract.z;
	w = w - vecToSubtract.w;
}

void Vec4::operator*=(const float uniformScale)
{
	x = x * uniformScale;
	y = y * uniformScale;
	z = z * uniformScale;
	w = w * uniformScale;
}

void Vec4::operator/=(const float uniformDivisor)
{
	x = x / uniformDivisor;
	y = y / uniformDivisor;
	z = z / uniformDivisor;
	w = w / uniformDivisor;
}

void Vec4::operator=(Vec4 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}
