#pragma once
struct Vec3;
struct IntVec2;

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	static const IntVec3 ZERO;
	static const IntVec3 ONE;

public:
	~IntVec3() {}
	IntVec3() {}
	IntVec3(const IntVec3& copyFrom);
	IntVec3(const IntVec2& copyFrom, int initialZ = 0);
	explicit IntVec3(int initialX, int initialY, int initialZ);
	explicit IntVec3(int initialXYZ); 

	void SetFromText(char const* text);

	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;

	IntVec3 GetAbsoluteValue() const;
	int GetManhattanDistance(const IntVec3& other) const;
	int GetChebyshevDistance(const IntVec3& other) const; 

// 	IntVec3 const GetRotated90DegreesAroundX() const;
// 	IntVec3 const GetRotated90DegreesAroundY() const;
// 	IntVec3 const GetRotated90DegreesAroundZ() const;
// 	IntVec3 const GetRotatedMinus90DegreesAroundX() const;
// 	IntVec3 const GetRotatedMinus90DegreesAroundY() const;
// 	IntVec3 const GetRotatedMinus90DegreesAroundZ() const;
// 
// 	void Rotate90DegreesAroundX();
// 	void Rotate90DegreesAroundY();
// 	void Rotate90DegreesAroundZ();
// 	void RotateMinus90DegreesAroundX();
// 	void RotateMinus90DegreesAroundY();
// 	void RotateMinus90DegreesAroundZ();

	// Project to plane
// 	IntVec2 GetXY() const;
// 	IntVec2 GetXZ() const;
// 	IntVec2 GetYZ() const;

	void operator=(const IntVec3& copyFrom);
	void operator=(const Vec3& copyFrom);
	bool operator==(IntVec3 const& compare) const;
	bool operator!=(IntVec3 const& compare) const;
	bool operator<(IntVec3 const& compare) const; 

	IntVec3 const operator-(IntVec3 const& vecToSubtract) const;
	IntVec3 const operator+(IntVec3 const& vecToAdd) const;
	IntVec3 const operator*(int scalarToMultiply) const;
	IntVec3 const operator/(int scalarToDivide) const;
	//IntVec3 const operator-() const; 

	void const operator+=(IntVec3 const& vecToAdd);
	void const operator-=(IntVec3 const& vecToSubtract);
	void const operator*=(int scalarToMultiply);
	void const operator/=(int scalarToDivide);

	int& operator[](int index);
	const int& operator[](int index) const;
};

// dealing with scalarToMultiply at left
IntVec3 const operator*(int scalarToMultiply, IntVec3 const& vecToMultiply);