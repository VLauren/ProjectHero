
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
	currentTimeScale = 1;
	targetTimeScale = 1;
}

void APHPawn::Tick(float DeltaTime)
{
	// if (freezeFramesCounter >= 0)
		// freezeFramesCounter--;
	// if(freezeFramesCounter == 0)
		// CustomTimeDilation = 1;

	// currentTimeScale = FMath::Lerp(currentTimeScale, targetTimeScale, 0.05f);
	// UGameplayStatics::SetGlobalTimeDilation(GetWorld(), currentTimeScale);

	if (currentTimeScale != 1)
	{
		// UE_LOG(LogTemp, Warning, TEXT("timescale: %f"), currentTimeScale);
	}
	Super::Tick(DeltaTime);
}

void APHPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APHPawn::FreezeFrames()
{
	// UE_LOG(LogTemp, Warning, TEXT("FREEZE"));
	// CustomTimeDilation = 0.50f;
	currentTimeScale = 1;
	targetTimeScale = 0.1f;
	freezeFramesCounter = 10;
	// UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1);
}

void APHPawn::Death()
{
	UE_LOG(LogTemp, Warning, TEXT("Pawn Death: %s"), *GetName());
}

bool APHPawn::IsInvulnerable()
{
	return invulnerable;
}

