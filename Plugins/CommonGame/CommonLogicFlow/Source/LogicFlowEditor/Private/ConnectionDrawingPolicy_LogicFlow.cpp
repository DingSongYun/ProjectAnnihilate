#include "ConnectionDrawingPolicy_LogicFlow.h"

#include "EdGraph_LogicFlow.h"
#include "Rendering/DrawElements.h"
#include "EdNode_LogicFlowNode.h"
#include "EdNode_LogicFlowEdge.h"
#include "LogicFlowEdge.h"
#include "LogicFlowNode.h"

FConnectionDrawingPolicy_LogicFlow::FConnectionDrawingPolicy_LogicFlow
(
	int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor,
	const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph_LogicFlow* InGraphObj
) : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements), FlowGraph(InGraphObj)
{
}

void FConnectionDrawingPolicy_LogicFlow::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = 1.5f;

	const FLinearColor Default(1.0f, 1.0f, 1.0f);
	const FLinearColor ActiveDebugging(0.2f, 0.80f, 0.3f);

	const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;
	if (bDeemphasizeUnhoveredPins)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, Params.WireThickness, Params.WireColor);
	}

	UEdNode_LogicFlowNode* FromNode1 = OutputPin ? Cast<UEdNode_LogicFlowNode>(OutputPin->GetOwningNode()) : nullptr;
	UEdNode_LogicFlowEdge* FromNode2 = OutputPin ? Cast<UEdNode_LogicFlowEdge>(OutputPin->GetOwningNode()) : nullptr;

	UEdNode_LogicFlowNode* ToNode1 = InputPin ? Cast<UEdNode_LogicFlowNode>(InputPin->GetOwningNode()) : nullptr;
	UEdNode_LogicFlowEdge* ToNode2 = InputPin ? Cast<UEdNode_LogicFlowEdge>(InputPin->GetOwningNode()) : nullptr;

	//Node=>Edge
	if (FromNode1 && ToNode2)
	{
		if (FlowGraph->ShouldShowDebug(ToNode2->FlowTransition))
		// if (FromNode1->FlowNode->bDebuggFlowSelect && ToNode2->FlowTransition->bDebuggFlowSelect)
		{
			Params.WireThickness = 5.0f;
			Params.bDrawBubbles = true;

			Params.WireColor = ActiveDebugging;
		}
	}
	else //Edge=>Node
	if (FromNode2 && ToNode1)
	{
		if (FlowGraph->ShouldShowDebug(FromNode2->FlowTransition))
			// FromNode2->FlowTransition->bDebuggFlowSelect && ToNode1->FlowNode->bDebuggFlowSelect)
		{
			Params.WireThickness = 5.0f;
			Params.bDrawBubbles = true;

			Params.WireColor = ActiveDebugging;
		}
	}
}

void FConnectionDrawingPolicy_LogicFlow::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	NodeWidgetMap.Empty();
	for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
	{
		FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
		TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
		NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
	}

	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FConnectionDrawingPolicy_LogicFlow::DrawPreviewConnector
(
	const FGeometry& PinGeometry, const FVector2D& StartPoint, 
	const FVector2D& EndPoint, UEdGraphPin* Pin
)
{
	FConnectionParams Params;
	DetermineWiringStyle(Pin, nullptr, /*inout*/ Params);

	if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
	{
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, EndPoint), EndPoint, Params);
		// DrawSplineWithArrow(StartPoint, EndPoint, Params);
	}
	else
	{
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, StartPoint), StartPoint, Params);
	}
}

void FConnectionDrawingPolicy_LogicFlow::DrawSplineWithArrow
(
	const FVector2D& StartAnchorPoint, 
	const FVector2D& EndAnchorPoint, 
	const FConnectionParams& Params
)
{
	// bUserFlag1 indicates that we need to reverse the direction of connection (used by debugger)
	const FVector2D& P0 = Params.bUserFlag1 ? EndAnchorPoint : StartAnchorPoint;
	const FVector2D& P1 = Params.bUserFlag1 ? StartAnchorPoint : EndAnchorPoint;

	Internal_DrawLineWithArrow(P0, P1, Params);
}

void FConnectionDrawingPolicy_LogicFlow::Internal_DrawLineWithArrow
(
	const FVector2D& StartAnchorPoint, 
	const FVector2D& EndAnchorPoint, 
	const FConnectionParams& Params
)
{
	//@TODO: Should this be scaled by zoom factor?
	const float LineSeparationAmount = 4.5f;

	const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	// Come up with the final start/end points
	const FVector2D DirectionBias = Normal * LineSeparationAmount;
	const FVector2D LengthBias = ArrowRadius.X * UnitDelta;
	const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

	// Draw a line/spline
	DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

	// Draw the arrow
	const FVector2D ArrowDrawPos = EndPoint - ArrowRadius;
	const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

	FSlateDrawElement::MakeRotatedBox
	(
		DrawElementsList,
		ArrowLayerID,
		FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
		ArrowImage,
		ESlateDrawEffect::None,
		AngleInRadians,
		TOptional<FVector2D>(),
		FSlateDrawElement::RelativeToElement,
		Params.WireColor
	);
}

void FConnectionDrawingPolicy_LogicFlow::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
#if false
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	const FVector2D StartPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);
#else
	const float StartFudgeX = 4.0f;
	const float EndFudgeX = 4.0f;
	const FVector2D StartPoint = FGeometryHelper::VerticalMiddleRightOf(StartGeom) - FVector2D(StartFudgeX, 0.0f);
	// const FVector2D EndPoint = FGeometryHelper::VerticalMiddleLeftOf(EndGeom) - FVector2D(ArrowRadius.X - EndFudgeX, 0);

	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	const FVector2D EndPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);
#endif


	DrawSplineWithArrow(StartPoint, EndPoint, Params);
}

FVector2D FConnectionDrawingPolicy_LogicFlow::ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const
{
	if (FlowGraph->ConnectionStyle == EFlowConnectionStyle::Straight)
	{
		const FVector2D Delta = End - Start;
		const FVector2D NormDelta = Delta.GetSafeNormal();

		return NormDelta;
	}
	return FConnectionDrawingPolicy::ComputeSplineTangent(Start, End);
}

void FConnectionDrawingPolicy_LogicFlow::DetermineLinkGeometry
(
	FArrangedChildren& ArrangedNodes, TSharedRef<SWidget>& OutputPinWidget,
	UEdGraphPin* OutputPin, UEdGraphPin* InputPin, 
	FArrangedWidget*& StartWidgetGeometry, FArrangedWidget*& EndWidgetGeometry
)
{
	if (UEdNode_LogicFlowEdge* EdgeNode = Cast<UEdNode_LogicFlowEdge>(InputPin->GetOwningNode()))
	{
		UEdNode_LogicFlowNode* Start = EdgeNode->GetStartNode();
		UEdNode_LogicFlowNode* End = EdgeNode->GetEndNode();
		if (Start != nullptr && End != nullptr)
		{
			int32* StartNodeIndex = NodeWidgetMap.Find(Start);
			int32* EndNodeIndex = NodeWidgetMap.Find(End);
			if (StartNodeIndex != nullptr && EndNodeIndex != nullptr)
			{
				StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
				// StartWidgetGeometry = &(ArrangedNodes[*StartNodeIndex]);
				EndWidgetGeometry = &(ArrangedNodes[*EndNodeIndex]);
			}
		}
	}
	else
	{
		StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);

		if (TSharedPtr<SGraphPin>* pTargetWidget = PinToPinWidgetMap.Find(InputPin))
		{
			TSharedRef<SGraphPin> InputWidget = pTargetWidget->ToSharedRef();
			EndWidgetGeometry = PinGeometries->Find(InputWidget);
		}
	}
}

