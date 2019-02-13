// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PHPawn.h"
#include "PHMovement.h"
#include "Enemy.generated.h"

UENUM()
enum class EEnemyState : uint8
{
	IDLE,
	MOVING,
	HIT,
	LAUNCHED,
	LAUNCHED_HIT,
	KNOCKED_DOWN,
	KD_HIT,
	WAKE_UP,
	ATTACK_A,
	ATTACK_B,
	ATTACK_C,
	DEATH
};

UCLASS()
class PROJECTHERO_API AEnemy : public APHPawn
{
	GENERATED_BODY()

protected:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UPHMovement* Movement;
public:

	// Sets default values for this pawn's properties
	AEnemy();

	UPROPERTY(EditDefaultsOnly, Category = HitPoints)
		int MaxHitPoints;

	UPROPERTY(BlueprintReadOnly, Category = HitPoints)
		int HitPoints;

	// State
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EEnemyState State;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	virtual void Death() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false, float riseAmount = 0, bool spLaunch = false);

	void QuickFall();

	UPHMovement* GetMovement();

	UFUNCTION(BlueprintCallable)
		FVector GetPlayerPosition();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool hitStart;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool hitToggle;

	UPROPERTY(EditAnywhere, Category = MovementValues)
		float MovementSpeed;
};
