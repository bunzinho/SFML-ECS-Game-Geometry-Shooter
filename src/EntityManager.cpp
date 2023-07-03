#include "EntityManager.h"

EntityManager::EntityManager()
{

}

void EntityManager::update()
{
	removeDeadEntities(m_entities);

	for (auto& [tag, entities] : m_entityMap)
	{
		removeDeadEntities(entities);
	}

	for (auto& e : m_entitiesToAdd)
	{
		m_entities.push_back(e);
		m_entityMap[e->tag()].push_back(e);
	}
	m_entitiesToAdd.clear();
}

void EntityManager::removeDeadEntities(EntityVec& entities)
{
	const auto isNotActive = [](const std::shared_ptr<Entity>& e)
	{
		return !e->isActive();
	};
	auto remove = std::remove_if(entities.begin(), entities.end(), isNotActive);
	entities.erase(remove, entities.end());
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	const auto e = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
	m_entitiesToAdd.push_back(e);
	return e;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}

void EntityManager::clearEntitiesByTag(const std::string& tag)
{
	m_entityMap[tag].clear();
}
