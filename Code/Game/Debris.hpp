#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class Debris : public Entity {
public:
	Debris(Game* game, Vec2 burstPosition, Vec2 burstDirection, 
			float burstSpeed, Rgba8 color, float meshScale);
	~Debris() override = default;

	void             Update() override;
	void             Render() const override;
	void             Die() override;
				     
	static void      GetLocalMesh(int vertexNum, Vertex* mesh, Rgba8 color);
				     
public:			     
	float            m_rotationSpeed           = 0;

private:
	void             SetColorDuringLifeTime();

private:
	float            m_lifeTime                = 0.f;
	float            m_lifeTimeCur             = 0.f;
	float            m_meshScale               = 1.f;
	static const int m_vertexNum               = MAX_DEBRIS_SUBDIVISION * 3;
};