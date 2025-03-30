#include <iostream>
#include <fstream>
#include <chrono>
#include "Game.h"
#include "Mathf.h"
#include "inicpp.h"

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	struct windowsettings {
		unsigned int width;
		unsigned int height;
		unsigned int frameLimit;
		unsigned int fullscreen;
	} window = { 1280u, 720u, 60u, 0u };

	ini::IniFileCaseInsensitive config;
	config.load(path);

	window.width      = config["window"]["width"].as<unsigned int>();
	window.height     = config["window"]["height"].as<unsigned int>();
	window.frameLimit = config["window"]["framelimit"].as<unsigned int>();
	window.fullscreen = config["window"]["fullscreen"].as<unsigned int>();

	auto& player = config["player"];
	m_playerConfig.shapeRadius      = player["shaperadius"].as<float>();
	m_playerConfig.collisionRadius  = player["collisionRadius"].as<float>();
	m_playerConfig.speed            = player["speed"].as<float>();
	m_playerConfig.color_r          = player["colorr"].as<int>();
	m_playerConfig.color_g          = player["colorg"].as<int>();
	m_playerConfig.color_b          = player["colorb"].as<int>();
	m_playerConfig.outline_r        = player["outliner"].as<int>();
	m_playerConfig.outline_g        = player["outlineg"].as<int>();
	m_playerConfig.outline_b        = player["outlineb"].as<int>();
	m_playerConfig.outlineThickness = player["outlinesize"].as<float>();
	m_playerConfig.vertices         = player["vertices"].as<int>();

	auto& enemy = config["enemy"];
	m_enemyConfig.shapeRadius       = enemy["shaperadius"].as<float>();
	m_enemyConfig.collisionRadius   = enemy["collisionRadius"].as<float>();
	m_enemyConfig.speedMin          = enemy["speedmin"].as<float>();
	m_enemyConfig.speedMax          = enemy["speedmax"].as<float>();
	m_enemyConfig.outline_r         = enemy["outliner"].as<int>();
	m_enemyConfig.outline_g         = enemy["outlineg"].as<int>();
	m_enemyConfig.outline_b         = enemy["outlineb"].as<int>();
	m_enemyConfig.outlineThickness  = enemy["outlinesize"].as<float>();
	m_enemyConfig.verticiesMin      = enemy["verticesmin"].as<int>();
	m_enemyConfig.verticiesMax      = enemy["verticesmax"].as<int>();
	m_enemyConfig.lifetime          = enemy["lifetime"].as<double>();
	m_enemyConfig.spawnInterval     = enemy["spawninterval"].as<double>();

	auto& bullet = config["bullet"];
	m_bulletConfig.shapeRadius      = bullet["shaperadius"].as<float>();
	m_bulletConfig.collisionRadius  = bullet["collisionRadius"].as<float>();
	m_bulletConfig.speed            = bullet["speed"].as<float>();
	m_bulletConfig.color_r          = bullet["colorr"].as<int>();
	m_bulletConfig.color_g          = bullet["colorg"].as<int>();
	m_bulletConfig.color_b          = bullet["colorb"].as<int>();
	m_bulletConfig.outline_r        = bullet["outliner"].as<int>();
	m_bulletConfig.outline_g        = bullet["outlineg"].as<int>();
	m_bulletConfig.outline_b        = bullet["outlineb"].as<int>();
	m_bulletConfig.outlineThickness = bullet["outlinesize"].as<float>();
	m_bulletConfig.vertices         = bullet["vertices"].as<int>();
	m_bulletConfig.lifetime         = bullet["lifetime"].as<double>();

	auto& font_ini = config["font"];

	struct {
		std::string path; unsigned int size; int r; int g; int b;
	} font = { "", 12, 127, 127, 127 };

	font.path = font_ini["path"].as<std::string>();
	font.size = font_ini["size"].as<unsigned int>();
	font.r = font_ini["r"].as<int>();
	font.g = font_ini["g"].as<int>();
	font.b = font_ini["b"].as<int>();

	m_font.loadFromFile(font.path);
	m_text.setFont(m_font);
	m_text.setCharacterSize(font.size);
	m_text.setPosition(10, 10);
	m_text.setFillColor(sf::Color(font.r, font.g, font.b));

	constexpr auto windowTitle = "SFML ECS Game Polygon Shooter";
	sf::Uint32 windowStyle = window.fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
	
	m_window.create(sf::VideoMode(window.width, window.height), windowTitle, windowStyle);
	m_window.setFramerateLimit(window.frameLimit);

	spawnPlayer();
}

void Game::run()
{
	while (m_running)
	{
		m_clock.update_delta_time();

		sUserInput();
		if (m_paused)
		{
			continue;
		}

		while (m_clock.is_tick_ready())
		{
			m_entities.update();

			sLifespan();
			sActions();
			sMovement();
			sCollision();
			sEnemySpawner();

			m_clock.update_accumulator();
		}
		sRender();
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
	m_clock.set_paused(paused);
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");

	const auto position = Vec2(static_cast<float>(m_window.getSize().x * 0.5f), static_cast<float>((m_window.getSize().y) * 0.5f));

	entity->addComponent<CTransform>(position, Vec2(0.0f, 0.0f), 0.0f);

	auto player_color = sf::Color(m_playerConfig.color_r, m_playerConfig.color_g, m_playerConfig.color_b);
	auto player_outline = sf::Color(m_playerConfig.outline_r, m_playerConfig.outline_g, m_playerConfig.outline_b);
	entity->addComponent<CShape>(m_playerConfig.shapeRadius, m_playerConfig.vertices, player_color, player_outline, m_playerConfig.outlineThickness);
	entity->addComponent<CCollision>(m_playerConfig.collisionRadius);

	// Add an input component to the player so that we can use inputs
	entity->addComponent<CInput>();

	// Since we want this Entity to be our player, set our Game's player variable to be this Entity
	// This goes slightly against the EntityManager paradigm, but we use the player so much it's worth it
	m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	auto speed = Lerp(m_enemyConfig.speedMin, m_enemyConfig.speedMax, random_float());

	auto min_x = m_enemyConfig.shapeRadius;
	auto max_x = m_window.getSize().x - m_enemyConfig.shapeRadius;
	auto min_y = m_enemyConfig.shapeRadius;
	auto max_y = m_window.getSize().y - m_enemyConfig.shapeRadius;

	Vec2 spawn_position;

	for (auto i = 0; i < 10; i++)
	{
		float x_pos = Lerp(min_x, max_x, random_float());
		float y_pos = Lerp(min_y, max_y, random_float());

		spawn_position = Vec2(x_pos, y_pos);
		Vec2 d = (spawn_position - m_player->getComponent<CTransform>().pos);

		float r = m_enemyConfig.collisionRadius + m_player->getComponent<CCollision>().radius * 5.0f;

		if (d.length_squared() > r * r)
		{
			break;
		}
		if (i + 1 == 10)
		{
			std::cerr << "could not find enemy spawn position after 10 tries" << std::endl;
			return;
		}

		// enemy spawn is too close to the player, try another random location
		std::cerr << m_clock.get_game_time() << " enemy spawn is too close to the player, generating new spawn point" << std::endl;
	}

	auto vertices = static_cast<int>(Lerp((float)m_enemyConfig.verticiesMin, (float)m_enemyConfig.verticiesMax, random_float()));
	auto angle = tau * random_float();
	auto outline = sf::Color(m_enemyConfig.outline_r, m_enemyConfig.outline_g, m_enemyConfig.outline_b);
	auto color = sf::Color(static_cast<sf::Uint8>(random_float() * 255), static_cast<sf::Uint8>(random_float() * 255), static_cast<sf::Uint8>(random_float() * 255));

	auto entity = m_entities.addEntity("enemy");

	entity->addComponent<CTransform>(spawn_position, Vec2(angle).normalized() * speed, 0.0f);
	entity->addComponent<CShape>(m_enemyConfig.shapeRadius, vertices, color, outline, m_enemyConfig.outlineThickness);
	entity->addComponent<CCollision>(m_enemyConfig.collisionRadius);
	entity->addComponent<CScore>(vertices * 100);

	m_lastEnemySpawnTime = m_clock.get_game_time();
}

// spawns the small enemies when a big one is destroyed
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	auto& shape = e->getComponent<CShape>();

	auto speed = Lerp(m_enemyConfig.speedMin, m_enemyConfig.speedMax, random_float());
	auto vertices = shape.circle.getPointCount();
	auto angleIncrement = tau / vertices;
	for (size_t i = 0; i < vertices; ++i)
	{
		auto entity = m_entities.addEntity("smallenemy");
		auto angle = angleIncrement * i;

		entity->addComponent<CTransform>(e->getComponent<CTransform>().pos, Vec2(angle).normalized() * speed, 0.0f);
		entity->addComponent<CShape>(m_enemyConfig.shapeRadius * 0.5f, shape.circle.getPointCount(), shape.circle.getFillColor(), shape.circle.getOutlineColor(), m_enemyConfig.outlineThickness);
		entity->addComponent<CCollision>(m_enemyConfig.collisionRadius * 0.5f);
		entity->addComponent<CLifespan>(m_enemyConfig.lifetime, m_clock.get_game_time());
		entity->addComponent<CScore>(vertices * 200);
	}
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto bullet = m_entities.addEntity("bullet");
	auto entity_position = entity->getComponent<CTransform>().pos;
	auto direction = target - entity_position;
	auto bullet_color = sf::Color(m_bulletConfig.color_r, m_bulletConfig.color_g, m_bulletConfig.color_b);
	auto bullet_outline = sf::Color(m_bulletConfig.outline_r, m_bulletConfig.outline_g, m_bulletConfig.outline_b);

	bullet->addComponent<CShape>(m_bulletConfig.shapeRadius, m_bulletConfig.vertices, bullet_color, bullet_outline, m_bulletConfig.outlineThickness);
	bullet->addComponent<CLifespan>(m_bulletConfig.lifetime, m_clock.get_game_time());
	bullet->addComponent<CCollision>(m_bulletConfig.collisionRadius);
	bullet->addComponent<CTransform>(entity_position, direction.normalized() * m_bulletConfig.speed, 0.0f);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	const auto bullets = 15;
	for (int i = 0; i < bullets; ++i)
	{
		auto angle = (6.283185f / bullets) * i;
		auto bullet_color = sf::Color(230, 20, 120);
		auto bullet_outline = sf::Color(30, 220, 215);
		const auto speed = 360.0f;

		auto bullet = m_entities.addEntity("bullet");
		bullet->addComponent<CShape>(m_bulletConfig.shapeRadius, m_bulletConfig.vertices, bullet_color, bullet_outline, m_bulletConfig.outlineThickness);
		bullet->addComponent<CLifespan>(m_bulletConfig.lifetime, m_clock.get_game_time());
		bullet->addComponent<CCollision>(m_bulletConfig.collisionRadius);
		bullet->addComponent<CTransform>(entity->getComponent<CTransform>().pos, Vec2(angle).normalized() * speed, 0.0f);
	}
}

bool Game::checkCollision(std::shared_ptr<Entity> e, std::shared_ptr<Entity> e2)
{
	Vec2 d = (e2->getComponent<CTransform>().pos - e->getComponent<CTransform>().pos);
	
	float r = e2->getComponent<CCollision>().radius + e->getComponent<CCollision>().radius;
	if (d.length_squared() < r * r)
	{
		return true;
	}
	return false;
}

void Game::sActions()
{
	auto& input = m_player->getComponent<CInput>();
	if (input.special)
	{
		spawnSpecialWeapon(m_player);
		input.special = false;
	}
	if (input.shoot)
	{
		spawnBullet(m_player, Vec2(static_cast<float>(input.x_mouse), static_cast<float>(input.y_mouse)));
		input.shoot = false;
	}
}

void Game::sMovement()
{
	auto playerDirection = Vec2();
	auto& player_input = m_player->getComponent<CInput>();
	if (player_input.left) {
		playerDirection.x += -1;
	}
	if (player_input.right) {
		playerDirection.x += 1;
	}
	if (player_input.up) {
		playerDirection.y += -1;
	}
	if (player_input.down) {
		playerDirection.y += 1;
	}
	m_player->getComponent<CTransform>().velocity = playerDirection.normalized() * m_playerConfig.speed;


	for (const auto &e : m_entities.getEntities())
	{
		auto &transform = e->getComponent<CTransform>();
		transform.last_pos = transform.pos;
		transform.pos += transform.velocity * static_cast<float>(m_clock.delta_time());
		transform.angle += 60.0f * static_cast<float>(m_clock.delta_time());
	}
}

void Game::sLifespan()
{
	for (const auto &e : m_entities.getEntities())
	{
		if (!e->hasComponent<CLifespan>() || !e->isActive())
		{
			continue;
		}

		auto const start_frame = e->getComponent<CLifespan>().frameCreated;
		auto const end_frame = start_frame + e->getComponent<CLifespan>().lifespan;

		if (m_clock.get_game_time() >= end_frame)
		{
			e->destroy();
			continue;
		}

		auto t = InvLerp(end_frame, start_frame, m_clock.get_game_time());
		auto pct_time_remaining = 1.0 - std::pow(1.0-t, 6.0);

		auto& circle = e->getComponent<CShape>().circle;

		auto color = circle.getFillColor();
		auto outline = circle.getOutlineColor();

		color.a = static_cast<sf::Uint8>(255 * pct_time_remaining);
		outline.a = static_cast<sf::Uint8>(255 * pct_time_remaining);

		circle.setFillColor(color);
		circle.setOutlineColor(outline);
	}
}

void Game::sCollision()
{
	auto& player_position = m_player->getComponent<CTransform>().pos;
	auto player_radius = m_player->getComponent<CCollision>().radius;
	if (player_position.x < 0 + player_radius)
	{
		player_position.x = 0 + player_radius;
	}
	if (player_position.x + player_radius > m_window.getSize().x)
	{
		player_position.x = m_window.getSize().x - player_radius;
	}
	if (player_position.y < 0 + player_radius)
	{
		player_position.y = 0 + player_radius;
	}
	if (player_position.y > m_window.getSize().y - player_radius)
	{
		player_position.y = m_window.getSize().y - player_radius;
	}

	// check bullet vs enemy collisions and destory them
	for (const auto &b : m_entities.getEntities("bullet"))
	{
		for (const auto &e : m_entities.getEntities("enemy"))
		{
			if (e->isActive() && checkCollision(b, e))
			{
				m_score += e->getComponent<CScore>().score;
				spawnSmallEnemies(e);
				b->destroy();
				e->destroy();
			}
		}

		for (const auto &e : m_entities.getEntities("smallenemy"))
		{
			if (e->isActive() && checkCollision(b, e))
			{
				m_score += e->getComponent<CScore>().score;
				b->destroy();
				e->destroy();
			}
		}
	}

	// check enemy vs player collision and bounce the enemies off the edge of the screen
	for (const auto &e : m_entities.getEntities("enemy"))
	{
		if (checkCollision(m_player, e))
		{
			m_player->destroy();
			spawnPlayer();
			e->destroy();
			spawnSmallEnemies(e);
		}

		auto window_size = m_window.getSize();
		auto &transform = e->getComponent<CTransform>();
		auto &shape = e->getComponent<CShape>();
		if (transform.pos.x < 0 + shape.circle.getRadius() || transform.pos.x > window_size.x - shape.circle.getRadius())
		{
			transform.velocity.x *= -1;
		}
		if (transform.pos.y < 0 + shape.circle.getRadius() || transform.pos.y > window_size.y - shape.circle.getRadius())
		{
			transform.velocity.y *= -1;
		}
	}

	// check small enemy vs player and bounce the small enemies off the edge of the screen
	for (const auto &e : m_entities.getEntities("smallenemy"))
	{
		if (checkCollision(m_player, e))
		{
			m_player->destroy();
			spawnPlayer();
			e->destroy();
		}

		auto window_size = m_window.getSize();

		auto &transform = e->getComponent<CTransform>();
		auto &shape = e->getComponent<CShape>();
		if (transform.pos.x < 0 + shape.circle.getRadius() || transform.pos.x > window_size.x - shape.circle.getRadius())
		{
			transform.velocity.x *= -1;
		}
		if (transform.pos.y < 0 + shape.circle.getRadius() || transform.pos.y > window_size.y - shape.circle.getRadius())
		{
			transform.velocity.y *= -1;
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_clock.get_game_time() - m_lastEnemySpawnTime < m_enemyConfig.spawnInterval)
	{
		return;
	}
	spawnEnemy();
}

void Game::renderEntity(std::shared_ptr<Entity> e)
{
	auto &transform = e->getComponent<CTransform>();
	auto &shape = e->getComponent<CShape>();
	if (m_should_interpoloate_physics)
	{
		auto interpolated_position = Vec2::Lerp(transform.last_pos, transform.pos, static_cast<float>(m_clock.alpha()));
		shape.circle.setPosition(interpolated_position.x, interpolated_position.y);
	}
	else {
		shape.circle.setPosition(transform.pos.x, transform.pos.y);
	}

	shape.circle.setRotation(transform.angle);

	m_window.draw(shape.circle);
}

void Game::sRender()
{
	m_window.clear();

	// Separate loops for draw order
	for (const auto &e : m_entities.getEntities("enemy"))
	{
		renderEntity(e);
	}
	for (const auto &e : m_entities.getEntities("smallenemy"))
	{
		renderEntity(e);
	}
	for (const auto &e : m_entities.getEntities("player"))
	{
		renderEntity(e);
	}
	for (const auto &e : m_entities.getEntities("bullet"))
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
		{
			sf::FloatRect visibleArea(0.0f, 0.0f, (float) event.size.width, (float) event.size.height);
			m_window.setView(sf::View(visibleArea));
		}
		break;

		case sf::Event::KeyPressed:
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->getComponent<CInput>().up = true;
				break;
			case sf::Keyboard::S:
				m_player->getComponent<CInput>().down = true;
				break;
			case sf::Keyboard::A:
				m_player->getComponent<CInput>().left = true;
				break;
			case sf::Keyboard::D:
				m_player->getComponent<CInput>().right = true;
				break;
			case sf::Keyboard::Escape:
				m_running = false;
				break;
			case sf::Keyboard::F1:
				setPaused(m_paused = !m_paused);
				break;
			case sf::Keyboard::F2:
				m_entities.clearEntitiesByTag("enemy");
				m_entities.clearEntitiesByTag("smallenemy");
				std::cout << "enemies cleared" << std::endl;
				break;
			case sf::Keyboard::F10:
				m_should_interpoloate_physics = !m_should_interpoloate_physics;
				std::cout << "interpolate rendering: " << m_should_interpoloate_physics << std::endl;
				break;
			case sf::Keyboard::Up:
				std::cout << "timescale changed: " << m_clock.add_time_scale(0.1) << std::endl;
				break;
			case sf::Keyboard::Down:
				std::cout << "timescale changed: " << m_clock.add_time_scale(-0.1) << std::endl;
				break;
			}
			break;

		case sf::Event::MouseMoved:
			m_player->getComponent<CInput>().x_mouse = event.mouseMove.x;
			m_player->getComponent<CInput>().y_mouse = event.mouseMove.y;
			break;

		case sf::Event::KeyReleased:
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->getComponent<CInput>().up = false;
				break;
			case sf::Keyboard::S:
				m_player->getComponent<CInput>().down = false;
				break;
			case sf::Keyboard::A:
				m_player->getComponent<CInput>().left = false;
				break;
			case sf::Keyboard::D:
				m_player->getComponent<CInput>().right = false;
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
				m_player->getComponent<CInput>().shoot = true;
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				m_player->getComponent<CInput>().special = true;
			}
			break;
		}
	}
}
