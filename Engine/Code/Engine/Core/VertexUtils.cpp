#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, 
    float rotationDegreesAboutZ, Vec2 const& translationXY) 
{
    for (int i = 0; i < numVerts; ++i)
    {
        TransformPositionXY3D(verts[i].m_position, uniformScaleXY, 
            rotationDegreesAboutZ, translationXY);
    }
}