#pragma once

struct IntVec2
{
public:
	int x = 0;
	int y = 0;

	static const IntVec2 ZERO;
public:
	~IntVec2(){}
	IntVec2(){}
	IntVec2(const IntVec2& copyFrom);
	explicit IntVec2(int initialX, int initialY);
	void SetFromText(char const* text);
	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const; 
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;

	void Rotate90Degrees();
	void RotateMinus90Degrees();

	void	operator=(const IntVec2& copyFrom);
	bool	operator==(IntVec2 const& compare) const;
	bool	operator!=(IntVec2 const& compare) const;
	IntVec2 const	operator-(IntVec2 const& vecToSubtract) const;	// vec2 - vec2
	IntVec2 const	operator+(IntVec2 const& vecToAdd) const;
	void const	operator+=(IntVec2 const& vecToAdd);
	void const	operator-=(IntVec2 const& vecToSubtract);
};