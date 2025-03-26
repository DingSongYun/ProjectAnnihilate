
#include "Widgets/SNeCharacterEditorToolbar.h"
#include "NeCharacterEditor.h"

#define LOCTEXT_NAMESPACE "CharacterEditor"

void FNeCharacterEditorToolbar::SetupToolbar(TSharedPtr<FExtender> Extender, TSharedPtr<FNeCharacterEditor> InCharacterEditor)
{
	CharacterEditor = InCharacterEditor;

}

void FNeCharacterEditorToolbar::AddHelperToolbar(TSharedPtr<FExtender> Extender)
{
	if (!CharacterEditor.IsValid())
		return;

	Extender->AddToolBarExtension
	(
		"Asset", EExtensionHook::After,
		CharacterEditor.Pin()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FNeCharacterEditorToolbar::FillHelperToolbar)
	);
}

void FNeCharacterEditorToolbar::FillHelperToolbar(FToolBarBuilder& ToolbarBuilder)
{
	const FCharacterEditorCommands& Commands = FCharacterEditorCommands::Get();

	ToolbarBuilder.BeginSection("Helper");
	ToolbarBuilder.AddToolBarButton(Commands.m_ShowLockable);
	ToolbarBuilder.AddToolBarButton(Commands.m_RefreshActor);
	ToolbarBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE