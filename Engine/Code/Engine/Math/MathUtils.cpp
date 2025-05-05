#include "Engine/Math/MathUtils.hpp"
#include <cmath> 
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Triangle2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <Engine/Core/EngineCommon.hpp>
#include "RandomNumberGenerator.hpp"
#include "OBB3.hpp"
#include "Mat44.hpp"



RandomNumberGenerator g_rng = RandomNumberGenerator();
//Angle
float ConvertDegreesToRadians(float degrees)
{
	return (PI / 180.0f) * degrees;
}
float ConvertRadiansToDegrees(float radians)
{
	return (180.0f * radians) / PI;
}
float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return std::cos(radians);
}
float SinDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return std::sin(radians);
}
float Atan2Degrees(float y, float x)
{
	//minus??
	float cur_radians = std::atan2(y, x);
	float cur_degrees = ConvertRadiansToDegrees(cur_radians);
	return cur_degrees;
}

float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	float disp = endDegrees - startDegrees;
	while (disp > 180.f)
	{
		disp -= 360.f;
	}

	while (disp < -180.f)
	{
		disp += 360.f;
	}

	return disp;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float angularDispDegrees = GetShortestAngularDispDegrees(currentDegrees, goalDegrees) ;
	if (fabsf(angularDispDegrees) > maxDeltaDegrees)
	{
		if(angularDispDegrees>=0)
			return currentDegrees + maxDeltaDegrees;
		else
			return currentDegrees - maxDeltaDegrees;
	}
	//if (fabsf(angularDispDegrees) <= maxDeltaDegrees)
	else
	{
		return goalDegrees;
	}
		
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	return ConvertRadiansToDegrees(acosf(DotProduct2D(a,b)/(a.GetLength()*b.GetLength())));
}

//basic 2d 3d utilities
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return std::sqrt((positionA.x-positionB.x)* (positionA.x - positionB.x)+
		(positionA.y - positionB.y) * (positionA.y - positionB.y));
}

float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x*b.x+a.y*b.y;
}

float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x*b.y-a.y*b.x;
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	return Vec3(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

//?
float GetDistance2DInt(IntVec2 const& positionA, IntVec2 const& positionB)
{
	return std::sqrtf((float)((positionA.x - positionB.x) * (positionA.x - positionB.x) +
		(positionA.y - positionB.y) * (positionA.y - positionB.y)));
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB) 
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x) +
		(positionA.y - positionB.y) * (positionA.y - positionB.y);
}
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	float dz = positionB.z - positionA.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	float dz = positionB.z - positionA.z;
	return dx * dx + dy * dy + dz * dz;

}
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	return std::sqrt(dx * dx + dy * dy);
}
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	return dx * dx + dy * dy;
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return DotProduct2D(vectorToProject,vectorToProjectOnto.GetNormalized());
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 normal = vectorToProjectOnto.GetNormalized();
	return DotProduct2D(vectorToProject, normal)* normal;
}

//Geomertric query utilities
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float cur_dis = GetDistance2D(centerA, centerB);
	if (cur_dis < radiusA + radiusB)
	{
		return true;
	}
	else
		return false;
}
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float cur_dis = GetDistance3D(centerA, centerB);
	if (cur_dis <= radiusA + radiusB)
	{
		return true;
	}
	else
		return false;
}

bool DoAABB2Overlap(AABB2 const& box1, AABB2 const& box2)
{
	if (box1.m_maxs.x <= box2.m_mins.x || box1.m_mins.x >= box2.m_maxs.x) 
		return false;
	if (box1.m_maxs.y <= box2.m_mins.y || box1.m_mins.y >= box2.m_maxs.y) 
		return false;

	return true;
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	if (GetDistanceSquared2D(point, discCenter) < discRadius*discRadius)
	{
		return true;
	}
	return false;
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistanceSquared2D(point, sectorTip) < sectorRadius* sectorRadius)
	{
	/*	if (GetAngleDegreesBetweenVectors2D(point - sectorTip, Vec2::MakeFromPolarDegrees(sectorApertureDegrees, sectorRadius).GetNormalized())
			< 0.5f * sectorApertureDegrees)*/
		Vec2 point_tip = point- sectorTip ;
		if(abs(GetShortestAngularDispDegrees(point_tip.GetOrientationDegrees(), sectorForwardDegrees))<0.5*sectorApertureDegrees)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) < sectorRadius)
	{
		if (GetAngleDegreesBetweenVectors2D(point-sectorTip,sectorForwardNormal)<0.5f*sectorApertureDegrees)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	if (box.IsPointInside(point))
	{
		return true;
	}
	else
		return false;
}

bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	Vec2 n = GetNearestPointOnLineSegment2D(point, capsule.m_bone);
	Vec2 pn = n - point;
	return pn.GetLengthSquared() < capsule.m_radius * capsule.m_radius;
}

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 n = GetNearestPointOnLineSegment2D(point, LineSegment2(boneStart,boneEnd));
	Vec2 pn = n - point;
	return pn.GetLengthSquared() < radius * radius;
}

bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientbox)
{
	Vec2 cp = point - orientbox.m_center;
	float cpi = DotProduct2D(orientbox.m_iBasisNormal , cp);
	float cpj = DotProduct2D(orientbox.m_iBasisNormal.GetRotated90Degrees(), cp);
	if (abs(cpi) >= orientbox.m_halfDimensions.x)
	{
		return false;
	}
	else if (abs(cpj) >= orientbox.m_halfDimensions.y)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool IsPointInsideTriangle2D(Vec2 const& point, Triangle2 const& triangle)
{
	Vec2 ap = point - triangle.m_pointsCounterClockwise[0];
	Vec2 ab = triangle.m_pointsCounterClockwise[1] - triangle.m_pointsCounterClockwise[0];
	if (CrossProduct2D(ab, ap) <= 0.f)
	{
		return false;
	}
	Vec2 bp = point - triangle.m_pointsCounterClockwise[1];
	Vec2 bc = triangle.m_pointsCounterClockwise[2] - triangle.m_pointsCounterClockwise[1];
	if (CrossProduct2D(bc, bp) <= 0.f)
	{
		return false;
	}
	Vec2 cp = point - triangle.m_pointsCounterClockwise[2];
	Vec2 ca = triangle.m_pointsCounterClockwise[0] - triangle.m_pointsCounterClockwise[2];
	if (CrossProduct2D(ca, cp) <= 0.f)
	{
		return false;
	}
	return true;
}

bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius)
{
	if (GetDistanceSquared3D(point, sphereCenter) < sphereRadius*sphereRadius)
	{
		return true;
	}
	return false;
}

bool IsPointInsideCylinder3D(Vec3 const& point, Vec3 const& center, float radius, float halfHeight)
{
	if (IsPointInsideDisc2D(Vec2(point.x, point.y), Vec2(center.x, center.y), radius))
	{
		if (point.z<center.z + halfHeight && point.z>center.z - halfHeight)
		{
			return true;
		}
	}
	return false;
}

bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box)
{
	if (point.x > box.m_mins.x && point.x<box.m_maxs.x
		&&point.y>box.m_mins.y && point.y < box.m_maxs.y
		&& point.z>box.m_mins.z && point.z < box.m_maxs.z)
	{
		return true;
	}
	return false;
}

bool IsPointInsideOBB3D(Vec3 const& point, OBB3 const& box)
{
	Vec3 boxOriginToPoint = point - box.m_center;
	Vec3 relativePos = Vec3(DotProduct3D(boxOriginToPoint, box.m_iBasis),
		DotProduct3D(boxOriginToPoint, box.m_jBasis),
		DotProduct3D(boxOriginToPoint, box.m_kBasis));

	if (relativePos.x > -box.m_halfDimensions.x && relativePos.x<box.m_halfDimensions.x
		&& relativePos.y>box.m_halfDimensions.y && relativePos.y < box.m_halfDimensions.y
		&& relativePos.z>box.m_halfDimensions.z && relativePos.z < box.m_halfDimensions.z)
	{
		return true;
	}
	return false;
}

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	if (GetDistance2D(referencePosition, discCenter) <= discRadius)
	{
		return referencePosition;
	}
	else
	{
		Vec2 refer_center = referencePosition - discCenter;
		refer_center.SetLength(discRadius);
		return refer_center + discCenter;
	}
}

Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePosition, AABB2 const& box)
{
	float x = GetClamped(referencePosition.x, box.m_mins.x, box.m_maxs.x);
	float y = GetClamped(referencePosition.y, box.m_mins.y, box.m_maxs.y);
	return Vec2(x, y);
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, Vec2 const& vecOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 se = anotherPointOnLine - vecOnLine;
	Vec2 sp = referencePosition - vecOnLine;
	if (DotProduct2D(se, sp) !=0.f)
	{
		Vec2 sn = GetProjectedOnto2D(sp, se);
		return vecOnLine + sn;
	}
	else
	{
		return vecOnLine;
	}
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, LineSegment2 const& infiniteLine)
{
	Vec2 se = infiniteLine.m_end - infiniteLine.m_start;
	Vec2 sp = referencePosition - infiniteLine.m_start;
	if (DotProduct2D(se, sp) != 0.f)
	{
		Vec2 sn = GetProjectedOnto2D(sp, se);
		return infiniteLine.m_start + sn;
	}
	else
	{
		return infiniteLine.m_start;
	}
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, Vec2 const& lineSegStart, Vec2 const& lineSegEnd)
{
	Vec2 sp = referencePosition - lineSegStart;
	Vec2 se = lineSegEnd - lineSegStart;
	Vec2 ep = referencePosition - lineSegEnd;
	if (DotProduct2D(se,sp)<=0.f)
	{
		return lineSegStart;
	}
	else if (DotProduct2D(se, ep) >= 0.f)
	{
		return lineSegEnd;
	}
	else
	{
		Vec2 sn = GetProjectedOnto2D(sp, se);
		return lineSegStart + sn;
	}
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, LineSegment2 const& lineSegment)
{
	Vec2 sp = referencePosition - lineSegment.m_start;
	Vec2 se = lineSegment.m_end - lineSegment.m_start;
	Vec2 ep = referencePosition - lineSegment.m_end;
	if (DotProduct2D(se, sp) <= 0.f)
	{
		return lineSegment.m_start;
	}
	else if (DotProduct2D(se, ep) >= 0.f)
	{
		return lineSegment.m_end;
	}
	else
	{
		Vec2 sn = GetProjectedOnto2D(sp, se);
		return lineSegment.m_start + sn;
	}
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Capsule2 const& capsule)
{
	if(IsPointInsideCapsule2D(referencePosition, capsule))
	{
		return referencePosition;
	}
	else
	{
		Vec2 sp = referencePosition - capsule.m_bone.m_start;
		Vec2 se = capsule.m_bone.m_end - capsule.m_bone.m_start;
		Vec2 ep = referencePosition - capsule.m_bone.m_end;
		if (DotProduct2D(se, sp) <= 0.f)
		{
			sp.SetLength(capsule.m_radius);
			return capsule.m_bone.m_start + sp;
		}
		else if (DotProduct2D(se, ep) >= 0.f)
		{
			ep.SetLength(capsule.m_radius);
			return capsule.m_bone.m_end + ep;
		}
		else
		{
			Vec2 sn = GetProjectedOnto2D(sp, se);
			Vec2 n = capsule.m_bone.m_start + sn;
			Vec2 np = referencePosition - n;
			np.SetLength(capsule.m_radius);
			return n + np;
		}
	}
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	if (IsPointInsideCapsule2D(referencePosition, boneStart, boneEnd, radius))
	{
		return referencePosition;
	}
	else
	{
		Vec2 sp = referencePosition - boneStart;
		Vec2 se = boneEnd - boneStart;
		Vec2 ep = referencePosition - boneEnd;
		if (DotProduct2D(se, sp) <= 0.f)
		{
			sp.SetLength(radius);
			return boneStart + sp;
		}
		else if (DotProduct2D(se, ep) >= 0.f)
		{
			ep.SetLength(radius);
			return boneEnd + ep;
		}
		else
		{
			Vec2 sn = GetProjectedOnto2D(sp, se);
			Vec2 n = boneStart + sn;
			Vec2 np = referencePosition - n;
			np.SetLength(radius);
			return n + np;
		}
	}
}

Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePosition, OBB2 const& orientedBox)
{
	Vec2 localP = orientedBox.GetLocalPosForWorldPos(referencePosition);
	AABB2 localBox = AABB2(-orientedBox.m_halfDimensions.x, -orientedBox.m_halfDimensions.y,
		orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.y);
	Vec2 nearLocalPoint = localBox.GetNearestPoint(localP);
	Vec2 worldNearPoint = orientedBox.GetWorldPosForLocalPos(nearLocalPoint);
	return worldNearPoint;
}

Vec2 const GetNearestPointOnTriangle2D(Vec2 const& referencePosition, Triangle2 const& triangle)
{
	if (IsPointInsideTriangle2D(referencePosition, triangle))
	{
		return referencePosition;
	}
	else
	{
		Vec2 n_ab=GetNearestPointOnLineSegment2D(referencePosition,
			triangle.m_pointsCounterClockwise[0], triangle.m_pointsCounterClockwise[1]);
		Vec2 n_bc = GetNearestPointOnLineSegment2D(referencePosition,
			triangle.m_pointsCounterClockwise[1], triangle.m_pointsCounterClockwise[2]);
		Vec2 n_ca = GetNearestPointOnLineSegment2D(referencePosition,
			triangle.m_pointsCounterClockwise[2], triangle.m_pointsCounterClockwise[0]);
		Vec2 n_real;
		if (GetDistanceSquared2D(n_ab, referencePosition) < GetDistanceSquared2D(n_bc, referencePosition))
		{
			n_real = n_ab;
		}
		else
		{
			n_real = n_bc;
		}

		if (GetDistanceSquared2D(n_real, referencePosition) < GetDistanceSquared2D(n_ca, referencePosition))
		{
			return n_real;
		}
		else
		{
			n_real = n_ca;
			return n_real;
		}
	}
}

Vec3 const GetNearestPointOnSphere3D(Vec3 const& referencePosition, Vec3 const& sphereCenter, float sphereRadius)
{
	if (GetDistance3D(referencePosition, sphereCenter) <= sphereRadius)
	{
		return referencePosition;
	}
	else
	{
		Vec3 refer_center = referencePosition - sphereCenter;
		refer_center.SetLength(sphereRadius);
		return refer_center + sphereCenter;
	}
}

Vec3 const GetNearestPointOnZCylinder3D(Vec3 const& referencePosition, Vec3 const& cylinderCenter, float cylinderRadius, float halfHeight)
{
	Vec2 nearPointOnDisc=GetNearestPointOnDisc2D(Vec2(referencePosition.x,referencePosition.y), Vec2(cylinderCenter.x, cylinderCenter.y), cylinderRadius);
	float nearZ=GetClamped(referencePosition.z, cylinderCenter.z - halfHeight, cylinderCenter.z + halfHeight);
	return Vec3(nearPointOnDisc.x, nearPointOnDisc.y, nearZ);
}

Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencePosition, AABB3 const& box)
{
	float x = GetClamped(referencePosition.x, box.m_mins.x, box.m_maxs.x);
	float y = GetClamped(referencePosition.y, box.m_mins.y, box.m_maxs.y);
	float z = GetClamped(referencePosition.z, box.m_mins.z, box.m_maxs.z);
	return Vec3(x, y, z);
}

Vec3 const GetNearestPointOnPlane3D(Vec3 const& referencePosition, Plane3 const& plane)
{
	float altitude = DotProduct3D(plane.m_normal, referencePosition)-plane.m_distance;
	return referencePosition - altitude * plane.m_normal;
}

Vec3 const GetNearestPointOnOBB3D(Vec3 const& referencePosition, OBB3 const& box)
{
	Vec3 boxOriginToPoint = referencePosition - box.m_center;
	Vec3 relativePos = Vec3(DotProduct3D(boxOriginToPoint, box.m_iBasis),
		DotProduct3D(boxOriginToPoint, box.m_jBasis),
		DotProduct3D(boxOriginToPoint, box.m_kBasis));

	float relativeX = GetClamped( relativePos.x, -box.m_halfDimensions.x, box.m_halfDimensions.x);
	float relativeY = GetClamped( relativePos.y, -box.m_halfDimensions.y, box.m_halfDimensions.y);
	float relativeZ = GetClamped( relativePos.z, -box.m_halfDimensions.z, box.m_halfDimensions.z); 

	Mat44 mat = Mat44(box.m_iBasis, box.m_jBasis, box.m_kBasis, box.m_center);
	return mat.TransformPosition3D(Vec3(relativeX, relativeY, relativeZ));
}

bool IsPointFrontOfPlane3(Vec3 const& referencePosition, Plane3 const& plane)
{
	if (DotProduct3D(plane.m_normal, referencePosition) > plane.m_distance)
	{
		return true;
	}
	return false;
}

bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	Vec2 cp = fixedPoint - mobileDiscCenter;
	float distanceSquared = discRadius * discRadius;
	if (cp.GetLengthSquared() >= distanceSquared)
	{
		return false;
	}

	if (cp.GetLengthSquared() < 0.0000001f)
	{
		// disc center is overlap with the nearest point
		// push the disc in +x (as default)
		mobileDiscCenter.x += discRadius;
		return true;
	}

	Vec2 newCp = cp;
	newCp.SetLength(discRadius);
	mobileDiscCenter = mobileDiscCenter - (newCp - cp);
	return true;
}

bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	Vec2 cp = mobileDiscCenter - fixedDiscCenter;
	if (cp.GetLengthSquared() >= (mobileDiscRadius + fixedDiscRadius)* (mobileDiscRadius + fixedDiscRadius))
	{
		return false;
	}
	else
	{

		if (cp.GetLengthSquared() < 0.0000001f)
		{
			// disc center is overlap with the nearest point
			// push the disc in +x (as default)
			mobileDiscCenter.x += mobileDiscRadius;
			return true;
		}
		cp.SetLength(mobileDiscRadius + fixedDiscRadius);
		mobileDiscCenter =fixedDiscCenter+cp;
		return true;
	}
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 ab = bCenter - aCenter;
	float centerDistSq = GetDistanceSquared2D(aCenter, bCenter);
	float totalRadius = aRadius + bRadius;
	if (centerDistSq >= totalRadius *totalRadius)
	{
		return false;
	}

	if (centerDistSq < 0.0001f)
	{
		// disc center is overlap with the nearest point
		// push the disc in +x (as default)
// 		float dirX = g_rng.RollRandomFloatZeroToOne();
// 		float dirY = g_rng.RollRandomFloatZeroToOne();
// 		Vec2 dir = Vec2(dirX, dirY);
// 		dir.SetLength((aRadius + bRadius) * 0.5f);
// 		aCenter += dir;
// 		bCenter-= dir;
		return true;
	}
	float overlap = totalRadius - std::sqrt(centerDistSq);
	ab.SetLength(overlap * 0.5f);
	aCenter += -ab;
	bCenter += ab;
	return true;


}

bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 p=fixedBox.GetNearestPoint(mobileDiscCenter);
	return PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, p);
}

bool DoAABBsOverlap3D(AABB3 const& boxA, AABB3 const& boxB)
{
	if (boxA.m_maxs.x <= boxB.m_mins.x || boxA.m_mins.x >= boxB.m_maxs.x)
		return false;
	if (boxA.m_maxs.y <= boxB.m_mins.y || boxA.m_mins.y >= boxB.m_maxs.y)
		return false;
	if (boxA.m_maxs.z <= boxB.m_mins.z || boxA.m_mins.z >= boxB.m_maxs.z)
		return false;
	return true;
}

bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float cur_dis_square = GetDistanceSquared3D(centerA, centerB);
	if (cur_dis_square < (radiusA + radiusB)*(radiusA + radiusB))
	{
		return true;
	}
	else
		return false;
}

bool DoZCylindersOverlap3D(Vec3 const& centerA, float radiusA, float halfHeightA, Vec3 const& centerB, float radiusB, float halfHeightB)
{
	if (DoDiscsOverlap(Vec2(centerA.x, centerA.y), radiusA, Vec2(centerB.x, centerB.y), radiusB))
	{
		if (centerA.z + halfHeightA <= centerB.z - halfHeightA || centerB.z + halfHeightB <= centerA.z - halfHeightA)
			return false;
		else
			return true;
	}
	return false;
}

bool DoSphereAndAABBOverlap3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box)
{
	Vec3 nearPoint=GetNearestPointOnAABB3D(sphereCenter, box);
	if (GetDistanceSquared3D(sphereCenter, nearPoint) < sphereRadius*sphereRadius)
	{
		return true;
	}
	return false;
}

bool DoZCylinderAndAABBOverlap3D(Vec3 cylinderCenter, float cylinderRadius, float halfHeight, AABB3 const& box)
{
	if (box.m_maxs.x <= cylinderCenter.x-cylinderRadius || box.m_mins.x >= cylinderCenter.x + cylinderRadius)
		return false;
	if (box.m_maxs.y <= cylinderCenter.y - cylinderRadius || box.m_mins.y >= cylinderCenter.y + cylinderRadius)
		return false;
	if (box.m_maxs.z <= cylinderCenter.z - halfHeight || box.m_mins.z >= cylinderCenter.z + halfHeight)
		return false;
	return true;
}

bool DoZCylinderAndShpereOVerlap3D(Vec3 cylinderCenter, float cylinderRadius, float halfHeight, Vec3 const& sphereCenter, float sphereRadius)
{
	Vec3 nearPoint = GetNearestPointOnZCylinder3D(sphereCenter,cylinderCenter,cylinderRadius,halfHeight);
	if (GetDistanceSquared3D(sphereCenter, nearPoint) < sphereRadius * sphereRadius)
	{
		return true;
	}
	return false;
}

bool DoAABB3AndPlane3Overlap3D(AABB3 const& aabb, Plane3 const& plane)
{
	Vec3 halfSize = (aabb.m_maxs - aabb.m_mins) * 0.5f;
	Vec3 center = (aabb.m_mins + aabb.m_maxs) * 0.5f;

	float r = halfSize.x * abs(plane.m_normal.x) +
		halfSize.y * abs(plane.m_normal.y) +
		halfSize.z * abs(plane.m_normal.z);

	float distance = DotProduct3D(plane.m_normal, center) - plane.m_distance;

	return abs(distance) <= r;
}

bool DoSphereAndPlane3Overlap3D(Vec3 const& center, float radius, Plane3 const& plane)
{
	float altitude = DotProduct3D(plane.m_normal, center) - plane.m_distance;
	if (abs(altitude) < radius)
	{
		return true;
	}
	return false;
}

bool DoOBB3AndPlane3Overlap3D(OBB3 const& obb, Plane3 const& plane)
{
	float r = fabsf(obb.m_halfDimensions.x * DotProduct3D(obb.m_iBasis, plane.m_normal)) +
		fabsf(obb.m_halfDimensions.y * DotProduct3D(obb.m_jBasis, plane.m_normal)) +
		fabsf(obb.m_halfDimensions.z * DotProduct3D(obb.m_kBasis, plane.m_normal));

	float s = DotProduct3D(plane.m_normal, obb.m_center) - plane.m_distance;

	return fabsf(s) <= r;
}

bool DoOBB3AndSphereOverlap3D(OBB3 const& obb, Vec3 const& center, float radius)
{
	Vec3 nearPoint = GetNearestPointOnOBB3D(center, obb);
	if (GetDistanceSquared3D(center, nearPoint) < radius * radius)
	{
		return true;
	}
	return false;
}

//bool PushZCylinderOutOfZCylinderFromZ3D(Vec3& mobileCylinderCenter, float mobileCylinderRadius, Vec3 const& fixedCylinderCenter, float fixedCylinderRadius)
//{
//	return false;
//}
//
//bool PushZCylinderOutOfEachOtherFromZ3D(Vec3& aCenter, float aRadius, Vec3& bCenter, float bRadius)
//{
//	Vec2 ab = bCenter - aCenter;
//	if (ab.GetLengthSquared() >= (aRadius + bRadius) * (aRadius + bRadius))
//	{
//		return false;
//	}
//	else
//	{
//		float overlap = aRadius + bRadius - ab.GetLength();
//		ab.SetLength(overlap * 0.5f);
//		aCenter += -ab;
//		bCenter += ab;
//		return true;
//	}
//}

//transform
void TransformPosition2D(Vec2& posToTransform, float uniformScale,
	float rotationDegrees, Vec2 const& translation)
{
	posToTransform.x *= uniformScale;
	posToTransform.y *= uniformScale;

	float cosTheta = CosDegrees(rotationDegrees);
	float sinTheta = SinDegrees(rotationDegrees);

	float newX = posToTransform.x * cosTheta - posToTransform.y * sinTheta;
	float newY = posToTransform.x * sinTheta + posToTransform.y * cosTheta;
	posToTransform.x = newX;
	posToTransform.y = newY;

	posToTransform.x += translation.x;
	posToTransform.y += translation.y;

}
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const jBasis, Vec2 const& translation)
{
	posToTransform=posToTransform.x * iBasis + posToTransform.y * jBasis + translation;
}

void TransformPositionXY3D(Vec3& posToTransform, float uniformScale,
	float rotationDegrees, Vec2 const& translation)
{
	posToTransform.x *= uniformScale;
	posToTransform.y *= uniformScale;

	float cosTheta = CosDegrees(rotationDegrees);
	float sinTheta = SinDegrees(rotationDegrees);

	float newX = posToTransform.x * cosTheta - posToTransform.y * sinTheta;
	float newY = posToTransform.x * sinTheta + posToTransform.y * cosTheta;
	posToTransform.x = newX;
	posToTransform.y = newY;
	
	posToTransform.x += translation.x;
	posToTransform.y += translation.y;
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const jBasis, Vec2 const& translation)
{
	Vec2 posToTransform2 = posToTransform.x * iBasis + posToTransform.y * jBasis + translation;
	posToTransform.x = posToTransform2.x;
	posToTransform.y = posToTransform2.y;

	// posToTransform.x will be changed while this func so the result will be wrong
	//posToTransform.x = posToTransform.x * iBasis.x + posToTransform.y * jBasis.x + translation.x;
	//posToTransform.y = posToTransform.x * iBasis.y + posToTransform.y * jBasis.y + translation.y;
}
float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	else if (value > maxValue)
	{
		return maxValue;
	}
	else
		return value;
}

float GetClampedZeroToOne(float value)
{
	if (value < 0)
	{
		return 0;
	}
	else if (value > 1)
	{
		return 1;
	}
	else
		return value;
}

float Interpolate(float start, float end, float fractionTowardEnd)
{
	return start + (end - start) * fractionTowardEnd;
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	if (rangeEnd != rangeStart) //cannot use rangeEnd>rangeStart cause the order of parameters can be reverted
	{
		return (value - rangeStart) / (rangeEnd - rangeStart);
	}
	return 0.0f;
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float t = GetFractionWithinRange(inValue, inStart, inEnd);
	return Interpolate(outStart, outEnd, t);
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if (inEnd > inStart)
	{
		if (inValue < inStart)
		{
			return outStart;
		}
		else if (inValue > inEnd)
		{
			return outEnd;
		}
		else
			return RangeMap(inValue, inStart, inEnd, outStart, outEnd);
	}
	else
	{
		if (inValue < inEnd)
		{
			return outEnd;
		}
		else if (inValue > inStart)
		{
			return outStart;
		}
		else
			return RangeMap(inValue, inStart, inEnd, outStart, outEnd);
	}
}

float RangeMapClamped(float inValue, FloatRange const& inRange, FloatRange const& outRange)
{
	return RangeMapClamped(inValue, inRange.m_min, inRange.m_max, outRange.m_min, outRange.m_max);
}

int RoundDownToInt(float value)
{
	//?
	return (int)floorf(value);
}

float NormalizeByte(unsigned char uc)
{
	if (uc == (unsigned char)255)
		return 1.f;
	else if (uc == (unsigned char)0)
		return 0.f;
	else
		return static_cast<float>(uc) / 255.0f;
}

float DenormalizeByte(float f)
{
	if (f == 1.f)
		return 255.f;
	else
		return (float)RoundDownToInt(f * 256.f);
}

Mat44 GetLookAtMatrix(const Vec3& pos,const Vec3& target)
{
	Vec3 I = (target - pos).GetNormalized();
	Vec3 J;
	Vec3 K;
	Vec3 z = Vec3(0.f, 0.f, 1.f);
	Vec3 x = Vec3(1.f, 0.f, 0.f);
	Vec3 y = Vec3(0.f, 1.f, 0.f);
	if (abs(DotProduct3D(z, I)) < 0.99f)
	{
		J = CrossProduct3D(z, I).GetNormalized();
		K = CrossProduct3D(I, J).GetNormalized();
	}
	else
	{
		K = CrossProduct3D(I, y).GetNormalized();
		J = CrossProduct3D(K, I).GetNormalized();
	}

	Mat44 rotationMatrix;
	rotationMatrix.SetIJK3D(I, J, K);
	Mat44 translationMatrix;
	translationMatrix.SetTranslation3D(pos);
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& targetMatrix, Vec3 const& targetPos, const Vec3& billboardPosition, const Vec2& billboardScale)
{
	UNUSED(billboardScale);
	if (billboardType == BillboardType::WORLD_UP_FACING)
	{
		Vec3 curTarget = Vec3(targetPos.x, targetPos.y, billboardPosition.z);
		return GetLookAtMatrix(billboardPosition, curTarget);
	}
	else if (billboardType == BillboardType::WORLD_UP_OPPOSING)
	{
		Mat44 translation = Mat44::MakeTranslation3D(billboardPosition);

		Mat44 reflectedMatrix = targetMatrix;
		reflectedMatrix.SetIJK3D(CrossProduct3D(targetMatrix.GetJBasis3D() * -1.f, Vec3(0.f, 0.f, 1.f)), targetMatrix.GetJBasis3D() * -1.f, Vec3(0.f,0.f,1.f));

		translation.Append(reflectedMatrix);
		return translation;
	}
	else if (billboardType == BillboardType::FULL_FACING)
	{
		return GetLookAtMatrix(billboardPosition, targetPos);
	}
	else if (billboardType == BillboardType::FULL_OPPOSING)
	{
		Mat44 translation = Mat44::MakeTranslation3D(billboardPosition);

		Mat44 reflectedMatrix = targetMatrix;
		reflectedMatrix.SetIJK3D(targetMatrix.GetIBasis3D() * -1.f, targetMatrix.GetJBasis3D() * -1.f, targetMatrix.GetKBasis3D());

		translation.Append(reflectedMatrix);
		return translation;
	}
	return Mat44();
}

float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
{
	float AB = A * (1.0f - t) + B * t;
	float BC = B * (1.0f - t) + C * t;
	float CD = C * (1.0f - t) + D * t;

	float ABC = AB * (1.0f - t) + BC * t;
	float BCD = BC * (1.0f - t) + CD * t;

	float ABCD = ABC * (1.0f - t) + BCD * t;

	return ABCD;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float points[6] = { A, B, C, D, E, F };
	int n = 6;

	for (int r = 1; r < n; ++r) 
	{
		for (int i = 0; i < n - r; ++i) 
		{
			points[i] = (1.0f - t) * points[i] + t * points[i + 1];
		}
	}

	return points[0];
}

void BounceDiscOffPoint(Vec2 const& point, Vec2 const& discCenter, Vec2& discVelocity, float discRadius, float elasity)
{
	UNUSED(discRadius);
	Vec2 normal = (discCenter - point).GetNormalized();
	float vnLen = DotProduct2D(discVelocity, -normal);
	Vec2 vn = vnLen * (-normal);
	Vec2 vt = discVelocity - vn;
	Vec2 newV = vt + (-vn * elasity);
	discVelocity = newV;
}

void BounceDiscOffStaticDisc2D(Vec2 const& bumperCenter, float bumperRadius, Vec2& discCenter, Vec2& discVelocity, float discRadius, 
	float elastcityBumper, float elastcityBall)
{
	if (PushDiscOutOfDisc2D(discCenter, discRadius, bumperCenter, bumperRadius))
	{
		Vec2 refPoint=GetNearestPointOnDisc2D(discCenter, bumperCenter, bumperRadius);
		BounceDiscOffPoint(refPoint, discCenter, discVelocity, discRadius, elastcityBall*elastcityBumper);
	}
}

void BounceDiscOffCapsule2D(Capsule2 const& bumper, Vec2& discCenter, float discRadius, Vec2& discVelocity, 
	float elastcityBumper, float elastcityBall)
{
	if (DoDiscsOverlap(bumper.m_bone.m_center, bumper.m_boundRadius, discCenter, discRadius))
	{
		Vec2 nearPoint = GetNearestPointOnCapsule2D(discCenter, bumper);
		if (PushDiscOutOfPoint2D(discCenter, discRadius, nearPoint))
		{
			BounceDiscOffPoint(nearPoint, discCenter, discVelocity, discRadius, elastcityBall * elastcityBumper);
		}
	}
}

void BounceDiscOffOBB2D(OBB2 const& bumper, Vec2& discCenter, float discRadius, Vec2& discVelocity, 
	float elastcityBumper, float elastcityBall)
{
	if (DoDiscsOverlap(bumper.m_center, bumper.m_boundRadius, discCenter, discRadius))
	{
		Vec2 nearPoint = GetNearestPointOnOBB2D(discCenter, bumper);
		if (PushDiscOutOfPoint2D(discCenter, discRadius, nearPoint))
		{
			BounceDiscOffPoint(nearPoint, discCenter, discVelocity, discRadius, elastcityBall * elastcityBumper);
		}
	}
}

void BounceDiscOffEachOther(Vec2& aCenter, float aRadius, Vec2& aVelocity, float aElasticity,
	Vec2& bCenter, float bRadius, Vec2& bVelocity,float bElasticity)
{
	if (PushDiscsOutOfEachOther2D(aCenter, aRadius, bCenter, bRadius))
	{
		float totalEla = aElasticity * bElasticity;

		// can not use get normalized?
		Vec2 normal = (bCenter - aCenter).GetNormalized();
		//Vec2 normal = bCenter - aCenter;
		float aVnLen = DotProduct2D(aVelocity, normal);
		float bVnLen = DotProduct2D(bVelocity, normal);

		if (aVnLen - bVnLen <=0.f)
		{
			return;
		}

		float newAVnLen = bVnLen * totalEla;
		float newBVnLen = aVnLen * totalEla;

		Vec2 aVt = aVelocity - (aVnLen * normal);
		Vec2 bVt = bVelocity - (bVnLen * normal);

		aVelocity = (newAVnLen * normal) + aVt;
		bVelocity = (newBVnLen * normal) + bVt;
	}
}

EulerAngles GetEulerFromRotationMat(Mat44 const& mat)
{
	EulerAngles euler;

	if (mat.m_values[Mat44::Jz] > 0.998f)
	{
		euler.m_yawDegrees = atan2f(-mat.m_values[Mat44::Ix], mat.m_values[Mat44::Kx]) * 180.0f / PI;
		euler.m_pitchDegrees = -90.0f;
		euler.m_rollDegrees = 0.0f;
	}
	else if (mat.m_values[Mat44::Jz] < -0.998f) {
		euler.m_yawDegrees = atan2f(mat.m_values[Mat44::Ix], -mat.m_values[Mat44::Kx]) * 180.0f / PI;
		euler.m_pitchDegrees = 90.0f;
		euler.m_rollDegrees = 0.0f;
	}
	else {
		euler.m_pitchDegrees = asinf(-mat.m_values[Mat44::Jz]) * 180.0f / PI;
		euler.m_yawDegrees = atan2f(mat.m_values[Mat44::Iz], mat.m_values[Mat44::Kz]) * 180.0f / PI;
		euler.m_rollDegrees = atan2f(mat.m_values[Mat44::Jx], mat.m_values[Mat44::Jy]) * 180.0f / PI;
	}

	return euler;
}


