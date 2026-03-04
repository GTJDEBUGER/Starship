#pragma once
#include "Engine/Core/EventSystem.hpp"

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
	void RunMainLoop();
	void RunFrame();
	void Update();
	void Render();
	bool IsQuitting();
	void Shutdown();
	static bool HandleQuit(EventArgs& args);
	static bool HandleSetTimeScale(EventArgs& args);

public:
	Game* m_game                        = nullptr;
					                    
	bool m_isQuitting                   = false;
	bool m_isShutdown                   = false;
	bool m_isFiring                     = false;
	bool m_isDebugDraw                  = false;
	bool m_isPlayerRespawn              = false;
	bool m_isAsteroidRespawn            = false;
	size_t m_attractSoundPlaybackID;
	size_t m_gameSoundPlaybackID;
	size_t m_accelerateSoundPlaybackID;

	Camera* m_screenCamera              = nullptr;
	float m_gameTimeScale               = 1.f;

private:
	void HandlePlayerInput();
	void RenderAttractMode() const;
};