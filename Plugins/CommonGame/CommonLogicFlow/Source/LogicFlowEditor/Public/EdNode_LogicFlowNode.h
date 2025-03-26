#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "EdNode_LogicFlowNode.generated.h"

class ULogicFlowNode;
class UEdGraph_LogicFlow;
class UEdNode_LogicFlowEdge;
class SEdNode_LogicFlowNode;

UCLASS(MinimalAPI)
class UEdNode_LogicFlowNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UEdNode_LogicFlowNode();
	virtual ~UEdNode_LogicFlowNode();

	void SetFlowNode(ULogicFlowNode* InNode);
	UEdGraph_LogicFlow* GetFlowEdGraph();

	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetNodeDescription() const;
	virtual void PrepareForCopying() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual void ReconstructNode() override;
	void RewireOldPinsToNewPins(TArray<UEdGraphPin*>& InOldPins, TArray<UEdGraphPin*>& InNewPins, TMap<UEdGraphPin*, UEdGraphPin*>* NewPinToOldPin);
	void DestroyPinList(TArray<UEdGraphPin*>& InPins);

	virtual FLinearColor GetBackgroundColor() const;
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

	// 获取该节点的深度
	virtual int32 GetNodeDepth();

	void DestroyNode() override;

	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	void AddSelection(class ULogicFlowNode_Select* SelectNode);
	void RemoveSelection(ULogicFlowNode_Select* SelectNode, int32 Index);
	void GetRemoveCurveMenuActions(class FMenuBuilder& MenuBuilder, ULogicFlowNode_Select* SelectNode) const;
	
public:
	UPROPERTY(VisibleAnywhere)
	class ULogicFlowNode* FlowNode = nullptr;

	UPROPERTY()
	FString EditName;
};

UCLASS(MinimalAPI)
class UEdNode_LogicFlowRootNode : public UEdNode_LogicFlowNode
{
	GENERATED_BODY()
	
public:
};