#pragma once

class App;
class Game;
class Camera;

extern App* g_app;

//-----------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();
	void RunFrame();
	void Update();
	void Render();
	bool IsQuitting();
	void Shutdown();

public:
	Game* m_game = nullptr;
	Camera* m_gameCamera = nullptr;
	Camera* m_UICamera = nullptr;

	bool m_isQuitting = false;
	bool m_isSlowDown = false;
	bool m_isPause = false;
	bool m_isShutdown = false;
	bool m_singleStep = false;
	bool m_isFiring = false;
	bool m_isDebugDraw = false;
	bool m_isPlayerRespawn = false;
	bool m_isAsteroidRespawn = false;
	bool m_isAttractMode = true;

private:
	void HandlePlayerInput();
	void RenderAttractMode() const;

private:
	float m_startButtonAnimationTotalTime = 1.f;
	float m_startButtonAnimationTimeCount = 0.;
	float m_lastFrameTime = 0.f;
};