// Copyright Epic Games, Inc. All Rights Reserved.

#include "SSelectedFlowDebugObjectWidget.h"

#include "EdGraph_LogicFlow.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "EditorStyleSet.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "PropertyCustomizationHelpers.h"
#include "IDocumentation.h"
#include "LogicFlowEditor.h"
#include "LogicFlowEvalContext.h"
#include "SLevelOfDetailBranchNode.h"
#include "Widgets/Input/STextComboBox.h"
#include "Components/Widget.h"

#define LOCTEXT_NAMESPACE "StateMachineToolbar"

static TAutoConsoleVariable<int32> CVarUseFastDebugObjectDiscovery(TEXT("r.UseFastDebugObjectDiscovery"), 1, TEXT("Enable new optimised debug object discovery"));

//////////////////////////////////////////////////////////////////////////
// SSelectedDebugObjectWidget

void SSelectedDebugObjectWidget::Construct(const FArguments& InArgs, TSharedPtr<FAssetEditorToolkit> InHostEditor)
{
	HostEditor = InHostEditor;

	GenerateDebugWorldNames(false);
	GenerateDebugObjectInstances(false);

	LastObjectObserved = nullptr;

	DebugWorldsComboBox = SNew(STextComboBox)
		// .ToolTip(IDocumentation::Get()->CreateToolTip(
		// LOCTEXT("DebugWorldTooltip", "Select a world to debug, will filter what to debug if no specific object selected"),
		// nullptr,
		// TEXT("Shared/Editors/HostEditor/LogicFlowDebugger"),
		// TEXT("DebugWorld")))
		.OptionsSource(&DebugWorldNames)
		.InitiallySelectedItem(GetDebugWorldName())
		.Visibility(this, &SSelectedDebugObjectWidget::IsDebugWorldComboVisible)
		.OnComboBoxOpening(this, &SSelectedDebugObjectWidget::GenerateDebugWorldNames, true)
		.OnSelectionChanged(this, &SSelectedDebugObjectWidget::DebugWorldSelectionChanged);

	DebugObjectsComboBox = SNew(SComboBox<TSharedPtr<FDebugObjectInstance>>)
		// .ToolTip(IDocumentation::Get()->CreateToolTip(
		// LOCTEXT("StatemachineDebugObjectTooltip", "Select an object to debug, if set to none will debug any object"),
		// nullptr,
		// TEXT("Shared/Editors/HostEditor/StatemachineDebugger"),
		// TEXT("DebugObject")))
		.OptionsSource(&DebugObjects)
		.InitiallySelectedItem(GetDebugObjectInstance())
		.OnComboBoxOpening(this, &SSelectedDebugObjectWidget::GenerateDebugObjectInstances, true)
		.OnSelectionChanged(this, &SSelectedDebugObjectWidget::DebugObjectSelectionChanged)
		.OnGenerateWidget(this, &SSelectedDebugObjectWidget::CreateDebugObjectItemWidget)
		.AddMetaData<FTagMetaData>(TEXT("SelectDebugObjectCobmo"))
		[
			SNew(STextBlock)
			.Text(this, &SSelectedDebugObjectWidget::GetSelectedDebugObjectTextLabel)
		];

	const TSharedRef<SWidget> BrowseButton = PropertyCustomizationHelpers::MakeBrowseButton(FSimpleDelegate::CreateSP(this, &SSelectedDebugObjectWidget::SelectedDebugObject_OnClicked));
	BrowseButton->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &SSelectedDebugObjectWidget::IsSelectDebugObjectButtonVisible)));
	BrowseButton->SetToolTipText(LOCTEXT("DebugSelectActor", "Select this Actor in level"));

	TSharedRef<SWidget> DebugObjectSelectionWidget =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
		[
			DebugObjectsComboBox.ToSharedRef()
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(4.0f)
		[
			BrowseButton
		];


	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(0.0f)
		.AutoWidth()
		[
			DebugWorldsComboBox.ToSharedRef()
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(0.0f)
		.AutoWidth()
		[
			DebugObjectSelectionWidget
		]
		// SNew(SLevelOfDetailBranchNode)
		// .UseLowDetailSlot(FMultiBoxSettings::UseSmallToolBarIcons)
		// .OnGetActiveDetailSlotContent(this, &SSelectedDebugObjectWidget::OnGetActiveDetailSlotContent)
	];
}

void SSelectedDebugObjectWidget::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (UObject* Object = GetObjectBeingDebugged())
	{
		if (Object != LastObjectObserved.Get())
		{
			// bRestoreSelection attempts to restore the selection by name, 
			// this ensures that if the last object we had selected was 
			// regenerated (spawning a new object), then we select that  
			// again, even if it is technically a different object
			GenerateDebugObjectInstances(/*bRestoreSelection =*/true);

			LastObjectObserved = Object;
		}
	}
	else
	{
		LastObjectObserved = nullptr;

		// If the object name is a name (rather than the 'No debug selected' string then regenerate the names (which will reset the combo box) as the object is invalid.
		TSharedPtr<FDebugObjectInstance> CurrentSelection = DebugObjectsComboBox->GetSelectedItem();
		if (CurrentSelection.IsValid() && CurrentSelection->IsEditorObject())
		{
			GenerateDebugObjectInstances(false);
		}
	}
}

const FString& SSelectedDebugObjectWidget::GetNoDebugString() const
{
	return NSLOCTEXT("HostEditor", "DebugObjectNothingSelected", "No debug object selected").ToString();
}

const FString& SSelectedDebugObjectWidget::GetDebugAllWorldsString() const
{
	return NSLOCTEXT("HostEditor", "DebugWorldNothingSelected", "All Worlds").ToString();
}

// TSharedRef<SWidget> SSelectedDebugObjectWidget::OnGetActiveDetailSlotContent(bool bChangedToHighDetail)
// {
// 	const TSharedRef<SWidget> BrowseButton = PropertyCustomizationHelpers::MakeBrowseButton(FSimpleDelegate::CreateSP(this, &SSelectedDebugObjectWidget::SelectedDebugObject_OnClicked));
// 	BrowseButton->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &SSelectedDebugObjectWidget::IsSelectDebugObjectButtonVisible)));
// 	BrowseButton->SetToolTipText(LOCTEXT("DebugSelectActor", "Select this Actor in level"));
//
// 	TSharedRef<SWidget> DebugObjectSelectionWidget =
// 		SNew(SHorizontalBox)
// 		+ SHorizontalBox::Slot()
// 		.AutoWidth()
// 		.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
// 		[
// 			DebugObjectsComboBox.ToSharedRef()
// 		]
// 		+ SHorizontalBox::Slot()
// 		.AutoWidth()
// 		.HAlign(HAlign_Right)
// 		.VAlign(VAlign_Center)
// 		.Padding(4.0f)
// 		[
// 			BrowseButton
// 		];
//
//
// 		return
// 			SNew(SHorizontalBox)
// 			+ SHorizontalBox::Slot()
// 			.VAlign(VAlign_Center)
// 			.Padding(0.0f)
// 			.AutoWidth()
// 			[
// 				DebugWorldsComboBox.ToSharedRef()
// 			]
// 			+ SHorizontalBox::Slot()
// 			.VAlign(VAlign_Center)
// 			.Padding(0.0f)
// 			.AutoWidth()
// 			[
// 				DebugObjectSelectionWidget
// 			];
// }

void SSelectedDebugObjectWidget::OnRefresh()
{
	GenerateDebugWorldNames(false);
	GenerateDebugObjectInstances(false);

	if (DebugObjectsComboBox.IsValid())
	{
		DebugWorldsComboBox->SetSelectedItem(GetDebugWorldName());
		DebugObjectsComboBox->SetSelectedItem(GetDebugObjectInstance());
	}
}

void SSelectedDebugObjectWidget::GenerateDebugWorldNames(bool bRestoreSelection)
{
	DebugWorldNames.Empty();
	DebugWorlds.Empty();

	DebugWorlds.Add(nullptr);
	DebugWorldNames.Add(MakeShareable(new FString(GetDebugAllWorldsString())));

	//UWorld* PreviewWorld = HostEditor.Pin()->GetPreviewScene()->GetWorld();

	for (TObjectIterator<UWorld> It; It; ++It)
	{
		UWorld *TestWorld = *It;

		// Include only PIE and worlds that own the persistent level (i.e. non-streaming levels).
		const bool bIsValidDebugWorld = (TestWorld != nullptr)
			&& TestWorld->WorldType == EWorldType::PIE
			&& TestWorld->PersistentLevel != nullptr
			&& TestWorld->PersistentLevel->OwningWorld == TestWorld;

		if (!bIsValidDebugWorld)
		{
			continue;
		}

		ENetMode NetMode = TestWorld->GetNetMode();

		FString WorldName;

		switch (NetMode)
		{
		case NM_Standalone:
			WorldName = NSLOCTEXT("HostEditor", "DebugWorldStandalone", "Standalone").ToString();
			break;

		case NM_ListenServer:
			WorldName = NSLOCTEXT("HostEditor", "DebugWorldListenServer", "Listen Server").ToString();
			break;

		case NM_DedicatedServer:
			WorldName = NSLOCTEXT("HostEditor", "DebugWorldDedicatedServer", "Dedicated Server").ToString();
			break;

		case NM_Client:
			if (FWorldContext* PieContext = GEngine->GetWorldContextFromWorld(TestWorld))
			{
				WorldName = FString::Printf(TEXT("%s %d"), *NSLOCTEXT("HostEditor", "DebugWorldClient", "Client").ToString(), PieContext->PIEInstance - 1);
			}
			break;
		};

		if (!WorldName.IsEmpty())
		{
			if (FWorldContext* PieContext = GEngine->GetWorldContextFromWorld(TestWorld))
			{
				if (!PieContext->CustomDescription.IsEmpty())
				{
					WorldName += TEXT(" ") + PieContext->CustomDescription;
				}
			}

			// DebugWorlds & DebugWorldNames need to be the same size (we expect
			// an index in one to correspond to the other) - DebugWorldNames is
			// what populates the dropdown, so it is the authority (if there's 
			// no name to present, they can't select from DebugWorlds)
			DebugWorlds.Add(TestWorld);
			DebugWorldNames.Add( MakeShareable(new FString(WorldName)) );
		}
	}

	if (DebugWorldsComboBox.IsValid())
	{
		// Attempt to restore the old selection
		if (bRestoreSelection)
		{
			TSharedPtr<FString> CurrentDebugWorld = GetDebugWorldName();
			if (CurrentDebugWorld.IsValid())
			{
				DebugWorldsComboBox->SetSelectedItem(CurrentDebugWorld);
			}
		}

		// Finally ensure we have a valid selection
		TSharedPtr<FString> CurrentSelection = DebugWorldsComboBox->GetSelectedItem();
		if (DebugWorldNames.Find(CurrentSelection) == INDEX_NONE)
		{
			if (DebugWorldNames.Num() > 0)
			{
				DebugWorldsComboBox->SetSelectedItem(DebugWorldNames[0]);
			}
			else
			{
				DebugWorldsComboBox->ClearSelection();
			}
		}

		DebugWorldsComboBox->RefreshOptions();
	}
}

void SSelectedDebugObjectWidget::GenerateDebugObjectInstances(bool bRestoreSelection)
{
	// Cache the current selection as we may need to restore it
	TSharedPtr<FDebugObjectInstance> LastSelection = GetDebugObjectInstance();

	// Empty the lists of actors and regenerate them
	DebugObjects.Empty();
	//DebugObjects.Add(MakeShareable(new FDebugObjectInstance(nullptr, GetNoDebugString())));

	// Check for a specific debug world. If DebugWorld=nullptr we take that as "any PIE world"
	UWorld* DebugWorld = nullptr;
	if (DebugWorldsComboBox.IsValid())
	{
		TSharedPtr<FString> CurrentWorldSelection = DebugWorldsComboBox->GetSelectedItem();
		int32 SelectedIndex = INDEX_NONE;
		for (int32 WorldIdx = 0; WorldIdx < DebugWorldNames.Num(); ++WorldIdx)
		{
			if (DebugWorldNames[WorldIdx].IsValid() && CurrentWorldSelection.IsValid()
				&& (*DebugWorldNames[WorldIdx] == *CurrentWorldSelection))
			{
				SelectedIndex = WorldIdx;
				break;
			}
		}
		if (SelectedIndex > 0 && DebugWorldNames.IsValidIndex(SelectedIndex))
		{
			DebugWorld = DebugWorlds[SelectedIndex].Get();
		}
	}

	//UWorld* PreviewWorld = HostEditor.Pin()->GetPreviewScene()->GetWorld();

	if (!false/*HostEditor.Pin()->OnlyShowCustomDebugObjects()*/)
	{
		const bool bModifiedIterator = CVarUseFastDebugObjectDiscovery.GetValueOnGameThread() == 1;
		UClass* DebugObjectType = GetDebugObjectClass();// GetStateMachineObj()->GetClass();// GeneratedClass;

		if (bModifiedIterator && DebugObjectType)
		{
			// Experimental new path for debug object discovery
			TArray<UObject*> MatchedDebugObjectList;
			GetObjectsOfClass(DebugObjectType, MatchedDebugObjectList, true);

			for (auto It = MatchedDebugObjectList.CreateIterator(); It; ++It)
			{
				UObject* TestObject = *It;

				if (FilterDebugObject(TestObject))
				{
					continue;
				}

				// check outer chain for pending kill objects
				bool bPendingKill = false;
				UObject* ObjOuter = TestObject;
				do
				{
					bPendingKill = !IsValid(ObjOuter);
					ObjOuter = ObjOuter->GetOuter();
				} while (!bPendingKill && ObjOuter != nullptr);

				if (!TestObject->HasAnyFlags(RF_ClassDefaultObject) && !bPendingKill)
				{
					ObjOuter = TestObject;
					UWorld *ObjWorld = nullptr;
					static bool bUseNewWorldCode = false;
					do		// Run through at least once in case the TestObject is a UGameInstance
					{
						ObjOuter = TestObject->GetOuter();
						ObjWorld = TestObject->GetTypedOuter<UWorld>();
					} while (ObjWorld == nullptr && ObjOuter != nullptr);

					if (ObjWorld)
					{
						// Make check on owning level (not streaming level)
						if (ObjWorld->PersistentLevel && ObjWorld->PersistentLevel->OwningWorld)
						{
							ObjWorld = ObjWorld->PersistentLevel->OwningWorld;
						}

						// We have a specific debug world and the object isn't in it
						if (DebugWorld && ObjWorld != DebugWorld)
						{
							continue;
						}

						if ((ObjWorld->WorldType == EWorldType::Editor) && (GUnrealEd->GetPIEViewport() == nullptr))
						{
							AddDebugObject(TestObject);
						}
						else if (ObjWorld->WorldType == EWorldType::PIE)
						{
							AddDebugObject(TestObject);
						}
					}
				}
			}
		}
		else
		{
			for (TObjectIterator<UObject> It; It; ++It)
			{
				UObject* TestObject = *It;

				// Skip Blueprint preview objects (don't allow them to be selected for debugging)
				//if (PreviewWorld != nullptr && TestObject->IsIn(PreviewWorld))
				//{
				//	continue;
				//}

				const bool bPassesFlags = !TestObject->HasAnyFlags(RF_ClassDefaultObject) && IsValid(TestObject);
				const bool bGeneratedByAnyBlueprint = TestObject->GetClass()->ClassGeneratedBy != nullptr;
				const bool bGeneratedByThisBlueprint = bGeneratedByAnyBlueprint && TestObject->IsA(GetDebugObjectClass());

				if (bPassesFlags && bGeneratedByThisBlueprint)
				{
					UObject *ObjOuter = TestObject;
					UWorld *ObjWorld = nullptr;
					do		// Run through at least once in case the TestObject is a UGameInstance
					{
						UGameInstance *ObjGameInstance = Cast<UGameInstance>(ObjOuter);

						ObjOuter = ObjOuter->GetOuter();
						ObjWorld = ObjGameInstance ? ObjGameInstance->GetWorld() : Cast<UWorld>(ObjOuter);
					} while (ObjWorld == nullptr && ObjOuter != nullptr);

					if (ObjWorld)
					{
						// Make check on owning level (not streaming level)
						if (ObjWorld->PersistentLevel && ObjWorld->PersistentLevel->OwningWorld)
						{
							ObjWorld = ObjWorld->PersistentLevel->OwningWorld;
						}

						// We have a specific debug world and the object isn't in it
						if (DebugWorld && ObjWorld != DebugWorld)
						{
							continue;
						}

						if ((ObjWorld->WorldType == EWorldType::Editor) && (GUnrealEd->GetPIEViewport() == nullptr))
						{
							AddDebugObject(TestObject);
						}
						else if (ObjWorld->WorldType == EWorldType::PIE)
						{
							AddDebugObject(TestObject);
						}
					}
				}
			}
		}
	}

	if (DebugObjectsComboBox.IsValid())
	{
	// 	if (bRestoreSelection)
	// 	{
	// 		TSharedPtr<FDebugObjectInstance> NewSelection = GetDebugObjectInstance();
	// 		if (NewSelection.IsValid() && !NewSelection->IsEmptyObject())
	// 		{
	// 			// If our new selection matches the actual debug object, set it
	// 			DebugObjectsComboBox->SetSelectedItem(NewSelection);
	// 		}
	// 		else if (LastSelection.IsValid() && !LastSelection->IsEditorObject() && !LastSelection->IsEmptyObject())
	// 		{
	// 			// Re-add the desired runtime object if needed, even though it is currently null
	// 			DebugObjects.Add(LastSelection);
	// 			DebugObjectsComboBox->SetSelectedItem(LastSelection);
	// 		}
	// 	}
	//
	// 	// Finally ensure we have a valid selection, this will set to all objects as a backup
	// 	TSharedPtr<FDebugObjectInstance> CurrentSelection = DebugObjectsComboBox->GetSelectedItem();
	// 	if (DebugObjects.Find(CurrentSelection) == INDEX_NONE)
	// 	{
	// 		if (DebugObjects.Num() > 0)
	// 		{
	// 			DebugObjectsComboBox->SetSelectedItem(DebugObjects[0]);
	// 		}
	// 		else
	// 		{
	// 			DebugObjectsComboBox->ClearSelection();
	// 		}
	// 	}
	//
		DebugObjectsComboBox->RefreshOptions();
	}
}

TSharedPtr<FDebugObjectInstance> SSelectedDebugObjectWidget::GetDebugObjectInstance() const
{
	const FString& PathToDebug = GetObjectPathBeingDebugged();
	if (!PathToDebug.IsEmpty())
	{
		for (int32 ObjectIndex = 0; ObjectIndex < DebugObjects.Num(); ++ObjectIndex)
		{
			if (DebugObjects[ObjectIndex].IsValid() && PathToDebug.Equals(DebugObjects[ObjectIndex]->ObjectPath))
			{
				return DebugObjects[ObjectIndex];
			}
		}
	}
	
	return nullptr;
}

TSharedPtr<FString> SSelectedDebugObjectWidget::GetDebugWorldName() const
{
	if (ensure(DebugWorlds.Num() == DebugWorldNames.Num()))
	{
		UWorld* DebugWorld = GetDebugWorld();
		if (DebugWorld != nullptr)
		{
			for (int32 WorldIndex = 0; WorldIndex < DebugWorlds.Num(); ++WorldIndex)
			{
				if (DebugWorlds[WorldIndex].IsValid() && (DebugWorlds[WorldIndex].Get() == DebugWorld))
				{
					return DebugWorldNames[WorldIndex];
				}
			}
		}
	}


	if (DebugWorldNames.Num() > 0)
	{
		return DebugWorldNames[0];
	}

	return nullptr;
}

void SSelectedDebugObjectWidget::DebugWorldSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection != GetDebugWorldName())
	{
		check(DebugWorlds.Num() == DebugWorldNames.Num());
		for (int32 WorldIdx = 0; WorldIdx < DebugWorldNames.Num(); ++WorldIdx)
		{
			if (DebugWorldNames[WorldIdx] == NewSelection)
			{
				SetWorldBeingDebugged(DebugWorlds[WorldIdx].Get());

				SetObjectBeingDebugged(nullptr);
				LastObjectObserved.Reset();

				GenerateDebugObjectInstances(false);
				break;
			}
		}
	}
}

void SSelectedDebugObjectWidget::DebugObjectSelectionChanged(TSharedPtr<FDebugObjectInstance> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection != GetDebugObjectInstance() && NewSelection.IsValid())
	{
		UObject* DebugObj = NewSelection->ObjectPtr.Get();
		SetObjectBeingDebugged(DebugObj);
		
		LastObjectObserved = DebugObj;
	}
}

EVisibility SSelectedDebugObjectWidget::IsSelectDebugObjectButtonVisible() const
{
	if (UObject* DebugObj = GetObjectBeingDebugged())
	{
		// if (UStateMachineContext* Actor = Cast<UStateMachineContext>(DebugObj))
		// {
		// 	return EVisibility::Visible;
		// }
	}
	return EVisibility::Visible;
}

void SSelectedDebugObjectWidget::SelectedDebugObject_OnClicked()
{
	if (UObject* DebugObj = GetObjectBeingDebugged())
	{
	}
}

EVisibility SSelectedDebugObjectWidget::IsDebugWorldComboVisible() const
{
	if (GEditor->PlayWorld != nullptr)
	{
		int32 LocalWorldCount = 0;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World() != nullptr)
			{
				++LocalWorldCount;
			}
		}

		if (LocalWorldCount > 1)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

FString SSelectedDebugObjectWidget::MakeDebugObjectLabel(UObject* TestObject, bool bAddContextIfSelectedInEditor, bool bAddSpawnedContext) const
{
	auto GetActorLabelStringLambda = [](AActor* InActor, bool bIncludeNetModeSuffix, bool bIncludeSelectedSuffix, bool bIncludeSpawnedContext)
	{
		FString Label = InActor->GetActorLabel();

		FString Context;

		if (bIncludeNetModeSuffix)
		{
			switch (InActor->GetNetMode())
			{
			case ENetMode::NM_Client:
			{
				Context = NSLOCTEXT("HostEditor", "DebugWorldClient", "Client").ToString();

				FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(InActor->GetWorld());
				if (WorldContext != nullptr && WorldContext->PIEInstance > 1)
				{
					Context += TEXT(" ");
					Context += FText::AsNumber(WorldContext->PIEInstance - 1).ToString();
				}
			}
			break;

			case ENetMode::NM_ListenServer:
			case ENetMode::NM_DedicatedServer:
				Context = NSLOCTEXT("HostEditor", "DebugWorldServer", "Server").ToString();
				break;
			}
		}

		if (bIncludeSpawnedContext)
		{
			if (!Context.IsEmpty())
			{
				Context += TEXT(", ");
			}

			Context += NSLOCTEXT("HostEditor", "DebugObjectSpawned", "spawned").ToString();
		}

		if (bIncludeSelectedSuffix && InActor->IsSelected())
		{
			if (!Context.IsEmpty())
			{
				Context += TEXT(", ");
			}

			Context += NSLOCTEXT("HostEditor", "DebugObjectSelected", "已选中Actor").ToString();
		}

		if (!Context.IsEmpty())
		{
			Label = FString::Printf(TEXT("%s (%s)"), *Label, *Context);
		}

		return Label;
	};

	// Include net mode suffix when "All worlds" is selected.
	const bool bIncludeNetModeSuffix = *GetDebugWorldName() == GetDebugAllWorldsString();

	FString Label;
	if (AActor* Actor = TestObject->GetTypedOuter<AActor>())
	{
		Label = GetActorLabelStringLambda(Actor, bIncludeNetModeSuffix, bAddContextIfSelectedInEditor, bAddSpawnedContext);
	}
	else
	{
		if (AActor* ParentActor = TestObject->GetTypedOuter<AActor>())
		{
			// We don't need the full path because it's in the tooltip
			const FString RelativePath = TestObject->GetName();
			Label = FString::Printf(TEXT("%s in %s"), *RelativePath, *GetActorLabelStringLambda(ParentActor, bIncludeNetModeSuffix, bAddContextIfSelectedInEditor, bAddSpawnedContext));
		}
		else
		{
			Label = TestObject->GetName();
		}
	}

	return Label;
}

void SSelectedDebugObjectWidget::FillDebugObjectInstance(TSharedPtr<FDebugObjectInstance> Instance)
{
	check(Instance.IsValid());
	FDebugObjectInstance& Ref = *Instance.Get();

	if (Ref.ObjectPtr.IsValid())
	{
		Ref.ObjectPath = Ref.ObjectPtr->GetPathName();

		// Compute non-PIE path
		FString OriginalPath = UWorld::RemovePIEPrefix(Ref.ObjectPath);

		// Look for original object
		UObject* OriginalObject = FindObjectSafe<UObject>(nullptr, *OriginalPath);

		if (OriginalObject)
		{
			Ref.EditorObjectPath = OriginalPath;
		}
		else
		{
			// No editor path, was dynamically spawned
			Ref.EditorObjectPath = FString();
		}
	}
	else
	{
		Ref.ObjectPath = Ref.EditorObjectPath = FString();
	}
}

void SSelectedDebugObjectWidget::AddDebugObject(UObject* TestObject, const FString& TestObjectName)
{
	TSharedPtr<FDebugObjectInstance> NewInstance = MakeShareable(new FDebugObjectInstance(TestObject, TestObjectName));
	FillDebugObjectInstance(NewInstance);

	if (TestObjectName.IsEmpty())
	{
		NewInstance->ObjectLabel = MakeDebugObjectLabel(TestObject, true, NewInstance->IsSpawnedObject());
	}

	DebugObjects.Add(NewInstance);
}

UClass* SSelectedDebugObjectWidget::GetDebugObjectClass() const
{
	return ULogicFlowEvalContext::StaticClass();
}

bool SSelectedDebugObjectWidget::FilterDebugObject(UObject* Object) const
{
	ULogicFlowEvalContext* EvalContext = Cast<ULogicFlowEvalContext>(Object);
	const TSharedPtr<FLogicFlowEditor> LogicFlowEditor = StaticCastSharedPtr<FLogicFlowEditor>(HostEditor.Pin());
	if (EvalContext && LogicFlowEditor.IsValid())
	{
		if (EvalContext->LogicFlow == LogicFlowEditor->GetEditingFlowAsset())
		{
			return false;
		}
	}

	return true;
}

TSharedRef<SWidget> SSelectedDebugObjectWidget::CreateDebugObjectItemWidget(TSharedPtr<FDebugObjectInstance> InItem)
{
	FString ItemString;
	FString ItemTooltip;

	if (InItem.IsValid())
	{
		ItemString = InItem->ObjectLabel;
		ItemTooltip = InItem->ObjectPath;
	}

	return SNew(STextBlock)
		.Text(FText::FromString(*ItemString))
		.ToolTipText(FText::FromString(*ItemTooltip));
}

FText SSelectedDebugObjectWidget::GetSelectedDebugObjectTextLabel() const
{
	FString Label;

	TSharedPtr<FDebugObjectInstance> DebugInstance = GetDebugObjectInstance();
	if (DebugInstance.IsValid())
	{
		Label = DebugInstance->ObjectLabel;

		UObject* DebugObj = GetObjectBeingDebugged();
		if (DebugObj != nullptr)
		{
			// Exclude the editor selection suffix for the combo button's label.
			Label = MakeDebugObjectLabel(DebugObj, true, DebugInstance->IsSpawnedObject());
		}
	}
	else
	{
		Label = TEXT("Select Debug Object");
	}

	return FText::FromString(Label);
}

UObject* SSelectedDebugObjectWidget::GetObjectBeingDebugged() const
{
	if (const TSharedPtr<FLogicFlowEditor> LogicFlowEditor = StaticCastSharedPtr<FLogicFlowEditor>(HostEditor.Pin()))
	{
		return LogicFlowEditor->GetEditingFlowGraph()->ObjectBeingDebugged.Get();
	}

	return nullptr;
}

UWorld* SSelectedDebugObjectWidget::GetDebugWorld() const
{
	if (const TSharedPtr<FLogicFlowEditor> LogicFlowEditor = StaticCastSharedPtr<FLogicFlowEditor>(HostEditor.Pin()))
	{
		return LogicFlowEditor->GetEditingFlowGraph()->WorldBeingDebugged.Get();
	}

	return nullptr;
}

void SSelectedDebugObjectWidget::SetWorldBeingDebugged(UWorld* InWorld)
{
	if (const TSharedPtr<FLogicFlowEditor> LogicFlowEditor = StaticCastSharedPtr<FLogicFlowEditor>(HostEditor.Pin()))
	{
		LogicFlowEditor->GetEditingFlowGraph()->WorldBeingDebugged = InWorld;
	}
}
void SSelectedDebugObjectWidget::SetObjectBeingDebugged(UObject* InObject)
{
	if (const TSharedPtr<FLogicFlowEditor> LogicFlowEditor = StaticCastSharedPtr<FLogicFlowEditor>(HostEditor.Pin()))
	{
		LogicFlowEditor->GetEditingFlowGraph()->ObjectBeingDebugged = Cast<ULogicFlowEvalContext>(InObject);
	}
}

FString SSelectedDebugObjectWidget::GetObjectPathBeingDebugged() const
{
	if (UObject* DebugObject = GetObjectBeingDebugged())
	{
		return DebugObject->GetPathName();
	}
	return TEXT("");
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
