#include "PABaseCharacter.h"

APABaseCharacter::APABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}