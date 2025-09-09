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
	bool IsQuitting();
	void Shutdown();
	void OnKeyDown(unsigned char keyCode);
	void OnKeyUp(unsigned char keyCode);

public:
	Game* m_game = nullptr;
	Camera* m_camera = nullptr;

	bool m_isQuitting = false;
	bool m_isSlowDown = false;
	bool m_isPause = false;
	bool m_singleStep = false;
	bool m_isFiring = false;
	bool m_isDebugDraw = false;
	bool m_isPlayerRespawn = false;
	bool m_isAsteroidRespawn = false;

	bool pauseTrigger = true;
	bool firingTrigger = true;
	bool debugDrawTrigger = true;
	bool playerRespawTrigger = true;
	bool asteroidRespawTrigger = true;

private:
	bool m_keyDownThisFrame[256];
	bool m_keyUpThisFrame[256];

};