#include "EdNode_LogicFlowNode.h"
#include "EdGraph_LogicFlow.h"
#include "EdNode_LogicFlowEdge.h"
#include "LogicFlowNode.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#define LOCTEXT_NAMESPACE "EdNode_LogicFlow"

UEdNode_LogicFlowNode::UEdNode_LogicFlowNode()
{
	bCanRenameNode = true;
}

UEdNode_LogicFlowNode::~UEdNode_LogicFlowNode()
{}

void UEdNode_LogicFlowNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNodes", FName(), TEXT("In"));
	check(FlowNode);
	TArray<FName> OutPinNames = FlowNode->GetOutPinNames();
	for (const FName& PinName : OutPinNames)
	{
		CreatePin( EGPD_Output, "MultipleNodes", FName(), PinName );
	}
}

UEdGraph_LogicFlow* UEdNode_LogicFlowNode::GetFlowEdGraph()
{
	return Cast<UEdGraph_LogicFlow>(GetGraph());
}

FText UEdNode_LogicFlowNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText BaseTitle = Super::GetNodeTitle(TitleType);
	if (IsValid(FlowNode))
	{
		BaseTitle = FlowNode->GetNodeTitle();
	}

	if (TitleType == ENodeTitleType::EditableTitle)
	{
		if (!EditName.IsEmpty() && EditName != TEXT("None")) BaseTitle = FText::FromString(EditName);
	}

	if (BaseTitle.IsEmpty()) BaseTitle = FText::FromString("None");

	return BaseTitle;
}

FText UEdNode_LogicFlowNode::GetNodeDescription() const
{
	if (IsValid(FlowNode))
	{
		return FlowNode->GetNodeDescription();
	}
	return FText::GetEmpty();
}

void UEdNode_LogicFlowNode::PrepareForCopying()
{
	
}

void UEdNode_LogicFlowNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
			FromPin->GetOwningNode()->NodeConnectionListChanged();
	}
}

void UEdNode_LogicFlowNode::ReconstructNode()
{
	Modify();

	// Clear previously set messages
	ErrorMsg.Reset();


	const UEdGraphSchema* Schema = GetSchema();

	// Break any links to 'orphan' pins
	for (int32 PinIndex = 0; PinIndex < Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = Pins[PinIndex];
		TArray<class UEdGraphPin*> LinkedToCopy = Pin->LinkedTo;
		for (int32 LinkIdx = 0; LinkIdx < LinkedToCopy.Num(); LinkIdx++)
		{
			UEdGraphPin* OtherPin = LinkedToCopy[LinkIdx];
			// If we are linked to a pin that its owner doesn't know about, break that link
			if ((OtherPin == nullptr) || !OtherPin->GetOwningNodeUnchecked() || !OtherPin->GetOwningNode()->Pins.Contains(OtherPin))
			{
				Pin->LinkedTo.Remove(OtherPin);
			}
		}
	}

	// Move the existing pins to a saved array
	TArray<UEdGraphPin*> OldPins(Pins);
	Pins.Reset();

	// Recreate the new pins
	TMap<UEdGraphPin*, UEdGraphPin*> NewPinsToOldPins;
	AllocateDefaultPins();
	// ReallocatePinsDuringReconstruction(OldPins);
	RewireOldPinsToNewPins(OldPins, Pins, &NewPinsToOldPins);

	GetGraph()->NotifyNodeChanged(this);
}

void UEdNode_LogicFlowNode::RewireOldPinsToNewPins(TArray<UEdGraphPin*>& InOldPins, TArray<UEdGraphPin*>& InNewPins, TMap<UEdGraphPin*, UEdGraphPin*>* NewPinToOldPin)
{
	TArray<UEdGraphPin*> OrphanedOldPins;
	TArray<bool> NewPinMatched; // Tracks whether a NewPin has already been matched to an OldPin
	TMap<UEdGraphPin*, UEdGraphPin*> MatchedPins; // Old to New

	const int32 NumNewPins = InNewPins.Num();
	NewPinMatched.AddDefaulted(NumNewPins);
	const bool bSaveUnconnectedDefaultPins = false;

	// Rewire any connection to pins that are matched by name (O(N^2) right now)
	// NOTE: we iterate backwards through the list because ReconstructSinglePin()
	//       destroys pins as we go along (clearing out parent pointers, etc.);
	//       we need the parent pin chain intact for DoPinsMatchForReconstruction();
	//       we want to destroy old pins from the split children (leafs) up, so
	//       we do this since split child pins are ordered later in the list
	//       (after their parents)
	for (int32 OldPinIndex = InOldPins.Num()-1; OldPinIndex >= 0; --OldPinIndex)
	{
		UEdGraphPin* OldPin = InOldPins[OldPinIndex];

		// common case is for InOldPins and InNewPins to match, so we start searching from the current index:
		bool bMatched = false;
		int32 NewPinIndex = (NumNewPins ? OldPinIndex % NumNewPins : 0);
		for (int32 NewPinCount = NumNewPins - 1; NewPinCount >= 0; --NewPinCount)
		{
			// if InNewPins grows then we may skip entries and fail to find a match or NewPinMatched will not be accurate
			check(NumNewPins == InNewPins.Num());
			if (!NewPinMatched[NewPinIndex])
			{
				UEdGraphPin* NewPin = InNewPins[NewPinIndex];

				// ERedirectType RedirectType = DoPinsMatchForReconstruction(NewPin, NewPinIndex, OldPin, OldPinIndex);
				//
				// if (RedirectType == ERedirectType_DefaultValue)
				// {
				// 	if (!TryInsertDefaultValueConversionNode(*OldPin, *NewPin))
				// 	{
				// 		RedirectType = ERedirectType_None;
				// 	}
				// }
				//
				// if (RedirectType != ERedirectType_None)
				// {
				// 	ReconstructSinglePin(NewPin, OldPin, RedirectType);
				// 	MatchedPins.Add(OldPin, NewPin);
				// 	if (NewPinToOldPin)
				// 	{
				// 		NewPinToOldPin->Add(NewPin, OldPin);
				// 	}
				// 	bMatched = true;
				// 	NewPinMatched[NewPinIndex] = true;
				// 	break;
				// }
			}
			NewPinIndex = (NewPinIndex + 1) % InNewPins.Num();
		}

		// Orphaned pins are those that existed in the OldPins array but do not in the NewPins.
		// We will save these pins and add them to the NewPins array if they are linked to other pins or have non-default value unless:
		// * The node has been flagged to not save orphaned pins
		// * The pin has been flagged not be saved if orphaned
		// * The pin is hidden and not a split pin
		const bool bVisibleOrSplitPin = (!OldPin->bHidden || (OldPin->SubPins.Num() > 0));
		if (UEdGraphPin::AreOrphanPinsEnabled() && !bDisableOrphanPinSaving && !bMatched && bVisibleOrSplitPin && OldPin->ShouldSavePinIfOrphaned())
		{
			// The node can specify to save no pins, all pins, or all but exec pins. However, even if all is specified Execute and Then are never saved
			 const bool bSaveOrphanedPin = OrphanedPinSaveMode == ESaveOrphanPinMode::SaveAll;

			if (bSaveOrphanedPin)
			{
				bool bSavePin = bSaveUnconnectedDefaultPins || (OldPin->LinkedTo.Num() > 0);

				if (!bSavePin && OldPin->SubPins.Num() > 0)
				{
					// If this is a split pin then we need to save it if any of its children are being saved
					for (UEdGraphPin* OldSubPin : OldPin->SubPins)
					{
						if (OldSubPin->bOrphanedPin)
						{
							bSavePin = true;
							break;
						}
					}
					// Once we know we are going to be saving it we need to clean up the SubPins list to be only pins being saved
					if (bSavePin)
					{
						for (int32 SubPinIndex = OldPin->SubPins.Num() - 1; SubPinIndex >= 0; --SubPinIndex)
						{
							UEdGraphPin* SubPin = OldPin->SubPins[SubPinIndex];
							if (!SubPin->bOrphanedPin)
							{
								OldPin->SubPins.RemoveAt(SubPinIndex, 1, false);
								SubPin->MarkAsGarbage();
							}
						}
					}
				}

				// Input pins with non-default value should be saved
				if (!bSavePin && OldPin->Direction == EGPD_Input && !OldPin->DoesDefaultValueMatchAutogenerated())
				{
					bSavePin = true;
				}

				if (bSavePin)
				{
					OldPin->bOrphanedPin = true;
					OldPin->bNotConnectable = true;
					OrphanedOldPins.Add(OldPin);
					InOldPins.RemoveAt(OldPinIndex, 1, false);
				}
			}
		}
	}

	// The orphaned pins get placed after the rest of the new pins unless it is a child of a split pin and other
	// children of that split pin were matched in which case it will be at the end of the list of its former siblings
	for (int32 OrphanedIndex = OrphanedOldPins.Num() - 1; OrphanedIndex >= 0; --OrphanedIndex)
	{
		UEdGraphPin* OrphanedPin = OrphanedOldPins[OrphanedIndex];
		if (OrphanedPin->ParentPin == nullptr)
		{
			InNewPins.Add(OrphanedPin);
		}
		// Otherwise we need to work out where we fit in the list
		else
		{
			UEdGraphPin* ParentPin = OrphanedPin->ParentPin;
			if (!ParentPin->bOrphanedPin)
			{
				// Our parent pin was matched, so we need to go to the end of the new pins sub pin section
				ParentPin->SubPins.Remove(OrphanedPin);
				ParentPin = MatchedPins.FindChecked(ParentPin);
				ParentPin->SubPins.Add(OrphanedPin);
				OrphanedPin->ParentPin = ParentPin;
			}
			int32 InsertIndex = InNewPins.Find(ParentPin);
			while (++InsertIndex < InNewPins.Num())
			{
				UEdGraphPin* PinToConsider = InNewPins[InsertIndex];
				if (PinToConsider->ParentPin != ParentPin)
				{
					break;
				}
				int32 WalkOffIndex = InsertIndex + PinToConsider->SubPins.Num();
				for (;InsertIndex < WalkOffIndex;++InsertIndex)
				{
					WalkOffIndex += InNewPins[WalkOffIndex]->SubPins.Num();
				}
			};

			InNewPins.Insert(OrphanedPin, InsertIndex);
		}
	}

	DestroyPinList(InOldPins);
}

void UEdNode_LogicFlowNode::DestroyPinList(TArray<UEdGraphPin*>& InPins)
{
	// Throw away the original pins
	for (UEdGraphPin* Pin : InPins)
	{
		Pin->Modify();
		Pin->BreakAllPinLinks(true);

		UEdGraphNode::DestroyPin(Pin);
	}
}

void UEdNode_LogicFlowNode::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	if (ULogicFlowNode_Select* SelectNode = Cast<ULogicFlowNode_Select>(FlowNode))
	{
		FToolMenuSection& Section = Menu->AddSection("LogicFlowNodeSelect", LOCTEXT("Selection", "Modify Selection"));
		Section.AddMenuEntry(
			"Add Selection",
			LOCTEXT("FlowNode_Select_AddSelection", "Add Selection"),
			LOCTEXT("FlowNode_Select_AddSelection_Tooltip", "Add a new selection"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateUObject(const_cast<UEdNode_LogicFlowNode*>(this), &UEdNode_LogicFlowNode::AddSelection, SelectNode))
			);

		if (SelectNode->GetOutPinNums() > 0)
		{
			Section.AddSubMenu(
				"Remove Selection",
				LOCTEXT("FlowNode_Select_RemoveSelection", "Remove Curve Pin"),
				LOCTEXT("FlowNode_Select_RemoveSelection_Tooltip", "Remove a selection"),
				FNewMenuDelegate::CreateUObject(this, &UEdNode_LogicFlowNode::GetRemoveCurveMenuActions, SelectNode));
		}
	}

}

void UEdNode_LogicFlowNode::SetFlowNode(ULogicFlowNode* InNode)
{
	FlowNode = InNode;
}

FLinearColor UEdNode_LogicFlowNode::GetBackgroundColor() const
{
	return FLinearColor::Black;
}

UEdGraphPin* UEdNode_LogicFlowNode::GetInputPin() const
{
	return Pins[0];
}

UEdGraphPin* UEdNode_LogicFlowNode::GetOutputPin() const
{
	return Pins[1];
}

void UEdNode_LogicFlowNode::PostEditUndo()
{
	UEdGraphNode::PostEditUndo();
}

void UEdNode_LogicFlowNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UEdGraphNode::PostEditChangeProperty(PropertyChangedEvent);
}

void UEdNode_LogicFlowNode::AddSelection(ULogicFlowNode_Select* SelectNode)
{
	Modify();
	SelectNode->AddNewSelection();
	FCreatePinParams Params;
	Params.Index = GetAllPins().Num() - 1; // 保持Out是最后一个Pin
	CreatePin( EGPD_Output, "MultipleNodes", FName(), SelectNode->GetOutputPinNameOfIndex(SelectNode->GetOutPinNums() - 1), Params );
	// ReconstructNode();
	GetGraph()->NotifyNodeChanged(this);
}

void UEdNode_LogicFlowNode::RemoveSelection(ULogicFlowNode_Select* SelectNode, int32 Index)
{
	Modify();
	if (SelectNode->GetOutPinNums() > 1)
	{
		if (UEdGraphPin* Pin = GetPinAt(Index + 1))
		{
			RemovePin(Pin);
		}
	}
	SelectNode->RemoveSelection(Index);
	// ReconstructNode();
	GetGraph()->NotifyNodeChanged(this);
}

void UEdNode_LogicFlowNode::GetRemoveCurveMenuActions(FMenuBuilder& MenuBuilder, ULogicFlowNode_Select* SelectNode) const
{
	for (int i = 0; i < SelectNode->Conditions.Num(); ++ i)
	{
		FUIAction Action = FUIAction(FExecuteAction::CreateUObject(const_cast<UEdNode_LogicFlowNode*>(this), &UEdNode_LogicFlowNode::RemoveSelection, SelectNode, i));
		MenuBuilder.AddMenuEntry(FText::FromString(FString::FromInt(i + 1)), FText::GetEmpty(), FSlateIcon(), Action);
	}
}

int32 UEdNode_LogicFlowNode::GetNodeDepth()
{
	if (Pins[0]->LinkedTo.Num() == 0)
		return 0;

	int32 Depth = 10000;
	for (int32 i = 0; i < Pins[0]->LinkedTo.Num(); ++i)
	{
		int32 TmpDepth = 10000;
		UEdNode_LogicFlowEdge* Edge = Cast<UEdNode_LogicFlowEdge>(Pins[0]->LinkedTo[i]->GetOwningNode());
		if (UEdNode_LogicFlowNode* FatherNode = Edge->GetStartNode())
		{
			TmpDepth = FatherNode->GetNodeDepth();
			if (TmpDepth < Depth)
				Depth = TmpDepth;
		}
	}

	return Depth + 1;
}

void UEdNode_LogicFlowNode::DestroyNode()
{
	FlowNode = nullptr;
	Super::DestroyNode();
}

#undef LOCTEXT_NAMESPACE
