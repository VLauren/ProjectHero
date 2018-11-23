// Fill out your copyright notice in the Description page of Project Settings.

#include "PHGame.h"


void APHGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	Enemies.Reset();

	UE_LOG(LogTemp, Warning, TEXT("PHGame InitGame !"));
}