// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeCharacterFeatureAction.h"
#include "NeCharacterFeature_MakeAvatar.generated.h"

/**
 * UNeCharacterFeature_MakeAvatar
 *
 * 角色Avatar组装
 */
UCLASS(BlueprintType, DisplayName="Make Avatar")
class NECHARACTERDATA_API UNeCharacterFeature_MakeAvatar : public UNeCharacterFeatureAction
{
	GENERATED_BODY()

public:
	UNeCharacterFeature_MakeAvatar(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	virtual void ApplyFeature(ACharacter* InOwnerCharacter) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance, Instanced)
	class UAvatarMakeData* AvatarData = nullptr;
};
