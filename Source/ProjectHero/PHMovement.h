// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "PHMovement.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API UPHMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

private:

	bool CheckGroundedAtPosition(FVector Position);
	bool CheckGroundedAhead(FVector Delta);

protected:

	virtual void BeginPlay() override;

	// Move over time vars
	FVector PushDir;
	float PushStrength;
	float PushTime;
	float PushActive;
	float PushElapsedTime;
	bool PushForward;
	bool PushStickToGround;

	float ZVel;
	bool QuickFall;
	float QuickFallSpeed;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void MoveOverTime(float strength, float time, bool forward, FVector direction = FVector::ZeroVector, bool stickToGround = false);

	void Descend(float Speed);

	UFUNCTION(BlueprintPure)
		virtual bool IsGrounded();

	bool UseGravity = true;

};
