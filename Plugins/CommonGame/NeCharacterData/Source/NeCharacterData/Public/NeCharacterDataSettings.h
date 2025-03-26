// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeCharacterDataSettings.generated.h"

class UNeCharacterAsset;

/**
 * UCharacterDataSettings
 *
 * 配置数据
 */
UCLASS(config = Game, defaultconfig)
class NECHARACTERDATA_API UCharacterDataSettings : public UObject
{
	GENERATED_BODY()
public:
	UCharacterDataSettings(const FObjectInitializer& Initializer);

	/**右键可以创建的Templates*/
	UPROPERTY(Config, EditAnywhere)
	TArray<TSoftClassPtr<class UNeCharacterAsset>> AssetTypeRegistration;
};
