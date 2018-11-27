// Fill out your copyright notice in the Description page of Project Settings.

#include "PHMovement.h"
#include "PHPawn.h"
#include "DrawDebugHelpers.h"
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

		if (!PushStickToGround || CheckGroundedAhead(PushFrameMove))
			SafeMoveUpdatedComponent(PushFrameMove, UpdatedComponent->GetComponentRotation(), true, Hit);

		PushElapsedTime += DeltaTime;
		if (PushElapsedTime >= PushTime)
			PushActive = false;
	}

	// UE_LOG(LogTemp, Warning, TEXT("%s UseGrav: %s, Grounded: %s"), *GetOwner()->GetName(), UseGravity ? TEXT("True") : TEXT("False"), IsGrounded() ? TEXT("True") : TEXT("False"));

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

	// Descent
	if (QuickFall)
	{
		float quickFallDelta = -DeltaTime * QuickFallSpeed;
		SafeMoveUpdatedComponent(FVector(0, 0, quickFallDelta), UpdatedComponent->GetComponentRotation(), true, Hit);
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(FVector(0, 0, quickFallDelta), 1.f - Hit.Time, Hit.Normal, Hit);

		if (IsGrounded())
			QuickFall = false;
	}
}

void UPHMovement::MoveOverTime(float strength, float time, bool forward, FVector direction, bool stickToGround)
{
	PushActive = true;
	PushStrength = strength;
	PushTime = time;
	PushElapsedTime = 0;
	PushDir = direction.GetSafeNormal();
	PushForward = forward;
	PushStickToGround = stickToGround;
}

void UPHMovement::Descend(float Speed)
{
	UE_LOG(LogTemp, Warning, TEXT("Descend"));
	QuickFallSpeed = Speed;
	QuickFall = true;
}

bool UPHMovement::CheckGroundedAtPosition(FVector Position)
{
	float Radius = Cast<UCapsuleComponent>(UpdatedComponent)->GetScaledCapsuleRadius();

	FHitResult OutHit;
	FCollisionQueryParams ColParams;

	if (GetWorld()->SweepSingleByChannel(OutHit, Position, Position, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(Radius)))
	{
		// UE_LOG(LogTemp, Warning, TEXT("GROUNDED"));
		return true;
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("NOT GROUNDED"));
		return false;
	}
}

bool UPHMovement::CheckGroundedAhead(FVector Delta)
{
	float CapsuleHalfHeight = Cast<UCapsuleComponent>(UpdatedComponent)->GetUnscaledCapsuleHalfHeight();
	float Radius = Cast<UCapsuleComponent>(UpdatedComponent)->GetScaledCapsuleRadius();
	FVector Position = UpdatedComponent->GetOwner()->GetActorLocation() - FVector(0, 0, CapsuleHalfHeight - Radius + 3.0f);

	return CheckGroundedAtPosition(Position + Delta);
}

bool UPHMovement::IsGrounded()
{
	float CapsuleHalfHeight = Cast<UCapsuleComponent>(UpdatedComponent)->GetUnscaledCapsuleHalfHeight();
	float Radius = Cast<UCapsuleComponent>(UpdatedComponent)->GetScaledCapsuleRadius();
	FVector Position = UpdatedComponent->GetOwner()->GetActorLocation() - FVector(0, 0, CapsuleHalfHeight - Radius + 3.0f);

	// Grounded sphere
	// DrawDebugSphere(GetWorld(), Position, Radius, 8, FColor::Green);

	return CheckGroundedAtPosition(Position);
}
