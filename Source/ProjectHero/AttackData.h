// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttackData.generated.h"

USTRUCT(BlueprintType)
struct FAttackInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int hitStart = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int hitEnd = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int linkStart = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int lastFrame = 40;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int moveAmount = 40;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int pushAmount = 40;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int riseAmount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool launchEnemy = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ascend = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool descend = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Damage = 20;
};

UCLASS()
class PROJECTHERO_API UAttackData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FAttackInfo> Attacks;
};
