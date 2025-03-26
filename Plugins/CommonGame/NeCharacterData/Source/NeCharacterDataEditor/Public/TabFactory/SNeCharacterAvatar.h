// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Misc/NotifyHook.h"

class UNeCharacterAsset;
class ABaseCharacter;
class FNeCharacterEditor;

class SNeCharacterAvatar : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SNeCharacterAvatar)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakObjectPtr<UObject> InPrototypeAsset, const TSharedPtr<FNeCharacterEditor>& InHostEdPtr);

	void SetDetailStruct(const UStruct* InScriptStruct, uint8* InData);
	void SetDetailObject(UObject* InObject);
	void SetDetailProperty(UObject* InObject, FName InPropertyName);
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

private:
	TSharedRef<SWidget> CreateAvatarInfoTypePicker();
	FText GetAvatarInfoTypeName() const;
	void OnChangeAvatarType();
	void PrepareMeshLink();
	void RefreshCharacterAvatar();
	struct FAvatarInfo* GetAvatarInfo() const;

private:
	TWeakObjectPtr<UObject> PrototypeAsset;
	TSharedPtr<FNeCharacterEditor> EditorPtr;

	TSharedPtr<SBorder> DetailContainer;
	TSharedPtr<class IDetailsView> DetailView;
	TSharedPtr<class IStructureDetailsView> StructureDetailView;
	TSharedPtr<ISinglePropertyView> PropertyView;
};
