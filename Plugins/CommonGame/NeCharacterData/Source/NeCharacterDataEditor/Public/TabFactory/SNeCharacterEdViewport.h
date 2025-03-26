// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "IPersonaViewport.h"
#include "Viewport\SNeSimpleEdViewport.h"

/*********************************************************/
// SNeCharacterEdViewportWidget
// Viewport Widget
/*********************************************************/
class SNeCharacterEdViewportWidget : public SNeSimpleEdViewport
{
public:
	SLATE_BEGIN_ARGS( SNeCharacterEdViewportWidget ) {};
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FEdViewportArgs& InRequiredArgs, const TSharedRef<class FNeCharacterEditor>& InEditor);
		
	// Begin: SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	// End: End of SEditorViewport interface

private:
	TSharedPtr<class FNeCharacterEditor> CharacterEdPtr;

};



/*********************************************************/
// SCharacterEdViewportTab
/*********************************************************/
class SCharacterEdViewportTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCharacterEdViewportTab) {};
	SLATE_END_ARGS()

public:
	void Construct
	(
		const FArguments& InArgs, const TSharedRef<class FNeCharacterEditorPreviewScene>& InPreviewScene,
		const TSharedRef<class FNeCharacterEditor>& InAssetEditorToolkit, int32 InViewportIndex
	);

private:
	TSharedPtr<SNeSimpleEdViewport> ViewportWidget;
};