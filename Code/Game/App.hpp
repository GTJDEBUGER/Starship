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
	bool IsKeyDownThisFrame(unsigned char keycode);
	bool IsKeyHeldThisFrame(unsigned char keyCode);
	bool IsKeyUpThisFrame(unsigned char keycode);

public:
	Game* m_game = nullptr;
	Camera* m_camera = nullptr;

	bool m_isQuitting = false;
	bool m_isSlowDown = false;
	bool m_isPause = false;
	bool m_isShutdown = false;
	bool m_singleStep = false;
	bool m_isFiring = false;
	bool m_isDebugDraw = false;
	bool m_isPlayerRespawn = false;
	bool m_isAsteroidRespawn = false;

private:
	void HandlePlayerInput();

private:
	bool m_keyDownThisFrame[256];
	bool m_keyDownLastFrame[256];

};