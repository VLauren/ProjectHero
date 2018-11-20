// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PHMovement.h"
#include "MainCharMovement.generated.h"

/**
 * MainCharMovement updates movement for the main character.
 * It manages control movement, jump, knockbacks, movement while attacking, etc.
 */
UCLASS()
class PROJECTHERO_API UMainCharMovement : public UPHMovement
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual bool IsGrounded();

	void Jump();
	void Dodge();
	void ResetYVel();

	UFUNCTION(BlueprintPure)
		bool IsMoving();

	void Cancel();

private:
	class AMainChar* MainChar = nullptr;
	class USkeletalMeshComponent* Mesh = nullptr;

	FVector InputVector;
	FVector Move;
	float YVel;
	int32 justJumped;
	FRotator CurrentRotation;
	bool isMoving;

};
