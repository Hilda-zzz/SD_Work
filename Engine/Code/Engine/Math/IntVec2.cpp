#include "IntVec2.hpp"
#include "MathUtils.hpp"
#include <cmath> 
IntVec2::IntVec2(const IntVec2& copyFrom):x(copyFrom.x),y(copyFrom.y)
{
}

IntVec2::IntVec2(int initialX, int initialY) :x(initialX), y(initialY)
{
}

float IntVec2::GetLength() const
{
	return GetDistance2DInt(IntVec2{ 0, 0 }, *this);
}

int IntVec2::GetTaxicabLength() const
{
	return std::abs(x) + std::abs(y);
}

int IntVec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float IntVec2::GetOrientationRadians() const
{
	float cur_degrees = Atan2Degrees(y, x);
	return  ConvertDegreesToRadians(cur_degrees);
}

float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees(y, x);
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2{ -y, x };
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2{ y, -x };
}

void IntVec2::Rotate90Degrees()
{
	float pre_x = x;
	float pre_y = y;
	x = -pre_y;
	y = pre_x;
}

void IntVec2::RotateMinus90Degrees()
{
	float pre_x = x;
	float pre_y = y;
	x = pre_y;
	y = -pre_x;
}

void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

