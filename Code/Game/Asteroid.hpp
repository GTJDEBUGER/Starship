#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class Asteroid : public Entity {
public:
	Asteroid(Game* game);
	~Asteroid() override = default;

	void             Update() override;
	void             Render() const override;
	void             Die() override;
	void             CheckCollide();

public:
	static const int m_vertexNum     = 48;
	float            m_flashFraction = 0.f;

private:
	void             BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale);
	void             BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor);
		             
private:             
	int              m_debrisNumMin       = 3;
	int              m_debrisNumMax       = 12;
	float            m_killSpeed          = 100.f;
	float            m_bounceShakeAmp     = 1.f;
		             				      
	float            m_randomScale        = 1.f;
	const float      m_randomScaleMin     = 1.f;
	const float      m_randomScaleMax     = 10.f;

	Rgba8            m_innerColor         = Rgba8(100,100,100,255);
	Rgba8            m_outerColor         = Rgba8(50,50,50,255);

	float            m_flashFractionDecay = 2.f;
};