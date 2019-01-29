// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "AttackData.h"
#include "BasicRangedEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API ABasicRangedEnemy : public AEnemy
{
	GENERATED_BODY()

public:

	ABasicRangedEnemy();
	
protected:

	bool CheckActiveFrame();
	void DoAttack(float DeltaTime);

	UFUNCTION()
		void OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Damage(int amount, FVector sourcePoint, float knockBack, bool launch = false, float riseAmount = 0, bool spLaunch = false) override;
	virtual void Death() override;

	UFUNCTION(BlueprintImplementableEvent, Category = CppEvents)
		void OnShoot(FVector target);
	UFUNCTION(BlueprintImplementableEvent, Category = CppEvents)
		void OnShootWarning(FVector target);

protected:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* hitBox = nullptr;

	float currentAttackFrame;

	UPROPERTY(EditAnywhere)
	FVector ShootTarget;

public:

	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		float AttackDistance;

	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		UAttackData* AttackData = nullptr;

};
