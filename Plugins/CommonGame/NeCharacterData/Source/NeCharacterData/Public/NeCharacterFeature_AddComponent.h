// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeCharacterFeatureAction.h"
#include "NeCharacterFeature_AddComponent.generated.h"

/**
 *
 */
UCLASS(DisplayName="添加组件")
class NECHARACTERDATA_API UNeCharacterFeature_AddComponent : public UNeCharacterFeatureAction
{
	GENERATED_BODY()

public:
	UNeCharacterFeature_AddComponent(const FObjectInitializer& Initializer = FObjectInitializer::Get());
	virtual void ApplyFeature(ACharacter* InOwnerCharacter) override;
};
