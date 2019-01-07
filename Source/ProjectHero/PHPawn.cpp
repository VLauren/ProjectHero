
#include "PHPawn.h"

APHPawn::APHPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APHPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APHPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APHPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APHPawn::Death()
{
	UE_LOG(LogTemp, Warning, TEXT("Death: %s"), *GetName());
}

