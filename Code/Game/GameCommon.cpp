#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"

//-----------------------------------------------------------------------------------------------
void DrawLine(Vec2 startPosition, Vec2 endPosition, Rgba8 lineCOlor, float thickness)
{
	Vec2 forwardDir = (endPosition - startPosition).GetNormalized();
	Vec2 leftDir = forwardDir.GetRotatedBy90Degrees().GetNormalized();

	Vertex topLeft(startPosition.GetVec3() + ((leftDir-forwardDir) * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));
	Vertex downLeft(startPosition.GetVec3() - ((leftDir + forwardDir) * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));
	Vertex topRight(endPosition.GetVec3() + ((leftDir + forwardDir) * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));
	Vertex downRight(endPosition.GetVec3() - ((leftDir - forwardDir) * (thickness / 2.f)).GetVec3(), lineCOlor, Vec2(0, 0));

	Vertex tempMesh[6];
	tempMesh[0] = topLeft;
	tempMesh[1] = topRight;
	tempMesh[2] = downLeft;
	tempMesh[3] = downLeft;
	tempMesh[4] = topRight;
	tempMesh[5] = downRight;

	g_engine->m_renderer->CopyCPUToGPU(
		tempMesh,
		sizeof(tempMesh),
		g_engine->m_renderer->m_immediateUnlitVBO
	);
	g_engine->m_renderer->BindVertexBuffer(g_engine->m_renderer->m_immediateUnlitVBO);
	g_engine->m_renderer->DrawVertexBuffer(g_engine->m_renderer->m_immediateUnlitVBO, sizeof(tempMesh) / sizeof(Vertex));
}

//-----------------------------------------------------------------------------------------------
void DrawRing(Vec2 centre, float radius, Rgba8 ringColor, float thickness)
{
	float angleIncrement = 360.f / (float) DEBUG_DRAWRING_SUBDIVISION; 
	Vec2 vertexPos[DEBUG_DRAWRING_SUBDIVISION];
	for (int i = 0; i < DEBUG_DRAWRING_SUBDIVISION; i++) {
		float angleDegrees = i * angleIncrement;
		float x = centre.x + radius * CosDegrees(angleDegrees);
		float y = centre.y + radius * SinDegrees(angleDegrees);
		vertexPos[i] = Vec2(x, y);
		if (i > 0) {
			DrawLine(vertexPos[i], vertexPos[i - 1], ringColor, thickness);
		}
	}
	DrawLine(vertexPos[0], vertexPos[DEBUG_DRAWRING_SUBDIVISION-1], ringColor, thickness);
}

//-----------------------------------------------------------------------------------------------
void DrawDisc(Vec2 centre, float radius, Rgba8 outerColor, Rgba8 innerColor) {
	Vertex tempMesh[DEBUG_DRAWRING_SUBDIVISION*3];
	for (int i = 0; i < (DEBUG_DRAWRING_SUBDIVISION-1)*3; i += 3) {
		tempMesh[i] = Vertex(
			Vec3(centre.x+radius * CosDegrees(((float)i/3.f + 1.f)  * 360.f/ (float)DEBUG_DRAWRING_SUBDIVISION),
				 centre.y+radius * SinDegrees(((float)i/3.f + 1.f)  * 360.f/ (float)DEBUG_DRAWRING_SUBDIVISION),
				 0.f),
			outerColor,
			Vec2(0.f, 0.f)
		);
		tempMesh[i + 1] = Vertex(
			Vec3(centre.x+radius * CosDegrees((float)i/3.f * 360.f / (float)DEBUG_DRAWRING_SUBDIVISION),
			     centre.y+radius * SinDegrees((float)i/3.f * 360.f / (float)DEBUG_DRAWRING_SUBDIVISION),
			     0.f),
			outerColor,
			Vec2(0.f, 0.f)
		);
		tempMesh[i + 2] = Vertex(
			Vec3(centre.x, centre.y, 0.f),
			innerColor,
			Vec2(0.f, 0.f)
		);
	}

	
	tempMesh[DEBUG_DRAWRING_SUBDIVISION * 3 - 3] = Vertex(
		Vec3(centre.x+radius, centre.y, 0.f),
		outerColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[DEBUG_DRAWRING_SUBDIVISION * 3 - 2] = Vertex(
		Vec3(centre.x+radius * CosDegrees(((float)DEBUG_DRAWRING_SUBDIVISION - 1.f) / (float)DEBUG_DRAWRING_SUBDIVISION * 360.f),
		     centre.y+radius * SinDegrees(((float)DEBUG_DRAWRING_SUBDIVISION - 1.f) / (float)DEBUG_DRAWRING_SUBDIVISION * 360.f),
		     0.f),
		outerColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[DEBUG_DRAWRING_SUBDIVISION*3 - 1] = Vertex(
		Vec3(centre.x, centre.y, 0.f),
		innerColor,
		Vec2(0.f, 0.f)
	);

	g_engine->m_renderer->CopyCPUToGPU(
		tempMesh,
		sizeof(tempMesh),
		g_engine->m_renderer->m_immediateUnlitVBO
	);
	g_engine->m_renderer->BindVertexBuffer(g_engine->m_renderer->m_immediateUnlitVBO);
	g_engine->m_renderer->DrawVertexBuffer(g_engine->m_renderer->m_immediateUnlitVBO, sizeof(tempMesh) / sizeof(Vertex));
}

//-----------------------------------------------------------------------------------------------
void DrawAABB(Vec2 buttomLeft, Vec2 topRight, Rgba8 leftColor, Rgba8 rightColor, Rgba8 centerColor) {
	Vertex tempMesh[12];
	Vec3 centerV3 = ((buttomLeft + topRight) * 0.5f).GetVec3();
	Vec3 buttomLeftV3 = buttomLeft.GetVec3();
	Vec3 topLeftV3 = Vec3(buttomLeft.x, topRight.y, 0.f);
	Vec3 topRightV3 = topRight.GetVec3();
	Vec3 buttomRightV3 = Vec3(topRight.x, buttomLeft.y, 0.f);

	tempMesh[0] = Vertex(
		buttomLeftV3,
		leftColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[1] = Vertex(
		topLeftV3,
		leftColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[2] = Vertex(
		centerV3,
		centerColor,
		Vec2(0.f, 0.f)
	);

	tempMesh[3] = Vertex(
		topLeftV3,
		leftColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[4] = Vertex(
		topRightV3,
		rightColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[5] = Vertex(
		centerV3,
		centerColor,
		Vec2(0.f, 0.f)
	);

	tempMesh[6] = Vertex(
		topRightV3,
		rightColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[7] = Vertex(
		buttomRightV3,
		rightColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[8] = Vertex(
		centerV3,
		centerColor,
		Vec2(0.f, 0.f)
	);

	tempMesh[9] = Vertex(
		buttomRightV3,
		rightColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[10] = Vertex(
		buttomLeftV3,
		leftColor,
		Vec2(0.f, 0.f)
	);
	tempMesh[11] = Vertex(
		centerV3,
		centerColor,
		Vec2(0.f, 0.f)
	);
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray(12, tempMesh);
}

//-----------------------------------------------------------------------------------------------
void TransformVertexArrayShine(int numVerts, Vertex* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY, float time) {
	for (int i = 0; i < numVerts; i++) {
		float fraction = GetClamped(SinDegrees(180.f * time - verts[i].m_position.x + verts[i].m_position.y),0.f,1.f)
						+ GetClamped(SinDegrees(180.f * time - 140.f - verts[i].m_position.x + verts[i].m_position.y), 0.f, 1.f);
		verts[i].m_color = Rgba8(255, 255, 255, (unsigned char)(255.f * (fraction > 0.98f ? 1.f : 0.f)));
		TransformPositionXY3D(verts[i].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY);
	}
}

//-----------------------------------------------------------------------------------------------
void TransformVertexArrayScaleNoneUniform(int numVerts, Vertex* verts, Vec2 scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY) {
	Vec2 iBias = Vec2::MakeFromPolarDegrees(rotationDegreesAboutZ, scaleXY.x);
	Vec2 jBias = Vec2::MakeFromPolarDegrees(rotationDegreesAboutZ + 90.0f, scaleXY.y);

	for (int i = 0; i < numVerts; i++) {
		TransformPositionXY3D(verts[i].m_position, iBias, jBias, translationXY);
	}
}
