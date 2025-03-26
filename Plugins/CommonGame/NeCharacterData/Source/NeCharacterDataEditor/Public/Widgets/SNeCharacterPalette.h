// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "SGraphPalette.h"

/*********************************************************************/
// SNeCharacterPaletteItem
/*********************************************************************/
class SNeCharacterPaletteItem : public SGraphPaletteItem
{
public:
	void Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData) {}
};

/*********************************************************************/
// SNeCharacterPalette
/*********************************************************************/
class SNeCharacterPalette : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS(SNeCharacterPalette) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual TSharedRef<SWidget> OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData) override;
	virtual void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;

private:
	void OnActionDoubleClick(const TArray< TSharedPtr<FEdGraphSchemaAction> >& Actions);
	TSharedPtr<SWidget> OnActionContextMenuOpening();
};