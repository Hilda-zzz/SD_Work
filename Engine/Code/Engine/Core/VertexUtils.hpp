#pragma once
#include "Vertex_PCU.hpp"

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY);