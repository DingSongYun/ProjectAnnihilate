#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Editor.h"
#include "NeCharacterAsset.h"
#include "NeCharacterAssetFactory.h"
#include "NeCharacterDataSettings.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CharacterAssetFactory"

UNeCharacterDataFactory::UNeCharacterDataFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UNeCharacterAsset::StaticClass();
}

UNeCharacterDataFactory::~UNeCharacterDataFactory()
{

}

bool UNeCharacterDataFactory::ConfigureProperties()
{
	class FCharacterTemplatetFilter : public IClassViewerFilter
	{
	public:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs) override
		{
			bool Valid = GetMutableDefault<UCharacterDataSettings>()->AssetTypeRegistration.Contains(InClass);;
			return Valid;
		}
		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const class IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs) override
		{
			return false;
		}
	};

	class FCharacterTemplateFactoryUI : public TSharedFromThis<FCharacterTemplateFactoryUI>
	{
	public:
		FReply OnCreate()
		{
			check(ResultClass);
			if (PickerWindow.IsValid())
			{
				PickerWindow->RequestDestroyWindow();
			}
			return FReply::Handled();
		}

		FReply OnCancel()
		{
			ResultClass = nullptr;
			if (PickerWindow.IsValid())
			{
				PickerWindow->RequestDestroyWindow();
			}
			return FReply::Handled();
		}

		bool IsClassSelected() const
		{
			return ResultClass != nullptr;
		}

		void OnPickedClass(UClass* ChosenClass)
		{
			ResultClass = ChosenClass;
			ClassPickerAnchor->SetIsOpen(false);
		}

		FText OnGetComboTextValue() const
		{
			return ResultClass
				? FText::AsCultureInvariant(ResultClass->GetName())
				: LOCTEXT("None", "None");
		}

		TSharedRef<SWidget> GenerateClassPicker()
		{
			FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

			// Fill in options
			FClassViewerInitializationOptions Options;
			Options.Mode = EClassViewerMode::Type::ClassPicker;
			Options.ClassFilters.Add(MakeShared<FCharacterTemplatetFilter>());
			Options.bIsBlueprintBaseOnly = true;
			return
				SNew(SBox)
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
					ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &FCharacterTemplateFactoryUI::OnPickedClass))
				]
				]
				];
		}

		const UClass* OpenClassSelector()
		{
			FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
			ResultClass = nullptr;

			// Fill in options

			FClassViewerInitializationOptions Options;
			Options.Mode = EClassViewerMode::Type::ClassPicker;
			Options.ClassFilters.Add(MakeShared<FCharacterTemplatetFilter>());

			PickerWindow = SNew(SWindow)
				.Title(LOCTEXT("CharacterTemplateOptions", "Pick Template"))
				.ClientSize(FVector2D(350, 100))
				.SupportsMinimize(false)
				.SupportsMaximize(false)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Menu.Background"))
				.Padding(10)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ClassPickerAnchor, SComboButton)
					.ContentPadding(FMargin(2, 2, 2, 1))
				.MenuPlacement(MenuPlacement_BelowAnchor)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FCharacterTemplateFactoryUI::OnGetComboTextValue)
				]
			.OnGetMenuContent(this, &FCharacterTemplateFactoryUI::GenerateClassPicker)
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("OK", "OK"))
				.IsEnabled(this, &FCharacterTemplateFactoryUI::IsClassSelected)
				.OnClicked(this, &FCharacterTemplateFactoryUI::OnCreate)
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("Cancel", "Cancel"))
				.OnClicked(this, &FCharacterTemplateFactoryUI::OnCancel)
				]
				]
				]
				];

			GEditor->EditorAddModalWindow(PickerWindow.ToSharedRef());
			PickerWindow.Reset();

			return ResultClass;
		}

	private:
		TSharedPtr<SWindow> PickerWindow;
		TSharedPtr<SComboButton> ClassPickerAnchor;
		const UClass* ResultClass = nullptr;
	};

	TSharedRef<FCharacterTemplateFactoryUI> StructSelector = MakeShareable(new FCharacterTemplateFactoryUI());
	SelectClass = StructSelector->OpenClassSelector();

	return SelectClass != nullptr;
};

UObject* UNeCharacterDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UObject* CharTemp = NewObject<UObject>(InParent, SelectClass, Name, Flags | RF_Transactional);

	return CharTemp;
}

UObject* UNeCharacterDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}
#undef LOCTEXT_NAMESPACE
