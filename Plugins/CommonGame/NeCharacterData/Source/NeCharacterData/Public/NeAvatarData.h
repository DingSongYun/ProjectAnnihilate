// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeComponentPath.h"
#include "NeAvatarMaterialInfo.h"
#include "Components/SkeletalMeshComponent.h"
#include "NeAvatarData.generated.h"

class UStaticMesh;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FVisualSceneComponent
{
	GENERATED_USTRUCT_BODY()

public:
	 void CopyDataFromOther(const USceneComponent* OtherComp);
	 void CopyDataToOther(USceneComponent* OtherComp, AActor* CompOwner)const;

public:
	/*该组件的名称(非必填)*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Important")
	FName ComponentName = NAME_None;

	/**挂接的父项组件名称*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.SkeletalMeshComponent,/Script/Engine.StaticMeshComponent"))
	FNeComponentPath AttachParentPath;
	
	/**挂接的Socket/骨骼名称*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	FNeComponentSocketName AttachSocket;

	/**相对位置偏移*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transform")
	FTransform RelativeTransform = FTransform::Identity;

	/**默认的显示设置*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visible")
	bool bHiddenInGame = false;

	/**该组件的自定义Tag*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag")
	TArray<FName> CustomTags;

#if WITH_EDITOR
	/**缓存该组件的层级*/
	int32 Hierarchy = 0;
#endif
#if WITH_EDITORONLY_DATA
	void SetPreviewActor(AActor* PreviewActor);
#endif
};

/**PrimitiveComponent对应的结构体*/
USTRUCT(BlueprintType)
struct FVisualPrimitiveComponent : public FVisualSceneComponent
{
	GENERATED_USTRUCT_BODY()

public:
	void CopyDataFromOther(const class UPrimitiveComponent* OtherComp);
	void CopyDataToOther(class UPrimitiveComponent* OtherComp, AActor* CompOwner)const;

public:
	// 碰撞信息
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision")
	//FBodyInstanceWrap CollisionMessage;

};


/**StaticMeshComponent对应的结构体*/
USTRUCT(BlueprintType)
struct FVisualStaticMeshComponent : public FVisualPrimitiveComponent
{
	GENERATED_USTRUCT_BODY()

public:
	void CopyDataFromOther(const class UStaticMeshComponent* OtherComp);
	void CopyDataToOther(class UStaticMeshComponent* OtherComp, AActor* CompOwner) const;

public:
	/**静态模型*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	TSoftObjectPtr<UStaticMesh> StaticMesh = nullptr;

	/**材质覆盖信息*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	TMap<int32, TSoftObjectPtr<UMaterialInterface> > OverrideMaterials;

};

/**SkeletalMeshComponent对应的结构体*/
USTRUCT(BlueprintType)
struct FVisualSkeletalMeshComponent : public FVisualPrimitiveComponent
{
	GENERATED_USTRUCT_BODY()

public:
	  void CopyDataFromOther(const class USkeletalMeshComponent* OtherComp);
	  void CopyDataToOther(class USkeletalMeshComponent* OtherComp, AActor* CompOwner) const;

public:
	/**骨骼模型*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	/**BoundsScale(不知道用途的不要改这个)*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	float BoundsScale = 1.0f;

	/**材质覆盖信息*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	TMap<int32, TSoftObjectPtr<UMaterialInterface> > OverrideMaterials;

	/**骨骼模型的动画模式*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	TEnumAsByte<EAnimationMode::Type> AnimationMode = EAnimationMode::AnimationBlueprint;

	/**骨骼模型的动画蓝图类*/
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	//TSubclassOf<UAnimInstance> AnimationClass = NULL;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh Info")
	//UAnimationAsset* AnimationAsset = nullptr;
	
};

/**ParticleSystemComponent对应的结构体*/
USTRUCT(BlueprintType)
struct FVisualParticleSystemComponent : public FVisualPrimitiveComponent
{
	GENERATED_USTRUCT_BODY()
public:
	void CopyDataFromOther(const class UParticleSystemComponent* OtherComp);
	void CopyDataToOther(class UParticleSystemComponent* OtherComp, AActor* CompOwner) const;
public:
	// 粒子信息*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UParticleSystem> ParticleSystem = nullptr;

};
USTRUCT(BlueprintType)
struct NECHARACTERDATA_API FAvatarPart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (ShowOnlyInnerProperties, AllowedClasses = "/Script/Engine.SkeletalMeshComponent"))
	FNeComponentPath MeshLink;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "重载部位模型"))
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	uint8 bModifyMaterial : 1;

	/**修改材质信息*/
	UPROPERTY(EditDefaultsOnly, EditFixedSize, meta = (EditCondition = "bModifyMaterial"))
	TArray<FAvatarMaterialInfo> Materials;

	// 修改模型偏移、缩放信息
	UPROPERTY(EditDefaultsOnly)
	FTransform MeshRelativeTransform = FTransform::Identity;

#if WITH_EDITORONLY_DATA
	/**用于记录修改的材质*/
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialFlag;
#endif
#if WITH_EDITOR
public:
	void OnPreEditChange();
	void OnPostEditChange(const struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};

/**
 * UAvatarMakerType
 * 角色外观设定基类
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NECHARACTERDATA_API UAvatarMakeData : public UObject
{
	GENERATED_BODY()
public:
	// 资源预加载
	UFUNCTION(BlueprintCallable)
	virtual void PreLoad(UObject* WorldContext);

	virtual void MakeAvatar(class ACharacter* Character);

	UFUNCTION(BlueprintCallable)
	static void ModifyMeshMaterial(UMeshComponent* MeshComponent, const TArray<FAvatarMaterialInfo>& NewMaterialInfo);
#if WITH_EDITOR
public:
	virtual void SetPreviewContext(class AActor* Actor) {}

	/**修改Mesh，扫描材质参数*/
	static void CopyAvatarMaterialParameter(const TSoftObjectPtr<USkeletalMesh>& SkeletalMesh, TArray<FAvatarMaterialInfo>& Materials);
	/**更新设置的Material里面的材质参数*/
	static void UpdateMaterialParameter(FAvatarMaterialInfo& MaterialData);


	virtual TArray<FName> GetAllMeshName() { return TArray<FName>(); }

#endif

};

/**
 * UAvatarMakeSimple
 * 简单的角色设定， 用于修改颜色，材质等
 */
UCLASS(Editinlinenew, BlueprintType, Blueprintable)
class NECHARACTERDATA_API UAvatarMakeSimple : public UAvatarMakeData
{
	GENERATED_BODY()

public:
	UAvatarMakeSimple(const FObjectInitializer& Initializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(DisplayName="主体模型"))
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	// // 动画蓝图类
	// UPROPERTY(EditDefaultsOnly)
	// TSubclassOf<UAnimInstance> AnimationClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector MeshOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator MeshRotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	uint8 bModifyMaterial : 1;

	/**修改材质信息*/
	UPROPERTY(EditDefaultsOnly, EditFixedSize,meta = (EditCondition="bModifyMaterial"))
	TArray<FAvatarMaterialInfo> Materials;

public:
	// 资源预加载
	virtual void PreLoad(UObject* WorldContext) override;
	virtual void MakeAvatar(class ACharacter* Character) override;

#if WITH_EDITORONLY_DATA
	/**用于记录修改的材质*/
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialFlag;
#endif
#if WITH_EDITOR
public:
	virtual void SetPreviewContext(AActor* Actor) override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual TArray<FName> GetAllMeshName() override;
#endif
};


/**
 * 角色外观信息
 */
USTRUCT(BlueprintType)
struct FAvatarInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Avatar")
	UAvatarMakeData* MakeData = nullptr;

};