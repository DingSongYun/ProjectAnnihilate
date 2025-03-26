// Copyright NetEase Games, Inc. All Rights Reserved.
// 定义一个通用行星接口

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NePawnAssetInterface.generated.h"

/**
 * NePawnAssetInterface
 * 定义一个可以用于PawnAsset类型设定的接口
 */
UINTERFACE(BlueprintType)
class NECHARACTERDATA_API UNePawnAssetInterface : public UInterface
{
	GENERATED_BODY()
};

class INePawnAssetInterface : public IInterface
{
	GENERATED_BODY()
};