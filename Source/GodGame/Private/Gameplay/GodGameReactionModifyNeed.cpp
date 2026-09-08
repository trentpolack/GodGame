// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Gameplay/GodGameReactionModifyNeed.h"

#include "GameFramework/Actor.h"

#include "Systems/Rules/SystemicRuleContext.h"

#include "GodGame.h"
#include "Simulation/CharacterNeedsComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameReactionModifyNeed)

bool UGodGameReactionModifyNeed::Execute(const FSystemicEvent& Event, FSystemicRuleContext& Context, FSystemicTrace& Trace)
{
	if(!bEnabled)
	{
		// Technically successful since this reaction is disabled.
		return true;
	}
	
	if(!NeedTag.IsValid())
	{
		UE_LOG(LogGodGame, Error, TEXT("ERROR [UGodGameReactionModifyNeed]: Invalid Need Tag supplied to the reaction execution: %hs."), __FUNCTION__);
		return false;
	}

	// Get the target of the event and its Needs component.
	UObject* pTarget = Event.Target.IsValid() ? Event.Target.Get() : Context.Target.Get();
	UCharacterNeedsComponent* pNeeds = Cast<UCharacterNeedsComponent>(pTarget);

	if(!pNeeds)
	{
		// The target didn't have a Needs component; look at its owning actor.
		const UActorComponent* pActorComponent = Cast<UActorComponent>(pTarget);
		const AActor* pTargetActor = pActorComponent ? pActorComponent->GetOwner() : Cast<AActor>(pTarget);
		pNeeds = IsValid(pTargetActor) ? pTargetActor->FindComponentByClass<UCharacterNeedsComponent>() : nullptr;
	}

	if(!IsValid(pNeeds))
	{
		// The target and/or its owning actor don't have a Needs component.
		Trace.RuleReactionNameAndResultList.Add(TPair<FName, bool>(GetReactionName(), false));
	
		UE_LOG(LogGodGame, Error, TEXT("ERROR [UGodGameReactionModifyNeed]: This requires a CharacterNeedsComponent on the target or its owning actor: %hs."), __FUNCTION__);
		return false;
	}
	
	// Determine the need delta.
	const FSystemicEventData* pData = Event.EventDataInstance.GetPtr<FSystemicEventData>();
	const float needModifierFinal = NeedModifier*((bScaleByEventValue && pData) ? pData->Value : 1.0f);

	// Success means the need delta is non-zero.
	const bool bSuccess = !FMath::IsNearlyEqual(needModifierFinal, pData->Value);
	if(bSuccess)
	{
		// Modify the need.
		pNeeds->ModifyNeed(NeedTag, needModifierFinal);
	}

	// Record the result of the operation.
	Trace.RuleReactionNameAndResultList.Add(TPair<FName, bool>(GetReactionName(), bSuccess));
	return bSuccess;
}
