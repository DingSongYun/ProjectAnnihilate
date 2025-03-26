// Copyright NetEase Games, Inc. All Rights Reserved.


#include "NeActorFactoryCharacterAsset.h"

#include "Editor.h"
#include "LevelEditorViewport.h"
#include "NeCharacterAsset.h"
#include "NeCharacterDataComponent.h"
#include "GameFramework/Character.h"
#include "Subsystems/PlacementSubsystem.h"

bool UNeActorFactoryCharacterAsset::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	const UClass* AssetClass = AssetData.GetClass();
	if (AssetClass == nullptr) return false;
	return AssetClass->IsChildOf(UNeCharacterAsset::StaticClass());
}

AActor* UNeActorFactoryCharacterAsset::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams)
{
	ULevel* LocalLevel = ValidateSpawnActorLevel(InLevel, InSpawnParams);

	AActor* DefaultActor = GetDefaultActor(FAssetData(InAsset));
	if ((DefaultActor != nullptr) && (LocalLevel != nullptr))
	{
		FActorSpawnParameters SpawnParamsCopy(InSpawnParams);
		SpawnParamsCopy.OverrideLevel = LocalLevel;

		const bool bIsCreatingPreviewElements = FLevelEditorViewportClient::IsDroppingPreviewActor();
		bool bIsPlacementSystemCreatingPreviewElements = false;
		if (const UPlacementSubsystem* PlacementSubsystem = GEditor->GetEditorSubsystem<UPlacementSubsystem>())
		{
			bIsPlacementSystemCreatingPreviewElements = PlacementSubsystem->IsCreatingPreviewElements();
		}
		SpawnParamsCopy.bTemporaryEditorActor = bIsCreatingPreviewElements || bIsPlacementSystemCreatingPreviewElements;
		SpawnParamsCopy.bHideFromSceneOutliner = bIsPlacementSystemCreatingPreviewElements;

		// AActor* NewActor =  LocalLevel->OwningWorld->SpawnActorDeferred<>()
		SpawnParamsCopy.bDeferConstruction = true;
		AActor* NewActor = LocalLevel->OwningWorld->SpawnActor(DefaultActor->GetClass(), &InTransform, SpawnParamsCopy);
		NewActor->FinishSpawning(InTransform, false, nullptr, SpawnParamsCopy.TransformScaleMethod);
		if (UNeCharacterDataComponent* CharacterDataComponent = NewActor ? NewActor->FindComponentByClass<UNeCharacterDataComponent>() : nullptr)
		{
			CharacterDataComponent->SetCharacterAsset(Cast<UNeCharacterAsset>(InAsset));
			CharacterDataComponent->FlushCharacterDataEditorOnly();
		}
		return NewActor;
	}

	return nullptr;
}


void UNeActorFactoryCharacterAsset::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);
}

UClass* UNeActorFactoryCharacterAsset::GetDefaultActorClass(const FAssetData& AssetData)
{
	const UNeCharacterAsset* CharacterAsset = Cast<UNeCharacterAsset>(AssetData.GetAsset());
	if (CharacterAsset == nullptr) return ACharacter::StaticClass();
	return CharacterAsset->ActorClass;
}
