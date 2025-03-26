#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "SEdNode_LogicFlowEdge.h"
#include "EdNode_LogicFlowEdge.generated.h"

class ULogicFlowNode;
class ULogicFlowTransition;
class UEdNode_LogicFlowNode;

UCLASS(MinimalAPI)
class UEdNode_LogicFlowEdge : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UEdNode_LogicFlowEdge();
	virtual ~UEdNode_LogicFlowEdge();

	UPROPERTY()
	class UEdGraph* Graph;

	UPROPERTY(VisibleAnywhere)
	ULogicFlowTransition* FlowTransition;

	SEdNode_LogicFlowEdge* SEdEdge = nullptr;

	void SetEdge(ULogicFlowTransition* Edge);

	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	virtual void PrepareForCopying() override;

	virtual UEdGraphPin* GetInputPin() const { return Pins[0]; }
	virtual UEdGraphPin* GetOutputPin() const { return Pins[1]; }

	UEdNode_LogicFlowNode* GetStartNode();
	UEdGraphPin* GetStartPin();
	UEdNode_LogicFlowNode* GetEndNode();

	virtual int32 GetNodeDepth();
	virtual void ResetEdgeMessage();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

};
