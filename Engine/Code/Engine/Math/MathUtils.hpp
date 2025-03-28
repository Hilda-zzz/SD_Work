# pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB3.hpp"
struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
class OBB2;
class LineSegment2;
class Capsule2;
class Triangle2;
class FloatRange;

enum class BillboardType
{
	NONE=-1,
	WORLD_UP_FACING,
	WORLD_UP_OPPOSING,
	FULL_FACING,
	FULL_OPPOSING,
	COUNT
};


//Clamp and lerp
float GetClamped(float value, float minValue, float maxValue);
float GetClampedZeroToOne(float value);
float Interpolate(float start, float end, float fractionTowardEnd);
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, FloatRange const& inRange, FloatRange const& outRange);
int RoundDownToInt(float value);

//Angle
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);

//basic 2d 3d utilities
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);

float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);

//Is Point Inside
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
//bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientbox);
bool IsPointInsideTriangle2D(Vec2 const& point, Triangle2 const& triangle);
//---------------------------------------------------------
bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius);
bool IsPointInsideCylinder3D(Vec3 const& point, Vec3 const& center, float radius, float halfHeight);
bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box);
//GetNearest
Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePosition, AABB2 const& box);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, Vec2 const& vecOnLine, Vec2 const& anotherPointOnLine);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, LineSegment2 const& infiniteLine);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, Vec2 const& lineSegStart, Vec2 const& lineSegEnd);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, LineSegment2 const& lineSegment);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Capsule2 const& capsule);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 const  GetNearestPointOnOBB2D(Vec2 const& referencePosition, OBB2 const& orientedBox);
Vec2 const GetNearestPointOnTriangle2D(Vec2 const& referencePosition, Triangle2 const& triangle);
//--------------------------------------------------------------------------------------------------
Vec3 const GetNearestPointOnSphere3D(Vec3 const& referencePosition, Vec3 const& sphereCenter, float sphereRadius);
Vec3 const GetNearestPointOnZCylinder3D(Vec3 const& referencePosition, Vec3 const& cylinderCenter, float cylinderRadius,float halfHeight);
Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencePosition, AABB3 const& box);

//Geometric query utilities
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoAABB2Overlap(AABB2 const& box1, AABB2 const& box2);
bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);
//------------------------------------------------------
bool DoAABBsOverlap3D(AABB3 const& boxA, AABB3 const& boxB);
bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoZCylindersOverlap3D(Vec3 const& centerA, float radiusA, float halfHeightA,
	Vec3 const& centerB, float radiusB, float halfHeightB);
bool DoSphereAndAABBOverlap3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box);
bool DoZCylinderAndAABBOverlap3D(Vec3 cylinderCenter, float cylinderRadius, float halfHeight, AABB3 const& box);
bool DoZCylinderAndShpereOVerlap3D(Vec3 cylinderCenter, float cylinderRadius, float halfHeight,
	Vec3 const& sphereCenter, float sphereRadius);
bool PushZCylinderOutOfZCylinderFromZ3D(Vec3& mobileCylinderCenter, float mobileCylinderRadius, 
	Vec3 const& fixedCylinderCenter, float fixedCylinderRadius);
bool PushZCylinderOutOfEachOtherFromZ3D(Vec3& aCenter, float aRadius, Vec3& bCenter, float bRadius);


//transform
void TransformPosition2D(Vec2& posToTransform, float uniformScale, 
	float rotationDegrees, Vec2 const& translation);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, float uniformScale,
	float rotationDegrees, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const jBasis, Vec2 const& translation);

//dot and cross
float DotProduct2D(Vec2 const& a, Vec2 const& b);
float DotProduct3D(Vec3 const& a, Vec3 const& b);
float DotProduct4D(Vec4 const& a, Vec4 const& b);
float CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b);

float GetDistance2DInt(IntVec2 const& positionA, IntVec2 const& positionB);

float NormalizeByte(unsigned char uc);
float DenormalizeByte(float f);

Mat44 GetLookAtMatrix(const Vec3& pos, const Vec3& target);
Mat44 GetBillboardMatrix(BillboardType billboardType,
	Mat44 const& targetMatrix,Vec3 const& targetPos,
	const Vec3& billboardPosition,
	const Vec2& billboardScale = Vec2(1.f, 1.f));