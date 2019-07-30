
#include "BasicRangedEnemy.h"
#include "EnemyMovement.h"
#include "MainChar.h"
#include "PHGame.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

ABasicRangedEnemy::ABasicRangedEnemy()
{
	HitRecooveryTime = 35;
	GroundRecoveryTime = 40;
	WakeUpTime = 40;
	AttackDistance = 3000;
	MovementSpeed = 200;
}

void ABasicRangedEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ABasicRangedEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == EEnemyState::IDLE)
	{
		if (frameCount >= 5)
			State = EEnemyState::MOVING;
	}
	else if (State == EEnemyState::MOVING)
	{
		// TODO random move
		// Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, AMainChar::GetPlayerGroundLocation());

		FVector v = (AMainChar::GetPlayerGroundLocation() - GetActorLocation()).GetSafeNormal();
		float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(v, GetActorForwardVector())));
		bool orientationOk = angle < 5;
		bool distanceOk = FVector::Distance(GetActorLocation(), AMainChar::GetPlayerGroundLocation()) < AttackDistance;

		if (!distanceOk)
			Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, AMainChar::GetPlayerGroundLocation());
		else
			Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, GetActorLocation() - v);

		if (distanceOk && orientationOk)
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

	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyState"), true);
	FString msg = FString::Printf(TEXT("State: %s, dtp:%f"), *EnumPtr->GetNameByValue((int64)State).ToString(), FVector::Dist(GetActorLocation(), AMainChar::GetPlayerGroundLocation()));
	if (GEngine) GEngine->AddOnScreenDebugMessage(4, 1.5, FColor::White, msg);
}

bool ABasicRangedEnemy::CheckActiveFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return false;
	}

	return currentAttackFrame >= AttackData->Attacks[0].hitStart && currentAttackFrame < AttackData->Attacks[0].hitEnd;
}

void ABasicRangedEnemy::DoAttack(float DeltaTime)
{
	if (State == EEnemyState::ATTACK_A)
	{
		currentAttackFrame += DeltaTime * 60;

		if (currentAttackFrame <= 2)
		{
			ShootTarget = AMainChar::GetPlayerLocation();
			OnShootWarning(ShootTarget);
		}

		if (CheckActiveFrame())
		{
			// First active frame
			if (currentAttackFrame - 1 < AttackData->Attacks[0].hitStart)
			{
				// UE_LOG(LogTemp, Warning, TEXT("Current attack frame: %f"), currentAttackFrame);
				// UE_LOG(LogTemp, Warning, TEXT("PEW PEW PEW"));
				FVector dir = (ShootTarget - GetActorLocation()).GetSafeNormal();
				float range = 8000;
				Cast<APHGame>(GetWorld()->GetAuthGameMode())->DamageLine(GetActorLocation(), GetActorLocation() + dir * range, AttackData->Attacks[0]);

				OnShoot(ShootTarget);
			}
		}

		if (AttackData != nullptr && currentAttackFrame >= AttackData->Attacks[0].lastFrame + 1)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
}

void ABasicRangedEnemy::OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{

}

void ABasicRangedEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount, bool spLaunch)
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

void ABasicRangedEnemy::Death()
{
	Super::Death();

}
