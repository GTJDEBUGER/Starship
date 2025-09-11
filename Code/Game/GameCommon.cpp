#include "GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"

//-----------------------------------------------------------------------------------------------
void DebugDrawLine(Vec2 startPosition, Vec2 endPosition, Rgba8 lineCOlor, float thickness)
{
	Vec2 leftDir = (endPosition - startPosition).GetRotatedBy90Degrees().GetNormalized();

	Vertex topLeft(startPosition.GetVec3() + (leftDir * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));
	Vertex downLeft(startPosition.GetVec3() - (leftDir * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));
	Vertex topRight(endPosition.GetVec3() + (leftDir * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));
	Vertex downRight(endPosition.GetVec3() - (leftDir * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));

	Vertex tempMesh[6];
	tempMesh[0] = topLeft;
	tempMesh[1] = topRight;
	tempMesh[2] = downLeft;
	tempMesh[3] = downLeft;
	tempMesh[4] = topRight;
	tempMesh[5] = downRight;

	g_engine->m_renderer->DrawVertexArray(6, tempMesh);
}

//-----------------------------------------------------------------------------------------------
void DebugDrawRing(Vec2 centre, float radius, Rgba8 ringColor, float thickness)
{
	float angleIncrement = 360.f / static_cast<float>(DEBUG_DRAWRING_SUBDIVISION); 
	Vec2 vertexPos[DEBUG_DRAWRING_SUBDIVISION];
	for (int i = 0; i < DEBUG_DRAWRING_SUBDIVISION; i++) {
		float angleDegrees = i * angleIncrement;
		float x = centre.x + radius * CosDegrees(angleDegrees);
		float y = centre.y + radius * SinDegrees(angleDegrees);
		vertexPos[i] = Vec2(x, y);
		if (i > 0) {
			DebugDrawLine(vertexPos[i], vertexPos[i - 1], ringColor, thickness);
		}
	}
	DebugDrawLine(vertexPos[0], vertexPos[DEBUG_DRAWRING_SUBDIVISION-1], ringColor, thickness);
}
