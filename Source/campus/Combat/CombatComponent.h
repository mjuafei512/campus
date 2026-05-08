#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYCAMPUSGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SendAttack(int64 TargetId, int32 SkillId);

	UFUNCTION(BlueprintCallable)
	void SendSkill(int32 SkillId);

	UFUNCTION(BlueprintCallable)
	bool IsInCombat() const { return bIsInCombat; }

	UFUNCTION(BlueprintCallable)
	int64 GetCurrentTargetId() const { return CurrentTargetId; }

	UFUNCTION(BlueprintCallable)
	void SetCombatState(bool bInCombat);

private:
	UPROPERTY()
	bool bIsInCombat = false;

	UPROPERTY()
	int64 CurrentTargetId = 0;
};