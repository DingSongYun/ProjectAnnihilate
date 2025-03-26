#include "AssetTypeActions_LogicFlow.h"
#include "LogicFlowEditor.h"
#include "LogicFlow.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeActions_LogicFlow::FAssetTypeActions_LogicFlow()
{
}

FText FAssetTypeActions_LogicFlow::GetName() const
{
	return LOCTEXT("FAssetTypeActions_LogicFlow", "LogicFlow");
}

FColor FAssetTypeActions_LogicFlow::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FAssetTypeActions_LogicFlow::GetSupportedClass() const
{
	return ULogicFlow::StaticClass();
}

void FAssetTypeActions_LogicFlow::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (ULogicFlow* Template = Cast<ULogicFlow>(*ObjIt))
		{
			TSharedRef<FLogicFlowEditor> NewEditor(new FLogicFlowEditor());
			NewEditor->InitEditor(Mode, EditWithinLevelEditor, Template);
		}
	}
}

const TArray<FText>& FAssetTypeActions_LogicFlow::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("LogicFlowSubMenu", "Logic Flow")
	};
	return SubMenus;
}


//=============================================================================
/** 
 * FAssetTypeActions_LogicFlowTypeDef
 */
FAssetTypeActions_LogicFlowTypeDef::FAssetTypeActions_LogicFlowTypeDef()
{}

FColor FAssetTypeActions_LogicFlowTypeDef::GetTypeColor() const
{
	return FColor::Cyan;
}

FText FAssetTypeActions_LogicFlowTypeDef::GetName() const
{
	return LOCTEXT("FAssetTypeActions_LogicFlowTypeDef", "LogicFlow Type Definition");
}

UClass* FAssetTypeActions_LogicFlowTypeDef::GetSupportedClass() const
{
	return ULogicFlowTypeDef::StaticClass();
}

const TArray<FText>& FAssetTypeActions_LogicFlowTypeDef::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("LogicFlowSubMenu", "Logic Flow")
	};
	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
