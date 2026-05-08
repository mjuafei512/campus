#include "Network/MessageDispatcher.h"

void UMessageDispatcher::Dispatch(int32 MsgId, const TArray<uint8>& Payload)
{
	switch (MsgId)
	{
	case 1002:
	{
		const FString Message = TEXT("Handshake Success");
		OnHandshakeSuccess.Broadcast(Message);
		break;
	}
	case 2002:
	{
		TArray<FString> Characters;
		Characters.Add(TEXT("Hero001"));
		Characters.Add(TEXT("Mage002"));
		OnCharacterList.Broadcast(Characters);
		break;
	}
	case 4002:
	{
		const int32 SceneId = 1001;
		OnEnterScene.Broadcast(SceneId);
		break;
	}
	case 7003:
	{
		const FString Content = TEXT("Hello from server");
		OnChatBroadcast.Broadcast(Content);
		break;
	}
	case 5003:
	{
		const int32 Damage = 25;
		OnDamageEvent.Broadcast(Damage);
		break;
	}
	case 8003:
	{
		const int64 MonsterId = 9001;
		OnMonsterDefeat.Broadcast(MonsterId);
		break;
	}
	case 6003:
	{
		const int64 DropId = 7001;
		OnDropPickup.Broadcast(DropId);
		break;
	}
	case 9004:
	{
		const int64 TaskId = 5001;
		OnTaskUpdate.Broadcast(TaskId);
		break;
	}
	case 11005:
	{
		const int64 PartyId = 3001;
		OnPartyUpdate.Broadcast(PartyId);
		break;
	}
	case 12004:
	{
		const int64 InstanceId = 12001;
		OnInstanceUpdate.Broadcast(InstanceId);
		break;
	}
	case 13005:
	{
		const int64 PetId = 6001;
		OnPetUpdate.Broadcast(PetId);
		break;
	}
	case 14004:
	{
		const int32 Level = 5;
		OnUpgradeUpdate.Broadcast(Level);
		break;
	}
	default:
		break;
	}
}