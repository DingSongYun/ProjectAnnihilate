// Copyright NetEase Games, Inc. All Rights Reserved.

#include "../Public/NeCharacterFeature_MakeAvatar.h"

#include "NeAvatarData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeCharacterFeature_MakeAvatar)

UNeCharacterFeature_MakeAvatar::UNeCharacterFeature_MakeAvatar(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	AvatarData = CreateDefaultSubobject<UAvatarMakeSimple>(TEXT("AvatarData"));
}

void UNeCharacterFeature_MakeAvatar::ApplyFeature(ACharacter* InOwnerCharacter)
{
	if (IsValid(AvatarData))
	{
		AvatarData->MakeAvatar(InOwnerCharacter);
	}
}

