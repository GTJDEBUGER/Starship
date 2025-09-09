#define WIN32_LEAN_AND_MEAN		
#include <windows.h>
#include <gl/gl.h>
#pragma comment( lib, "opengl32" )

#include "GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
void DebugDrawLine(Vec2 startPosition, Vec2 endPosition, Rgba8 lineCOlor)
{
	glBegin(GL_LINES);
	glColor4ub(lineCOlor.r, lineCOlor.g, lineCOlor.b, lineCOlor.a);
	glVertex2f(startPosition.x, startPosition.y);
	glVertex2f(endPosition.x, endPosition.y);
	glEnd();
}

//-----------------------------------------------------------------------------------------------
void DebugDrawRing(Vec2 centre, float radius, Rgba8 circleColor)
{
	const int NUM_SEGMENTS = 32;
	const float ANGLE_INCREMENT = 360.f / (float)NUM_SEGMENTS;
	glBegin(GL_LINE_LOOP);
	glColor4ub(circleColor.r, circleColor.g, circleColor.b, circleColor.a);
	for (int i = 0; i < NUM_SEGMENTS; i++) {
		float angleDegrees = i * ANGLE_INCREMENT;
		float x = centre.x + radius * CosDegrees(angleDegrees);
		float y = centre.y + radius * SinDegrees(angleDegrees);
		glVertex2f(x, y);
	}
	glEnd();
}
