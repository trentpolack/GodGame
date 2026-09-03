// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "NativeGameplayTags.h"

/**
 *	Native Tag Declarations for the GodGame plugin.
 *		A general-purpose set of tags that can apply to a wide variety of projects and provide a foundation and convention for more defined in native or in the Gameplay Tag Manager.
 */

/**
 *	Character Need Tags.
 *		Base set of Needs that villagers (or potentially other character types) have.
 */
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Need_Hunger);							// A character's need for food ([0.0, 1.0] where 1.0 is starving).
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Need_Rest);								// A character's need for sleep ([0.0, 1.0] where 1.0 is quite tired).
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Need_Fear);								// A character's need for safety ([0.0, 1.0] where 1.0 is scared).
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Need_Faith);								// A character's need for faith ([0.0, 1.0] where 1.0 has no faith).
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Need_Socialization);						// A character's need for socialization ([0.0, 1.0] where 1.0 needs socialization).

/**
 *	Miracle Type Tags.
 *		Base set of natively defined Miracles (should be minimal).
 */
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Miracle_Rain);							// Rain miracle.

/**
 *	Resource Type Tags.
 *		Base set of natively defined Resources available in game.
 */
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Resource_Food);							// Food resource type.
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Resource_Wood);							// Wood resource type.
GODGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GodGame_Resource_Power);							// "Power" resource type (used as overall god rating).
