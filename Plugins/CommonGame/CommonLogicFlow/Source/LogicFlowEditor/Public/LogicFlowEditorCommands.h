#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandInfo.h"
#include "EditorStyleSet.h"

class FLogicFlowEditorCommands : public TCommands<FLogicFlowEditorCommands>
{
public:
	FLogicFlowEditorCommands() : TCommands<FLogicFlowEditorCommands>
	(
		TEXT("LogicFlowEditor"),
		NSLOCTEXT("Contexts", "LogicFlowEditor", "Flow Editor"),
		NAME_None, 
		FAppStyle::Get().GetStyleSetName()
	) {}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> BackToPreviousGraph;

};