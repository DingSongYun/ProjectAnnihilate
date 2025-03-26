// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Viewport\NeSimpleEdViewportClient.h"

class FNeCharacterEdViewportClient : public FNeSimpleEdViewportClient
{
public:
	FNeCharacterEdViewportClient
	(
		const TSharedRef<class FAssetEditorToolkit> InAssetEditorToolkit, const TSharedRef<class FPreviewScene>& InPreviewScene, 
		const TSharedRef<class SEditorViewport>& InViewport, const TSharedPtr<class FNeCharacterEditor>& InEditorPtr
	);

	virtual ~FNeCharacterEdViewportClient();
	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;

private:
	TSharedPtr<class FNeCharacterEditor> CharacterEdPtr;
};
