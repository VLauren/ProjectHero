#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "PHMovement.generated.h"

/**
 * PHMovement manages basic pawn movement.
 * It has move over time functionality and manages vertical velocity movement such as gravity.
 */
UCLASS()
class PROJECTHERO_API UPHMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

protected:

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

	bool Disabled = false;

	bool UseGravity = true;
	bool Flying = false;

private:

	bool CheckGroundedAtPosition(FVector Position);
	bool CheckGroundedAhead(FVector Delta);

protected:

	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void MoveOverTime(float strength, float time, bool forward, FVector direction = FVector::ZeroVector, bool stickToGround = false);

	void Descend(float Speed);

	UFUNCTION(BlueprintPure)
		virtual bool IsGrounded();

};
