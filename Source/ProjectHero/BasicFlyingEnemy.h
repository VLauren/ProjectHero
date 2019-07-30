// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "BasicFlyingEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API ABasicFlyingEnemy : public AEnemy
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int FlyingHeight = 500;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int DistanceToSeek = 600;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FlyingInclination = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AngleLerpStrength = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AttackTime = 3;

	UFUNCTION(BlueprintImplementableEvent, Category = CppEvents)
		void OnAttack();

	ABasicFlyingEnemy();

private:

	FRotator CurrentRotation;
	
protected:

	virtual void BeginPlay() override;

	void RotateTowards(float DeltaTime, FRotator TargetRot);

public:

	virtual void Tick(float DeltaTime) override;
	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false, float riseAmount = 0, bool spLaunch = false);
	virtual void Death() override;



};
