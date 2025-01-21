#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath> 
#include "Engine/Core/StringUtils.hpp"

const IntVec2 IntVec2::ZERO = IntVec2(0,0);

IntVec2::IntVec2(const IntVec2& copyFrom):x(copyFrom.x),y(copyFrom.y)
{
}

IntVec2::IntVec2(int initialX, int initialY) :x(initialX), y(initialY)
{
}

void IntVec2::SetFromText(char const* text)
{
	Strings result = SplitStringOnDelimiterIgnoreSpace(text, ',');
	x = atoi(result[0].c_str());
	y = atoi(result[1].c_str());
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
	float cur_degrees = Atan2Degrees((float)y, (float)x);
	return  ConvertDegreesToRadians(cur_degrees);
}

float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees((float)y, (float)x);
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
	float pre_x = (float)x;
	float pre_y = (float)y;
	x = (int)-pre_y;
	y = (int)pre_x;
}

void IntVec2::RotateMinus90Degrees()
{
	float pre_x = (float)x;
	float pre_y = (float)y;
	x = (int)pre_y;
	y = (int)-pre_x;
}

void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

bool IntVec2::operator==(IntVec2 const& compare) const
{
	if (compare.x == x && compare.y == y)
	{
		return true;
	}
	else
		return false;
}
bool IntVec2::operator!=(IntVec2 const& compare) const
{
	if (compare.x == x && compare.y == y)
	{
		return false;
	}
	else
		return true;
}
IntVec2 const IntVec2::operator-(IntVec2 const& vecToSubtract) const
{
	return IntVec2(x - vecToSubtract.x, y - vecToSubtract.y);
}

IntVec2 const IntVec2::operator+(IntVec2 const& vecToSubtract) const
{
	return IntVec2(x + vecToSubtract.x, y + vecToSubtract.y);
}

