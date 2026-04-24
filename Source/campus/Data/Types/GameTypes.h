#pragma once

#include "CoreMinimal.h"
#include "GameTypes.generated.h"

USTRUCT(BlueprintType)
struct FCharacterInfoBP
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 CharacterId;

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	int32 Level;
};

USTRUCT(BlueprintType)
struct FPlayerInfoBP
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 UserId;

	UPROPERTY(BlueprintReadOnly)
	FString Username;

	UPROPERTY(BlueprintReadOnly)
	FString Token;
};

USTRUCT(BlueprintType)
struct FSceneSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 SceneId = 0;

	UPROPERTY(BlueprintReadWrite)
	TMap<int64, int32> Entities;
};