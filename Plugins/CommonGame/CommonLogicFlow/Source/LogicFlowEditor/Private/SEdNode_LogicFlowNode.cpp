#include "SEdNode_LogicFlowNode.h"
#include "EdGraph_LogicFlow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SCommentBubble.h"
#include "SlateOptMacros.h"
#include "SGraphPin.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "ScopedTransaction.h"
#include "EdNode_LogicFlowNode.h"
#include "LogicFlowEditor.h"
#include "LogicFlowNode.h"
#include "SLogicFlowNodePin.h"
#include "Styling/SlateStyleMacros.h"

#define LOCTEXT_NAMESPACE "EdNode_LogicFlow"


//////////////////////////////////////////////////////////////////////////
void SEdNode_LogicFlowNode::Construct(const FArguments& InArgs, UEdNode_LogicFlowNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SEdNode_LogicFlowNode::UpdateGraphNode()
{
	const FMargin NodePadding = FMargin(2.0f);

	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	OutputPinBox.Reset();

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	EFlowDrawDirection FlowDrawStyle = Cast<UEdGraph_LogicFlow>(GraphNode->GetGraph())->FlowDrawDirection;

	auto CreateInputPinArea = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.MinDesiredHeight(NodePadding.Top)
			[
				SAssignNew(LeftNodeBox, SVerticalBox)
			];
	};
	auto CreateOutputPinArea = [&] () -> TSharedRef<SWidget>
	{
		switch(FlowDrawStyle)
		{
		case EFlowDrawDirection::Vertical:
			SAssignNew(OutputPinBox, SHorizontalBox);
			break;
		case EFlowDrawDirection::Horizontal:
			SAssignNew(OutputPinBox, SVerticalBox);
			break;
		}
		return SNew(SBox)
			.MinDesiredHeight(NodePadding.Bottom)
			[
				SAssignNew(RightNodeBox, SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(5.0f, 0.0f)
				.FillHeight(1.0f)
				[
					OutputPinBox.ToSharedRef()
				]
			];
	};
	auto CreateNodeContent = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SOverlay)
		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(5.0f)
		[
			SNew(SBorder)
			.BorderImage( FAppStyle::GetBrush("Graph.StateNode.ColorSpill") )
			.BorderBackgroundColor( GetNameBackgroundColor() )
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Visibility(EVisibility::SelfHitTestInvisible)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					// POPUP ERROR MESSAGE
					SAssignNew(ErrorText, SErrorText )
					.BackgroundColor( this, &SEdNode_LogicFlowNode::GetErrorColor )
					.ToolTipText( this, &SEdNode_LogicFlowNode::GetErrorMsgToolTip )
				]
				+SHorizontalBox::Slot()
				.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
						.AutoHeight()
					[
						SAssignNew(InlineEditableText, SInlineEditableTextBlock)
						.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText" )
						.Text( NodeTitle.Get(), &SNodeTitle::GetHeadTitle )
						.OnVerifyTextChanged(this, &SEdNode_LogicFlowNode::OnVerifyNameTextChanged)
						.OnTextCommitted(this, &SEdNode_LogicFlowNode::OnNameTextCommited)
						.IsReadOnly( this, &SEdNode_LogicFlowNode::IsNameReadOnly )
						.IsSelected(this, &SEdNode_LogicFlowNode::IsSelectedExclusively)
					]
					+SVerticalBox::Slot()
						.AutoHeight()
					[
						NodeTitle.ToSharedRef()
					]
					+SVerticalBox::Slot()
						.AutoHeight()
					[
						SNew(STextBlock)
						.Font(DEFAULT_FONT("Regular", 10))
						.Text( this, &SEdNode_LogicFlowNode::GetNodeDescription)
						.Visibility_Lambda([this] ()
						{
							return GetNodeDescription().IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible;
						})
					]
				]
			]
		];
	};

	auto CreateNodeBody = [&]() -> TSharedRef<SWidget>
	{
		if (FlowDrawStyle == EFlowDrawDirection::Horizontal)
		{
			return SNew(SHorizontalBox)
			// Input Pins
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				CreateInputPinArea()
			]
			// Node Context
			+ SHorizontalBox::Slot()
			.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
			[
				CreateNodeContent()
			]
			// Output Pins
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				CreateOutputPinArea()
			];
		}
		else if (FlowDrawStyle == EFlowDrawDirection::Vertical)
		{
			return SNew(SVerticalBox)

			// Input Pins
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateInputPinArea()
			]
			// Node Context
			+ SVerticalBox::Slot()
			.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
			[
				CreateNodeContent()
			]

			// Output Pins
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateOutputPinArea()
			];
		}

		return SNullWidget::NullWidget;
	};

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0.0f)
			.BorderBackgroundColor(this, &SEdNode_LogicFlowNode::GetBorderBackgroundColor)
			[
				SNew(SOverlay)

				// Pins and node details
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					CreateNodeBody()
				]
			]
		];

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = FLinearColor::White;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

void SEdNode_LogicFlowNode::CreatePinWidgets()
{
	UEdNode_LogicFlowNode* StateNode = CastChecked<UEdNode_LogicFlowNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SLogicFlowPin, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SEdNode_LogicFlowNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		InputPins.Add(PinToAdd);
	}
	else
	{
		EFlowDrawDirection FlowDrawStyle = Cast<UEdGraph_LogicFlow>(GraphNode->GetGraph())->FlowDrawDirection;
		if (FlowDrawStyle == EFlowDrawDirection::Horizontal)
		{
			TSharedPtr<SVerticalBox> OutputPinVerticalBox = StaticCastSharedPtr<SVerticalBox>(OutputPinBox);
			OutputPinVerticalBox->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillHeight(1.0f)
				[
					PinToAdd
				];

		}
		else if (FlowDrawStyle == EFlowDrawDirection::Vertical)
		{
			TSharedPtr<SHorizontalBox> OutputPinHorizontalBox = StaticCastSharedPtr<SHorizontalBox>(OutputPinBox);
			OutputPinHorizontalBox->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(1.0f)
				[
					PinToAdd
				];
		}
		OutputPins.Add(PinToAdd);
	}
}

bool SEdNode_LogicFlowNode::IsNameReadOnly() const
{
	return false;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SEdNode_LogicFlowNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UEdNode_LogicFlowNode* MyNode = CastChecked<UEdNode_LogicFlowNode>(GraphNode);

	if (MyNode != nullptr && MyNode->FlowNode != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("LogicFlowEditorRenameNode", "Logic Flow Editor: Rename Node"));
		MyNode->Modify();
		MyNode->FlowNode->Modify();
		MyNode->EditName = InText.ToString();
		UpdateGraphNode();
	}
}

FSlateColor SEdNode_LogicFlowNode::GetBorderBackgroundColor() const
{
	FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);
	const FLinearColor ActiveDebugging(0.2f, 0.80f, 0.3f);

	UEdNode_LogicFlowNode* MyNode = CastChecked<UEdNode_LogicFlowNode>(GraphNode);
	check(MyNode);
	const UEdGraph_LogicFlow* FlowGraph = Cast<UEdGraph_LogicFlow>(GraphNode->GetGraph());
	if (FlowGraph->ShouldShowDebug(MyNode->FlowNode))
	{
		return ActiveDebugging;
	}

	if (MyNode && MyNode->FlowNode)
	{
		return MyNode->FlowNode->GetNodeColor();
	}
	return InactiveStateColor;
}

FSlateColor SEdNode_LogicFlowNode::GetNameBackgroundColor() const
{
	return FLinearColor(0.6f, 0.6f, 0.6f, 0.0f);
}

FSlateColor SEdNode_LogicFlowNode::GetBackgroundColor() const
{
	return FLinearColor(0.1f, 0.1f, 0.1f);
}

EVisibility SEdNode_LogicFlowNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* SEdNode_LogicFlowNode::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

FText SEdNode_LogicFlowNode::GetNodeTitle() const
{
	UEdNode_LogicFlowNode* FlowNode = Cast<UEdNode_LogicFlowNode>(this->GraphNode);
	if (FlowNode)
	{
		return FlowNode->GetNodeTitle(ENodeTitleType::Type::FullTitle);
	}
	return FText();
}

FText SEdNode_LogicFlowNode::GetNodeDescription() const
{
	UEdNode_LogicFlowNode* FlowNode = Cast<UEdNode_LogicFlowNode>(this->GraphNode);
	if (FlowNode)
	{
		return FlowNode->GetNodeDescription();
	}
	return FText::GetEmpty();
}
#undef LOCTEXT_NAMESPACE
