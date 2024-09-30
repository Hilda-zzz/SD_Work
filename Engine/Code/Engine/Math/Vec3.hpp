#pragma once
//-----------------------------------------------------------------------------------------------
struct Vec3
{
public: 
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
public:
	Vec3() = default;
	explicit Vec3(float initialX, float initialY,float initialZ);

	//accessors(const methods)
	//The function will not modify the state of the object when being called
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;

	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;

	Vec3 const GetRotatedAboutZRadians(float deltaRadians) const;
	Vec3 const GetRotatedAboutZDegrees(float deltaDegrees) const;
	Vec3 const GetClamped(float maxLength) const;
	Vec3 const GetNormalized() const;

	// Operators (const)
	bool		operator==(Vec3 const& compare) const;		// vec2 == vec2
	bool		operator!=(Vec3 const& compare) const;		// vec2 != vec2
	Vec3 const	operator+(Vec3 const& vecToAdd) const;		// vec2 + vec2
	Vec3 const	operator-(Vec3 const& vecToSubtract) const;	// vec2 - vec2
	Vec3 const	operator*(float uniformScale) const;			// vec2 * float
	Vec3 const	operator/(float inverseScale) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=(Vec3 const& vecToAdd);				// vec2 += vec2
	void		operator-=(Vec3 const& vecToSubtract);		// vec2 -= vec2
	void		operator*=(float uniformScale);			// vec2 *= float
	void		operator/=(float uniformDivisor);		// vec2 /= float
	void		operator=(Vec3 const& copyFrom);				// vec2 = vec2

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend Vec3 const operator*(float uniformScale, Vec3 const& vecToScale);	// float * vec2
};


