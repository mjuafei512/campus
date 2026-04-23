#include "Scene/SceneManagerSubsystem.h"

void USceneManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USceneManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USceneManagerSubsystem::EnterScene(int32 SceneId)
{
	CurrentSceneId = SceneId;
}

void USceneManagerSubsystem::LeaveScene()
{
	CurrentSceneId = 0;
}