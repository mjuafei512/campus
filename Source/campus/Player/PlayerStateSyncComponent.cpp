#include "Player/PlayerStateSyncComponent.h"

void UPlayerStateSyncComponent::SyncPosition(float X, float Y, float Z)
{
	LastPosition = FVector(X, Y, Z);
}

void UPlayerStateSyncComponent::SyncCombatState(bool bInCombatState, int64 TargetId)
{
	bInCombat = bInCombatState;
	CurrentTargetId = TargetId;
}