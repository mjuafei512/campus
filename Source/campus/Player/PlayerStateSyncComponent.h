#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStateSyncComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYCAMPUSGAME_API UPlayerStateSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SyncPosition(float X, float Y, float Z);

	UFUNCTION(BlueprintCallable)
	void SyncCombatState(bool bInCombat, int64 TargetId);

	UFUNCTION(BlueprintCallable)
	FVector GetLastPosition() const { return LastPosition; }

	UFUNCTION(BlueprintCallable)
	bool IsInCombat() const { return bInCombat; }

	UFUNCTION(BlueprintCallable)
	int64 GetCurrentTargetId() const { return CurrentTargetId; }

private:
	FVector LastPosition = FVector::ZeroVector;
	bool bInCombat = false;
	int64 CurrentTargetId = 0;
};