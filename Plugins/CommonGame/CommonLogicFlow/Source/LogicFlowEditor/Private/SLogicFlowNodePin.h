#pragma once

#include "SGraphPin.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Layout/Margin.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SLogicFlowPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SLogicFlowPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		this->SetCursor(EMouseCursor::Default);
		bShowLabel = true;
		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);
		SGraphPin::Construct(SGraphPin::FArguments(), GraphPinObj);
	}
};

class SLogicFlowPinV2 : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SLogicFlowPinV2) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel = true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &SLogicFlowPinV2::GetMyPinBorder)
			.BorderBackgroundColor(this, &SLogicFlowPinV2::GetPinColor)
			.OnMouseButtonDown(this, &SLogicFlowPinV2::OnPinMouseDown)
			.Cursor(this, &SLogicFlowPinV2::GetPinCursor)
			.Padding(FMargin(10.0f))
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		return FLinearColor(0.02f, 0.02f, 0.02f);
	}

	virtual TSharedRef<SWidget> GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	const FSlateBrush* GetMyPinBorder() const
	{
		return FAppStyle::GetBrush(TEXT("Graph.StateNode.Body"));
	}
};
