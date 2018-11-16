// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "MainCharMovement.generated.h"

/**
 * MainCharMovement updates movement for the main character.
 * It manages control movement, jump, knockbacks, movement while attacking, etc.
 */
UCLASS()
class PROJECTHERO_API UMainCharMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void Jump();
	void Dodge();
	void ResetYVel();

	bool IsGrounded();

	void Push(float strength, float time, bool forward, FVector direction = FVector::ZeroVector);

	void Cancel();

private:
	class AMainChar* MainChar = nullptr;
	class USkeletalMeshComponent* Mesh = nullptr;

	FVector InputVector;
	FVector Move;
	float YVel;
	int32 justJumped;
	FRotator CurrentRotation;

	// Variables para push
	FVector PushDir;
	float PushStrength;
	float PushTime;
	float PushActive;
	float PushElapsedTime;
	bool PushForward;
};
