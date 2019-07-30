// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "MainChar.h"
#include "AttackData.h"
#include "GameFramework/GameModeBase.h"
#include "PHGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API APHGame : public AGameModeBase
{
	GENERATED_BODY()

private:

	static APHGame* Instance;

	int freezeFramesCounter;
	float targetTimeScale;
	float currentTimeScale;

public:
	APHGame();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);
	virtual void Tick(float DeltaTime);

	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<AEnemy*> Enemies;
	AMainChar* Player;

	UFUNCTION(BlueprintPure)
		TArray<AEnemy*> GetEnemies();

	UFUNCTION(BlueprintPure)
		AMainChar* GetPlayer();

	UFUNCTION(BlueprintPure)
		AEnemy * GetEnemyToTheRight(AEnemy * Current);

	UFUNCTION(BlueprintPure)
		AEnemy * GetEnemyToTheLeft(AEnemy * Current);

	UFUNCTION(BlueprintPure)
		AEnemy * GetClosestEnemy(TSet<AEnemy*> Enems, FVector Position);

	UFUNCTION(BlueprintPure)
		TSet<AEnemy*> GetEnemiesInFront(FVector Position, FVector Direction);

	void AddEnemy(AEnemy* enemy);
	void SetPlayer(AMainChar* enemy);

	UFUNCTION(BLueprintCallable)
	void RemoveEnemy(AEnemy* enemy);

	void DamageArea(FVector Center, float radius, FAttackInfo attackInfo);
	void DamageLine(FVector Start, FVector End, FAttackInfo attackInfo);

	static void FreezeFrames();
};
