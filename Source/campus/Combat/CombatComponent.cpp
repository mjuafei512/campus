#include "Combat/CombatComponent.h"

void UCombatComponent::SendAttack(int64 TargetId, int32 SkillId)
{
	CurrentTargetId = TargetId;
	bIsInCombat = true;
}

void UCombatComponent::SendSkill(int32 SkillId)
{
}

void UCombatComponent::SetCombatState(bool bInCombat)
{
	bIsInCombat = bInCombat;
	if (!bInCombat)
	{
		CurrentTargetId = 0;
	}
}