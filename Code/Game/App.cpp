#include "Engine/Core/Engine.hpp"
#include "Engine//Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"

App* g_app = nullptr;

//-----------------------------------------------------------------------------------------------
App::App()
{
	g_engine = new Engine();
	m_game = new Game();
	m_gameCamera = new Camera(Vec2(0.f,0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_UICamera = new Camera(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	for (int i = 0; i < 256; i++) {
		m_keyDownLastFrame[i] = false;
		m_keyDownThisFrame[i] = false;
	}
}

//-----------------------------------------------------------------------------------------------
App::~App()
{
	delete m_game;
	m_game = nullptr;
	delete g_engine;
	g_engine = nullptr;
}

//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	//-------------------------------------------------------------------------------------------
	HandlePlayerInput();
	if (m_isShutdown) {
		m_isShutdown = false;
		return;
	}

	//-------------------------------------------------------------------------------------------
	g_engine->BeginFrame();	
		Update();
		Render();
	g_engine->EndFrame();

}

//-----------------------------------------------------------------------------------------------
void App::Update() {
	//-------------------------------------------------------------------------------------------
	float deltaSeconds = 1.f / 60.f;

	float originalDeltaSeconds = deltaSeconds;
	if (m_isSlowDown) {
		deltaSeconds *= 0.1f;
		originalDeltaSeconds = deltaSeconds;
	}
	if (m_isPause) {
		deltaSeconds = 0.f;
	}
	if (m_singleStep) {
		m_singleStep = false;
		deltaSeconds = originalDeltaSeconds;

		m_isPause = true;
	}

	//-------------------------------------------------------------------------------------------
	if (!m_isAttractMode) {
		m_game->Update(deltaSeconds);
	}
}

//-----------------------------------------------------------------------------------------------
void App::Render() {
	if (!m_isAttractMode) {
		m_game->Render();
	}
	else {
		RenderAttractMode();
	}
}

//-----------------------------------------------------------------------------------------------
bool App::IsQuitting() {
	return m_isQuitting;
}

//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	delete g_app;
	g_app = new App();
}

//-----------------------------------------------------------------------------------------------
void App::OnKeyDown(unsigned char keyCode)
{
	m_keyDownThisFrame[keyCode] = true;
}

//-----------------------------------------------------------------------------------------------
void App::OnKeyUp(unsigned char keyCode)
{
	m_keyDownThisFrame[keyCode] = false;
}

//-----------------------------------------------------------------------------------------------
void App::HandlePlayerInput(){
	if (IsKeyDownThisFrame(119)) { //F8
		Shutdown();
		m_isShutdown = true;
		return;
	}

	if (IsKeyHeldThisFrame('T') || IsKeyDownThisFrame('T')) {
		m_isSlowDown = true;
	}

	if (IsKeyDownThisFrame('P')) {
		m_isPause = !m_isPause;
	}

	if (IsKeyDownThisFrame('O')) {
		m_singleStep = true;
	}

	if (IsKeyDownThisFrame(32)) { //SPACE
		if (!m_isAttractMode) {
			m_isFiring = true;
		}
		else {
			m_isAttractMode = false;
			delete m_game;
			m_game = nullptr;
			m_game = new Game();
		}
	}

	if (IsKeyDownThisFrame(112)) {
		m_isDebugDraw = !m_isDebugDraw;
	}

	if (IsKeyDownThisFrame('N')) {
		m_isPlayerRespawn = true;
	}

	if (IsKeyDownThisFrame('I')) {
		m_isAsteroidRespawn = true;
	}

	if (IsKeyDownThisFrame(27)) { // ESC
		if (m_isAttractMode) {
			m_isQuitting = true;
		}
		else {
			m_isAttractMode = true;
		}
	}

	if (IsKeyHeldThisFrame('E') || IsKeyDownThisFrame('E')) {
		m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION;
	}

	if ((IsKeyHeldThisFrame('F') || IsKeyDownThisFrame('F')) && !(IsKeyHeldThisFrame('S') || IsKeyDownThisFrame('S'))) {
		m_game->m_player->m_rotationSpeed = -PLAYER_SHIP_TURN_SPEED;
	}

	if ((IsKeyHeldThisFrame('S') || IsKeyDownThisFrame('S')) && !(IsKeyHeldThisFrame('F') || IsKeyDownThisFrame('F'))) {
		m_game->m_player->m_rotationSpeed = PLAYER_SHIP_TURN_SPEED;
	}


	if (IsKeyUpThisFrame('T')) {
		m_isSlowDown = false;
	}

	if (IsKeyUpThisFrame('E')) {
		m_game->m_player->m_acceleration = 0;
	}

	if ((IsKeyUpThisFrame('S') || IsKeyUpThisFrame('F')) || (IsKeyHeldThisFrame('F') && IsKeyHeldThisFrame('S'))) {
		m_game->m_player->m_rotationSpeed = 0;
	}

	//------------------------------------------------------------------------------------------
	for (int i = 0; i < 256; i++) {
		m_keyDownLastFrame[i] = m_keyDownThisFrame[i];
	}
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyDownThisFrame(unsigned char keyCode) {
	return m_keyDownThisFrame[keyCode] && !m_keyDownLastFrame[keyCode];
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyHeldThisFrame(unsigned char keyCode) {
	return m_keyDownThisFrame[keyCode] && m_keyDownLastFrame[keyCode];
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyUpThisFrame(unsigned char keyCode) {
	return !m_keyDownThisFrame[keyCode] && m_keyDownLastFrame[keyCode];
}

//-----------------------------------------------------------------------------------------------
void App::RenderAttractMode() const {
	g_engine->m_renderer->BeginCamera(*g_app->m_UICamera);

	Vertex worldMesh[WaspEnemy::m_vertexNum];

	PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, worldMesh);
	TransformVertexArrayXY3D(PlayerShip::m_vertexNum, worldMesh, 5.f, 0.f, Vec2(WORLD_CENTER_X - 20*PLAYER_SHIP_COSMETIC_RADIUS , WORLD_CENTER_Y));
	g_engine->m_renderer->DrawVertexArray(15, worldMesh);

	BeetleEnemy::GetLocalMesh(BeetleEnemy::m_vertexNum, worldMesh);
	TransformVertexArrayXY3D(BeetleEnemy::m_vertexNum-6, worldMesh, 5.f, 200.f, 
		Vec2(WORLD_CENTER_X + 20*PLAYER_SHIP_COSMETIC_RADIUS, WORLD_CENTER_Y + 10 * PLAYER_SHIP_COSMETIC_RADIUS));
	g_engine->m_renderer->DrawVertexArray(BeetleEnemy::m_vertexNum - 6, worldMesh);

	WaspEnemy::GetLocalMesh(WaspEnemy::m_vertexNum, worldMesh);
	TransformVertexArrayXY3D(WaspEnemy::m_vertexNum - 12, worldMesh, 5.f, 160.f,
		Vec2(WORLD_CENTER_X + 20 * PLAYER_SHIP_COSMETIC_RADIUS, WORLD_CENTER_Y - 10 * PLAYER_SHIP_COSMETIC_RADIUS));
	g_engine->m_renderer->DrawVertexArray(WaspEnemy::m_vertexNum - 12, worldMesh);

	Vertex startButtonMesh[3];
	startButtonMesh[0] = Vertex(Vec3(-1.f, 1.7f, 0.f), Rgba8(0, 255, 0, 255), Vec2(0, 0));
	startButtonMesh[1] = Vertex(Vec3(-1.f, -1.7f, 0.f), Rgba8(0, 255, 0, 255), Vec2(0, 0));
	startButtonMesh[2] = Vertex(Vec3(2.f, 0.f, 0.f), Rgba8(0, 255, 0, 255), Vec2(0, 0));
	TransformVertexArrayXY3D(3, startButtonMesh, 5.f, 0.f, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
	g_engine->m_renderer->DrawVertexArray(3, startButtonMesh);

	g_engine->m_renderer->EndCamera(*g_app->m_UICamera);
}