// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterAssetThumbnailRenderer.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "CanvasTypes.h"
#include "NeCharacterAsset.h"
#include "Animation/Skeleton.h"
#include "Engine/Font.h"
#include "Styling/CoreStyle.h"
#include "UObject/Package.h"
#include "GameFramework/Character.h"

ANeCharacterAssetThumbnailPreviewActor::ANeCharacterAssetThumbnailPreviewActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDebugSkelMeshComponent>(TEXT("SkeletalMeshComponent0")))
{

}

FNeCharacterAssetThumbnailScene::FNeCharacterAssetThumbnailScene() : FThumbnailPreviewScene()
{
	bForceAllUsedMipsResident = false;
	// Create preview actor
	// checked
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bNoFail = true;
	SpawnInfo.ObjectFlags = RF_Transient;
	PreviewActor = GetWorld()->SpawnActor<ANeCharacterAssetThumbnailPreviewActor>(SpawnInfo);

	PreviewActor->SetActorEnableCollision(false);
}

bool FNeCharacterAssetThumbnailScene::SetAsset(class UNeCharacterAsset* InAsset)
{
	PreviewActor->GetSkeletalMeshComponent()->OverrideMaterials.Empty();

	bool bSetSucessfully = false;

	PreviewAsset = InAsset;
	USkeletalMeshComponent* MeshComponent = nullptr;
	if (PreviewAsset.IsValid())
	{
		const APawn* Pawn = PreviewAsset->ActorClass ? PreviewAsset->ActorClass->GetDefaultObject<APawn>() : nullptr;
		if (const ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			MeshComponent = Character ? Character->GetMesh() : nullptr;
		}
		else
		{
			MeshComponent = Pawn->GetComponentByClass<USkeletalMeshComponent>();
		}
	}

	if (IsValid(MeshComponent))
	{
		if (USkeletalMesh* SkeletonMesh = MeshComponent->GetSkeletalMeshAsset())
		{
			PreviewActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkeletonMesh);

			if (SkeletonMesh)
			{
				bSetSucessfully = true;

				PreviewActor->SetActorLocation(FVector(0, 0, 0), false);
				PreviewActor->GetSkeletalMeshComponent()->UpdateBounds();

				// Center the mesh at the world origin then offset to put it on top of the plane
				const float BoundsZOffset = GetBoundsZOffset(PreviewActor->GetSkeletalMeshComponent()->Bounds);
				PreviewActor->SetActorLocation(-PreviewActor->GetSkeletalMeshComponent()->Bounds.Origin + FVector(0, 0, BoundsZOffset), false);
				PreviewActor->GetSkeletalMeshComponent()->RecreateRenderState_Concurrent();
			}
		}
	}

	if(!bSetSucessfully)
	{
		CleanupComponentChildren(PreviewActor->GetSkeletalMeshComponent());
		PreviewActor->GetSkeletalMeshComponent()->SetAnimation(NULL);
		PreviewActor->GetSkeletalMeshComponent()->SetSkeletalMesh(nullptr);
	}

	return bSetSucessfully;

}

void FNeCharacterAssetThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const
{
	check(PreviewAsset.IsValid());
	check(PreviewActor->GetSkeletalMeshComponent());
	check(PreviewActor->GetSkeletalMeshComponent()->GetSkeletalMeshAsset());

	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;
	const float HalfMeshSize = PreviewActor->GetSkeletalMeshComponent()->Bounds.SphereRadius;
	const float BoundsZOffset = GetBoundsZOffset(PreviewActor->GetSkeletalMeshComponent()->Bounds);
	const float TargetDistance = HalfMeshSize / FMath::Tan(HalfFOVRadians);

	// USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(PreviewAnimation->ThumbnailInfo);
	const USceneThumbnailInfo* ThumbnailInfo = USceneThumbnailInfo::StaticClass()->GetDefaultObject<USceneThumbnailInfo>();
	float OrbitZoom = ThumbnailInfo->OrbitZoom;
	if (TargetDistance + OrbitZoom < 0)
	{
		OrbitZoom = -TargetDistance;
	}

	OutOrigin = FVector(0, 0, -BoundsZOffset);
	OutOrbitPitch = ThumbnailInfo->OrbitPitch;
	OutOrbitYaw = ThumbnailInfo->OrbitYaw;
	OutOrbitZoom = TargetDistance + OrbitZoom;
}

void FNeCharacterAssetThumbnailScene::CleanupComponentChildren(USceneComponent* Component)
{
	if (Component)
	{
		for(int32 ComponentIdx = Component->GetAttachChildren().Num() - 1 ; ComponentIdx >= 0 ; --ComponentIdx)
		{
			CleanupComponentChildren(Component->GetAttachChildren()[ComponentIdx]);
			Component->GetAttachChildren()[ComponentIdx]->DestroyComponent();
		}
		check(Component->GetAttachChildren().Num() == 0);
	}
}

UNeCharacterAssetThumbnailRenderer::UNeCharacterAssetThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> TextureObject;
		UFont* NameFont = nullptr;
		FConstructorStatics()
			: TextureObject(TEXT("/Engine/EditorMaterials/ParticleSystems/PSysThumbnail_NoImage"))
		{
			NameFont = NewObject<UFont>(GetTransientPackage(), NAME_None, RF_Transient);
			NameFont->FontCacheType = EFontCacheType::Runtime;
			NameFont->LegacyFontSize = 27.0f;
			NameFont->CompositeFont = *FCoreStyle::GetDefaultFont();
		}
	};
	static FConstructorStatics ConstructorStatics;

	TitleImage = ConstructorStatics.TextureObject.Get();
	NameFont = ConstructorStatics.NameFont;
	ThumbnailScene = nullptr;
}

void UNeCharacterAssetThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UNeCharacterAsset* AssetToRender = Cast<UNeCharacterAsset>(Object);
	if (!AssetToRender)
	{
		return;
	}

	if ( ThumbnailScene == nullptr )
	{
		ThumbnailScene = new FNeCharacterAssetThumbnailScene();
	}

	if(ThumbnailScene->SetAsset(AssetToRender))
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetTime(UThumbnailRenderer::GetTime())
			.SetAdditionalViewFamily(bAdditionalViewFamily));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		RenderViewFamily(Canvas, &ViewFamily, ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height));
		ThumbnailScene->SetAsset(nullptr);
	}


	// Canvas->DrawTile(X, Y, Width, Height, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor::Black, nullptr, false);
	static FLinearColor LinearColor = FLinearColor::White;
	static FLinearColor ShadowColor = FColor(235, 177, 77);

	const int32 LineHeight = FMath::TruncToInt(NameFont->GetMaxCharHeight());
	FText DisplayName = AssetToRender->Name.IsEmpty() ? FText::FromName(AssetToRender->GetFName()) : AssetToRender->Name;
	const int32 MessageWidth = NameFont->GetStringSize(*DisplayName.ToString());
	float Xpos = FMath::Max(((int32)Width - MessageWidth) / 2, 0.f);
	Canvas->DrawShadowedText(Xpos, LineHeight*1.5, DisplayName, NameFont, LinearColor, ShadowColor);
}

void UNeCharacterAssetThumbnailRenderer::BeginDestroy()
{
	if ( ThumbnailScene != nullptr )
	{
		delete ThumbnailScene;
		ThumbnailScene = nullptr;
	}

	Super::BeginDestroy();
}