#include "Scene/SceneEntityManager.h"

void USceneEntityManager::AddEntity(int64 EntityId, const FSceneEntityData& Data)
{
	Entities.Add(EntityId, Data);
}

void USceneEntityManager::RemoveEntity(int64 EntityId)
{
	Entities.Remove(EntityId);
}

void USceneEntityManager::UpdateEntity(int64 EntityId, const FSceneEntityData& Data)
{
	if (Entities.Contains(EntityId))
	{
		Entities[EntityId] = Data;
	}
}

FSceneEntityData* USceneEntityManager::GetEntity(int64 EntityId)
{
	return Entities.Find(EntityId);
}

void USceneEntityManager::ClearAllEntities()
{
	Entities.Empty();
}