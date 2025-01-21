#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Vertex_PCU.hpp"
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

//void AddVertsForLinSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
//{
//}
