#include "Network/GameNetworkSubsystem.h"
#include "Network/AuthService.h"
#include "Network/ProtocolManager.h"
#include "Network/TcpGameClient.h"
#include "Network/MessageDispatcher.h"
#include "Engine/Engine.h"

void UGameNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	AuthService = NewObject<UAuthService>(this);
	ProtocolManager = NewObject<UProtocolManager>(this);
	TcpClient = NewObject<UTcpGameClient>(this);
	MessageDispatcher = NewObject<UMessageDispatcher>(this);

	if (TcpClient)
	{
		TcpClient->OnPacket.BindUObject(this, &UGameNetworkSubsystem::HandleTcpPacket);
	}
}

void UGameNetworkSubsystem::Deinitialize()
{
	if (TcpClient)
	{
		TcpClient->Disconnect();
	}

	Super::Deinitialize();
}

void UGameNetworkSubsystem::SetSessionToken(const FString& InToken, int64 InUserId)
{
	SessionToken = InToken;
	SessionUserId = InUserId;
	OnAuthSuccess.Broadcast(SessionToken, SessionUserId);
}

void UGameNetworkSubsystem::ClearSession()
{
	SessionToken.Empty();
	SessionUserId = 0;
}

bool UGameNetworkSubsystem::LoginAndConnect(const FString& Username, const FString& Password)
{
	if (!AuthService)
	{
		return false;
	}

	return AuthService->Login(
		Username,
		Password,
		FOnLoginResult::CreateLambda([this](const FString& Token, int64 UserId)
		{
			SetSessionToken(Token, UserId);
			ConnectGameServer();
		})
	);
}

bool UGameNetworkSubsystem::ConnectGameServer()
{
	if (!TcpClient)
	{
		return false;
	}

	const bool bConnected = TcpClient->Connect(TEXT("127.0.0.1"), 9000);
	OnTcpConnected.Broadcast(bConnected);

	if (bConnected)
	{
		SendHandshake();
	}

	return bConnected;
}

bool UGameNetworkSubsystem::SendHandshake()
{
	if (!TcpClient || !ProtocolManager || SessionToken.IsEmpty())
	{
		return false;
	}

	const TArray<uint8> Payload = ProtocolManager->BuildHandshakePayload(SessionToken);
	return TcpClient->SendPacket(1001, Payload);
}

bool UGameNetworkSubsystem::SendHeartbeat()
{
	if (!TcpClient || SessionUserId <= 0)
	{
		return false;
	}

	const TArray<uint8> Payload = ProtocolManager->BuildHeartbeatPayload(SessionUserId);
	return TcpClient->SendPacket(3001, Payload);
}

bool UGameNetworkSubsystem::SendChat(const FString& Content, int32 Channel)
{
	if (!TcpClient || !ProtocolManager || SessionUserId <= 0)
	{
		return false;
	}

	const TArray<uint8> Payload = ProtocolManager->BuildChatPayload(SessionUserId, Content, Channel);
	return TcpClient->SendPacket(7001, Payload);
}

void UGameNetworkSubsystem::HandleTcpPacket(int32 MsgId, const TArray<uint8>& Payload)
{
	OnGameMessage.Broadcast(MsgId, Payload);

	if (MessageDispatcher)
	{
		MessageDispatcher->Dispatch(MsgId, Payload);
	}
}