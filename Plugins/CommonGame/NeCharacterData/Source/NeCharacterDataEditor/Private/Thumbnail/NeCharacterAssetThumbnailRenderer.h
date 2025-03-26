// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "ThumbnailHelpers.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "NeCharacterAssetThumbnailRenderer.generated.h"

UCLASS(ComponentWrapperClass, ConversionRoot, meta = (ChildCanTick))
class ANeCharacterAssetThumbnailPreviewActor : public ASkeletalMeshActor
{
	GENERATED_UCLASS_BODY()
};

class FNeCharacterAssetThumbnailScene : public FThumbnailPreviewScene
{
public:
	FNeCharacterAssetThumbnailScene();

	bool SetAsset(class UNeCharacterAsset* InAsset);

protected:
	virtual void GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const override;

	void CleanupComponentChildren(USceneComponent* Component);

private:
	TWeakObjectPtr<ANeCharacterAssetThumbnailPreviewActor> PreviewActor;

	TWeakObjectPtr<class UNeCharacterAsset> PreviewAsset;
};


UCLASS()
class UNeCharacterAssetThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;

	virtual void BeginDestroy() override;

private:
	UPROPERTY()
	class UTexture2D* TitleImage = nullptr;

	UPROPERTY()
	class UFont* NameFont = nullptr;

	class FNeCharacterAssetThumbnailScene* ThumbnailScene = nullptr;
};