#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ProtocolManager.generated.h"

UCLASS()
class MYCAMPUSGAME_API UProtocolManager : public UObject
{
	GENERATED_BODY()

public:
	TArray<uint8> BuildHandshakePayload(const FString& Token) const;
	TArray<uint8> BuildHeartbeatPayload(int64 UserId) const;
	TArray<uint8> BuildChatPayload(int64 UserId, const FString& Content, int32 Channel) const;
	TArray<uint8> BuildMovePayload(int64 UserId, float X, float Y, float Z) const;
};