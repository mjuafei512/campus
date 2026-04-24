#pragma once

#include "CoreMinimal.h"
#include "NetworkTypes.generated.h"

USTRUCT(BlueprintType)
struct FProtoRoute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 MsgId;

	UPROPERTY(BlueprintReadOnly)
	FString Name;
};

UENUM(BlueprintType)
enum class EGameNetworkState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Handshaking,
	Authenticated,
	InGame,
	Reconnecting,
	Closing
};