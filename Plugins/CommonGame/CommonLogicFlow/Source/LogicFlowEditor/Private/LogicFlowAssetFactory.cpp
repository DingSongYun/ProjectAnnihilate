#include "LogicFlowAssetFactory.h"
#include "Modules/ModuleManager.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Editor.h"
#include "LogicFlow.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Input/Reply.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "LogicFlowAssetFactory"

ULogicFlowAssetFactory::ULogicFlowAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = ULogicFlow::StaticClass();
}

ULogicFlowAssetFactory::~ULogicFlowAssetFactory()
{
	
}

bool ULogicFlowAssetFactory::ConfigureProperties()
{
	class FFlowTypeFilter : public IClassViewerFilter
	{
	public:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->IsChildOf<ULogicFlowTypeDef>() && !InClass->HasAnyClassFlags(CLASS_Abstract);
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->IsChildOf(ULogicFlowTypeDef::StaticClass()) && !InClass->HasAnyClassFlags(CLASS_Abstract);
		}
	};

	class FLogicFlowFactoryUI : public TSharedFromThis<FLogicFlowFactoryUI>
	{
	public:
		FReply OnConfirm()
		{
			CachedFactory->TypeDefinition = SelectedClass;
			if (PickerWindow.IsValid())
			{
				PickerWindow->RequestDestroyWindow();
			}
			return FReply::Handled();

		}

		FReply OnCancel()
		{

			if (PickerWindow.IsValid())
			{
				PickerWindow->RequestDestroyWindow();
			}
			return FReply::Handled();
		}

		TSharedRef<SWidget> GenerateFlowTypePicker()
		{
			FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

			FClassViewerInitializationOptions Options;
			Options.Mode = EClassViewerMode::ClassPicker;
			Options.bIsActorsOnly = false;
			Options.bIsBlueprintBaseOnly = false;
			Options.ClassFilters.Add(MakeShared<FFlowTypeFilter>());
			Options.ExtraPickerCommonClasses.Add(nullptr);

			return SNew(SBox)
				.WidthOverride(330)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.MaxHeight(500)
					[
						SNew(SBorder)
						.Padding(4)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &FLogicFlowFactoryUI::OnPickedFlowType))
						]
					]
				];
		}

		void OpenNewFlowConfiguration(ULogicFlowAssetFactory* TheFactory)
		{
			CachedFactory = TheFactory;
			SelectedClass = TheFactory->TypeDefinition ? TheFactory->TypeDefinition : ULogicFlowTypeDef::StaticClass();

			FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

			PickerWindow = SNew(SWindow)
				.Title(LOCTEXT("LogicFlowConfigurator", "Select Flow Options"))
				.ClientSize(FVector2D(350, 100))
				.SupportsMinimize(false)
				.SupportsMaximize(false)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Menu.Background"))
					.Padding(10)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(FlowTypePickerAnchor, SComboButton)
							.ContentPadding(FMargin(2, 2, 2, 1))
							.MenuPlacement(MenuPlacement_BelowAnchor)
							.ButtonContent()
							[
								SNew(STextBlock)
								.Text(this, &FLogicFlowFactoryUI::OnGetFlowTypeTextValue)
							]
							.OnGetMenuContent(this, &FLogicFlowFactoryUI::GenerateFlowTypePicker)
						]

						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Left)
							[
								SNew(SButton)
								.Text(LOCTEXT("OK", "OK"))
								.OnClicked(this, &FLogicFlowFactoryUI::OnConfirm)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Right)
							[
								SNew(SButton)
								.Text(LOCTEXT("Cancel", "Cancel"))
								.OnClicked(this, &FLogicFlowFactoryUI::OnCancel)
							]
						]
					]
				];

			GEditor->EditorAddModalWindow(PickerWindow.ToSharedRef());
			PickerWindow.Reset();
		}

	public:
		FText OnGetFlowTypeTextValue() const
		{
			FString OutString;

			if (SelectedClass)
			{
				OutString = SelectedClass->GetDisplayNameText().ToString();
			}
			else
			{
				OutString = TEXT("无效的编辑类型集合!");
			}

			return FText::FromString(*OutString);
		}

		void OnPickedFlowType(UClass* ChosenClass)
		{
			FSlateApplication::Get().DismissAllMenus();
			SelectedClass = ChosenClass;
		}

	private:
		UClass* SelectedClass = nullptr;
		TSharedPtr<SWindow> PickerWindow;
		TSharedPtr<SComboButton> FlowTypePickerAnchor;

		ULogicFlowAssetFactory* CachedFactory = nullptr;
	};

	const TSharedRef<FLogicFlowFactoryUI> LogicFlowFactoryUI = MakeShareable(new FLogicFlowFactoryUI());
	LogicFlowFactoryUI->OpenNewFlowConfiguration(this);

	return true;
};

UObject* ULogicFlowAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if (!TypeDefinition)
		return nullptr;

	const ULogicFlowTypeDef* TypeDefCDO = TypeDefinition->GetDefaultObject<ULogicFlowTypeDef>();
	const UClass* LogicFlowClass = TypeDefCDO->LogicFlowType.Get() ? TypeDefCDO->LogicFlowType.Get() : ULogicFlow::StaticClass();
	ULogicFlow* NewAsset = NewObject<ULogicFlow>(InParent, LogicFlowClass, Name, Flags | RF_Transactional);
	NewAsset->FlowTypeDefinition = TypeDefinition;

	return NewAsset;
}

UObject* ULogicFlowAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (!TypeDefinition) return nullptr;

	const ULogicFlowTypeDef* TypeDefCDO = TypeDefinition->GetDefaultObject<ULogicFlowTypeDef>();
	const UClass* LogicFlowClass = TypeDefCDO->LogicFlowType.Get() ? TypeDefCDO->LogicFlowType.Get() : ULogicFlow::StaticClass();
	ULogicFlow* NewAsset = NewObject<ULogicFlow>(InParent, LogicFlowClass, Name, Flags | RF_Transactional);
	NewAsset->FlowTypeDefinition = TypeDefinition;

	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
