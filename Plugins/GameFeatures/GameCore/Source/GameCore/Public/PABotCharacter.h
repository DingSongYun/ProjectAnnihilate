#pragma once

#include "CoreMinimal.h"
#include "PABaseCharacter.h"
#include "PABotCharacter.generated.h"

UCLASS()
class GAMECORE_API APABotCharacter : public APABaseCharacter
{
	GENERATED_BODY()
	
public:
	APABotCharacter();
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DetectionRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ChaseSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class APlayerCharacter* TargetPlayer;
	
protected:
	virtual void BeginPlay() override;
};