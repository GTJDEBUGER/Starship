#pragma once

class App;
extern App* g_app;
class Game;
class Camera;

//-----------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();
	void RunFrame();
	bool IsQuitting();
	bool IsKeyDown(unsigned char keyCode) const;
	bool WasKeyJustPressed(unsigned char keyCode) const;

public:
	Game* m_game = nullptr;
	Camera* m_camera = nullptr;

	bool g_isQuitting = false;
	bool g_isSlowDown = false;
	bool g_isPause = false;
	bool pauseTrigger = true;
	bool g_singleStep = false;

private:
	bool* m_keyDownThisFrame[256];
	bool* m_keyDownLastFrame[256];

};