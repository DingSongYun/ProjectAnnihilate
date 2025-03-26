// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "NeActorFactoryCharacterAsset.generated.h"

/**
 * UActorFactoryCharacterAsset
 *
 * 支持CharacterAsset拖拽到场景内自动生成Actor
 */
UCLASS()
class NECHARACTERDATAEDITOR_API UNeActorFactoryCharacterAsset : public UActorFactory
{
	GENERATED_BODY()
public:
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual UClass* GetDefaultActorClass(const FAssetData& AssetData) override;
};