#include "Game.h"
#include <chrono>

double time_in_seconds()
{
	typedef std::chrono::high_resolution_clock hires_clock;
	using namespace std::chrono;
	return 0.001 * duration_cast<milliseconds>(hires_clock::now().time_since_epoch()).count();
}

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	struct {
		unsigned int width;
		unsigned int height;
		unsigned int frameLimit;
		unsigned int fullscreen;
	} window = { 1280u, 720u, 60u, 0u };

	std::ifstream file(path);
	while (file.good())
	{
		std::string option;
		file >> option;
		if (option == "Window")
		{
			file >> window.width >> window.height >> window.frameLimit >> window.fullscreen;
		}
		else if (option == "Font")
		{
			struct {
				std::string path; unsigned int size; int r; int g; int b;
			} font = { "", 12, 127, 127, 127 };
			file >> font.path >> font.size >> font.r >> font.g >> font.b;
			m_font.loadFromFile(font.path);
			m_text.setFont(m_font);
			m_text.setCharacterSize(font.size);
			m_text.setPosition(10, 10);
			m_text.setFillColor(sf::Color(font.r, font.g, font.b));
		}
		else if (option == "Player")
		{
			file >> m_playerConfig.shapeRadius >> m_playerConfig.collisionRadius >> m_playerConfig.speed
				>> m_playerConfig.color_r >> m_playerConfig.color_g >> m_playerConfig.color_b
				>> m_playerConfig.outline_r >> m_playerConfig.outline_g >> m_playerConfig.outline_b
				>> m_playerConfig.outlineThickness >> m_playerConfig.vertices;
		}
		else if (option == "Enemy")
		{
			file >> m_enemyConfig.shapeRadius >> m_enemyConfig.collisionRadius >> m_enemyConfig.speedMin >> m_enemyConfig.speedMax
				>> m_enemyConfig.outline_r >> m_enemyConfig.outline_g >> m_enemyConfig.outline_b >> m_enemyConfig.outlineThickness
				>> m_enemyConfig.verticiesMin >> m_enemyConfig.verticiesMax >> m_enemyConfig.lifetime >> m_enemyConfig.spawnInterval;
		}
		else if (option == "Bullet")
		{
			file >> m_bulletConfig.shapeRadius >> m_bulletConfig.collisionRadius >> m_bulletConfig.speed
				>> m_bulletConfig.color_r >> m_bulletConfig.color_g >> m_bulletConfig.color_b
				>> m_bulletConfig.outline_r >> m_bulletConfig.outline_g >> m_bulletConfig.outline_b
				>> m_bulletConfig.outlineThickness >> m_bulletConfig.vertices >> m_bulletConfig.lifetime;
		}
		else
		{
			std::cout << "Unknown config option" << std::endl;
		}
	}

	const std::string windowTitle = "SFML ECS Game Polygon Shooter";
	if (window.fullscreen)
	{
		m_window.create(sf::VideoMode(window.width, window.height), windowTitle, sf::Style::Fullscreen);
	}
	else
	{
		m_window.create(sf::VideoMode(window.width, window.height), windowTitle);
	}

	m_window.setFramerateLimit(window.frameLimit);

	spawnPlayer();
}

void Game::run()
{
	while (m_running)
	{
		
		if (m_paused)
		{
			m_time.update_time();
			sUserInput();
			continue;
		}

		m_time.update_delta_time();

		while (m_time.is_tick_ready())
		{
			sUserInput();
			m_entities.update();
			sEnemySpawner();
			sMovement();
			sCollision();
			sLifespan();
			m_time.update_accumulator();
			m_currentFrame++;
		}
		
		sRender();
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");

	entity->cTransform = std::make_shared<CTransform>(Vec2(m_window.getSize().x, m_window.getSize().y) * 0.5, Vec2(0.0f, 0.0f), 0.0f);

	auto player_color = sf::Color(m_playerConfig.color_r, m_playerConfig.color_g, m_playerConfig.color_b);
	auto player_outline = sf::Color(m_playerConfig.outline_r, m_playerConfig.outline_g, m_playerConfig.outline_b);
	entity->cShape = std::make_shared<CShape>(m_playerConfig.shapeRadius, m_playerConfig.vertices, player_color, player_outline, m_playerConfig.outlineThickness);
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.collisionRadius);

	// Add an input component to the player so that we can use inputs
	entity->cInput = std::make_shared<CInput>();

	// Since we want this Entity to be our player, set our Game's player variable to be this Entity
	// This goes slightly against the EntityManager paradigm, but we use the player so much it's worth it
	m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	auto speed = m_enemyConfig.speedMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.speedMax - m_enemyConfig.speedMin)));


	auto minX = m_enemyConfig.shapeRadius;
	auto maxX = m_window.getSize().x - m_enemyConfig.shapeRadius;
	auto minY = m_enemyConfig.shapeRadius;
	auto maxY = m_window.getSize().y - m_enemyConfig.shapeRadius;

	Vec2 spawnPosition;

	for (auto i = 0; i < 10; i++)
	{
		auto xPos = (rand() % (maxX - minX + 1)) + minX;
		auto yPos = (rand() % (maxY - minY + 1)) + minY;
		spawnPosition = Vec2(xPos, yPos);
		Vec2 d = (spawnPosition - m_player->cTransform->pos);
		float r = m_enemyConfig.collisionRadius + m_player->cCollision->radius * 4;

		if (d.lengthSquared() > r * r)
		{
			break;
		}
		if (i + 1 == 10)
		{
			std::cerr << "could not find enemy spawn position after 10 tries" << std::endl;
			return;
		}
		std::cerr << m_currentFrame << " spawn collides with player, generating new spawn point" << std::endl;
	}

	auto vertices = (rand() % (m_enemyConfig.verticiesMax - m_enemyConfig.verticiesMin + 1)) + m_enemyConfig.verticiesMin;
	auto angle = rand();

	auto outline = sf::Color(m_enemyConfig.outline_r, m_enemyConfig.outline_g, m_enemyConfig.outline_b);

	auto entity = m_entities.addEntity("enemy");
	entity->cTransform = std::make_shared<CTransform>(spawnPosition, Vec2(angle).normalized() * speed, 0.0f);
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.shapeRadius, vertices, sf::Color(rand() % 255, rand() % 255, rand() % 255), outline, m_enemyConfig.outlineThickness);
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.collisionRadius);
	entity->cScore = std::make_shared<CScore>(vertices * 100);
	m_lastEnemySpawnTime = m_time.get_game_time();
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	auto speed = m_enemyConfig.speedMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.speedMax - m_enemyConfig.speedMin)));
	auto vertices = e->cShape->circle.getPointCount();

	auto angleIncrement = ((6.283185f) / (vertices));
	for (int i = 0; i < vertices; ++i)
	{
		auto entity = m_entities.addEntity("smallenemy");
		auto angle = angleIncrement * i;

		entity->cTransform = std::make_shared<CTransform>(e->cTransform->pos, Vec2(angle).normalized() * speed, 0);
		entity->cShape = std::make_shared<CShape>(m_enemyConfig.shapeRadius / 2, e->cShape->circle.getPointCount(), e->cShape->circle.getFillColor(), e->cShape->circle.getOutlineColor(), m_enemyConfig.outlineThickness);
		entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.collisionRadius / 2);
		entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.lifetime, m_currentFrame);
		entity->cScore = std::make_shared<CScore>(vertices * 200);
	}
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto bullet = m_entities.addEntity("bullet");
	auto direction = target - entity->cTransform->pos;
	auto bullet_color = sf::Color(m_bulletConfig.color_r, m_bulletConfig.color_g, m_bulletConfig.color_b);
	auto bullet_outline = sf::Color(m_bulletConfig.outline_r, m_bulletConfig.outline_g, m_bulletConfig.outline_b);

	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.shapeRadius, m_bulletConfig.vertices, bullet_color, bullet_outline, m_bulletConfig.outlineThickness);
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.lifetime, m_currentFrame);
	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.collisionRadius);
	bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, direction.normalized() * m_bulletConfig.speed, 0.0f);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	const auto bullets = 30;
	for (int i = 0; i < bullets; ++i)
	{
		auto angle = ((6.283185f) / (bullets)) * i;
		auto bullet = m_entities.addEntity("bullet");
		auto bullet_color = sf::Color(230, 20, 120);
		auto bullet_outline = sf::Color(30, 220, 215);
		const auto speed = 360.0f;

		bullet->cShape = std::make_shared<CShape>(m_bulletConfig.shapeRadius, m_bulletConfig.vertices, bullet_color, bullet_outline, m_bulletConfig.outlineThickness);
		bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.lifetime, m_currentFrame);
		bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.collisionRadius);
		bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, Vec2(angle).normalized() * speed, 0.0f);
	}
}

bool Game::checkCollision(std::shared_ptr<Entity> e, std::shared_ptr<Entity> e2)
{
	Vec2 d = (e2->cTransform->pos - e->cTransform->pos);
	float r = e2->cCollision->radius + e->cCollision->radius;
	if (d.lengthSquared() < r * r)
	{
		return true;
	}
	return false;
}

void Game::sMovement()
{
	auto playerDirection = Vec2();
	if (m_player->cInput->left) {
		playerDirection.x += -1;
	}
	if (m_player->cInput->right) {
		playerDirection.x += 1;
	}
	if (m_player->cInput->up) {
		playerDirection.y += -1;
	}
	if (m_player->cInput->down) {
		playerDirection.y += 1;
	}
	m_player->cTransform->velocity = playerDirection.normalized() * m_playerConfig.speed;


	for (const auto e : m_entities.getEntities())
	{
		e->cTransform->last_pos = e->cTransform->pos;
		e->cTransform->pos += e->cTransform->velocity * m_time.delta_time();
		e->cTransform->angle += 60.0f * m_time.delta_time();
	}
}

void Game::sLifespan()
{
	for (const auto e : m_entities.getEntities())
	{
		if (!e->cLifespan || !e->isActive())
		{
			continue;
		}

		auto const startFrame = e->cLifespan->frameCreated;
		auto const endFrame = startFrame + e->cLifespan->lifespan;
		if (m_currentFrame >= endFrame)
		{
			e->destroy();
		}
		else
		{
			auto pct_time_remaining = InvLerp(endFrame, startFrame, m_currentFrame);

			auto color = e->cShape->circle.getFillColor();
			auto outline = e->cShape->circle.getOutlineColor();

			color.a = static_cast<sf::Uint8>(255 * pct_time_remaining);
			outline.a = static_cast<sf::Uint8>(255 * pct_time_remaining);

			e->cShape->circle.setFillColor(color);
			e->cShape->circle.setOutlineColor(outline);
		}
		
	}
}

void Game::sCollision()
{
	if (m_player->cTransform->pos.x < 0 + m_player->cCollision->radius)
	{
		m_player->cTransform->pos.x = 0 + m_player->cCollision->radius;
	}
	if (m_player->cTransform->pos.x + m_player->cCollision->radius > m_window.getSize().x)
	{
		m_player->cTransform->pos.x = m_window.getSize().x - m_player->cCollision->radius;
	}
	if (m_player->cTransform->pos.y < 0 + m_player->cCollision->radius)
	{
		m_player->cTransform->pos.y = 0 + m_player->cCollision->radius;
	}
	if (m_player->cTransform->pos.y > m_window.getSize().y - m_player->cCollision->radius)
	{
		m_player->cTransform->pos.y = m_window.getSize().y - m_player->cCollision->radius;
	}

	// check bullet vs enemy collisions and destory them
	for (const auto b : m_entities.getEntities("bullet"))
	{
		for (const auto e : m_entities.getEntities("enemy"))
		{
			if (e->isActive() && checkCollision(b, e))
			{
				m_score += e->cScore->score;
				spawnSmallEnemies(e);
				b->destroy();
				e->destroy();
			}
		}

		for (const auto e : m_entities.getEntities("smallenemy"))
		{
			if (e->isActive() && checkCollision(b, e))
			{
				m_score += e->cScore->score;
				b->destroy();
				e->destroy();
			}
		}
	}

	// check enemy vs player collision and bounce the enemies off the edge of the screen
	for (const auto e : m_entities.getEntities("enemy"))
	{
		if (checkCollision(m_player, e))
		{
			m_player->destroy();
			spawnPlayer();
			e->destroy();
			spawnSmallEnemies(e);
		}

		auto window_size = m_window.getSize();
		if (e->cTransform->pos.x < 0 + e->cShape->circle.getRadius() || e->cTransform->pos.x > window_size.x - e->cShape->circle.getRadius())
		{
			e->cTransform->velocity.x *= -1;
		}
		if (e->cTransform->pos.y < 0 + e->cShape->circle.getRadius() || e->cTransform->pos.y > window_size.y - e->cShape->circle.getRadius())
		{
			e->cTransform->velocity.y *= -1;
		}
	}

	// check small enemy vs player and bounce the small enemies off the edge of the screen
	for (const auto e : m_entities.getEntities("smallenemy"))
	{
		if (checkCollision(m_player, e))
		{
			m_player->destroy();
			spawnPlayer();
			e->destroy();
		}

		auto window_size = m_window.getSize();
		if (e->cTransform->pos.x < 0 + e->cShape->circle.getRadius() || e->cTransform->pos.x > window_size.x - e->cShape->circle.getRadius())
		{
			e->cTransform->velocity.x *= -1;
		}
		if (e->cTransform->pos.y < 0 + e->cShape->circle.getRadius() || e->cTransform->pos.y > window_size.y - e->cShape->circle.getRadius())
		{
			e->cTransform->velocity.y *= -1;
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_time.get_game_time() - m_lastEnemySpawnTime < m_enemyConfig.spawnInterval)
	{
		return;
	}
	spawnEnemy();
}

void Game::sRender()
{
	m_window.clear();

	auto renderEntity = [&](std::shared_ptr<Entity> e)
	{
		if (m_should_interpoloate_physics)
		{
			auto interpolated_position = e->cTransform->pos * m_time.alpha() + e->cTransform->last_pos * (1.0 - m_time.alpha());
			e->cShape->circle.setPosition(interpolated_position.x, interpolated_position.y);
		}
		else {
			e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		}

		e->cShape->circle.setRotation(e->cTransform->angle);

		m_window.draw(e->cShape->circle);
	};

	// Separate loops for draw order
	for (const auto e : m_entities.getEntities("enemy"))
	{
		renderEntity(e);
	}
	for (const auto e : m_entities.getEntities("smallenemy"))
	{
		renderEntity(e);
	}
	for (const auto e : m_entities.getEntities("player"))
	{
		renderEntity(e);
	}
	for (const auto e : m_entities.getEntities("bullet"))
	{
		renderEntity(e);
	}

	m_text.setString("Score: " + std::to_string(m_score));
	m_window.draw(m_text);

	m_window.display();
}

void Game::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			m_running = false;
			break;

		case sf::Event::Resized:
		{sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
		m_window.setView(sf::View(visibleArea)); }
		break;

		case sf::Event::KeyPressed:
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = true;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = true;
				break;
			case sf::Keyboard::Escape:
				m_running = false;
				break;
			case sf::Keyboard::F1:
				m_paused = !m_paused;
				break;
			case sf::Keyboard::F2:
				m_entities.clearEntitiesByTag("enemy");
				break;
			case sf::Keyboard::F10:
				m_should_interpoloate_physics = !m_should_interpoloate_physics;
				break;
			}
			break;

		case sf::Event::KeyReleased:
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = false;
				break;
			}
			break;

		case sf::Event::MouseButtonPressed:
			if (m_paused)
			{
				break;
			}
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				//std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				spawnSpecialWeapon(m_player);
			}
			break;
		}
	}
}
