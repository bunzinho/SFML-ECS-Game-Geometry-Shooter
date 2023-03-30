#pragma once

#include "Common.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Time.h"

struct PlayerConfig { int shapeRadius, collisionRadius, color_r, color_g, color_b, outline_r, outline_g, outline_b, outlineThickness, vertices; float speed; };
struct EnemyConfig { int shapeRadius, collisionRadius, outline_r, outline_g, outline_b, outlineThickness, verticiesMin, verticiesMax; double lifetime, spawnInterval, speedMin, speedMax; };
struct BulletConfig { int shapeRadius, collisionRadius, color_r, color_g, color_b, outline_r, outline_g, outline_b, outlineThickness, vertices; double lifetime, speed; };

class Game
{
	sf::RenderWindow    m_window;           // the window we will draw to
	EntityManager       m_entities;         // vector of entities to maintain
	sf::Font            m_font;             // the font we will use to draw
	sf::Text            m_text;             // the score text to be drawn to the screen
	PlayerConfig        m_playerConfig;
	EnemyConfig         m_enemyConfig;
	BulletConfig        m_bulletConfig;
	int                 m_score = 0;
	int                 m_currentFrame = 0;
	double              m_lastEnemySpawnTime = 0.0;
	bool                m_paused = false;   // whether we update game logic
	bool                m_running = true;   // whether the game is running
	bool                m_should_interpoloate_physics = true;
	Time                m_time = Time();

	std::shared_ptr<Entity> m_player;

	void init(const std::string& config);   // initialize the GameState with a config file path
	void setPaused(bool paused);            // pause the game

	void sMovement();                       // System: Entity position / movement update
	void sUserInput();                      // System: User Input
	void sLifespan();                       // System: Lifespan
	void sRender();                         // System: Render / Drawing
	void sEnemySpawner();                   // System: Spawns Enemies
	void sCollision();                      // System: Collisions
	void sSinmovement();

	bool checkCollision(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> entity2);

	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);

public:

	Game(const std::string& config);  // constructor, takes in game config

	void run();
};