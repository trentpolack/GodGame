#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "GodGameReactionInterface.generated.h"

// Marker UClass for the Blueprint-friendly systemic reaction interface.
UINTERFACE(BlueprintType)
class GODGAME_API UGodGameReactionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Content contract for systemic reactions such as Rain, Fire, Fear, or Blessing.
 *	TODO (trent, 8/24/26): Revisit this.
 */
class GODGAME_API IGodGameReactionInterface
{
	GENERATED_BODY()

public:
	/**
	 * Determines whether this object accepts a systemic God Game reaction.
	 * @param ReactionTag The reaction type being offered.
	 * @param SourceActor The Actor that originated the reaction, if any.
	 * @return True when ReceiveGodGameReaction should be invoked.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GodGame|Reaction")
	bool CanReceiveGodGameReaction(FGameplayTag ReactionTag, AActor* SourceActor) const;

	/**
	 * Default native implementation that accepts every reaction.
	 * @return True when ReceiveGodGameReaction should be invoked.
	 */
	virtual bool CanReceiveGodGameReaction_Implementation(FGameplayTag ReactionTag, AActor* SourceActor) const
	{
		return true;
	}

	/**
	 * Handles a systemic God Game reaction accepted by CanReceiveGodGameReaction.
	 * @param ReactionTag The gameplay tag identifying the reaction type.
	 * @param Strength The reaction intensity supplied by the source.
	 * @param WorldLocation The world-space origin of the reaction.
	 * @param SourceActor The actor that originated the reaction, if any.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GodGame|Reaction")
	void ReceiveGodGameReaction(FGameplayTag ReactionTag, float Strength, FVector WorldLocation, AActor* SourceActor);

	/** 
	 * Default native implementation with no reaction behavior.
	 */
	virtual void ReceiveGodGameReaction_Implementation(FGameplayTag ReactionTag, float Strength, FVector WorldLocation, AActor* SourceActor)
	{
	}
};
