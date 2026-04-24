#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameNetworkSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthSuccess, const FString&, Token, int64, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTcpConnected, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameMessage, int32, MsgId, const TArray<uint8>&, Payload);

class UTcpGameClient;
class UAuthService;
class UProtocolManager;
class UMessageDispatcher;

UCLASS()
class MYCAMPUSGAME_API UGameNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable)
	FOnAuthSuccess OnAuthSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnTcpConnected OnTcpConnected;

	UPROPERTY(BlueprintAssignable)
	FOnGameMessage OnGameMessage;

	UFUNCTION(BlueprintCallable, Category="Network")
	void SetSessionToken(const FString& InToken, int64 InUserId);

	UFUNCTION(BlueprintCallable, Category="Network")
	void ClearSession();

	UFUNCTION(BlueprintCallable, Category="Network")
	bool LoginAndConnect(const FString& Username, const FString& Password);

	UFUNCTION(BlueprintCallable, Category="Network")
	bool ConnectGameServer();

	UFUNCTION(BlueprintCallable, Category="Network")
	bool SendHandshake();

	UFUNCTION(BlueprintCallable, Category="Network")
	bool SendHeartbeat();

	UFUNCTION(BlueprintCallable, Category="Network")
	bool SendChat(const FString& Content, int32 Channel = 1);

	UFUNCTION(BlueprintCallable, Category="Network")
	FString GetToken() const { return SessionToken; }

	UFUNCTION(BlueprintCallable, Category="Network")
	int64 GetUserId() const { return SessionUserId; }

private:
	FString SessionToken;
	int64 SessionUserId = 0;

	UPROPERTY()
	TObjectPtr<UTcpGameClient> TcpClient;

	UPROPERTY()
	TObjectPtr<UAuthService> AuthService;

	UPROPERTY()
	TObjectPtr<UProtocolManager> ProtocolManager;

	UPROPERTY()
	TObjectPtr<UMessageDispatcher> MessageDispatcher;

	void HandleTcpPacket(int32 MsgId, const TArray<uint8>& Payload);
};