// Fill out your copyright notice in the Description page of Project Settings.

#include "PHMovement.h"
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