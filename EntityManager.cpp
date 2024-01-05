#include"EntityManager.h"
#include<iostream>

EntityManager::EntityManager() {}

EntityVector& EntityManager::getEntities() {
	return m_entities;
}

EntityVector& EntityManager::getEntities(const std::string& tag) {
	return m_entityMap[tag];
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag) {
	auto e = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
	m_toAdd.push_back(e);
	return e;
}

void EntityManager::removeDeadEntities(EntityVector& ev) {
	ev.erase(std::remove_if(ev.begin(), ev.end(), [](auto e) { return (e->isActive() == false); }), ev.end());
}

void EntityManager::update() {
	//add ents
	for (auto e : m_toAdd) {
		m_entities.push_back(e);
		m_entityMap[e->tag()].push_back(e);
	}
	m_toAdd.clear();

	//remove ents
	removeDeadEntities(m_entities);
	for (auto& p : m_entityMap) {
		removeDeadEntities(p.second);
	}
}