#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SceneEntityManager.generated.h"

USTRUCT(BlueprintType)
struct FSceneEntityData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int64 EntityId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 EntityType = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float RotationYaw = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 Hp = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxHp = 0;

	UPROPERTY(BlueprintReadWrite)
	bool bVisible = true;
};

UCLASS()
class MYCAMPUSGAME_API USceneEntityManager : public UObject
{
	GENERATED_BODY()

public:
	void AddEntity(int64 EntityId, const FSceneEntityData& Data);
	void RemoveEntity(int64 EntityId);
	void UpdateEntity(int64 EntityId, const FSceneEntityData& Data);
	FSceneEntityData* GetEntity(int64 EntityId);
	const TMap<int64, FSceneEntityData>& GetAllEntities() const { return Entities; }
	void ClearAllEntities();

	UFUNCTION(BlueprintCallable)
	int32 GetEntityCount() const { return Entities.Num(); }

private:
	UPROPERTY()
	TMap<int64, FSceneEntityData> Entities;
};