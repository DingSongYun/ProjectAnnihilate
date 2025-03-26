// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "NeCharacterAsset.generated.h"

class UTexture2D;
class ACharacter;
class UNeCharacterFeatureAction;

/**
 * UNeCharacterAsset
 *
 * 角色数据配置资源, 项目应该扩展这个结构增加所需配置字段
 */
UCLASS(Abstract, Blueprintable, meta=(PrioritizeCategories="Basic"))
class NECHARACTERDATA_API UNeCharacterAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Constructor */
	UNeCharacterAsset(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	FORCEINLINE void SetPreviewActor(AActor* InActor) { PreviewActor = InActor; }
	FORCEINLINE AActor* GetPreviewActor() const { return PreviewActor.Get(); }
private:
	/** 用于编辑器中的一些预览操作，绝大部分时候请忽略这个的存在 */
	TWeakObjectPtr<AActor> PreviewActor = nullptr;
#endif

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Basic")
	FText Name;
#endif

	/**对应的Actor Class*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Basic", meta=(ShowDisplayNames, AllowAbstract=false, OnlyPlaceable, MustImplement="/Script/NeCharacterData.NePawnAssetInterface"))
	class TSubclassOf<APawn> ActorClass;

	/** 胶囊体半高 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Basic|Capsule")
	float CapsuleHalfHeight = 90.0f;

	/** 胶囊体半径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Basic|Capsule")
	float CapsuleRadius = 45.0f;

	/** 角色头像 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Basic|Icon")
	UTexture2D* CharacterIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Avatar")
	TObjectPtr<class UNeCharacterFeature_MakeAvatar> AvatarData;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Features")
	TArray<TObjectPtr<UNeCharacterFeatureAction>> Actions;

};