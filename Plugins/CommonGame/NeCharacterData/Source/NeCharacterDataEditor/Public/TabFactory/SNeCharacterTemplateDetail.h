// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SCompoundWidget.h"
#include "PropertyEditorModule.h"
#include "NeCharacterEditor.h"

class SNeCharacterTemplateDetail : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SNeCharacterTemplateDetail) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TWeakObjectPtr<UObject> InTemplateAsset, const TSharedPtr<FNeCharacterEditor>& InHostEdPtr);

	void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

private:
	TWeakObjectPtr<UObject> TemplateAsset;
	TSharedPtr<FNeCharacterEditor> EditorPtr;
};