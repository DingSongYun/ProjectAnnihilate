#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "LogicFlowAssetFactory.generated.h"

UCLASS(HideCategories = Object, MinimalAPI)
class ULogicFlowAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	ULogicFlowAssetFactory(const FObjectInitializer& ObjectInitializer);
	virtual ~ULogicFlowAssetFactory();

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface	

public:
	UClass* TypeDefinition = nullptr;
};
