
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
	if (freezeFramesCounter >= 0)
		freezeFramesCounter--;
	if(freezeFramesCounter == 0)
		// UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1);
		CustomTimeDilation = 1;

	Super::Tick(DeltaTime);
}

void APHPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APHPawn::FreezeFrames()
{
	UE_LOG(LogTemp, Warning, TEXT("FREEZE"));
	CustomTimeDilation = 0.00f;
	freezeFramesCounter = 3;
	// UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0);
}

void APHPawn::Death()
{
	UE_LOG(LogTemp, Warning, TEXT("Pawn Death: %s"), *GetName());
}

bool APHPawn::IsInvulnerable()
{
	return invulnerable;
}

