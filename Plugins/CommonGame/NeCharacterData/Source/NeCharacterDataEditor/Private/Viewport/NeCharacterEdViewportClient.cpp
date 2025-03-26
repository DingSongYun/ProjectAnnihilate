// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterEditor.h"
#include "NeCharacterEditorPreviewScene.h"
#include "NeCharacterEditorPreviewSettings.h"
#include "Selection.h"
#include "Editor/UnrealEdEngine.h"
#include "EngineUtils.h"
#include "TabFactory/SNeCharacterEdViewport.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Viewport/NeCharacterEdViewportClient.h"

#define LOCTEXT_NAMESPACE "CharacterEdViewport"

FNeCharacterEdViewportClient::FNeCharacterEdViewportClient
(
	const TSharedRef<FAssetEditorToolkit> InAssetEditorToolkit, const TSharedRef<FPreviewScene>& InPreviewScene,
	const TSharedRef<SEditorViewport>& InViewport, const TSharedPtr<FNeCharacterEditor>& InEditorPtr
) : FNeSimpleEdViewportClient(&GLevelEditorModeTools(), &InPreviewScene.Get(), InViewport)
{ 
	AddRealtimeOverride(true, LOCTEXT("RealtimeOverrideMessage_CharacterEd", "Character Editor"));
	//SetRealtimeOverride(true, LOCTEXT("RealtimeOverrideMessage_CharacterEd", "Character Editor"));
	if(GEditor->PlayWorld)
	{
		AddRealtimeOverride(false, LOCTEXT("RealtimeOverrideMessage_CharacterEd", "Character Editor"));
	}

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);
	EngineShowFlags.SetSeparateTranslucency(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetSelectionOutline(true);

	EngineShowFlags.DisableAdvancedFeatures();

	// Init view default Location & Rotation
	TSharedRef<FNeCharacterEditor> CharacterEditor = StaticCastSharedRef<FNeCharacterEditor>(InAssetEditorToolkit);
	TSharedRef<FNeCharacterEditorPreviewScene> CharacterPreviewScene = StaticCastSharedRef<FNeCharacterEditorPreviewScene>(InPreviewScene);
	UNeCharacterEditorPreviewSetting* PrevSettings = CharacterEditor->GetAdditionalPreviewSettings();
	AActor* PreviewActor = CharacterPreviewScene->GetPreviewActors().Num() > 0 ? CharacterPreviewScene->GetPreviewActors()[0] : NULL;
	if (PrevSettings && PreviewActor)
	{
		if (PreviewActor && PrevSettings->bDefaultViewTransformRelateCharacter)
		{
			FTransform Transform = PreviewActor->GetActorTransform();
			const FVector ViewLoc = Transform.TransformPosition(PrevSettings->DefaultViewLocation);
			const FQuat ViewQuat = (PreviewActor->GetActorLocation() - ViewLoc).ToOrientationQuat() * PrevSettings->DefaultViewRotation.Quaternion();
			SetViewLocation(ViewLoc);
			SetViewRotation(ViewQuat.Rotator());
		}
		else
		{
			SetViewLocation(PrevSettings->DefaultViewLocation);
			SetViewRotation(PrevSettings->DefaultViewRotation);
		}
	}
	
	CharacterEdPtr = InEditorPtr;
}

FNeCharacterEdViewportClient::~FNeCharacterEdViewportClient()
{

}

void FNeCharacterEdViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	FNeSimpleEdViewportClient::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);

	if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
	{
		HActor* ActorHitProxy = (HActor*)HitProxy;
		AActor* ConsideredActor = ActorHitProxy->Actor;
		if (ConsideredActor)
		{
			while (ConsideredActor->IsChildActor())
			{
				ConsideredActor = ConsideredActor->GetParentActor();
			}

			GEditor->GetSelectedActors()->Modify();
			GEditor->SelectNone(false, true, false);
			GEditor->SelectActor(ConsideredActor, true, true, true);
		}
	}
	else if (HitProxy && HitProxy->IsA(HComponentVisProxy::StaticGetType()))
	{
		HComponentVisProxy* ComponentHitProxy = (HComponentVisProxy*)HitProxy;
		UActorComponent* ConsideredComponent = const_cast<UActorComponent*>(ComponentHitProxy->Component.Get());
		if (ConsideredComponent )
		{
			GEditor->GetSelectedComponents()->Modify();
			GEditor->SelectNone(false, true, false);
			GEditor->SelectComponent(ConsideredComponent, true, true, true);
		}
	}

	GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click);
}

#undef LOCTEXT_NAMESPACE
