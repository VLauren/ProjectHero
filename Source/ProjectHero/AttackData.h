// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttackData.generated.h"

USTRUCT(BlueprintType)
struct FAttackInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
		int hitStart = 10;
	UPROPERTY(EditAnywhere)
		int hitEnd = 30;
	UPROPERTY(EditAnywhere)
		int linkStart = 20;
	UPROPERTY(EditAnywhere)
		int lastFrame = 40;
	UPROPERTY(EditAnywhere)
		bool launchEnemy = false;
	UPROPERTY(EditAnywhere)
		bool rise = false;

};

UCLASS()
class PROJECTHERO_API UAttackData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
		TArray<FAttackInfo> Attacks;
};
