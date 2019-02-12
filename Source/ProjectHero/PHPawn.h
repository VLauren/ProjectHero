// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PHPawn.generated.h"

UCLASS()
class PROJECTHERO_API APHPawn : public APawn
{
	GENERATED_BODY()

private:

	int freezeFramesCounter;
	float targetTimeScale;
	float currentTimeScale;

protected:

	float frameCount;
	bool invulnerable;

public:

	UPROPERTY(EditAnywhere, Category = MovementValues)
		float GravityStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		int HitRecooveryTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		int GroundRecoveryTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		int WakeUpTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recovery)
		bool UseDamageFreeze;

public:

	APHPawn();

	UFUNCTION(BlueprintImplementableEvent, Category = CppEvents)
		void OnDeath();

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void FreezeFrames();

	virtual void Death();

	bool IsInvulnerable();
};
