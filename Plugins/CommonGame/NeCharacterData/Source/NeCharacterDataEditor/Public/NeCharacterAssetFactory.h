#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "NeCharacterAssetFactory.generated.h"

UCLASS(HideCategories = Object, MinimalAPI)
class UNeCharacterDataFactory : public UFactory
{
	GENERATED_BODY()

public:
	UNeCharacterDataFactory(const FObjectInitializer& ObjectInitializer);
	virtual ~UNeCharacterDataFactory() override;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface	

	UPROPERTY(BlueprintReadWrite)
	const class UClass* SelectClass = nullptr;
};