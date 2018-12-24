// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "AttackData.h"
#include "BasicEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API ABasicEnemy : public AEnemy
{
	GENERATED_BODY()

public:

	ABasicEnemy();
	
protected:

	virtual void BeginPlay() override;

	bool CheckActiveFrame();
	void DoAttack(float DeltaTime);

	void OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:	

	virtual void Tick(float DeltaTime) override;
	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false);

protected:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* hitBox = nullptr;

	float currentAttackFrame;
	bool AlreadyHit;

public:

	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		float AttackDistance;

	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		UAttackData* AttackData = nullptr;

};
