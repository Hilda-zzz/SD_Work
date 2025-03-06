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

#define PI 3.1415926535897f

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
	if (GetDistance2D(point, discCenter) < discRadius)
	{
		return true;
	}
	return false;
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) < sectorRadius)
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
	return pn.GetLengthSquared() < capsule.radius * capsule.radius;
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
			sp.SetLength(capsule.radius);
			return capsule.m_bone.m_start + sp;
		}
		else if (DotProduct2D(se, ep) >= 0.f)
		{
			se.SetLength(capsule.radius);
			return capsule.m_bone.m_end + se;
		}
		else
		{
			Vec2 sn = GetProjectedOnto2D(sp, se);
			Vec2 n = capsule.m_bone.m_start + sn;
			Vec2 np = referencePosition - n;
			np.SetLength(capsule.radius);
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

bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	Vec2 cp = fixedPoint - mobileDiscCenter;
	if (cp.GetLengthSquared() >= discRadius * discRadius)
	{
		return false;
	}
	else
	{
		Vec2 newCp = cp;
		newCp.SetLength(discRadius);
		mobileDiscCenter = mobileDiscCenter - (newCp - cp);
		return true;
	}
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
		cp.SetLength(mobileDiscRadius + fixedDiscRadius);
		mobileDiscCenter =fixedDiscCenter+cp;
		return true;
	}
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 ab = bCenter - aCenter;
	if (ab.GetLengthSquared() >= (aRadius + bRadius) * (aRadius + bRadius))
	{
		return false;
	}
	else
	{
		float overlap = aRadius + bRadius - ab.GetLength();
		ab.SetLength(overlap * 0.5f);
		aCenter += -ab;
		bCenter += ab;
		return true;
	}
}

bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 p=fixedBox.GetNearestPoint(mobileDiscCenter);
	return PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, p);
}

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
	if (abs(DotProduct3D(z, I)) < 1.f)
	{
		J = CrossProduct3D(z, I);
		K = CrossProduct3D(I, J);
	}
	else
	{
		K = CrossProduct3D(I, x);
		J = CrossProduct3D(K, I);
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
		return GetLookAtMatrix(billboardPosition,targetPos);
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
		Vec3 curTarget = Vec3(targetPos.x, targetPos.y, billboardPosition.z);
		return GetLookAtMatrix(billboardPosition, curTarget);
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
