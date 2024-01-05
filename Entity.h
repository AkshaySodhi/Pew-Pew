#pragma once

#include "Components.h"
#include<memory>
#include<string>

class Entity {
	friend class EntityManager;

	size_t m_id = 0;
	std::string m_tag = "default";
	bool m_active = true;

	Entity(const size_t id, const std::string& tag);

public:

	std::shared_ptr<CTransform> cTransform;
	std::shared_ptr<CShape> cShape;
	std::shared_ptr<CCollision> cCollision;
	std::shared_ptr<CInput> cInput;
	std::shared_ptr<CScore> cScore;
	std::shared_ptr<CLifespan> cLifespan;

	const size_t id() const;
	const std::string& tag() const;
	bool isActive() const;
	void destroy();
};