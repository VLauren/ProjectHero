// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "GameFramework/GameModeBase.h"
#include "PHGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API APHGame : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);

	TSet<AEnemy*> Enemies;
};
