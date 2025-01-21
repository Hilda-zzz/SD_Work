#include "Engine/Math/Vec2.hpp"
#include "MathUtils.hpp"
#include <Engine/Core/StringUtils.hpp>

const Vec2 Vec2::ZERO = Vec2(0.f, 0.f);
const Vec2 Vec2::ONE = Vec2(1.f, 1.f);
Vec2 ori = Vec2(0, 0);
//-----------------------------------------------------------------------------------------------
Vec2::Vec2( Vec2 const& copy )
	: x(copy.x)
	, y(copy.y)
{
}

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x(initialX)
	, y(initialY)
{

}

void Vec2::SetFromText(char const* text)
{
	Strings result=SplitStringOnDelimiterIgnoreSpace(text,',');
	x = (float)atof(result[0].c_str());
	y = (float)atof(result[1].c_str());
}

//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator + ( Vec2 const& vecToAdd ) const
{
	return Vec2( x+vecToAdd.x, y+vecToAdd.y );
}

//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator-( Vec2 const& vecToSubtract ) const
{
	return Vec2( x-vecToSubtract.x,y-vecToSubtract.y );
}

//------------------------------------------------------------------------------------------------
Vec2 const Vec2::operator-() const
{
	return Vec2( -x, -y );
}

//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator*( float uniformScale ) const
{
	return Vec2( x*uniformScale, y*uniformScale );
}


//------------------------------------------------------------------------------------------------
Vec2 const Vec2::operator*( Vec2 const& vecToMultiply ) const
{
	return Vec2( x*vecToMultiply.x, y*vecToMultiply.y );
}

//-----------------------------------------------------------------------------------------------
Vec2 const Vec2::operator/( float inverseScale ) const
{
	return Vec2( x/inverseScale, y/inverseScale );
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( Vec2 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( Vec2 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator=( Vec2 const& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
Vec2 const operator*( float uniformScale, Vec2 const& vecToScale )
{
	return Vec2( uniformScale*vecToScale.x,uniformScale*vecToScale.y );
}

//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( Vec2 const& compare ) const
{
	if (compare.x == x && compare.y == y)
	{
		return true;
	}
	else
		return false;
}

//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( Vec2 const& compare ) const
{
	if (compare.x == x && compare.y == y)
	{
		return false;
	}
	else
		return true;
}

/////---------------------------------------------------------------------------------------------
//static methods (e.g. creation func)
	//Don’t need to access or modify instance members, so defining as static is suitable
	//The purpose of the function is to return an indispensable value
Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length )
{
	float cur_degrees = ConvertRadiansToDegrees(orientationRadians);
	float cur_x = length * CosDegrees(cur_degrees);
	float cur_y = length * SinDegrees(cur_degrees);
	return Vec2(cur_x, cur_y);
}
Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float cur_x = length * CosDegrees(orientationDegrees);
	float cur_y = length * SinDegrees(orientationDegrees);
	return Vec2(cur_x, cur_y);
}

//accessors(const methods)
//The function will not modify the state of the object when being called
float Vec2::GetLength() const
{
	return GetDistance2D(Vec2{ 0.0f, 0.0f }, *this);
}

float Vec2::GetLengthSquared() const
{
	return GetDistanceSquared2D(Vec2{ 0.0f, 0.0f }, *this);
}

float Vec2::GetOrientationRadians() const
{
	float cur_degrees=Atan2Degrees(y, x);
	return  ConvertDegreesToRadians(cur_degrees);
}

float Vec2::GetOrientationDegrees() const
{
	return Atan2Degrees(y, x);
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2{ -y, x };
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2{ y, -x };
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	Vec2 newPosition(this->x,this->y );
	float deltaDegrees = ConvertRadiansToDegrees(deltaRadians);
	TransformPosition2D(newPosition, 1.f, deltaDegrees, Vec2{ 0,0 });
	return newPosition;
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	Vec2 newPosition(this->x, this->y);
	TransformPosition2D(newPosition, 1.f, deltaDegrees, Vec2{ 0,0 });
	return newPosition;
}

Vec2 const Vec2::GetClamped(float maxLength) const  //limited the maxLen
{
	float length = GetLength();
	if (length > maxLength) 
	{
		float scale = maxLength / length;
		return Vec2{ x * scale, y * scale };
	}
	return *this;
}

Vec2 const Vec2::GetNormalized() const
{
	float length = GetLength();
	if (length > 0.0f) 
	{
		float scale = 1.0f / length;
		return Vec2{ x * scale, y * scale };
	}
	return *this;
}

//mutators(non-const methods)
void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float length = GetLength();
	float newOrientationDegrees = ConvertRadiansToDegrees(newOrientationRadians);
	x = CosDegrees(newOrientationDegrees) * length;
	y = SinDegrees(newOrientationDegrees) * length;
}
void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float length = GetLength();
	x = CosDegrees(newOrientationDegrees) * length;
	y = SinDegrees(newOrientationDegrees) * length;
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	float newOrientationDegrees = ConvertRadiansToDegrees(newOrientationRadians);
	x = CosDegrees(newOrientationDegrees) * newLength;
	y = SinDegrees(newOrientationDegrees) * newLength;
}
void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	x = CosDegrees(newOrientationDegrees) * newLength;
	y = SinDegrees(newOrientationDegrees) * newLength;
}

void Vec2::Rotate90Degrees()
{
	float pre_x = x;
	float pre_y = y;
	x = -pre_y;
	y = pre_x;
}
void Vec2::RotateMinus90Degrees()
{
	float pre_x = x;
	float pre_y = y;
	x = pre_y;
	y = -pre_x;
}
void Vec2::RotateRadians(float deltaRadians)
{
	float deltaDegrees = ConvertRadiansToDegrees(deltaRadians);
	TransformPosition2D(*this, 1.f, deltaDegrees, Vec2{ 0,0 });
}
void Vec2::RotateDegrees(float deltaDegrees) 
{
	TransformPosition2D(*this, 1.f, deltaDegrees, Vec2{ 0,0 });
}

void Vec2::SetLength(float newLength)
{
	float length = GetLength();
	float scale = newLength / length;
	x *= scale;
	y *= scale;
}
void Vec2::ClampLength(float maxLength)
{
	float length = GetLength();
	if (length > maxLength)
	{
		float scale = maxLength / length;
		x *= scale;
		y *= scale;
	}
}

void Vec2::Normalize()
{
	float length = GetLength();
	if (length > 0.0f)
	{
		float scale = 1.0f / length;
		x *= scale;
		y *= scale;
	}
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	if (length > 0.0f)
	{
		float scale = 1.0f / length;
		x *= scale;
		y *= scale;
	}
	return length;
}

Vec2 const Vec2::GetReflected(Vec2 const& normalOfSurfaceToReflectOffOf) const
{
	float Vn_f = DotProduct2D(Vec2(x, y), normalOfSurfaceToReflectOffOf);
	Vec2 Vn = normalOfSurfaceToReflectOffOf * Vn_f;
	Vec2 Vt = Vec2(x, y) - Vn;
	Vec2 R = Vt - Vn;
	return R;
}

void Vec2::Reflect(Vec2 const& normalOfSurfaceToReflectOffOf)
{
	float Vn_f = DotProduct2D(Vec2(x, y), normalOfSurfaceToReflectOffOf);
	Vec2 Vn = normalOfSurfaceToReflectOffOf * Vn_f;
	Vec2 Vt = Vec2(x, y) - Vn;
	*this = Vt - Vn;
}

