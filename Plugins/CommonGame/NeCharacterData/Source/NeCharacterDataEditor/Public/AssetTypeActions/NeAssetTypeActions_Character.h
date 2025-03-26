// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "AssetTypeActions_Base.h"
#include "NeCharacterAsset.h"
#include "NeCharacterEditor.h"
#include "AssetTypeActions/NeAssetTypeActions_Base.h"

/**
 * FNeAssetTypeActions_CharacterAsset
 */
class FNeAssetTypeActions_CharacterAsset : public FNeAssetTypeActions_Base
{
public:
	FNeAssetTypeActions_CharacterAsset(EAssetTypeCategories::Type InAssetCategory)
	 : FNeAssetTypeActions_Base(InAssetCategory, UNeCharacterAsset::StaticClass(), FColor(0.0f, 178.0f, 191.0f))
	{}

	// ~Begin: IAssetTypeActions interface
	virtual FText GetName() const override;
	// ~End: IAssetTypeActions interface

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override
	{
		EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

		if (InObjects.Num() > 0)
		{
			ICharacterEditor::CreateEditor(InObjects[0], EditWithinLevelEditor);
		}
	}
//
// 	virtual uint32 GetCategories() override { return AssetCategory; }
// 	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
// 	virtual UClass* GetSupportedClass() const override
// 	{
// 		return SupportClass;
// 	}
// 	virtual FColor GetTypeColor() const override
// 	{
// 		return AssetColor;
// 	}
// 	// ~End: IAssetTypeActions interface
//
// protected:
// 	EAssetTypeCategories::Type AssetCategory;
// 	UClass* SupportClass;
// 	FColor AssetColor;
};
