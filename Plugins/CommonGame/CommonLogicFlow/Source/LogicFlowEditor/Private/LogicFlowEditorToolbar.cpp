#include "LogicFlowEditorToolbar.h"
#include "LogicFlowEditor.h"
#include "LogicFlowEditorCommands.h"
#include "SSelectedFlowDebugObjectWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "LogicFlowEditorToolbar"

void FLogicFlowEditorToolbar::AddGraphToolbar(TSharedPtr<FExtender> Extender)
{
	check(FlowEditorPtr.IsValid());
	TSharedPtr<FLogicFlowEditor> FlowEditor = FlowEditorPtr.Pin();

	const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	// ToolbarExtender->AddMenuExtension("Asset", EExtensionHook::After, FlowEditor->GetToolkitCommands(), FMenuExtensionDelegate::CreateSP(this, &FLogicFlowEditorToolbar::FillGraphToolbar));
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, FlowEditor->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FLogicFlowEditorToolbar::FillGraphToolbar));
	FlowEditor->AddToolbarExtender(ToolbarExtender);
}

void FLogicFlowEditorToolbar::FillGraphToolbar(FToolBarBuilder& ToolbarBuilder) const
{
	const FLogicFlowEditorCommands& Commands = FLogicFlowEditorCommands::Get();

	ToolbarBuilder.BeginSection("Graph");
	ToolbarBuilder.AddToolBarButton(Commands.BackToPreviousGraph);
	ToolbarBuilder.EndSection();

	AddDebuggingToolbar(ToolbarBuilder);
}

void FLogicFlowEditorToolbar::AddDebuggingToolbar(FToolBarBuilder& ToolbarBuilder) const
{
	ToolbarBuilder.BeginSection("Debugging");
	ToolbarBuilder.AddWidget( SNew(SSelectedDebugObjectWidget, FlowEditorPtr.Pin()) );
	ToolbarBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE
