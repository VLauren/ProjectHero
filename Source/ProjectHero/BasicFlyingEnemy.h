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

	ABasicFlyingEnemy();
	
protected:
	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;
	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false, float riseAmount = 0, bool spLaunch = false);
	virtual void Death() override;



};
