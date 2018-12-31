// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PHPawn.generated.h"

UCLASS()
class PROJECTHERO_API APHPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APHPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Death();

protected:

	float frameCount;

public:

	UPROPERTY(EditAnywhere, Category = MovementValues)
		float GravityStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		int HitRecooveryTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		int GroundRecoveryTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		int WakeUpTime;

};
