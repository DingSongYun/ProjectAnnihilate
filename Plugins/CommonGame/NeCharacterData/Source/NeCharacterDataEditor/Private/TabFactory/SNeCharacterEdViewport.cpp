// Copyright NetEase Games, Inc. All Rights Reserved.
#include "NeCharacterEditor.h"
#include "NeCharacterEditorPreviewScene.h"
#include "Framework/Application/SlateApplication.h"
#include "TabFactory/SNeCharacterEdViewport.h"
#include "Viewport/NeCharacterEdViewportClient.h"
#include "Viewport/NeCharacterEdViewportToolbar.h"


/*********************************************************/
// SNeCharacterEdViewportWidget
// Viewport Widget
/*********************************************************/
void SNeCharacterEdViewportWidget::Construct(const FArguments& InArgs, const FEdViewportArgs& InRequiredArgs, const TSharedRef<FNeCharacterEditor>& InEditor)
{
	SNeSimpleEdViewport::Construct(SNeSimpleEdViewport::FArguments(), InRequiredArgs);

	CharacterEdPtr = InEditor;
}

TSharedRef<FEditorViewportClient> SNeCharacterEdViewportWidget::MakeEditorViewportClient()
{
	// Create viewport client
	ViewportClient = MakeShareable(new FNeCharacterEdViewportClient(AssetEditorToolkitPtr.Pin().ToSharedRef(), PreviewScenePtr.Pin().ToSharedRef(), SharedThis(this), CharacterEdPtr));
	ViewportClient->ViewportType = LVT_Perspective;
	ViewportClient->bSetListenerPosition = false;

	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SNeCharacterEdViewportWidget::MakeViewportToolbar()
{
	return SAssignNew(ViewportToolbar, SNeCharacterEdViewportToolbar, SharedThis(this));
}




/*********************************************************/
// SCharacterEdViewportTab
/*********************************************************/
void SCharacterEdViewportTab::Construct
(
	const FArguments& InArgs, const TSharedRef<FNeCharacterEditorPreviewScene>& InPreviewScene,
	const TSharedRef<FNeCharacterEditor>& InAssetEditorToolkit, int32 InViewportIndex
)
{
	FEdViewportArgs ViewportArgs(InPreviewScene, InAssetEditorToolkit, InViewportIndex);
	ViewportWidget = SNew(SNeCharacterEdViewportWidget, ViewportArgs, InAssetEditorToolkit);

	TSharedPtr<SVerticalBox> ViewportContainer = nullptr;
	this->ChildSlot
	[
		SAssignNew(ViewportContainer, SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1)
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			[
				ViewportWidget.ToSharedRef()
			]
		]
	];
}
