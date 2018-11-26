// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PHPawn.h"
#include "PHMovement.h"
#include "Enemy.generated.h"

UENUM()
enum class EEnemyState : uint8
{
	MOVING,
	LAUNCHED
};

UCLASS()
class PROJECTHERO_API AEnemy : public APHPawn
{
	GENERATED_BODY()

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UPHMovement* Movement;
public:

	// Sets default values for this pawn's properties
	AEnemy();

	EEnemyState State;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false);
	void QuickFall();
};
