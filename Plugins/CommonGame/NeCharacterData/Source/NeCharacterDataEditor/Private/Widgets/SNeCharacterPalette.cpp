// Copyright NetEase Games, Inc. All Rights Reserved.

#include "Widgets/SNeCharacterPalette.h"
#include "Widgets/NeCharacterAssetSchemaAction.h"
#include "Kismet2/KismetEditorUtilities.h"

/*********************************************************************/
// SNeCharacterPalette
/*********************************************************************/
void SNeCharacterPalette::Construct(const FArguments& InArgs)
{
	this->ChildSlot
	[
		SNew(SBorder)
		.Padding(2.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					// Old Expression and Function lists were auto expanded so do the same here for now
					SAssignNew(GraphActionMenu, SGraphActionMenu)
					.OnActionDoubleClicked(this, &SNeCharacterPalette::OnActionDoubleClick)
					.OnCreateWidgetForAction(this, &SNeCharacterPalette::OnCreateWidgetForAction)
					.OnContextMenuOpening(this, &SNeCharacterPalette::OnActionContextMenuOpening)
					.OnCollectAllActions(this, &SNeCharacterPalette::CollectAllActions)
					.AutoExpandActionMenu(true)
				]
			]
		]
	];
}

TSharedRef<SWidget> SNeCharacterPalette::OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData)
{
	return SNew(SNeCharacterPaletteItem, InCreateData);
}

void SNeCharacterPalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{

}

void SNeCharacterPalette::OnActionDoubleClick(const TArray< TSharedPtr<FEdGraphSchemaAction> >& Actions)
{
	if (Actions.Num())
	{
		if (FNeCharacterAssetSchemaAction* SelectedAction = static_cast<FNeCharacterAssetSchemaAction*>(Actions[0].Get()))
		{
			FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(SelectedAction->CharacterAsset.Get());
		}
	}
}

TSharedPtr<SWidget> SNeCharacterPalette::OnActionContextMenuOpening()
{
	return SNullWidget::NullWidget;
}
