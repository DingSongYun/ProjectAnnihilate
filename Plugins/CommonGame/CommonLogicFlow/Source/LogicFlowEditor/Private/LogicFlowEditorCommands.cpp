#include "LogicFlowEditorCommands.h"

#define LOCTEXT_NAMESPACE "EditorCommands_GenericGraph"

void FLogicFlowEditorCommands::RegisterCommands()
{
	UI_COMMAND(BackToPreviousGraph, "Back To Previous Graph", "Back To Previous Graph", EUserInterfaceActionType::Button, FInputChord(EKeys::Q));
}

#undef LOCTEXT_NAMESPACE
