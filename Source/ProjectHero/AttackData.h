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
		int hitStart = 24;
	UPROPERTY(EditAnywhere)
		int hitEnd = 44;
	UPROPERTY(EditAnywhere)
		int linkStart = 40;
	UPROPERTY(EditAnywhere)
		int lastFrame = 60;

};

UCLASS()
class PROJECTHERO_API UAttackData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
		TArray<FAttackInfo> Attacks;
	
	
};
