// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "Interaction/GodGameTagComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GodGameTagComponent)

// Constructor.
UGodGameTagComponent::UGodGameTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Display name presented for the component owner.
FName UGodGameTagComponent::GetDisplayName() const
{
	if(!DisplayName.IsNone())
	{
		return DisplayName;
	}

	return(GetOwner() ? FName(GetOwner()->GetName()) : NAME_None);
}

// Whether God Game interaction may select the component owner.
bool UGodGameTagComponent::GetIsSelectable() const
{
	return bSelectable;
}
