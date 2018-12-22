// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "BasicEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API ABasicEnemy : public AEnemy
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false);
};
