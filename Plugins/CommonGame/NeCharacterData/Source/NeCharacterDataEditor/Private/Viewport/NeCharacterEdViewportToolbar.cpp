// Copyright NetEase Games, Inc. All Rights Reserved.
#include "Viewport/NeCharacterEdViewportToolbar.h"
#include "EditorStyleSet.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SBoxPanel.h"
#include "SEditorViewportViewMenu.h"
#include "STransformViewportToolbar.h"
#include "EditorViewportCommands.h"
#include "SEditorViewport.h"

#define LOCTEXT_NAMESPACE "CharacterEdViewportToolbar"

static const FName DefaultForegroundName("DefaultForeground");
static const FMargin ToolbarSlotPadding(2.0f, 2.0f);
static const FMargin ToolbarButtonPadding(2.0f, 0.0f);

void SNeCharacterEdViewportToolbar::Construct(const FArguments& InArgs, const TSharedRef<class SEditorViewport> InViewport)
{
	Viewport = InViewport;
	Extenders = InArgs._Extenders;
	if (Extenders.Num() == 0)
	{
		Extenders.Add(MakeShared<FExtender>());
	}

	TSharedRef<SHorizontalBox> LeftToolbar = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(ToolbarSlotPadding)
	[
		SNew(SEditorViewportToolbarMenu)
		.ToolTipText(LOCTEXT("ViewMenuTooltip", "View Options.\nShift-clicking items will 'pin' them to the toolbar."))
		.ParentToolBar(SharedThis(this))
		.Cursor(EMouseCursor::Default)
		.Image("EditorViewportToolBar.MenuDropdown")
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.MenuDropdown")))
		.OnGetMenuContent(this, &SNeCharacterEdViewportToolbar::GenerateViewMenu)
	]

	+SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(ToolbarSlotPadding)
	[
		SNew(SEditorViewportToolbarMenu)
		.ToolTipText(LOCTEXT("ViewportMenuTooltip", "Viewport Options. Use this to switch between different orthographic or perspective views."))
		.ParentToolBar(SharedThis(this))
		.Cursor(EMouseCursor::Default)
		.Label(this, &SNeCharacterEdViewportToolbar::GetCameraMenuLabel)
		.LabelIcon(this, &SNeCharacterEdViewportToolbar::GetCameraMenuLabelIcon)
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.CameraMenu")))
		.OnGetMenuContent(this, &SNeCharacterEdViewportToolbar::GenerateViewportTypeMenu)
	]

	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(ToolbarSlotPadding)
	[
		SNew(SEditorViewportViewMenu, InViewport, SharedThis(this))
		.ToolTipText(LOCTEXT("ViewModeMenuTooltip", "View Mode Options. Use this to change how the view is rendered, e.g. Lit/Unlit."))
		.MenuExtenders(FExtender::Combine(Extenders))
	]

	+ SHorizontalBox::Slot()
	.Padding(ToolbarSlotPadding)
	.HAlign(HAlign_Right)
	[
		SNew(STransformViewportToolBar)
		.Viewport(InViewport)
		.CommandList(InViewport->GetCommandList())
		.Visibility(this, &SNeCharacterEdViewportToolbar::GetTransformToolbarVisibility)
	];


	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( FAppStyle::GetBrush("NoBorder") )
		// Color and opacity is changed based on whether or not the mouse cursor is hovering over the toolbar area
		.ForegroundColor( FAppStyle::GetSlateColor(DefaultForegroundName) )
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				LeftToolbar
			]
		]
	];

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> SNeCharacterEdViewportToolbar::GenerateViewMenu() const
{
	TSharedPtr<FExtender> MenuExtender = FExtender::Combine(Extenders);
	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder InMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList(), MenuExtender);
	InMenuBuilder.PushExtender(MenuExtender.ToSharedRef());

	return InMenuBuilder.MakeWidget();
}

EVisibility SNeCharacterEdViewportToolbar::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

FText SNeCharacterEdViewportToolbar::GetCameraMenuLabel() const
{
	return GetCameraMenuLabelFromViewportType( Viewport.Pin()->GetViewportClient()->GetViewportType() );
}

const FSlateBrush* SNeCharacterEdViewportToolbar::GetCameraMenuLabelIcon() const
{
	return GetCameraMenuLabelIconFromViewportType( Viewport.Pin()->GetViewportClient()->GetViewportType() );

}

TSharedRef<SWidget> SNeCharacterEdViewportToolbar::GenerateViewportTypeMenu() const
{
	TSharedPtr<FExtender> MenuExtender = FExtender::Combine(Extenders);

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder InMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList(), MenuExtender);
	InMenuBuilder.SetStyle(&FAppStyle::Get(), "Menu");
	InMenuBuilder.PushCommandList(Viewport.Pin()->GetCommandList().ToSharedRef());
	InMenuBuilder.PushExtender(MenuExtender.ToSharedRef());

	// Camera types
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Perspective);

	InMenuBuilder.BeginSection("LevelViewportCameraType_Ortho", LOCTEXT("CameraTypeHeader_Ortho", "Orthographic"));
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Top);
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Bottom);
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Left);
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Right);
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Front);
	InMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Back);
	InMenuBuilder.EndSection();

	InMenuBuilder.PopCommandList();
	InMenuBuilder.PopExtender();

	return InMenuBuilder.MakeWidget();
}
#undef LOCTEXT_NAMESPACE
