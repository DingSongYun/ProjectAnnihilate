// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterAsset.h"
#include "NeAvatarData.h"
#include "NeCharacterFeature_MakeAvatar.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeCharacterAsset)

UNeCharacterAsset::UNeCharacterAsset(const FObjectInitializer& Initializer) : Super(Initializer)
{
#if WITH_EDITORONLY_DATA
	Name = FText::GetEmpty();
#endif
	ActorClass = ACharacter::StaticClass();
	AvatarData = CreateDefaultSubobject<UNeCharacterFeature_MakeAvatar>(TEXT("Feature_MakeAvatar"));
}

FPrimaryAssetId UNeCharacterAsset::GetPrimaryAssetId() const
{
	const FName AssetType = UNeCharacterAsset::StaticClass()->GetFName();
	const FName AssetName = GetFName();

	return FPrimaryAssetId(AssetType, AssetName);
}

#if WITH_EDITOR
void UNeCharacterAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
