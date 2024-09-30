#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, 
    float rotationDegreesAboutZ, Vec2 const& translationXY) 
{
    // Convert rotation angle from degrees to radians
    //float radians = DegreesToRadians(rotationDegreesAboutZ);
    //float cosTheta = std::cos(radians);
    //float sinTheta = std::sin(radians);

    for (int i = 0; i < numVerts; ++i)
    {
        TransformPositionXY3D(verts[i].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY);
        //// Access the vertex position
        //Vec2& pos = verts[i].m_position;

        //// Apply scaling
        //pos.x *= uniformScaleXY;
        //pos.y *= uniformScaleXY;

        //// Apply rotation
        //float xNew = cosTheta * pos.x - sinTheta * pos.y;
        //float yNew = sinTheta * pos.x + cosTheta * pos.y;
        //pos.x = xNew;
        //pos.y = yNew;

        //// Apply translation
        //pos.x += translationXY.x;
        //pos.y += translationXY.y;
    }
}