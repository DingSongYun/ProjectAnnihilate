// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NeCharacterFeatureAction.generated.h"

class ACharacter;

/**
 * UNeCharacterFeatureAction
 *
 * 用于模块化组装Character功能
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, HideDropdown)
class NECHARACTERDATA_API UNeCharacterFeatureAction : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void ApplyFeature(ACharacter* InOwnerCharacter);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="ApplyFeature")
	void ReceiveApplyFeature(ACharacter* InOwnerCharacter);
};
