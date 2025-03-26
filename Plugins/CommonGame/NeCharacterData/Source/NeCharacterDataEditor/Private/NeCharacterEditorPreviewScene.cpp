// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Editor.h"
#include "NeCharacterEditorPreviewScene.h"
#include "Components/DirectionalLightComponent.h"
#include "GameFramework/WorldSettings.h"
#include "NeCharacterAsset.h"
#include "NeCharacterDataComponent.h"
#include "GameFramework/Character.h"

FNeCharacterEditorPreviewScene::FNeCharacterEditorPreviewScene(ConstructionValues CVS) : FAdvancedPreviewScene(CVS)
{
	if (GEditor)
		GEditor->RegisterForUndo(this);

	AWorldSettings* WorldSetting = GetWorld()->GetWorldSettings(true);
	WorldSetting->bEnableWorldBoundsChecks = false;
	WorldSetting->SetFlags(RF_Transactional);

	PreviewActors.Empty();

	// 辅助灯光
	SubDirectionalLight = NewObject<UDirectionalLightComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	AddComponent(SubDirectionalLight, FTransform(DirectionalLight->GetRelativeRotation()));

	SubDirectionalLight->Intensity = DirectionalLight->Intensity;
	SubDirectionalLight->LightColor = DirectionalLight->LightColor;
	SubDirectionalLight->SetRelativeLocation(DirectionalLight->GetRelativeLocation());
	SubDirectionalLight->SetRelativeScale3D(DirectionalLight->GetRelativeScale3D());
	SubDirectionalLight->Mobility = DirectionalLight->Mobility;
	SubDirectionalLight->DynamicShadowDistanceStationaryLight = DirectionalLight->DynamicShadowDistanceStationaryLight;
	SubDirectionalLight->SetLightBrightness(DirectionalLight->ComputeLightBrightness());
	SubDirectionalLight->InvalidateLightingCache();
	SubDirectionalLight->RecreateRenderState_Concurrent();
	SubDirectionalLight->SetLightingChannels(false, true, false);
}

FNeCharacterEditorPreviewScene::~FNeCharacterEditorPreviewScene()
{
	UWorld* PrevWorld = GetWorld();
	if (GEditor)
		GEditor->UnregisterForUndo(this);
}

void FNeCharacterEditorPreviewScene::CreatePreviewActors(UNeCharacterAsset* InCharacterAsset)
{
	check(InCharacterAsset);

	if (PreviewActors.Num() > 0)
	{
		DestroyPreviewActors();
	}

	// Actor出生配置
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = false;
	Params.bAllowDuringConstructionScript = true;
	Params.OverrideLevel = PreviewWorld->GetLevel(0);
	Params.ObjectFlags |= (RF_TextExportTransient | RF_NonPIEDuplicateTransient);
	Params.bDeferConstruction = true;
	
	UClass* ActorClass = InCharacterAsset->ActorClass.Get();
	// 调整预览角色的出生位置
	const FTransform Transform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, InCharacterAsset->CapsuleHalfHeight));
	APawn* NewActor = Cast<APawn>(PreviewWorld->SpawnActor(ActorClass, &Transform, Params));
	check(NewActor);
	if (NewActor)
	{
		if (UNeCharacterDataComponent* DataComponent = NewActor->FindComponentByClass<UNeCharacterDataComponent>())
		{
			DataComponent->SetCharacterAsset(InCharacterAsset);
			DataComponent->FlushCharacterDataEditorOnly();
		}
	}
	NewActor->FinishSpawning(Transform, false, nullptr, Params.TransformScaleMethod);
	PreviewActors.Add(NewActor);
	InCharacterAsset->SetPreviewActor(NewActor);
}

void FNeCharacterEditorPreviewScene::Tick(float InDeltaTime)
{
	FAdvancedPreviewScene::Tick(InDeltaTime);

	// if (!GIntraFrameDebuggingGameThread)
	// 	PreviewWorld->Tick(LEVELTICK_All, InDeltaTime);

	LastTickTime = FPlatformTime::Seconds();
}

bool FNeCharacterEditorPreviewScene::IsTickable() const
{
	const float VisibilityTimeThreshold = 0.25f;

	// The preview scene is tickable if any viewport can see it
	return  LastTickTime == 0.0	||	// Never been ticked
			FPlatformTime::Seconds() - LastTickTime <= VisibilityTimeThreshold;	// Ticked recently

}

void FNeCharacterEditorPreviewScene::DestroyPreviewActors()
{
	for (int32 i = 0; i < PreviewActors.Num(); ++i)
	{
		PreviewWorld->DestroyActor(PreviewActors[i]);
	}
	PreviewActors.Empty();
}
