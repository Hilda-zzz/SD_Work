
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <cmath>
extern Vec2 ori;
Vec3::Vec3(float x, float y, float z):x(x),y(y),z(z)
{

}

void Vec3::SetFromText(char const* text)
{
	Strings result = SplitStringOnDelimiterIgnoreSpace(text, ',');
	x = (float)atof(result[0].c_str());
	y = (float)atof(result[1].c_str());
	z = (float)atof(result[2].c_str());
}

//-----------------------------------------------------------------------------------------------
//Vec3::Vec3(Vec3 const& copy)
//	: x(copy.x)
//	, y(copy.y)
//{
//	x = copy.x;
//	y = copy.y;
//}

//-----------------------------------------------------------------------------------------------
//Vec3::Vec3(float initialX, float initialY)
//	: x(123.f)
//	, y(456.f)
//{
//	x = initialX;
//	y = initialY;
//}

//-----------------------------------------------------------------------------------------------
Vec3 const Vec3::operator + (Vec3 const& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y,z+vecToAdd.z);
}

//-----------------------------------------------------------------------------------------------
Vec3 const Vec3::operator-(Vec3 const& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y,z-vecToSubtract.z);
}

//------------------------------------------------------------------------------------------------
//Vec3 const Vec3::operator-() const
//{
//	return Vec2(-x, -y);
//}

//-----------------------------------------------------------------------------------------------
Vec3 const Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale,z*uniformScale);
}

//------------------------------------------------------------------------------------------------
//Vec3 const Vec3::operator*(Vec3 const& vecToMultiply) const
//{
//	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y);
//}

//-----------------------------------------------------------------------------------------------
Vec3 const Vec3::operator/(float inverseScale) const
{
	return Vec3(x / inverseScale, y / inverseScale,z/inverseScale);
}

Vec3 const Vec3::operator-() const
{
	return Vec3(-x, -y,-z);
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(Vec3 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(Vec3 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator=(Vec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

const Vec3 Vec3::MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length)
{
	return Vec3(length * std::cos(latitudeRadians) * std::cos(longitudeRadians),
		length * std::cos(latitudeRadians) * std::sin(longitudeRadians),
		-length* std::sin(longitudeRadians));
}

const Vec3 Vec3::MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length)
{
	return Vec3(length * CosDegrees(latitudeDegrees) * CosDegrees(longitudeDegrees),
		length * CosDegrees(latitudeDegrees) * SinDegrees(longitudeDegrees),
		length * SinDegrees(latitudeDegrees));
}

void Vec3::SetLength(float newLength)
{
	float length = GetLength();
	if (length > 0.f)
	{
		float scale = newLength / length;
		x *= scale;
		y *= scale;
		z *= scale;
	}
}

//-----------------------------------------------------------------------------------------------
Vec3 const operator*(float uniformScale, Vec3 const& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}

//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(Vec3 const& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z)
	{
		return true;
	}
	else
		return false;
}

//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(Vec3 const& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z)
	{
		return false;
	}
	else
		return true;
}

///-----------------------------------------------------------------------------------------------
//accessors(const methods)
	//The function will not modify the state of the object when being called
float Vec3::GetLength() const
{
	return GetDistance3D(Vec3{ 0.0f, 0.0f,0.0f }, *this);
}
float Vec3::GetLengthXY() const
{
	return GetDistanceXY3D(Vec3{ 0.0f, 0.0f,0.0f }, *this);
}
float Vec3::GetLengthSquared() const
{
	return GetDistanceSquared3D(Vec3{ 0.0f, 0.0f,0.0f }, *this);
}
float Vec3::GetLengthXYSquared() const
{
	return GetDistanceXYSquared3D(Vec3{ 0.0f, 0.0f,0.0f }, *this);
}

float Vec3::GetAngleAboutZRadians() const
{
	float cur_degrees = Atan2Degrees(y,x);
	return  ConvertDegreesToRadians(cur_degrees);
}
float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees(y, x);
}

Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	Vec3 newPosition(this->x, this->y, this->z);
	float deltaDegrees = ConvertRadiansToDegrees(deltaRadians);
	TransformPositionXY3D(newPosition, 1.f, deltaDegrees, ori);
	return newPosition;
}
Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	Vec3 newPosition(this->x, this->y,this->z);
	TransformPositionXY3D(newPosition, 1.f, deltaDegrees,ori);
	return newPosition;
}
Vec3 const Vec3::GetClamped(float maxLength) const
{
	float length = GetLength();
	if (length > maxLength)
	{
		float scale = maxLength / length;
		return Vec3{ x * scale, y * scale,z * scale };
	}
	return *this;
}
Vec3 const Vec3::GetNormalized() const
{
	float length = GetLength();
	if (length > 0.0f)
	{
		float scale = 1.0f / length;
		return Vec3{ x * scale, y * scale,z*scale };
	}
	return *this;
}

void Vec3::Normalized()
{
	float length = GetLength();
	if (length > 0.0f)
	{
		float scale = 1.0f / length;
		x *= scale;
		y *= scale;
		z *= scale;
	}
}
