#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Sockets.h"
#include "TcpGameClient.generated.h"

DECLARE_DELEGATE_TwoParams(FOnTcpPacket, int32 /*MsgId*/, const TArray<uint8>& /*Payload*/);

UCLASS()
class MYCAMPUSGAME_API UTcpGameClient : public UObject
{
	GENERATED_BODY()

public:
	bool Connect(const FString& Host, int32 Port);
	void Disconnect();
	bool SendPacket(int32 MsgId, const TArray<uint8>& Payload);
	void PumpReceive();

	FOnTcpPacket OnPacket;

private:
	FSocket* Socket = nullptr;
	TArray<uint8> ReceiveBuffer;

	bool ReadExact(uint8* Dest, int32 NumBytes);
	void ParseBuffer();
};