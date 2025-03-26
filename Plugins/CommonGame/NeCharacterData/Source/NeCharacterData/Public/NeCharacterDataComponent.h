// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "Components/PawnComponent.h"
#include "NeCharacterDataComponent.generated.h"

class UNeCharacterAsset;

NECHARACTERDATA_API DECLARE_LOG_CATEGORY_EXTERN(LogNeCharacterData, Log, All);

UCLASS(Blueprintable, BlueprintType)
class NECHARACTERDATA_API UNeCharacterDataComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UNeCharacterDataComponent(const FObjectInitializer& Initializer =  FObjectInitializer::Get());
	virtual void BeginPlay() override;

	UFUNCTION(Blueprintable)
	void CheckInitializationState();

	UFUNCTION(BlueprintPure, Category = "NeCharacter")
	const UNeCharacterAsset* GetCharacterAsset() const { return CharacterAsset; }

	UFUNCTION(BlueprintCallable, Category = "NeCharacter")
	virtual void SetCharacterAsset(UNeCharacterAsset* InCharacterAsset);

protected:
	UFUNCTION()
	virtual void OnRep_CharacterData();

	/** 角色数据同步成功之后，开始进行相应初始化 */
	virtual void WhenCharacterDataReady();

	virtual void InitializeWithCharacterData();

	//~==============================================================
	// For Editor
#if WITH_EDITOR
public:
	void FlushCharacterDataEditorOnly();
#endif

public:
	UPROPERTY(BlueprintReadOnly, Transient)
	uint8 bReady : 1;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing = OnRep_CharacterData, Category = "NeCharacter")
	TObjectPtr<const UNeCharacterAsset> CharacterAsset;
};
