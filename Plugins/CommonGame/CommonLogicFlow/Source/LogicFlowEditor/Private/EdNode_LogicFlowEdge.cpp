#include "EdNode_LogicFlowEdge.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdNode_LogicFlowNode.h"
#include "LogicFlowEdge.h"

#define LOCTEXT_NAMESPACE "EdNode_LogicFlowEdge"

UEdNode_LogicFlowEdge::UEdNode_LogicFlowEdge()
{}

UEdNode_LogicFlowEdge::~UEdNode_LogicFlowEdge()
{}

void UEdNode_LogicFlowEdge::SetEdge(ULogicFlowTransition* Edge)
{
	FlowTransition = Edge;
}

void UEdNode_LogicFlowEdge::AllocateDefaultPins()
{
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, TEXT("Edge"), FName(), TEXT("In"));
	Inputs->bHidden = true;

	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("Edge"), FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

FText UEdNode_LogicFlowEdge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (IsValid(FlowTransition))
	{
		return FlowTransition->GetTransitionTitle();
	}
	return Super::GetNodeTitle(TitleType);
}

void UEdNode_LogicFlowEdge::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		Modify();

		if (UEdGraph* ParentGraph = GetGraph())
			ParentGraph->Modify();

		DestroyNode();
	}
}

void UEdNode_LogicFlowEdge::PrepareForCopying()
{
	
}

UEdNode_LogicFlowNode* UEdNode_LogicFlowEdge::GetStartNode()
{
	if (Pins.Num() >= 2 && Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UEdNode_LogicFlowNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}
	
	return nullptr;
}

UEdGraphPin* UEdNode_LogicFlowEdge::GetStartPin()
{
	if (Pins.Num() >= 2 && Pins[0]->LinkedTo.Num() > 0)
	{
		return Pins[0]->LinkedTo[0];
	}

	return nullptr;
}

UEdNode_LogicFlowNode* UEdNode_LogicFlowEdge::GetEndNode()
{
	if (Pins.Num() >= 2 && Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UEdNode_LogicFlowNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}
	
	return nullptr;
}

int32 UEdNode_LogicFlowEdge::GetNodeDepth()
{
	UEdNode_LogicFlowNode* StartNode = GetStartNode();
	if (StartNode)
		return StartNode->GetNodeDepth();

	return -1;
}

void UEdNode_LogicFlowEdge::ResetEdgeMessage()
{
	
}

void UEdNode_LogicFlowEdge::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (SEdEdge)
		SEdEdge->UpdateGraphNode();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#undef LOCTEXT_NAMESPACE

