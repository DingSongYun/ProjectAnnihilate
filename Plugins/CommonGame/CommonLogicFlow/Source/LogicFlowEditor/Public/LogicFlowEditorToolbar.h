#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

class FLogicFlowEditor;
class FExtender;
class FToolBarBuilder;


class FLogicFlowEditorToolbar : public TSharedFromThis<FLogicFlowEditorToolbar>
{
public:
	FLogicFlowEditorToolbar(const TSharedPtr<FLogicFlowEditor>& InFlowEditor) : FlowEditorPtr(InFlowEditor) {}

	void AddGraphToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillGraphToolbar(FToolBarBuilder& ToolbarBuilder) const;
	void AddDebuggingToolbar(FToolBarBuilder& ToolbarBuilder) const;

protected:
	TWeakPtr<FLogicFlowEditor> FlowEditorPtr;

};