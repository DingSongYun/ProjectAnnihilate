// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "NeAvatarMaterialInfo.generated.h"

class UTexture;
class UMaterialInterface;
class UTexture2D;

USTRUCT(BlueprintType)
struct FAvatarMatParam_Scalar
{
	GENERATED_BODY()
	/**材质参数名*/
	UPROPERTY(VisibleAnywhere)
	FName ParamName;

	/**材质参数是否使用*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	uint8 bEnable:1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bEnable"))
	float Value = 0.f;
};

USTRUCT(BlueprintType)
struct FAvatarMatParam_Vector
{
	GENERATED_BODY()
	/**材质参数名*/
	UPROPERTY(VisibleAnywhere)
	FName ParamName;

	/**材质参数是否使用*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	uint8 bEnable : 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bEnable"))
	FLinearColor Value = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FAvatarMatParam_Texture
{
	GENERATED_BODY()
	/**材质参数名*/
	UPROPERTY(VisibleAnywhere)
	FName ParamName;

	/**材质参数是否使用*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	uint8 bEnable : 1 ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bEnable"))
	TSoftObjectPtr<UTexture2D> Value;
};

USTRUCT(BlueprintType)
struct NECHARACTERDATA_API FAvatarMaterialInfo
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FName SlotName;
#endif

	//是否手动修改材质索引，默认关闭
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	uint8 MaterialIndexHandle:1;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "MaterialIndexHandle"))
	int MaterialIndex = INDEX_NONE;

	/**新的材质*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> Material = nullptr;

	/** 材质参数: scalar */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, EditFixedSize)
	TArray<FAvatarMatParam_Scalar> MaterialParamsScalar;

	/** 材质参数: vector */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, EditFixedSize)
	TArray<FAvatarMatParam_Vector> MaterialParamsVector;

	/** 材质参数: texture */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, EditFixedSize)
	TArray<FAvatarMatParam_Texture> MaterialParamsTexture;

	void AddParameter(const FName& Name, float Value);
	void AddParameter(const FName& Name, const FLinearColor& Value);
	void AddParameter(const FName& Name, UTexture* Value);
	void RemoveParameter(const FName& Name);
	bool ContainsParameter(const FName& Name);
};