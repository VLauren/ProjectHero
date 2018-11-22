// Fill out your copyright notice in the Description page of Project Settings.

#include "PHMovement.h"
#include "PHPawn.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, text)

void UPHMovement::BeginPlay()
{
	Super::BeginPlay();

}

void UPHMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Move over time
	if (PushActive)
	{
		FHitResult Hit;
		FVector PushFrameMove;

		if (!PushForward)
			PushFrameMove = PushDir * DeltaTime * PushStrength;
		else
			PushFrameMove = GetOwner()->GetActorForwardVector() * DeltaTime * PushStrength;
		SafeMoveUpdatedComponent(PushFrameMove, UpdatedComponent->GetComponentRotation(), true, Hit);

		PushElapsedTime += DeltaTime;
		if (PushElapsedTime >= PushTime)
			PushActive = false;
	}

	// Gravity
	if (IsGrounded() || !UseGravity)
	{
		if (ZVel != 0.0f)
		{
			//UE_LOG(LogTemp, Warning, TEXT("GRAV 0!!!!!"));
			// UseGravity = false;
		}

		// Vertical velocity is cero
		ZVel = 0.0f;
	}
	else
	{
		// Apply gravity
		ZVel -= Cast<APHPawn>(GetOwner())->GravityStrength * DeltaTime;
	}

	// Vertical movement
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector(0, 0, ZVel), UpdatedComponent->GetComponentRotation(), true, Hit);
	if (Hit.IsValidBlockingHit())
		SlideAlongSurface(FVector(0, 0, ZVel), 1.f - Hit.Time, Hit.Normal, Hit);
}

void UPHMovement::MoveOverTime(float strength, float time, bool forward, FVector direction)
{
	PushActive = true;
	PushStrength = strength;
	PushTime = time;
	PushElapsedTime = 0;
	PushDir = direction.GetSafeNormal();
	PushForward = forward;
}

bool UPHMovement::IsGrounded()
{
	FHitResult OutHit;
	
	FVector Start = UpdatedComponent->GetOwner()->GetActorLocation();

	float CapsuleHalfHeight = Cast<UCapsuleComponent>(UpdatedComponent)->GetUnscaledCapsuleHalfHeight();
	FVector End = Start + FVector(0, 0, -CapsuleHalfHeight - 12); // Capsule Half Height = 88

	FCollisionQueryParams ColParams;

	// DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1, 0, 1);

	// TODO change to SwipeTrace using a sphere

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, ColParams))
	{
		if (OutHit.bBlockingHit)
		{
			return true;

			print(FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
			print(FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
			print(FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));
		}
		return false;
	}

	return false;
}