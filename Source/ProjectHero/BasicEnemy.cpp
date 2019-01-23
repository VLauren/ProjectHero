
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
	MovementSpeed = 400;
}

void ABasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	hitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());
	if (hitBox != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy: There is a hitbox"));
		hitBox->SetGenerateOverlapEvents(false);
		// hitBox->SetVisibility(false);
		// hitBox->SetHiddenInGame(true);

		// Hit box overlap event
		hitBox->OnComponentBeginOverlap.AddDynamic(this, &ABasicEnemy::OnHitboxOverlap);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Enemy: There is no hitbox"));

	CapsuleComponent->SetVisibility(true);
	// CapsuleComponent->SetHiddenInGame(false);
}

void ABasicEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HitPoints <= 0)
		return;

	if (State == EEnemyState::IDLE)
	{
		if (frameCount >= 5)
			State = EEnemyState::MOVING;
	}
	else if (State == EEnemyState::MOVING)
	{
		Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, AMainChar::GetPlayerGroundLocation());

		FVector v = (AMainChar::GetPlayerGroundLocation() - GetActorLocation()).GetSafeNormal();
		float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(v, GetActorForwardVector())));
		bool orientationOk = angle < 30;
		bool distanceOk = FVector::Distance(GetActorLocation(), AMainChar::GetPlayerGroundLocation()) < AttackDistance;

		if (distanceOk && orientationOk)
		{
			currentAttackFrame = 0;
			AlreadyHit = false;
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
	else if (State == EEnemyState::KD_HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::WAKE_UP;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::LAUNCHED_HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::LAUNCHED;
			frameCount = 0;
		}
		if (Movement->IsGrounded())
		{
			State = EEnemyState::KNOCKED_DOWN;
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

	DoAttack(DeltaTime);

	// Screen log
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyState"), true);
	FString msg = FString::Printf(TEXT("State: %s, dtp:%f"), *EnumPtr->GetNameByValue((int64)State).ToString(), FVector::Dist(GetActorLocation(), AMainChar::GetPlayerGroundLocation()));
	if (GEngine) GEngine->AddOnScreenDebugMessage(4, 1.5, FColor::White, msg);
}

void ABasicEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount, bool spLaunch)
{
	// Wake up armor
	if (State == EEnemyState::WAKE_UP)
		return;

	if (launch)
	{
		State = EEnemyState::LAUNCHED;
	}
	else
	{
		if(State == EEnemyState::KNOCKED_DOWN || State == EEnemyState::KD_HIT)
			State = EEnemyState::KD_HIT;
		else if (Movement->IsGrounded())
			State = EEnemyState::HIT;
		else
			State = EEnemyState::LAUNCHED_HIT;
	}

	frameCount = 0;

	Super::Damage(amount, sourcePoint, knockBack, launch, riseAmount, spLaunch);
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
				// hitBox->SetHiddenInGame(false);
			}

			// First active frame
			if (currentAttackFrame == 0 || currentAttackFrame - 1 < AttackData->Attacks[0].hitStart)
			{
				// Attack movement
				FAttackInfo attack = AttackData->Attacks[0];
				Movement->MoveOverTime(attack.moveAmount, 0.15f, true, FVector::ZeroVector, Movement->IsGrounded());
			}
		}
		else
		{
			if (hitBox != nullptr)
			{
				hitBox->SetGenerateOverlapEvents(false);
				hitBox->SetVisibility(false);
				// hitBox->SetHiddenInGame(true);
			}
		}

		if (AttackData != nullptr && currentAttackFrame >= AttackData->Attacks[0].lastFrame + 1)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
	else
	{
		if (hitBox != nullptr)
		{
			hitBox->SetGenerateOverlapEvents(false);
			hitBox->SetVisibility(false);
			// hitBox->SetHiddenInGame(true);
		}
	}
}

void ABasicEnemy::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherComp != nullptr)
	{
		if (OtherComp->GetOwner()->GetClass()->IsChildOf<AMainChar>() && OtherComp->IsA(UCapsuleComponent::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("PLAYER HIT %d damagee"), AttackData->Attacks[0].Damage);

			FAttackInfo attackInfo = AttackData->Attacks[0];

			((AMainChar*)OtherComp->GetOwner())->Damage(attackInfo.Damage, GetActorLocation(), attackInfo.moveAmount, attackInfo.launchEnemy, attackInfo.riseAmount, attackInfo.spLaunch);
			AlreadyHit = true;
		}
	}
}

void ABasicEnemy::Death()
{
	Super::Death();

	UE_LOG(LogTemp, Warning, TEXT("Basic Enemy Death"));

	// Destroy();
}
