#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MessageDispatcher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHandshakeSuccessBP, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterListBP, const TArray<FString>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnterSceneBP, int32, SceneId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatBroadcastBP, const FString&, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageEventBP, int32, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDefeatBP, int64, MonsterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropPickupBP, int64, DropId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskUpdateBP, int64, TaskId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyUpdateBP, int64, PartyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceUpdateBP, int64, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetUpdateBP, int64, PetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeUpdateBP, int32, Level);

UCLASS()
class MYCAMPUSGAME_API UMessageDispatcher : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnHandshakeSuccessBP OnHandshakeSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterListBP OnCharacterList;

	UPROPERTY(BlueprintAssignable)
	FOnEnterSceneBP OnEnterScene;

	UPROPERTY(BlueprintAssignable)
	FOnChatBroadcastBP OnChatBroadcast;

	UPROPERTY(BlueprintAssignable)
	FOnDamageEventBP OnDamageEvent;

	UPROPERTY(BlueprintAssignable)
	FOnMonsterDefeatBP OnMonsterDefeat;

	UPROPERTY(BlueprintAssignable)
	FOnDropPickupBP OnDropPickup;

	UPROPERTY(BlueprintAssignable)
	FOnTaskUpdateBP OnTaskUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnPartyUpdateBP OnPartyUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnInstanceUpdateBP OnInstanceUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnPetUpdateBP OnPetUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnUpgradeUpdateBP OnUpgradeUpdate;

	void Dispatch(int32 MsgId, const TArray<uint8>& Payload);
};