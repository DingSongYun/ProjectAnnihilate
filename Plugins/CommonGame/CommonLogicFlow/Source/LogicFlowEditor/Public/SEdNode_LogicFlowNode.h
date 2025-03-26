#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class FLogicFlowEditor;
class UEdNode_LogicFlowNode;

class SEdNode_LogicFlowNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SEdNode_LogicFlowNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdNode_LogicFlowNode* InNode);

	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual bool IsNameReadOnly() const override;

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

	virtual FSlateColor GetBorderBackgroundColor() const;
	virtual FSlateColor GetNameBackgroundColor() const;
	virtual FSlateColor GetBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

protected:
	FText GetNodeTitle() const;
	FText GetNodeDescription() const;

	TSharedPtr<SBorder> NodeBody;
	TSharedPtr<class SBoxPanel> OutputPinBox;
};
