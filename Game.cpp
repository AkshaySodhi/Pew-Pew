#include "Game.h"
#include<iostream>
#include<string>
#include<fstream>

Game::Game(const std::string& config) {
	srand(time(NULL));
	init(config);
}

void Game::init(const std::string& config) {
	std::ifstream fin("config.txt");
	std::string configType;

	while (fin >> configType) {
		if (configType == "Window") {
			int wWidth, wHeight, FL, FS;
			fin >> wWidth >> wHeight >> FL >> FS;

			m_window.create(sf::VideoMode(wWidth, wHeight), "Pew Pew", sf::Style::Titlebar | sf::Style::Close);
			m_window.setFramerateLimit(FL);
		}
		else if (configType == "Font") {
			std::string path = "";
			int charSize, r, g, b;
			fin >> path >> charSize >> r >> g >> b;
			if (m_font.loadFromFile(path)) {
				m_text.setFont(m_font);
				m_text.setCharacterSize(charSize);
				m_text.setFillColor(sf::Color(r, g, b));
				m_text.setPosition(0, 0);
			}
		}
		else if (configType == "Player") {
			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (configType == "Enemy") {
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		else if (configType == "Bullet") {
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
	}

	spawnPlayer();
}

void Game::run() {
	while (m_running) {
		if (!m_paused) {
			m_entities.update();
			sEnemySpawner();
			sLifespan();
			sMovement();
			sCollision();
			m_currentFrame++;
		};
		sUserInput();
		sRender();
	}
}

void Game::spawnPlayer() {
	auto entity = m_entities.addEntity("player");

	float mx = m_window.getSize().x / 2.f;
	float my = m_window.getSize().y / 2.f;

	entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(0.f, 0.f), 0.0f);
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	entity->cInput = std::make_shared<CInput>();
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	m_player = entity;
}

void Game::spawnEnemy() {
	int diffX = 1 + m_window.getSize().x - m_enemyConfig.CR - m_playerConfig.CR;
	int diffY = 1 + m_window.getSize().y - m_enemyConfig.CR - m_playerConfig.CR;
	float px = rand() % diffX + m_playerConfig.CR / 1.f;
	float py = rand() % diffY + m_playerConfig.CR / 1.f;

	float sDiff = 1 + m_enemyConfig.SMAX - m_enemyConfig.SMIN;
	float s = rand() % int(sDiff) + m_enemyConfig.SMIN;
	float vx = rand() % int(1 + s) + 0.f;
	float vy = sqrt(s * s) - (vx * vx);

	int vertices = rand() % (1 + m_enemyConfig.VMAX - m_enemyConfig.VMIN) + m_enemyConfig.VMIN;

	auto entity = m_entities.addEntity("enemy");
	entity->cTransform = std::make_shared<CTransform>(Vec2(px, py), Vec2(vx, vy), 0.f);
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
	if (isCollided(m_player, entity)) {
		entity->destroy();
		return;
	}
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, vertices, sf::Color(rand() % 256, rand() % 256, rand() % 256), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
	entity->cScore = std::make_shared<CScore>(100);

	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEntities(std::shared_ptr<Entity> entity) {
	if (entity->cLifespan) return;

	int n = entity->cShape->circle.getPointCount();
	float s = 2.f;

	for (int i = 0; i < n; i++) {
		float angle = (360.f / n) * i;
		float rad = angle * 3.14159f / 180;

		Vec2 velo(s * cosf(rad), s * sinf(rad));

		auto smallEntity = m_entities.addEntity("enemy");
		smallEntity->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, velo, 0.f);
		smallEntity->cShape = std::make_shared<CShape>(m_enemyConfig.SR / 2.f, n, entity->cShape->circle.getFillColor(), entity->cShape->circle.getOutlineColor(), m_enemyConfig.OT / 2.f);
		smallEntity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2.f);
		smallEntity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
		smallEntity->cScore = std::make_shared<CScore>(200);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity>entity, const Vec2& target) {
	float bulletSpeed = m_bulletConfig.S;
	Vec2 n = (target - entity->cTransform->pos).normalize();
	Vec2 velocity(bulletSpeed * n.x, bulletSpeed * n.y);

	auto e = m_entities.addEntity("bullet");
	e->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, velocity, 0.f);
	e->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	e->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
	e->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity) {
	if (m_currentFrame - m_lastSpecialAttackTime < 300) return;
	int n = 12;
	float s = 20.f;
	for (int i = 0; i < n; i++) {
		float angle = (360.f / n) * i;
		float rad = angle * 3.14159f / 180;

		Vec2 velocity(s * cosf(rad), s * sinf(rad));

		auto e = m_entities.addEntity("bullet");
		e->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, velocity, 0.f);
		e->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(0, 0, 0), sf::Color(255, 0, 0), m_bulletConfig.OT);
		e->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L / 2);
		e->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
	}
	m_lastSpecialAttackTime = m_currentFrame;
}

bool Game::hitLeft(std::shared_ptr<Entity> e) {
	return (e->cTransform->pos.x <= e->cCollision->radius);
}

bool Game::hitRight(std::shared_ptr<Entity> e) {
	return (e->cTransform->pos.x >= m_window.getSize().x - e->cCollision->radius);
}

bool Game::hitTop(std::shared_ptr<Entity> e) {
	return (e->cTransform->pos.y <= e->cCollision->radius);
}

bool Game::hitBottom(std::shared_ptr<Entity> e) {
	return (e->cTransform->pos.y >= m_window.getSize().y - e->cCollision->radius);
}

bool Game::isCollided(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2) const {
	return (e1->cTransform->pos.dist(e2->cTransform->pos) <= e1->cCollision->radius + e2->cCollision->radius);
}

void Game::sMovement() {
	m_player->cTransform->velocity = { 0,0 };

	if (m_player->cInput->up) m_player->cTransform->velocity.y = -5.f;
	else if (m_player->cInput->down) m_player->cTransform->velocity.y = 5.f;

	if (m_player->cInput->left) m_player->cTransform->velocity.x = -5.f;
	else if (m_player->cInput->right) m_player->cTransform->velocity.x = 5.f;

	if (m_player->cTransform->velocity.length() != 0) {
		m_player->cTransform->velocity = m_player->cTransform->velocity.normalize() * 5;
	}

	for (auto e : m_entities.getEntities()) {
		e->cTransform->pos += e->cTransform->velocity;
	}
}

void Game::sLifespan() {
	for (auto e : m_entities.getEntities()) {
		if (e->cLifespan) {
			e->cLifespan->remaining--;
			if (e->cLifespan->remaining > 0) {
				sf::Color newC = e->cShape->circle.getFillColor();
				sf::Color newBC = e->cShape->circle.getOutlineColor();

				newC.a = e->cLifespan->remaining / e->cLifespan->total * 255;
				newBC.a = newC.a;

				e->cShape->circle.setFillColor(newC);
				e->cShape->circle.setOutlineColor(newBC);
			}
			else e->destroy();
		}
	}
}

void Game::sCollision() {
	if (hitTop(m_player)) m_player->cTransform->pos.y = m_player->cCollision->radius;
	else if (hitBottom(m_player)) m_player->cTransform->pos.y = m_window.getSize().y - m_player->cCollision->radius;

	if (hitLeft(m_player)) m_player->cTransform->pos.x = m_player->cCollision->radius;
	else if (hitRight(m_player)) m_player->cTransform->pos.x = m_window.getSize().x - m_player->cCollision->radius;

	for (auto e : m_entities.getEntities("enemy")) {
		if (hitTop(e) || hitBottom(e)) e->cTransform->velocity.y *= -1.f;
		if (hitLeft(e) || hitRight(e)) e->cTransform->velocity.x *= -1.f;

		for (auto b : m_entities.getEntities("bullet")) {
			if (isCollided(b, e)) {
				m_score += e->cScore->score;
				spawnSmallEntities(e);
				e->destroy();
				b->destroy();
			}
		}
		if (isCollided(m_player, e) && !e->cLifespan) {
			spawnSmallEntities(e);
			reset();
		}
	}
}

void Game::sEnemySpawner() {
	if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.SI) spawnEnemy();
}

void Game::sRender() {
	m_window.clear();

	for (auto e : m_entities.getEntities()) {
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		e->cTransform->angle += 2.5f;
		e->cShape->circle.setRotation(e->cTransform->angle);
		m_window.draw(e->cShape->circle);
	}

	if (m_score > m_highscore) m_highscore = m_score;
	std::string scoreStr = "Score-> " + std::to_string(m_score) + "\nHighscore-> " + std::to_string(m_highscore);
	m_text.setString(scoreStr);
	m_window.draw(m_text);

	m_window.display();
}

void Game::sUserInput() {
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			m_running = false;
		}

		if (event.type == sf::Event::KeyPressed) {
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
			case sf::Keyboard::P:
				m_paused = !m_paused;
				break;
			case sf::Keyboard::Escape:
				m_running = false;
				break;
			default:
				break;
			}

		}

		if (event.type == sf::Event::KeyReleased) {
			switch (event.key.code) {
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
			default:
				break;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed && !m_paused) {
			if (event.mouseButton.button == sf::Mouse::Left) {
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
			if (event.mouseButton.button == sf::Mouse::Right) {
				spawnSpecialWeapon(m_player);
			}
		}
	}
}

void Game::reset() {
	for (auto e : m_entities.getEntities()) {
		e->destroy();
	}
	spawnPlayer();
	m_score = 0;
}