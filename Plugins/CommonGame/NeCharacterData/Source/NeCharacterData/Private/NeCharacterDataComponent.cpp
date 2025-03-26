// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterDataComponent.h"
#include "NeCharacterAsset.h"
#include "NeCharacterFeature_MakeAvatar.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeCharacterDataComponent)

DEFINE_LOG_CATEGORY(LogNeCharacterData);

UNeCharacterDataComponent::UNeCharacterDataComponent(const FObjectInitializer& Initializer)
	: Super(Initializer), bReady(false), CharacterAsset(nullptr)
{
	// bWantsInitializeComponent = true;
}

void UNeCharacterDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNeCharacterDataComponent, CharacterAsset);
}

void UNeCharacterDataComponent::BeginPlay()
{
	bReady = false;
	Super::BeginPlay();
}

void UNeCharacterDataComponent::CheckInitializationState()
{
	if (CharacterAsset && !bReady)
	{
		InitializeWithCharacterData();
	}
}

void UNeCharacterDataComponent::OnRep_CharacterData()
{
	CheckInitializationState();

	WhenCharacterDataReady();
}

void UNeCharacterDataComponent::WhenCharacterDataReady()
{
	if (CharacterAsset)
	{
		CheckInitializationState();
	}
}

void UNeCharacterDataComponent::InitializeWithCharacterData()
{
	check(CharacterAsset);

	if (ACharacter* Character = GetPawn<ACharacter>())
	{
		// Apply feature actions
		//  1. Apply build-in features
		CharacterAsset->AvatarData->ApplyFeature(Character);

		//  2. Apply user defined features
		for (const TObjectPtr<UNeCharacterFeatureAction>& FeatureAction : CharacterAsset->Actions)
		{
			if (FeatureAction)
			{
				FeatureAction->ApplyFeature(Character);
			}
		}
	}

	bReady = true;
}

void UNeCharacterDataComponent::SetCharacterAsset(UNeCharacterAsset* InCharacterAsset)
{
	check(InCharacterAsset);

	APawn* Pawn = GetPawnChecked<APawn>();

	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// 一般而言，我们应该只会在角色初始化的时候设定角色CharacterAsset
	// 如果有重复设置的操作，请确保是你希望的
	ensureMsgf(CharacterAsset == nullptr,
			TEXT("Trying to set CharacterData [%s] on character [%s] that already has valid PawnData [%s]. Please ensure you really need to do this."),
			*InCharacterAsset->GetName(), *Pawn->GetName(), *CharacterAsset->GetName());

	CharacterAsset = InCharacterAsset;

	Pawn->ForceNetUpdate();
}

#if WITH_EDITOR
void UNeCharacterDataComponent::FlushCharacterDataEditorOnly()
{
	InitializeWithCharacterData();
}
#endif
