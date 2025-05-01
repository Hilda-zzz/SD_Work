#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Vertex_PCU.hpp"
#include "Engine/Math/AABB3.hpp"
#include "EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"
constexpr int DISCS_SLICES_COUNT = 64;
constexpr int SEMICIRCLE_SLICES_COUNT = 16;
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, 
    float rotationDegreesAboutZ, Vec2 const& translationXY) 
{
    for (int i = 0; i < numVerts; ++i)
    {
        TransformPositionXY3D(verts[i].m_position, uniformScaleXY, 
            rotationDegreesAboutZ, translationXY);
    }
}

void TransformVertexArrayXY3D(int numVerts, std::vector<Vertex_PCU>& verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int i = 0; i < numVerts; ++i)
	{
		TransformPositionXY3D(verts[i].m_position, uniformScaleXY,
			rotationDegreesAboutZ, translationXY);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (Vertex_PCU& vert : verts)
	{
		vert.m_position=transform.TransformPosition3D(vert.m_position);
	}
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	float deltaDegrees = 180.f / SEMICIRCLE_SLICES_COUNT;
	Vec2 step_forward = boneEnd - boneStart;
	step_forward.SetLength(radius);
	step_forward.Rotate90Degrees();
	float step_left_deg = step_forward.GetOrientationDegrees();
	
	Vec2 SL = boneStart + step_forward;
	Vec2 SR = boneStart - step_forward;
	Vec2 EL = boneEnd + step_forward;
	Vec2 ER = boneEnd - step_forward;

	verts.push_back(Vertex_PCU(Vec3(SL.x, SL.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(ER.x, ER.y, 0.f), color, Vec2(0.f, 0.f)));

	verts.push_back(Vertex_PCU(Vec3(SL.x, SL.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(ER.x, ER.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.f, 0.f)));

	for (int i = 0; i < SEMICIRCLE_SLICES_COUNT; i++)
	{
		Vec2 first = boneStart+Vec2::MakeFromPolarDegrees(step_left_deg + i * deltaDegrees, radius);
		Vec2 second = boneStart+Vec2::MakeFromPolarDegrees(step_left_deg + (i+1) * deltaDegrees, radius);
		verts.push_back(Vertex_PCU(Vec3(boneStart.x, boneStart.y, 0.f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(first.x, first.y, 0.f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(second.x,second.y, 0.f), color, Vec2(0.f, 0.f)));
	}

	for (int i = 0; i < SEMICIRCLE_SLICES_COUNT; i++)
	{
		Vec2 first = boneEnd + Vec2::MakeFromPolarDegrees(step_left_deg-180.f + i * deltaDegrees, radius);
		Vec2 second = boneEnd + Vec2::MakeFromPolarDegrees(step_left_deg-180.f + (i + 1) * deltaDegrees, radius);
		verts.push_back(Vertex_PCU(Vec3(boneEnd.x, boneEnd.y, 0.f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(first.x, first.y, 0.f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(second.x, second.y, 0.f), color, Vec2(0.f, 0.f)));
	}
}

void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color)
{
	float deltaDegrees = 360.f / DISCS_SLICES_COUNT;
	int j = 0;
	for (int i = 0; i < DISCS_SLICES_COUNT*3; )
	{
		Vec2 curPosFirst = Vec2::MakeFromPolarDegrees(deltaDegrees * j, radius) + center;
		Vec2 curPosSecond = Vec2::MakeFromPolarDegrees(deltaDegrees * (j + 1), radius) + center;
		verts.push_back(Vertex_PCU(Vec3(center.x, center.y, 0.f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), color, Vec2(0.f, 0.f)));
		i += 3;
		j += 1;
	}
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color)
{
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.f), color, Vec2(0.f, 1.f)));
	
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.f), color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), color, Vec2(1.f, 1.f)));

}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uv_mins, Vec2 const& uv_maxs)
{
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), color, uv_mins));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), color, uv_maxs));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.f), color, Vec2(uv_mins.x, uv_maxs.y)));

	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f), color, uv_mins));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.f), color, Vec2(uv_maxs.x, uv_mins.y)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f), color, uv_maxs));
}

void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	Vec2 corners[4];
	box.GetCornerPoints(corners);

	verts.push_back(Vertex_PCU(Vec3(corners[0].x, corners[0].y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(corners[1].x, corners[1].y, 0.f), color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(corners[2].x, corners[2].y, 0.f), color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(Vec3(corners[0].x, corners[0].y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(corners[2].x, corners[2].y, 0.f), color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(Vec3(corners[3].x, corners[3].y, 0.f), color, Vec2(0.f, 1.f)));
}

void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, Vec2 const& uv_mins, Vec2 const& uv_maxs)
{
	Vec2 corners[4];
	box.GetCornerPoints(corners);

	verts.push_back(Vertex_PCU(Vec3(corners[0].x, corners[0].y, 0.f), color, uv_mins));
	verts.push_back(Vertex_PCU(Vec3(corners[1].x, corners[1].y, 0.f), color, Vec2(uv_maxs.x, uv_mins.y)));
	verts.push_back(Vertex_PCU(Vec3(corners[2].x, corners[2].y, 0.f), color, uv_maxs));

	verts.push_back(Vertex_PCU(Vec3(corners[0].x, corners[0].y, 0.f), color, uv_mins));
	verts.push_back(Vertex_PCU(Vec3(corners[2].x, corners[2].y, 0.f), color, uv_maxs));
	verts.push_back(Vertex_PCU(Vec3(corners[3].x, corners[3].y, 0.f), color, Vec2(uv_mins.x, uv_maxs.y)));
}

void AddVertsForTriangle2D(std::vector<Vertex_PCU>& verts, Triangle2 const& triangle, Rgba8 const& color)
{
	verts.push_back(Vertex_PCU(Vec3(triangle.m_pointsCounterClockwise[0].x, triangle.m_pointsCounterClockwise[0].y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(triangle.m_pointsCounterClockwise[1].x, triangle.m_pointsCounterClockwise[1].y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(triangle.m_pointsCounterClockwise[2].x, triangle.m_pointsCounterClockwise[2].y, 0.f), color, Vec2(0.f, 0.f)));
}

//void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, Vec2 const& iBasis, Vec2 dimensions, Rgba8 const& color)
//{
//}

void AddVertsForLinSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 step_fwd = end - start;
	step_fwd = step_fwd.GetNormalized();
	Vec2 step_left = step_fwd.GetRotated90Degrees();
	float halfWidth = thickness / 2.f;
	step_fwd *= halfWidth;
	step_left *= halfWidth;
	Vec2 SL = start - step_fwd + step_left;
	Vec2 SR = start - step_fwd - step_left;
	Vec2 EL = end + step_fwd + step_left;
	Vec2 ER = end + step_fwd - step_left;

	verts.push_back(Vertex_PCU(Vec3(SL.x, SL.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.f, 0.f)));

	verts.push_back(Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(Vec3(ER.x, ER.y, 0.f), color, Vec2(0.f, 0.f)));
	
}

void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness,Rgba8 const& color)
{
	Vec2 direction = tipPos - tailPos;
	Vec2 leftArrow = (direction.GetRotatedDegrees(135.f)).GetNormalized()*arrowSize+tipPos;
	Vec2 rightArrow = (direction.GetRotatedDegrees(-135.f)).GetNormalized() * arrowSize + tipPos;
	AddVertsForLinSegment2D(verts, tailPos, tipPos, lineThickness, color);
	AddVertsForLinSegment2D(verts, leftArrow, tipPos, lineThickness, color);
	AddVertsForLinSegment2D(verts, rightArrow, tipPos, lineThickness, color);
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	verts.push_back(Vertex_PCU(Vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z), color, UVs.m_mins));                    
	verts.push_back(Vertex_PCU(Vec3(bottomRight.x, bottomRight.y, bottomRight.z), color, Vec2(UVs.m_maxs.x,UVs.m_mins.y)));			   
	verts.push_back(Vertex_PCU(Vec3(topRight.x, topRight.y, topRight.z), color, UVs.m_maxs));					  

	verts.push_back(Vertex_PCU(Vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z), color, UVs.m_mins));				 
	verts.push_back(Vertex_PCU(Vec3(topRight.x, topRight.y, topRight.z), color, UVs.m_maxs));					
	verts.push_back(Vertex_PCU(Vec3(topLeft.x, topLeft.y, topLeft.z), color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));						  
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	unsigned int startIndex = (unsigned int)verts.size();

	verts.push_back(Vertex_PCU(Vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z), color, UVs.m_mins));                    
	verts.push_back(Vertex_PCU(Vec3(bottomRight.x, bottomRight.y, bottomRight.z), color, Vec2(UVs.m_maxs.x, UVs.m_mins.y))); 
	verts.push_back(Vertex_PCU(Vec3(topRight.x, topRight.y, topRight.z), color, UVs.m_maxs));                        
	verts.push_back(Vertex_PCU(Vec3(topLeft.x, topLeft.y, topLeft.z), color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));    

	indexes.push_back(startIndex + 0);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);

	indexes.push_back(startIndex + 0);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
}

void AddVertsForQuad3D_WithTBN(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 edge1 = bottomRight - bottomLeft;
	Vec3 edge2 = topLeft - bottomLeft;
	Vec3 normal = CrossProduct3D(edge1, edge2).GetNormalized();
	Vec3 tangent = edge1.GetNormalized();
	Vec3 bitangent = CrossProduct3D(normal, tangent);

	unsigned int startIndex = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, normal));

	indexes.push_back(startIndex + 0);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);

	indexes.push_back(startIndex + 0);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
}

void AddVertsForQuad3D_WithTBN(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 edge1 = bottomRight - bottomLeft;
	Vec3 edge2 = topLeft - bottomLeft;
	Vec3 normal = CrossProduct3D(edge1, edge2).GetNormalized();
	Vec3 tangent = edge1.GetNormalized();
	Vec3 bitangent = CrossProduct3D(normal, tangent);

	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, normal));

	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, normal));
}

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& vertexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 edge1 = bottomRight - bottomLeft;
	Vec3 edge2 = topLeft - bottomLeft;
	Vec3 normal = CrossProduct3D(edge1, edge2).GetNormalized();
	Vec3 tangent = edge1.GetNormalized();
	Vec3 bitangent = CrossProduct3D(normal, tangent);
	
	Vec3 bottomMiddle = (bottomLeft + bottomRight) * 0.5f;
	Vec3 topMiddle = (topLeft + topRight) * 0.5f;

	Vec2 bottomUV = Vec2(UVs.m_mins.x + (UVs.m_maxs.x - UVs.m_mins.x) * 0.5f, UVs.m_mins.y);
	Vec2 topUV = Vec2(UVs.m_mins.x + (UVs.m_maxs.x - UVs.m_mins.x) * 0.5f, UVs.m_maxs.y);
	Vec2 leftUV = Vec2(UVs.m_mins.x, UVs.m_mins.y + (UVs.m_maxs.y - UVs.m_mins.y) * 0.5f);
	Vec2 rightUV = Vec2(UVs.m_maxs.x, UVs.m_mins.y + (UVs.m_maxs.y - UVs.m_mins.y) * 0.5f);

	Vec3 leftNormal = -tangent; 
	Vec3 rightNormal = tangent; 

	vertexes.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, leftNormal));
	vertexes.push_back(Vertex_PCUTBN(bottomMiddle, color, bottomUV, tangent, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(topMiddle, color, topUV, tangent, bitangent, normal));

	vertexes.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, leftNormal));
	vertexes.push_back(Vertex_PCUTBN(topMiddle, color, topUV, tangent, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, leftNormal));

	vertexes.push_back(Vertex_PCUTBN(bottomMiddle, color, bottomUV, tangent, bitangent, normal));
	vertexes.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, rightNormal));
	vertexes.push_back(Vertex_PCUTBN(topMiddle, color, topUV, tangent, bitangent, normal));

	vertexes.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, rightNormal));
	vertexes.push_back(Vertex_PCUTBN(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y), tangent, bitangent, rightNormal));
	vertexes.push_back(Vertex_PCUTBN(topMiddle, color, topUV, tangent, bitangent, normal));
}


void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	Vec3 mins = bounds.m_mins;
	Vec3 maxs = bounds.m_maxs;
	Vec2 uvMins = UVs.m_mins;
	Vec2 uvMaxs = UVs.m_maxs;

	// X
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, mins.y, mins.z),  
		Vec3(maxs.x, maxs.y, mins.z),  
		Vec3(maxs.x, maxs.y, maxs.z),  
		Vec3(maxs.x, mins.y, maxs.z),  
		color, AABB2(uvMins,uvMaxs));

	// -X
	AddVertsForQuad3D(verts,
		Vec3(mins.x, maxs.y, mins.z),  
		Vec3(mins.x, mins.y, mins.z),  
		Vec3(mins.x, mins.y, maxs.z), 
		Vec3(mins.x, maxs.y, maxs.z), 
		color, AABB2(uvMins, uvMaxs));

	// Y
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, maxs.y, mins.z),  
		Vec3(mins.x, maxs.y, mins.z),  
		Vec3(mins.x, maxs.y, maxs.z), 
		Vec3(maxs.x, maxs.y, maxs.z),  
		color, AABB2(uvMins, uvMaxs));

	// -Y
	AddVertsForQuad3D(verts,
		Vec3(mins.x, mins.y, mins.z),  
		Vec3(maxs.x, mins.y, mins.z),  
		Vec3(maxs.x, mins.y, maxs.z),  
		Vec3(mins.x, mins.y, maxs.z),  
		color, AABB2(uvMins, uvMaxs));

	// Z
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, mins.y, maxs.z),  
		Vec3(maxs.x, maxs.y, maxs.z),  
		Vec3(mins.x, maxs.y, maxs.z),  
		Vec3(mins.x, mins.y, maxs.z),  
		color,AABB2(uvMins, uvMaxs));

	// -Z
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, mins.y, mins.z),
		Vec3(mins.x, mins.y, mins.z),
		Vec3(mins.x, maxs.y, mins.z),
		Vec3(maxs.x, maxs.y, mins.z),
		color, AABB2(uvMins, uvMaxs));
}

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numSlices, int numStacks)
{
	UNUSED(center);
	std::vector<Vec3>	sphereVerts;
	sphereVerts.reserve(numSlices * (numStacks - 1) + 2);
	sphereVerts.push_back(Vec3(0.f, 0.f, -radius));
	float eachLongitude = 360.f / numSlices;
	float eachLatitude= 180.f / numStacks;
	for (int curStack = 0; curStack < numStacks - 1; curStack++)
	{
		float curLatitude= -90.f + (curStack + 1) * eachLatitude;
		for (int curSlice = 0; curSlice < numSlices; curSlice++)
		{
			float curLongitude = curSlice * eachLongitude;
			sphereVerts.push_back(Vec3::MakeFromPolarDegrees(curLatitude, curLongitude, radius));
		}
	}
	sphereVerts.push_back(Vec3(0.f, 0.f, radius));


	float eachUVHeight = (1.f / numStacks)*UVs.GetDimensions().y;
	float eachUVWidth = 1.f / numSlices * UVs.GetDimensions().x;
	//east
	for (int curSlice = 0; curSlice < numSlices; curSlice++) 
	{
		int tl = 1 + curSlice;
		int tr = 1 + (curSlice + 1) ;

		Vec2 blUV = UVs.m_mins+Vec2(curSlice * eachUVWidth, eachUVHeight);
		Vec2 trUV = UVs.m_mins + Vec2((curSlice + 1) * eachUVWidth, eachUVHeight);

		AddVertsForQuad3D(verts, sphereVerts[0], sphereVerts[0], sphereVerts[tr], sphereVerts[tl], color, AABB2(blUV, trUV));
	}
	//mid
	for (int curStack = 0; curStack < numStacks - 2; curStack++)
	{
		for (int curSlice = 0; curSlice < numSlices; curSlice++)
		{
			int baseIndex = 1 + curStack * numSlices; 
			int bl = baseIndex + curSlice;
			int br = baseIndex + ((curSlice + 1) % numSlices);
			int tl = bl + numSlices;
			int tr = br + numSlices;

			Vec2 blUV = UVs.m_mins + Vec2(curSlice * eachUVWidth, (curStack + 1) * eachUVHeight);
			Vec2 trUV = UVs.m_mins + Vec2(((curSlice + 1)) * eachUVWidth, (curStack + 2) * eachUVHeight);
			AddVertsForQuad3D(verts, sphereVerts[bl], sphereVerts[br], sphereVerts[tr], sphereVerts[tl],color, AABB2(blUV, trUV));
		}
	}

	//north
	int northPole = (int)sphereVerts.size() - 1;
	int lastRingStart = northPole - numSlices;
	for (int curSlice = 0; curSlice < numSlices; curSlice++)
	{
		int bl = lastRingStart + curSlice;
		int br = lastRingStart + (curSlice + 1) % numSlices;

		Vec2 blUV = UVs.m_mins + Vec2(curSlice * eachUVWidth, (numStacks - 1) * eachUVHeight);
		Vec2 trUV = UVs.m_mins + Vec2((curSlice + 1) * eachUVWidth, numStacks * eachUVHeight);

		AddVertsForQuad3D(verts, sphereVerts[bl], sphereVerts[br], sphereVerts[northPole], sphereVerts[northPole], color, AABB2(blUV, trUV));
	}
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8 color, const AABB2& UVs, int numSlices)
{
	size_t originalSize = verts.size();

	float height = (end - start).GetLength();

	Vec3 localStart(0.0f, 0.0f, 0.f);
	Vec3 localEnd(0.f, 0.f, height);

	std::vector<Vec3> bottomCircle;
	std::vector<Vec3> topCircle;
	for (int i = 0; i < numSlices; i++)
	{
		float x = radius * CosDegrees(i*360.f / numSlices);
		float y = radius * SinDegrees(i*360.f / numSlices);
		bottomCircle.push_back(Vec3(x, y, 0.f));
		topCircle.push_back(Vec3(x, y, height));
	}

	//side
	float eachUVWidth = (1.f / numSlices) * UVs.GetDimensions().x;
	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1)%numSlices ;
		Vec2 blUV = UVs.m_mins + Vec2(i * eachUVWidth, 0.f);
		Vec2 trUV = blUV + Vec2(eachUVWidth, UVs.GetDimensions().y);
		AddVertsForQuad3D(verts, bottomCircle[i], bottomCircle[nextI], topCircle[nextI], topCircle[i], color, AABB2(blUV, trUV));
	}

	//bot
	Vec2 uvCenter = Vec2(UVs.GetCenter().x, UVs.GetCenter().y);
	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1) % numSlices;
		
		float u1 = (UVs.GetDimensions().x / 2.f) * CosDegrees((numSlices - i) * 360.f / numSlices);
		float v1 = (UVs.GetDimensions().y / 2.f) * SinDegrees((numSlices - i) * 360.f / numSlices);
		float u2 = (UVs.GetDimensions().x / 2.f) * CosDegrees((numSlices - i - 1) * 360.f / numSlices);
		float v2 = (UVs.GetDimensions().y / 2.f) * SinDegrees((numSlices - i - 1) * 360.f / numSlices);
		verts.push_back(Vertex_PCU(localStart, color, uvCenter));
		verts.push_back(Vertex_PCU(bottomCircle[nextI], color, uvCenter + Vec2(u2, v2)));
		verts.push_back(Vertex_PCU(bottomCircle[i], color, uvCenter + Vec2(u1, v1)));
	}

	//top
	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1) % numSlices;
		float u1 = (UVs.GetDimensions().x / 2.f) * CosDegrees(i * 360.f / numSlices);
		float v1 = (UVs.GetDimensions().y / 2.f) * SinDegrees(i * 360.f / numSlices);
		float u2 = (UVs.GetDimensions().x / 2.f) * CosDegrees((i + 1) * 360.f / numSlices);
		float v2 = (UVs.GetDimensions().y / 2.f) * SinDegrees((i + 1) * 360.f / numSlices);
		verts.push_back(Vertex_PCU(localEnd, color, uvCenter));
		verts.push_back(Vertex_PCU(topCircle[i], color, uvCenter + Vec2(u1, v1)));
		verts.push_back(Vertex_PCU(topCircle[nextI], color, uvCenter + Vec2(u2, v2)));
	}


	Mat44 rotateMat = Mat44::MakeYRotationDegrees(90.f);
	Mat44 lookAtMatrix = GetLookAtMatrix(start, end);
	lookAtMatrix.Append(rotateMat);


	for (size_t i = originalSize; i < verts.size(); i++)
	{
		verts[i].m_position = lookAtMatrix.TransformPosition3D(verts[i].m_position);
	}
}


void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8 color, const AABB2& UVs, int numSlices)
{
	size_t originalSize = verts.size();
	float height = (end - start).GetLength();

	Vec3 localStart(0.0f, 0.0f, 0.f);
	Vec3 localEnd(0.f, 0.f, height);

	std::vector<Vec3> bottomCircle;
	for (int i = 0; i < numSlices; i++)
	{
		float x = radius * CosDegrees(i * 360.f / numSlices);
		float y = radius * SinDegrees(i * 360.f / numSlices);
		bottomCircle.push_back(Vec3(x, y, 0.f));
	}

	//side
	//float eachUVWidth = (1.f / numSlices) * UVs.GetDimensions().x;
	Vec2 uvCenter = Vec2(UVs.GetCenter().x, UVs.GetCenter().y);
	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1) % numSlices;
		float u1 = (UVs.GetDimensions().x / 2.f) * CosDegrees(i * 360.f / numSlices);
		float v1 = (UVs.GetDimensions().y / 2.f) * SinDegrees(i * 360.f / numSlices);
		float u2 = (UVs.GetDimensions().x / 2.f) * CosDegrees((i + 1) * 360.f / numSlices);
		float v2 = (UVs.GetDimensions().y / 2.f) * SinDegrees((i + 1) * 360.f / numSlices);
		verts.push_back(Vertex_PCU(bottomCircle[i], color, uvCenter + Vec2(u1, v1)));
		verts.push_back(Vertex_PCU(bottomCircle[nextI], color, uvCenter + Vec2(u2, v2)));
		verts.push_back(Vertex_PCU(localEnd, color, uvCenter));
		
	}

	//bot
	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1) % numSlices;
		float u1 = (UVs.GetDimensions().x / 2.f) * CosDegrees((numSlices - i) * 360.f / numSlices);
		float v1 = (UVs.GetDimensions().y / 2.f) * SinDegrees((numSlices - i) * 360.f / numSlices);
		float u2 = (UVs.GetDimensions().x / 2.f) * CosDegrees((numSlices - i - 1) * 360.f / numSlices);
		float v2 = (UVs.GetDimensions().y / 2.f) * SinDegrees((numSlices - i - 1) * 360.f / numSlices);
		verts.push_back(Vertex_PCU(localStart, color, uvCenter));
		verts.push_back(Vertex_PCU(bottomCircle[nextI], color, uvCenter + Vec2(u2, v2)));
		verts.push_back(Vertex_PCU(bottomCircle[i], color, uvCenter + Vec2(u1, v1)));
	}

	Mat44 rotateMat = Mat44::MakeYRotationDegrees(90.f);
	Mat44 lookAtMatrix = GetLookAtMatrix(start, end);
	lookAtMatrix.Append(rotateMat);

	for (size_t i = originalSize; i < verts.size(); i++)
	{
		verts[i].m_position = lookAtMatrix.TransformPosition3D(verts[i].m_position);
	}
}

void AddVertsForAABB3DWireFrame(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	Vec3 mins = bounds.m_mins;
	Vec3 maxs = bounds.m_maxs;
	float radius = 0.03f; 

	AddVertsForCylinder3D(verts, Vec3(mins.x, mins.y, mins.z), Vec3(maxs.x, mins.y, mins.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(maxs.x, mins.y, mins.z), Vec3(maxs.x, maxs.y, mins.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(maxs.x, maxs.y, mins.z), Vec3(mins.x, maxs.y, mins.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(mins.x, maxs.y, mins.z), Vec3(mins.x, mins.y, mins.z), radius, color, UVs);

	AddVertsForCylinder3D(verts, Vec3(mins.x, mins.y, maxs.z), Vec3(maxs.x, mins.y, maxs.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(maxs.x, mins.y, maxs.z), Vec3(maxs.x, maxs.y, maxs.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(maxs.x, maxs.y, maxs.z), Vec3(mins.x, maxs.y, maxs.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(mins.x, maxs.y, maxs.z), Vec3(mins.x, mins.y, maxs.z), radius, color, UVs);

	AddVertsForCylinder3D(verts, Vec3(mins.x, mins.y, mins.z), Vec3(mins.x, mins.y, maxs.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(maxs.x, mins.y, mins.z), Vec3(maxs.x, mins.y, maxs.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(maxs.x, maxs.y, mins.z), Vec3(maxs.x, maxs.y, maxs.z), radius, color, UVs);
	AddVertsForCylinder3D(verts, Vec3(mins.x, maxs.y, mins.z), Vec3(mins.x, maxs.y, maxs.z), radius, color, UVs);
}

void AddVertsForSphere3DWireFrame(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numSlices, int numStacks)
{
	size_t originalSize = verts.size();

	float lineRadius = radius * 0.01f; 

	std::vector<Vec3> sphereVerts;
	sphereVerts.reserve(numSlices * (numStacks - 1) + 2);

	sphereVerts.push_back(Vec3(0.f, 0.f, -radius));

	float eachLongitude = 360.f / numSlices;
	float eachLatitude = 180.f / numStacks;
	for (int curStack = 0; curStack < numStacks - 1; curStack++)
	{
		float curLatitude = -90.f + (curStack + 1) * eachLatitude;
		for (int curSlice = 0; curSlice < numSlices; curSlice++)
		{
			float curLongitude = curSlice * eachLongitude;
			sphereVerts.push_back(Vec3::MakeFromPolarDegrees(curLatitude, curLongitude, radius));
		}
	}

	sphereVerts.push_back(Vec3(0.f, 0.f, radius));

	for (int curSlice = 0; curSlice < numSlices; curSlice++)
	{
		AddVertsForCylinder3D(verts, sphereVerts[0], sphereVerts[1 + curSlice], lineRadius, color, UVs, 8);
		for (int curStack = 0; curStack < numStacks - 2; curStack++)
		{
			int baseIndex = 1 + curStack * numSlices;
			int currentIndex = baseIndex + curSlice;
			int nextIndex = baseIndex + numSlices + curSlice;

			AddVertsForCylinder3D(verts, sphereVerts[currentIndex], sphereVerts[nextIndex], lineRadius, color, UVs, 8);
		}
		int lastRingStart = (int)sphereVerts.size() - 1 - numSlices;
		AddVertsForCylinder3D(verts, sphereVerts[lastRingStart + curSlice], sphereVerts[sphereVerts.size() - 1], lineRadius, color, UVs, 8);
	}

	for (int curStack = 0; curStack < numStacks - 1; curStack++)
	{
		int baseIndex = 1 + curStack * numSlices;

		for (int curSlice = 0; curSlice < numSlices; curSlice++)
		{
			int currentIndex = baseIndex + curSlice;
			int nextIndex = baseIndex + ((curSlice + 1) % numSlices);

			AddVertsForCylinder3D(verts, sphereVerts[currentIndex], sphereVerts[nextIndex], lineRadius, color, UVs, 8);
		}
	}

	for (size_t i = originalSize; i < verts.size(); i++)
	{
		verts[i].m_position += center;
	}
}

void AddVertsForCylinder3DWireFrame(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8 color, const AABB2& UVs, int numSlices)
{
	size_t originalSize = verts.size();
	float height = (end - start).GetLength();
	float lineRadius = radius * 0.02f; 

	Vec3 localStart(0.0f, 0.0f, 0.f);
	Vec3 localEnd(0.f, 0.f, height);

	std::vector<Vec3> bottomCircle;
	std::vector<Vec3> topCircle;

	for (int i = 0; i < numSlices; i++)
	{
		float angle = i * 360.f / numSlices;
		float x = radius * CosDegrees(angle);
		float y = radius * SinDegrees(angle);

		bottomCircle.push_back(Vec3(x, y, 0.f));
		topCircle.push_back(Vec3(x, y, height));
	}

	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1) % numSlices;
		AddVertsForCylinder3D(verts, bottomCircle[i], bottomCircle[nextI], lineRadius, color, UVs, 4);
	}

	for (int i = 0; i < numSlices; i++)
	{
		int nextI = (i + 1) % numSlices;
		AddVertsForCylinder3D(verts, topCircle[i], topCircle[nextI], lineRadius, color, UVs, 4);
	}

	for (int i = 0; i < numSlices; i++)
	{
		AddVertsForCylinder3D(verts, bottomCircle[i], topCircle[i], lineRadius, color, UVs, 4);
	}

	Mat44 rotateMat = Mat44::MakeYRotationDegrees(90.f);
	Mat44 lookAtMatrix = GetLookAtMatrix(start, end);
	lookAtMatrix.Append(rotateMat);
	for (size_t i = originalSize; i < verts.size(); i++)
	{
		verts[i].m_position = lookAtMatrix.TransformPosition3D(verts[i].m_position);
	}
}

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts)
{
	if (verts.empty()) 
	{
		return AABB2(); 
	}

	float minX = verts[0].m_position.x;
	float minY = verts[0].m_position.y;
	float maxX = verts[0].m_position.x;
	float maxY = verts[0].m_position.y;

	for (int i = 1; i < (int)verts.size(); i++)
	{
		const Vec2& pos = Vec2(verts[i].m_position.x, verts[i].m_position.y);

		if (pos.x < minX) minX = pos.x;
		if (pos.y < minY) minY = pos.y;

		if (pos.x > maxX) maxX = pos.x;
		if (pos.y > maxY) maxY = pos.y;
	}
	return AABB2(Vec2(minX, minY), Vec2(maxX, maxY));
}

void AddVertsForCubeSkyBox(std::vector<Vertex_PCU>& verts, const Vec3& center, float size, const Rgba8& color)
{
	float halfSize = size * 0.5f;
	Vec3 mins = center - Vec3(halfSize, halfSize, halfSize);
	Vec3 maxs = center + Vec3(halfSize, halfSize, halfSize);

	// +X  ft
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, mins.y, mins.z),
		Vec3(maxs.x, mins.y, maxs.z),
		Vec3(maxs.x, maxs.y, maxs.z),
		Vec3(maxs.x, maxs.y, mins.z),
		color, AABB2(Vec2(0, 0), Vec2(1, 1)));

	// -X bk
	AddVertsForQuad3D(verts,
		Vec3(mins.x, mins.y, mins.z),  
		Vec3(mins.x, maxs.y, mins.z),
		Vec3(mins.x, maxs.y, maxs.z),
		Vec3(mins.x, mins.y, maxs.z),
		color, AABB2(Vec2(0, 0), Vec2(1, 1)));

	// +Y
	AddVertsForQuad3D(verts,
		Vec3(mins.x, maxs.y, mins.z),  
		Vec3(maxs.x, maxs.y, mins.z),  
		Vec3(maxs.x, maxs.y, maxs.z),  
		Vec3(mins.x, maxs.y, maxs.z),  
		color, AABB2(Vec2(0, 0), Vec2(1, 1)));

	// -Y
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, mins.y, mins.z), 
		Vec3(mins.x, mins.y, mins.z), 
		Vec3(mins.x, mins.y, maxs.z), 
		Vec3(maxs.x, mins.y, maxs.z), 
		color, AABB2(Vec2(0, 0), Vec2(1, 1)));

	// +Z
	AddVertsForQuad3D(verts,
		Vec3(maxs.x, maxs.y, maxs.z),
		Vec3(maxs.x, mins.y, maxs.z),
		Vec3(mins.x, mins.y, maxs.z),
		Vec3(mins.x, maxs.y, maxs.z),
		color, AABB2(Vec2(0, 0), Vec2(1, 1)));

	AddVertsForQuad3D(verts,
		Vec3(maxs.x, mins.y, mins.z),
		Vec3(maxs.x, maxs.y, mins.z),
		Vec3(mins.x, maxs.y, mins.z),
		Vec3(mins.x, mins.y, mins.z),
		color, AABB2(Vec2(0, 0), Vec2(1, 1)));
}
