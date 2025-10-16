#pragma once
#include "Game/Entity.hpp"

class Asteroid;

//-----------------------------------------------------------------------------------------------
class ShockWave : public Entity {
public:
	ShockWave(Game* game, Vec2 position, float duration, float maxSpreadDistance, Rgba8 waveColor);
	~ShockWave() override = default;

	void             Update(float deltaSeconds) override;
	void             Render() const override;
	void             Die() override;

public:
	static const int m_vertexNum = 48;

private:
	float            m_duration;
	float            m_lifeTime;
	float            m_spreadDistance;
	Rgba8            m_waveColor;
};