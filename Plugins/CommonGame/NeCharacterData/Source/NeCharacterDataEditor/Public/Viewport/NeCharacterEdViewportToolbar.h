// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SViewportToolBar.h"

class FExtender;

class SNeCharacterEdViewportToolbar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS( SNeCharacterEdViewportToolbar )
	{}
		SLATE_ARGUMENT(TArray<TSharedPtr<FExtender>>, Extenders)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<class SEditorViewport> InViewport);

private:
	TSharedRef<SWidget> GenerateViewMenu() const;
	EVisibility GetTransformToolbarVisibility() const;
	FText GetCameraMenuLabel() const;
	const FSlateBrush* GetCameraMenuLabelIcon() const;
	TSharedRef<SWidget> GenerateViewportTypeMenu() const;

private:
	/** The viewport that we are in */
	TWeakPtr<class SEditorViewport> Viewport;

	/** Extenders */
	TArray<TSharedPtr<class FExtender>> Extenders;
};
