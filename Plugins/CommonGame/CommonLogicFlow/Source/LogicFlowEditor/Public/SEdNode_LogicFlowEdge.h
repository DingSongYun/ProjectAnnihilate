#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "SNodePanel.h"
#include "SGraphNode.h"

class SToolTip;
class UEdNode_LogicFlowEdge;

class SEdNode_LogicFlowEdge : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SEdNode_LogicFlowEdge) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdNode_LogicFlowEdge* InNode);

	virtual bool RequiresSecondPassLayout() const override;
	virtual void PerformSecondPassLayout(const TMap< UObject*, TSharedRef<SNode> >& NodeToWidgetLookup) const override;

	virtual void UpdateGraphNode() override;

	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;
	

protected:
	FText GetNodeTitle() const;
	FSlateColor GetEdgeColor() const;
	const FSlateBrush* GetEdgeIconImage() const;

	virtual bool IsNameReadOnly() const override
	{
		return true;
	}

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const;

private:
	TSharedPtr<SBorder> NodeBody;
	TSharedPtr<STextEntryPopup> TextEntryWidget;
};
