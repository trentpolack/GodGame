// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "GodGameNativeGameplayTags.h"

/**
 *	Character Need Tags.
 *		Base set of Needs that villagers (or potentially other character types) have.
 */
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Need_Hunger, "GodGame.Need.Hunger", "A character's need for food ([0.0, 1.0] where 1.0 is starving).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Need_Rest, "GodGame.Need.Rest", "A character's need for sleep ([0.0, 1.0] where 1.0 is quite tired).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Need_Fear, "GodGame.Need.Fear", "A character's need for safety ([0.0, 1.0] where 1.0 is scared).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Need_Faith, "GodGame.Need.Faith", "A character's need for faith ([0.0, 1.0] where 1.0 has no faith).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Need_Socialization, "GodGame.Need.Socialization", "A character's need for socialization ([0.0, 1.0] where 1.0 needs socialization).");

// Need lifecycle events dispatched through JoyCore.
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Event_Need_Changed, "System.Event.GodGame.Need.Changed", "Event for character need changes.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Event_Need_Critical, "System.Event.GodGame.Need.Critical", "Event for a character's need being critical.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Event_Need_Recovered, "System.Event.GodGame.Need.Recovered", "Event for a character's need being recovered (non-critical).");

/**
 *	Miracle Type Tags.
 *		Base set of natively defined Miracles (should be minimal).
 */
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Miracle_Rain, "GodGame.Miracle.Rain", "Rain Miracle.");

/**
 *	Resource Type Tags.
 *		Base set of natively defined Resources available in game.
 */
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Resource_Food, "GodGame.Resource.Food", "Food resource type.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Resource_Wood, "GodGame.Resource.Wood", "Wood resource type.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GodGame_Resource_Power, "GodGame.Resource.Power", "\"Power\" resource type (used as overall god rating).");
