#pragma once

#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

//=============================================================================
/** 
 * FAssetTypeActions_LogicFlow
 */
class FAssetTypeActions_LogicFlow : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_LogicFlow();

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override {}
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
	const TArray<FText>& GetSubMenus() const;
	// End of IAssetTypeActions interface
};

//=============================================================================
/** 
 * FAssetTypeActions_LogicFlowTypeDef
 */
class FAssetTypeActions_LogicFlowTypeDef : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_LogicFlowTypeDef();

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
	const TArray<FText>& GetSubMenus() const;
	// End of IAssetTypeActions interface
private:
	EAssetTypeCategories::Type AssetCategory;
};