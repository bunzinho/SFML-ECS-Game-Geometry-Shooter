#include "Game.h"

Game::Game(const std::string & config)
{ 
    init(config);
}

void Game::init(const std::string & path)
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

    const std::string windowTitle = "Assignment 2";
    if (window.fullscreen)
    {
        m_window.create(sf::VideoMode(window.width, window.height), windowTitle, sf::Style::Fullscreen);
    }
    else {
        m_window.create(sf::VideoMode(window.width, window.height), windowTitle);
    }
    m_window.setFramerateLimit(window.frameLimit);

    spawnPlayer();
}

void Game::run()
{
    // TODO: add pause functionality in here
    //       some systems should function while paused (rendering)
    //       some systems shouldn't (movement / input)
    while (m_running)
    {
        m_entities.update();

        if (m_paused == false)
        {
            sEnemySpawner();
            sMovement();
            sCollision();
            sLifespan();
            m_currentFrame++;
        }
        sUserInput();
        sRender();
        
        // increment the current frame
        // may need to be moved when pause implemented
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

    entity->cTransform = std::make_shared<CTransform>(Vec2(m_window.getSize().x, m_window.getSize().y)*0.5, Vec2(0.0f, 0.0f), 0.0f);

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
    // TODO: make sure the enemy is spawned properly with the m_enemyConfig variables
    //       the enemy must be spawned completely within the bounds of the window
    //
    
    // record when the most recent enemy was spawned
    auto entity = m_entities.addEntity("enemy");
	auto speed = m_enemyConfig.speedMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.speedMax - m_enemyConfig.speedMin)));


    auto minX = m_enemyConfig.shapeRadius;
    auto maxX = m_window.getSize().x - m_enemyConfig.shapeRadius;
    auto minY = m_enemyConfig.shapeRadius;
    auto maxY = m_window.getSize().y - m_enemyConfig.shapeRadius;

    auto xPos = (rand() % (maxX - minX + 1)) + minX;
    auto yPos = (rand() % (maxY - minY + 1)) + minY;
    auto spawnPosition = Vec2(xPos, yPos);

    auto vertices = (rand() % (m_enemyConfig.verticiesMax - m_enemyConfig.verticiesMin + 1)) + m_enemyConfig.verticiesMin;

    auto angle = rand();

    entity->cTransform = std::make_shared<CTransform>(spawnPosition, Vec2(cosf(angle), sinf(angle)).normalized()*speed, 0.0f);

    auto outline = sf::Color(m_enemyConfig.outline_r, m_enemyConfig.outline_g, m_enemyConfig.outline_b);
    entity->cShape = std::make_shared<CShape>(m_enemyConfig.shapeRadius, vertices, sf::Color(rand()%255, rand()%255, rand()%255), outline, m_enemyConfig.outlineThickness);
    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.collisionRadius);
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    auto speed = m_enemyConfig.speedMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_enemyConfig.speedMax - m_enemyConfig.speedMin)));
    auto vertices = e->cShape->circle.getPointCount();
    for (int i = 0; i < vertices; ++i)
    {
    auto entity = m_entities.addEntity("smallenemy");
    //auto angle = rand();
    auto angle = ((2 * 3.14159) / (vertices)) * i;

    entity->cTransform = std::make_shared<CTransform>(e->cTransform->pos, Vec2(cosf(angle), sinf(angle)).normalized() * speed, 0.0f);

    //auto outline = sf::Color(m_enemyConfig.outline_r, m_enemyConfig.outline_g, m_enemyConfig.outline_b);
    auto outlineColor = e->cShape->circle.getOutlineColor();
    entity->cShape = std::make_shared<CShape>(m_enemyConfig.shapeRadius/2, e->cShape->circle.getPointCount(), e->cShape->circle.getFillColor(), outlineColor, m_enemyConfig.outlineThickness);
    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.collisionRadius/2);
    entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.lifetime, m_currentFrame);
    }



    // TODO: spawn small enemies at the location of the input enemy e

    // when we create the smaller enemy, we have to read the values of the original enemy
    // - spawn a number of small enemies equal to the vertices of the original enemy
    // - set each small enemy to the same color as the original, half the size
    // - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto bullet = m_entities.addEntity("bullet");

	auto direction = target - entity->cTransform->pos;

	bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, direction.normalized() * m_bulletConfig.speed, 0.0f);

	auto bullet_color = sf::Color(m_bulletConfig.color_r, m_bulletConfig.color_g, m_bulletConfig.color_b);
	auto bullet_outline = sf::Color(m_bulletConfig.outline_r, m_bulletConfig.outline_g, m_bulletConfig.outline_b);
	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.shapeRadius, m_bulletConfig.vertices, bullet_color, bullet_outline, m_bulletConfig.outlineThickness);
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.lifetime, m_currentFrame);
    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.collisionRadius);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO: implement your own special weapon
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
		e->cTransform->pos += e->cTransform->velocity;
	}
}

void Game::lifeSpanFade(std::shared_ptr<Entity> e)
{
    auto const color = e->cShape->circle.getFillColor();
    auto const outline = e->cShape->circle.getOutlineColor();

    auto const time_remaining = (e->cLifespan->frameCreated + e->cLifespan->lifespan) - m_currentFrame;
    auto const percent = static_cast<float>(time_remaining) / static_cast<float>(e->cLifespan->lifespan);

    e->cShape->circle.setFillColor(sf::Color(color.r, color.g, color.b, static_cast<sf::Uint8>(255 * percent)));
    e->cShape->circle.setOutlineColor(sf::Color(outline.r, outline.g, outline.b, static_cast<sf::Uint8>(255 * percent)));
}

void Game::sLifespan()
{
    for (const auto e : m_entities.getEntities("bullet"))
    {
        if (e->cLifespan && e->isActive())
        {
            lifeSpanFade(e);
            if (m_currentFrame >= e->cLifespan->lifespan + e->cLifespan->frameCreated)
            {
                e->destroy();
            }
        }
    }

    for (const auto e : m_entities.getEntities("smallenemy"))
    {
        if (e->cLifespan && e->isActive())
        {
            lifeSpanFade(e);
            if (m_currentFrame >= e->cLifespan->lifespan + e->cLifespan->frameCreated)
            {
                e->destroy();
            }
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
            Vec2 d = (b->cTransform->pos - e->cTransform->pos);
            float r = b->cCollision->radius + e->cCollision->radius;
            if(d.x*d.x + d.y*d.y < r*r)
            {
                spawnSmallEnemies(e);
                b->destroy();
                e->destroy();
            }
        }

        for (const auto e : m_entities.getEntities("smallenemy"))
        {
            Vec2 d = (b->cTransform->pos - e->cTransform->pos);
            float r = b->cCollision->radius + e->cCollision->radius;
            if (d.x * d.x + d.y * d.y < r * r)
            {
                b->destroy();
                e->destroy();
            }
        }
    }

    // bounce the enemies off the edge of the screen
    for (const auto e : m_entities.getEntities("enemy"))
    {
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

    // bounce the small enemies off the edge of the screen
    for (const auto e : m_entities.getEntities("smallenemy"))
    {
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
    if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.spawnInterval)
    {
        spawnEnemy();
    }
}

void Game::sRender()
{
    m_window.clear();

    auto renderEntity = [this](std::shared_ptr<Entity> e)
    {
        if (e->cTransform && e->cShape)
        {
            e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

            e->cTransform->angle += 1.0f;
            e->cShape->circle.setRotation(e->cTransform->angle);

            m_window.draw(e->cShape->circle);
        }
    };

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
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                //std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
                spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
            }

            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                //std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
                // call spawnSpecialWeapon here
            }
            break;

        }
    }
}