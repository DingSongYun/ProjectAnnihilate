#include "SEdNode_LogicFlowEdge.h"
#include "SGraphPanel.h"
#include "ConnectionDrawingPolicy.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "EdNode_LogicFlowNode.h"
#include "EdNode_LogicFlowEdge.h"

#define LOCTEXT_NAMESPACE "SLogicFlowEdge"

void SEdNode_LogicFlowEdge::Construct(const FArguments& InArgs, UEdNode_LogicFlowEdge* InNode)
{
	this->GraphNode = InNode;
	this->UpdateGraphNode();
	InNode->SEdEdge = this;
}

bool SEdNode_LogicFlowEdge::RequiresSecondPassLayout() const
{
	return true;
}

void SEdNode_LogicFlowEdge::PerformSecondPassLayout(const TMap< UObject*, TSharedRef<SNode> >& NodeToWidgetLookup) const
{
	UEdNode_LogicFlowEdge* EdgeNode = CastChecked<UEdNode_LogicFlowEdge>(GraphNode);

	FGeometry StartGeom;
	FGeometry EndGeom;

	UEdNode_LogicFlowNode* Start = EdgeNode->GetStartNode();
	UEdNode_LogicFlowNode* End = EdgeNode->GetEndNode();
	if (Start != nullptr && End != nullptr)
	{
		const TSharedRef<SNode>* pFromWidget = NodeToWidgetLookup.Find(Start);
		const TSharedRef<SNode>* pToWidget = NodeToWidgetLookup.Find(End);
		if (pFromWidget != nullptr && pToWidget != nullptr)
		{
			const TSharedRef<SNode>& FromWidget = *pFromWidget;
			const TSharedRef<SNode>& ToWidget = *pToWidget;

			StartGeom = FGeometry(FVector2D(Start->NodePosX, Start->NodePosY), FVector2D::ZeroVector, FromWidget->GetDesiredSize(), 1.0f);
			EndGeom = FGeometry(FVector2D(End->NodePosX, End->NodePosY), FVector2D::ZeroVector, ToWidget->GetDesiredSize(), 1.0f);
		}
	}

	PositionBetweenTwoNodesWithOffset(StartGeom, EndGeom, 0, 1);
}

void SEdNode_LogicFlowEdge::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();


	FSlateFontInfo StaticFont = FAppStyle::GetFontStyle("LargeText");
	FSlateFontInfo NewFont;
	NewFont.FontObject = StaticFont.FontObject;
	NewFont.OutlineSettings = StaticFont.OutlineSettings;
	NewFont.FontMaterial = StaticFont.FontMaterial;
	NewFont.CompositeFont = StaticFont.CompositeFont;
	NewFont.Size = 10;

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	bool bShowTransitionDetail = false;
	if (bShowTransitionDetail)
	{
		this->GetOrAddSlot(ENodeZone::Center)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SImage)
					.Image( FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body") )
					.ColorAndOpacity( this, &SEdNode_LogicFlowEdge::GetEdgeColor )
				]
				+SOverlay::Slot()
				.Padding(4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						SNew(SImage)
						.Image( this, &SEdNode_LogicFlowEdge::GetEdgeIconImage )
					]
					+ SHorizontalBox::Slot()
					.Padding(4.f, 2.f, 4.f, 2.f)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
						.Font(NewFont)
						.Text(this, &SEdNode_LogicFlowEdge::GetNodeTitle)
						.Visibility_Lambda([&] (){ return GetNodeTitle().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
					]
				]
			];
	}
	else
	{
		this->SetVisibility(EVisibility::Hidden);
	}
}

void SEdNode_LogicFlowEdge::PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const
{
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	// Position ourselves halfway along the connecting line between the nodes, elevated away perpendicular to the direction of the line
	const float Height = 30.0f;

	const FVector2D DesiredNodeSize = GetDesiredSize();

	FVector2D DeltaPos(EndAnchorPoint - StartAnchorPoint);

	if (DeltaPos.IsNearlyZero())
	{
		DeltaPos = FVector2D(10.0f, 0.0f);
	}

	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	const FVector2D NewCenter = StartAnchorPoint + (0.5f * DeltaPos) + (Height * Normal);

	FVector2D DeltaNormal = DeltaPos.GetSafeNormal();

	// Calculate node offset in the case of multiple transitions between the same two nodes
	// MultiNodeOffset: the offset where 0 is the centre of the transition, -1 is 1 <size of node>
	// towards the PrevStateNode and +1 is 1 <size of node> towards the NextStateNode.

	const float MutliNodeSpace = 0.2f; // Space between multiple transition nodes (in units of <size of node> )
	const float MultiNodeStep = (1.f + MutliNodeSpace); //Step between node centres (Size of node + size of node spacer)

	const float MultiNodeStart = -((MaxNodes - 1) * MultiNodeStep) / 2.f;
	const float MultiNodeOffset = MultiNodeStart + (NodeIndex * MultiNodeStep);

	// Now we need to adjust the new center by the node size, zoom factor and multi node offset
	const FVector2D NewCorner = NewCenter - (0.5f * DesiredNodeSize) + (DeltaNormal * MultiNodeOffset * DesiredNodeSize.Size());

	GraphNode->NodePosX = NewCorner.X;
	GraphNode->NodePosY = NewCorner.Y;
}

FSlateColor SEdNode_LogicFlowEdge::GetEdgeColor() const
{
	return FLinearColor(0.24f, 0.24f, 0.24f, 1.0f);
	// return FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
}

const FSlateBrush* SEdNode_LogicFlowEdge::GetEdgeIconImage() const
{
	return FAppStyle::GetBrush("BTEditor.Graph.BTNode.Decorator.Conditional.Icon");
}

FVector2D SEdNode_LogicFlowEdge::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return SGraphNode::ComputeDesiredSize(LayoutScaleMultiplier);
	// if (UEdNode_LogicFlowEdge* EdgeNode = CastChecked<UEdNode_LogicFlowEdge>(FlowNode))
	// {
	// 	if (EdgeNode->FlowTransition)
	// 		return EdgeNode->FlowTransition->EdgeMessageSize * LayoutScaleMultiplier;
	// }
	//
	// return FVector2D(5.0, 5.0f) * LayoutScaleMultiplier;
}

FText SEdNode_LogicFlowEdge::GetNodeTitle() const
{
	FText NodeTitle;
	UEdNode_LogicFlowEdge* TheEdge = Cast<UEdNode_LogicFlowEdge>(this->GraphNode);
	if (TheEdge)
	{
		NodeTitle = TheEdge->GetNodeTitle(ENodeTitleType::Type::FullTitle);
	}
	return NodeTitle;
}
#undef LOCTEXT_NAMESPACE
