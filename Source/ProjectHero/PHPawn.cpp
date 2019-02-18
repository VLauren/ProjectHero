
#include "PHPawn.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

APHPawn::APHPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APHPawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("time dil: %f"), CustomTimeDilation);
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
	UE_LOG(LogTemp, Warning, TEXT("Pawn Death: %s"), *GetName());
}

bool APHPawn::IsInvulnerable()
{
	return invulnerable;
}

