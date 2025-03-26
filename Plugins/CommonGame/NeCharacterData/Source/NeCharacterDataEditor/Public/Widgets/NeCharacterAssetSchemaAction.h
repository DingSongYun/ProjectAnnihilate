// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "NeCharacterAsset.h"
#include "NeCharacterAssetSchemaAction.generated.h"

USTRUCT()
struct FNeCharacterAssetSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_BODY();

	// Simple type info
	static FName StaticGetTypeId() {static FName Type("FNeCharacterAssetSchemaAction"); return Type;}
	virtual FName GetTypeId() const override { return StaticGetTypeId(); } 

	FNeCharacterAssetSchemaAction()
		: FEdGraphSchemaAction()
	{}

	FNeCharacterAssetSchemaAction(TWeakObjectPtr<UNeCharacterAsset> InCharacterAsset)
	{
		check(InCharacterAsset.IsValid());
		CharacterAsset = InCharacterAsset;
		Grouping = 0;
		SectionID = 0;
		UpdateSearchData(GetCharacterName(), GetCharacterName(), GetCharacterCategory(), FText());
	}

	void Refresh()
	{
		UpdateSearchData(GetCharacterName(), GetCharacterName(), GetCharacterCategory(), FText());
	}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override
	{
		return nullptr;
	}
	//~ End FEdGraphSchemaAction Interface

	FText GetCharacterCategory()
	{
		return FText::FromName("All");
	}

	FText GetCharacterName()
	{
		return FText::FromString(FName::NameToDisplayString(CharacterAsset->GetName(), false));
	}
public:
	/** Pointer to CharacterTemplate */
	TWeakObjectPtr<UNeCharacterAsset> CharacterAsset;
};