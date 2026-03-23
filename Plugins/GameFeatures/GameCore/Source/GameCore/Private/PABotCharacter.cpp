#include "PABotCharacter.h"
#include "PAPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

APABotCharacter::APABotCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APABotCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APABotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
