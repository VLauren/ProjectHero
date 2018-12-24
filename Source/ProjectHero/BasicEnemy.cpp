// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEnemy.h"
#include "EnemyMovement.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Engine/Engine.h"

ABasicEnemy::ABasicEnemy()
{
	HitRecooveryTime = 35;
	GroundRecoveryTime = 40;
	WakeUpTime = 40;
	AttackDistance = 150;
}

void ABasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	hitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());
	if (hitBox != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("There is a hitbox"));
		hitBox->SetGenerateOverlapEvents(false);
		// hitBox->SetVisibility(false);
		// hitBox->SetHiddenInGame(true);

		// Hit box overlap event
		hitBox->OnComponentBeginOverlap.AddDynamic(this, &ABasicEnemy::OnHitboxOverlap);
	}

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);
}

void ABasicEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == EEnemyState::IDLE)
	{
		if (frameCount >= 120)
			State = EEnemyState::MOVING;
	}
	else if (State == EEnemyState::MOVING)
	{
		Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, AMainChar::GetPlayerLocation());
		if (FVector::Distance(GetActorLocation(), AMainChar::GetPlayerLocation()) < AttackDistance)
		{
			currentAttackFrame = 0;
			State = EEnemyState::ATTACK_A;
		}
	}

	else if (State == EEnemyState::LAUNCHED)
	{
		if (Movement->IsGrounded())
		{
			State = EEnemyState::KNOCKED_DOWN;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::KNOCKED_DOWN)
	{
		if (frameCount >= GroundRecoveryTime)
		{
			State = EEnemyState::WAKE_UP;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::WAKE_UP)
	{
		if (frameCount >= WakeUpTime)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}

	frameCount += DeltaTime * 60;

	// DoAttack(DeltaTime);

	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyState"), true);
	FString msg = FString::Printf(TEXT("State: %s, dtp:%f"), *EnumPtr->GetNameByValue((int64)State).ToString(), FVector::Dist(GetActorLocation(), AMainChar::GetPlayerLocation()));
	if (GEngine) GEngine->AddOnScreenDebugMessage(4, 1.5, FColor::White, msg);
}

void ABasicEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch)
{
	if (launch)
	{
		State = EEnemyState::LAUNCHED;
		frameCount = 0;
	}
	else
	{
		State = EEnemyState::HIT;
		frameCount = 0;
	}

	Super::Damage(amount, sourcePoint, knockBack, launch);
}

bool ABasicEnemy::CheckActiveFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return false;
	}

	return currentAttackFrame >= AttackData->Attacks[0].hitStart && currentAttackFrame < AttackData->Attacks[0].hitEnd;
}
void ABasicEnemy::DoAttack(float DeltaTime)
{
	if (State == EEnemyState::ATTACK_A)
	{
		currentAttackFrame += DeltaTime * 60;

		if (CheckActiveFrame() && !AlreadyHit)
		{
			if (hitBox != nullptr)
			{
				hitBox->SetGenerateOverlapEvents(true);
				hitBox->SetVisibility(true);
				hitBox->SetHiddenInGame(false);
			}

			// First active frame
			if (currentAttackFrame == 0 || currentAttackFrame - 1 < AttackData->Attacks[0].hitStart)
			{

			}
		}
		else
		{
			if (hitBox != nullptr)
			{
				hitBox->SetGenerateOverlapEvents(false);
				hitBox->SetVisibility(false);
				hitBox->SetHiddenInGame(true);
			}
		}

		UE_LOG(LogTemp, Error, TEXT("DO Attack frame: %f"), currentAttackFrame);
		if (AttackData != nullptr && currentAttackFrame >= AttackData->Attacks[0].hitEnd + 1)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
}

void ABasicEnemy::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherComp != nullptr)
	{
		if (OtherComp->GetOwner()->GetClass()->IsChildOf<AMainChar>() && OtherComp->IsA(UCapsuleComponent::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("PLAYER HIT"));
			((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), AttackData->Attacks[0].moveAmount);
			AlreadyHit = true;
		}
	}
}
