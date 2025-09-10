#include "IntVec3.hpp"
#include "IntVec2.hpp"
#include "../Core/StringUtils.hpp"
#include "MathUtils.hpp"
IntVec3::IntVec3(const IntVec3& copyFrom):x(copyFrom.x),y(copyFrom.y),z(copyFrom.z)
{
}

IntVec3::IntVec3(const IntVec2& copyFrom, int initialZ):x(copyFrom.x),y(copyFrom.y),z(initialZ)
{
}

IntVec3::IntVec3(int initialX, int initialY, int initialZ) 
	:x(initialX), y(initialY), z(initialZ)
{
}

IntVec3::IntVec3(int initialXYZ) : x(initialXYZ), y(initialXYZ), z(initialXYZ)
{
}

void IntVec3::SetFromText(char const* text)
{
	Strings result = SplitStringOnDelimiterIgnoreSpace(text, ',');
	x = atoi(result[0].c_str());
	y = atoi(result[1].c_str());
	z = atoi(result[2].c_str());
}

float IntVec3::GetLength() const
{
	return GetDistance3DInt(IntVec3{ 0, 0 ,0}, *this);
}

int IntVec3::GetTaxicabLength() const
{
	return std::abs(x) + std::abs(y) + std::abs(z);
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

IntVec3 IntVec3::GetAbsoluteValue() const
{
	return IntVec3();
}

void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

void IntVec3::operator=(const Vec3& copyFrom)
{
	x = static_cast<int>(copyFrom.x);
	y = static_cast<int>(copyFrom.y);
	z = static_cast<int>(copyFrom.z);
}

bool IntVec3::operator==(IntVec3 const& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z)
	{
		return true;
	}
	else
		return false;
}

bool IntVec3::operator!=(IntVec3 const& compare) const
{
	if (compare.x == x && compare.y == y && compare.z == z)
	{
		return false;
	}
	else
		return true;
}

IntVec3 const IntVec3::operator-(IntVec3 const& vecToSubtract) const
{
	return IntVec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}

IntVec3 const IntVec3::operator+(IntVec3 const& vecToAdd) const
{
	return IntVec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

IntVec3 const IntVec3::operator*(int scalarToMultiply) const
{
	return IntVec3(x * scalarToMultiply, y * scalarToMultiply, z * scalarToMultiply);
}

IntVec3 const IntVec3::operator/(int scalarToDivide) const
{
	return IntVec3(x / scalarToDivide, y / scalarToDivide, z / scalarToDivide);
}


