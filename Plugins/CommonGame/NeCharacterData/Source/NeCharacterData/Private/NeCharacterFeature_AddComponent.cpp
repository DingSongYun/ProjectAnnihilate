// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterFeature_AddComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeCharacterFeature_AddComponent)

UNeCharacterFeature_AddComponent::UNeCharacterFeature_AddComponent(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

void UNeCharacterFeature_AddComponent::ApplyFeature(ACharacter* InOwnerCharacter)
{
	Super::ApplyFeature(InOwnerCharacter);
}
