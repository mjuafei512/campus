#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SceneManagerSubsystem.generated.h"

UCLASS()
class MYCAMPUSGAME_API USceneManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void EnterScene(int32 SceneId);

	UFUNCTION(BlueprintCallable)
	void LeaveScene();

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentSceneId() const { return CurrentSceneId; }

	UFUNCTION(BlueprintCallable)
	bool IsInScene() const { return CurrentSceneId > 0; }

private:
	int32 CurrentSceneId = 0;
};