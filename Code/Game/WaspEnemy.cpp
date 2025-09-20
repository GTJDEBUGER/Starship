#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Game/WaspEnemy.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Playership.hpp"
#include "Game/Bullet.hpp"
#include "Game/Debris.hpp"
#include "Game/Game.hpp"

//-----------------------------------------------------------------------------------------------
WaspEnemy::WaspEnemy(Game* game)
	: Entity(game, Vec2(0, 0))
{
	m_physicsRadius = ENEMY_WASP_PHYSICS_RADIUS * m_meshScale;
	m_cosmeticRadius = ENEMY_WASP_COSMETIC_RADIUS * m_meshScale;
	m_curFrame = 0;
	m_maxFrame = 16;
	m_health = 3;
	SetPositionRandomOffscreen();

	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh);
}

//-----------------------------------------------------------------------------------------------
void WaspEnemy::Update(float deltaSeconds)
{
	//--------------------------------------------------------------------------------
	if (m_health <= 0) {
		Die();
		return;
	}
	//--------------------------------------------------------------------------------
	CollideTest();
	//--------------------------------------------------------------------------------
	m_curFrame = (m_curFrame + 1) % m_maxFrame;
	//--------------------------------------------------------------------------------
	if (!m_game->m_player->m_isDead) {
		Vec2 directionToPlayer = (m_game->m_player->m_position - m_position).GetNormalized();
		m_orientationDegrees = directionToPlayer.GetOrientationDegrees();

		m_velocity += ENEMY_WASP_ACCELERATION * directionToPlayer * deltaSeconds;
	}

	if (m_velocity.GetLengthSquared() > ENEMY_WASP_MAX_SPEED * ENEMY_WASP_MAX_SPEED) {
		m_velocity = m_velocity.GetNormalized() * ENEMY_WASP_MAX_SPEED;
	}
	m_position += m_velocity * deltaSeconds;

}

//-----------------------------------------------------------------------------------------------
void WaspEnemy::Render() const
{
	Vertex m_worldMesh[m_vertexNum];
	for (int i = 0; i < m_vertexNum - 12; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
		m_worldMesh[i].m_position *= m_meshScale;
	}
	
	float frameFraction = static_cast<float>(m_curFrame) / static_cast<float>(m_maxFrame);

	if (m_curFrame <= m_maxFrame / 2) {
		for (int i = m_vertexNum - 24; i < m_vertexNum - 12; i++) {
			m_worldMesh[i] = Interpolate(m_localMesh[i], m_localMesh[i + 12], frameFraction);
			m_worldMesh[i].m_position *= m_meshScale;
		}
	}
	else {
		for (int i = m_vertexNum - 24; i < m_vertexNum - 12; i++) {
			m_worldMesh[i] = Interpolate(m_localMesh[i+12], m_localMesh[i], (frameFraction-0.5f)*2.f);
			m_worldMesh[i].m_position *= m_meshScale;
		}
	}

	TransformVertexArrayXY3D(m_vertexNum-12, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(m_vertexNum-12, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void WaspEnemy::Die()
{
	if (!m_isDead) {
		BurstDebris(m_debrisNumMin, m_debrisNumMax, -GetForwardVector(), 180.f, Rgba8(255, 222, 0, 255), 1.f);
		BurstDebris(m_debrisNumMin, m_debrisNumMax, -GetForwardVector(), 90.f, Rgba8(255, 0, 0, 255), 0.5f);
		m_isDead = true;
	}
}

//-----------------------------------------------------------------------------------------------
void WaspEnemy::GetLocalMesh(int vertexNum, Vertex* mesh) {
	GUARANTEE_OR_DIE(vertexNum == m_vertexNum, "The array you provided can not save wasps mesh!");

	// Define the Wasp mesh (in local space)
	Rgba8 carapaceColor_1 = Rgba8(255, 222, 0, 255);
	Rgba8 carapaceColor_2 = Rgba8(80, 65, 0, 255);
	Rgba8 wingColor = Rgba8(233, 233, 233, 128);
	Rgba8 eyeColor = Rgba8(255, 0, 0, 255);
	Rgba8 needleColor = Rgba8(255, 255, 255, 255);
	// Head
	mesh[0] = Vertex(Vec3(1.5f, 0.f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[1] = Vertex(Vec3(0.9f, 0.36f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[2] = Vertex(Vec3(1.16f, 0.34f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[3] = Vertex(Vec3(1.5f, 0.f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[4] = Vertex(Vec3(0.79f, 0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[5] = Vertex(Vec3(0.9f, 0.36f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[6] = Vertex(Vec3(1.16f, 0.34f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[7] = Vertex(Vec3(1.5f, 0.6f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[8] = Vertex(Vec3(1.3f, 0.2f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[9] = Vertex(Vec3(1.46f, 0.52f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[10] = Vertex(Vec3(1.5f, 0.6f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[11] = Vertex(Vec3(1.93f, 0.5f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[12] = Vertex(Vec3(1.16f, 0.34f, 0.f), eyeColor, Vec2(0.f, 0.f));
	mesh[13] = Vertex(Vec3(0.9f, 0.36f, 0.f), eyeColor, Vec2(0.f, 0.f));
	mesh[14] = Vertex(Vec3(1.f, 0.5f, 0.f), eyeColor, Vec2(0.f, 0.f));

	mesh[15] = Vertex(Vec3(1.5f, 0.f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[16] = Vertex(Vec3(0.9f, -0.36f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[17] = Vertex(Vec3(1.16f, -0.34f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[18] = Vertex(Vec3(1.5f, 0.f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[19] = Vertex(Vec3(0.79f, -0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[20] = Vertex(Vec3(0.9f, -0.36f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[21] = Vertex(Vec3(1.16f, -0.34f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[22] = Vertex(Vec3(1.5f, -0.6f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[23] = Vertex(Vec3(1.3f, -0.2f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[24] = Vertex(Vec3(1.46f, -0.52f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[25] = Vertex(Vec3(1.5f, -0.6f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[26] = Vertex(Vec3(1.93f, -0.5f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[27] = Vertex(Vec3(1.16f, -0.34f, 0.f), eyeColor, Vec2(0.f, 0.f));
	mesh[28] = Vertex(Vec3(0.9f, -0.36f, 0.f), eyeColor, Vec2(0.f, 0.f));
	mesh[29] = Vertex(Vec3(1.f, -0.5f, 0.f), eyeColor, Vec2(0.f, 0.f));

	mesh[30] = Vertex(Vec3(0.79f, 0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[31] = Vertex(Vec3(1.5f, 0.f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[32] = Vertex(Vec3(0.79f, -0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	//Chest
	mesh[33] = Vertex(Vec3(0.5f, 0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[34] = Vertex(Vec3(0.79f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[35] = Vertex(Vec3(0.3f, 0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[36] = Vertex(Vec3(0.3f, 0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[37] = Vertex(Vec3(0.79f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[38] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[39] = Vertex(Vec3(0.5f, -0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[40] = Vertex(Vec3(0.79f, -0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[41] = Vertex(Vec3(0.3f, -0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[42] = Vertex(Vec3(0.3f, -0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[43] = Vertex(Vec3(0.79f, -0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[44] = Vertex(Vec3(0.2f, -0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[45] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[46] = Vertex(Vec3(0.79f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[47] = Vertex(Vec3(0.8f, -0.2f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[48] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[49] = Vertex(Vec3(0.8f, -0.2f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[50] = Vertex(Vec3(0.2f, -0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	// Legs
	mesh[51] = Vertex(Vec3(0.5f, 0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[52] = Vertex(Vec3(0.7f, 0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[53] = Vertex(Vec3(0.7f, 0.27f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[54] = Vertex(Vec3(0.7f, 0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[55] = Vertex(Vec3(1.2f, 1.f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[56] = Vertex(Vec3(0.7f, 0.67f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	//
	mesh[57] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[58] = Vertex(Vec3(0.31f, 0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[59] = Vertex(Vec3(0.3f, 0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[60] = Vertex(Vec3(0.31f, 0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[61] = Vertex(Vec3(0.53f, 1.23f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[62] = Vertex(Vec3(0.3f, 0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[63] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[64] = Vertex(Vec3(0.02f, 0.29f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[65] = Vertex(Vec3(0.08f, 0.64f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[66] = Vertex(Vec3(0.08f, 0.64f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[67] = Vertex(Vec3(0.06f, 0.51f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[68] = Vertex(Vec3(-0.97f, 0.88f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[69] = Vertex(Vec3(0.5f, -0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[70] = Vertex(Vec3(0.7f, -0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[71] = Vertex(Vec3(0.7f, -0.27f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[72] = Vertex(Vec3(0.7f, -0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[73] = Vertex(Vec3(1.2f, -1.f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[74] = Vertex(Vec3(0.7f, -0.67f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	//
	mesh[75] = Vertex(Vec3(0.2f, -0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[76] = Vertex(Vec3(0.31f, -0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[77] = Vertex(Vec3(0.3f, -0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[78] = Vertex(Vec3(0.31f, -0.9f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[79] = Vertex(Vec3(0.53f, -1.23f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[80] = Vertex(Vec3(0.3f, -0.4f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[81] = Vertex(Vec3(0.2f, -0.21f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[82] = Vertex(Vec3(0.02f, -0.29f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[83] = Vertex(Vec3(0.08f, -0.64f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	mesh[84] = Vertex(Vec3(0.08f, -0.64f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[85] = Vertex(Vec3(0.06f, -0.51f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));
	mesh[86] = Vertex(Vec3(-0.97f, -0.88f, 0.f), carapaceColor_2, Vec2(0.f, 0.f));

	//Tail
	mesh[87] = Vertex(Vec3(-0.5f, 0.5f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[88] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[89] = Vertex(Vec3(-1.49f, 0.17f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[90] = Vertex(Vec3(-0.5f, -0.5f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[91] = Vertex(Vec3(0.2f, -0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[92] = Vertex(Vec3(-1.49f, -0.17f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[93] = Vertex(Vec3(-1.49f, 0.17f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[94] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[95] = Vertex(Vec3(-1.49f, -0.17f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[96] = Vertex(Vec3(-1.49f, -0.17f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[97] = Vertex(Vec3(0.2f, 0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));
	mesh[98] = Vertex(Vec3(0.2f, -0.21f, 0.f), carapaceColor_1, Vec2(0.f, 0.f));

	mesh[99] = Vertex(Vec3(-1.49f, 0.17f, 0.f), needleColor, Vec2(0.f, 0.f));
	mesh[100] = Vertex(Vec3(-1.49f, -0.17f, 0.f), needleColor, Vec2(0.f, 0.f));
	mesh[101] = Vertex(Vec3(-2.f, 0.f, 0.f), needleColor, Vec2(0.f, 0.f));

	//Wings Frame 1
	mesh[102] = Vertex(Vec3(0.79f, 0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[103] = Vertex(Vec3(0.32f, 1.02f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[104] = Vertex(Vec3(0.5f, 1.94f, 0.f), wingColor, Vec2(0.f, 0.f));

	mesh[105] = Vertex(Vec3(0.79f, 0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[106] = Vertex(Vec3(-0.3f, 0.89f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[107] = Vertex(Vec3(-0.5f, 1.94f, 0.f), wingColor, Vec2(0.f, 0.f));

	mesh[108] = Vertex(Vec3(0.79f, -0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[109] = Vertex(Vec3(0.32f, -1.02f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[110] = Vertex(Vec3(0.5f, -1.94f, 0.f), wingColor, Vec2(0.f, 0.f));

	mesh[111] = Vertex(Vec3(0.79f, -0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[112] = Vertex(Vec3(-0.3f, -0.89f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[113] = Vertex(Vec3(-0.5f, -1.94f, 0.f), wingColor, Vec2(0.f, 0.f));

	//Wings Frame 2
	mesh[114] = Vertex(Vec3(0.79f, 0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[115] = Vertex(Vec3(-0.58f, 0.44f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[116] = Vertex(Vec3(-1.25f, 1.56f, 0.f), wingColor, Vec2(0.f, 0.f));

	mesh[117] = Vertex(Vec3(0.79f, 0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[118] = Vertex(Vec3(-1.2f, 0.31f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[119] = Vertex(Vec3(-1.76f, 0.96f, 0.f), wingColor, Vec2(0.f, 0.f));

	mesh[120] = Vertex(Vec3(0.79f, -0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[121] = Vertex(Vec3(-0.58f, -0.44f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[122] = Vertex(Vec3(-1.25f, -1.56f, 0.f), wingColor, Vec2(0.f, 0.f));

	mesh[123] = Vertex(Vec3(0.79f, -0.21f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[124] = Vertex(Vec3(-1.2f, -0.31f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[125] = Vertex(Vec3(-1.76f, -0.96f, 0.f), wingColor, Vec2(0.f, 0.f));
}

//-----------------------------------------------------------------------------------------------
void WaspEnemy::CollideTest() {
	if (DoDiscsOverlap(m_position, m_physicsRadius,
		m_game->m_player->m_position, m_game->m_player->m_physicsRadius)) {
		m_game->m_player->Die();
	}
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (m_game->m_bullets[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_physicsRadius,
				m_game->m_bullets[i]->m_position, m_game->m_bullets[i]->m_physicsRadius)) {
				m_game->m_bullets[i]->Die();
				m_health--;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
void WaspEnemy::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
	int debrisNum = m_randomGenerator.RollRandomIntInRange(numMin, numMax);
	for (int i = 0; i < debrisNum; i++) {
		int freeDebrisIndex = -1;
		for (int j = 0; j < MAX_DEBRIS; j++) {
			if (m_game->m_debris[j] == nullptr) {
				freeDebrisIndex = j;
				break;
			}
		}
		if (freeDebrisIndex > -1) {
			float randomAngle = m_randomGenerator.RollRandomFloatInRange(-burstAngle / 2.f, burstAngle / 2.f);
			float randomSpeed = m_randomGenerator.RollRandomFloatInRange(m_velocity.GetLength()*0.2f, m_velocity.GetLength());
			m_game->m_debris[freeDebrisIndex] = new Debris(m_game, m_position, 
				burstDirection.GetRotatedByDegrees(randomAngle),
				randomSpeed, color, scale);
		}
	}

}