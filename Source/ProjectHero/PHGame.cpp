#include "PHGame.h"
#include "MainChar.h"
#include "DrawDebugHelpers.h"
#include "Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

APHGame* APHGame::Instance;

APHGame::APHGame()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APHGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Warning, TEXT("Init Game PHGame"), Enemies.Num());
	Enemies.Reset();

	Instance = this;
}

void APHGame::Tick(float DeltaTime)
{
	if (freezeFramesCounter >= 0)
		freezeFramesCounter--;

	if (freezeFramesCounter == 0)
	{
		UGameplayStatics::SetGlobalTimeDilation(Instance->GetWorld(), 1);
		// UE_LOG(LogTemp, Warning, TEXT("FREEZE STOP"));
	}

	// currentTimeScale = FMath::Lerp(currentTimeScale, targetTimeScale, 0.05f);
	// UGameplayStatics::SetGlobalTimeDilation(GetWorld(), currentTimeScale);

	Super::Tick(DeltaTime);
}

TArray<AEnemy*> APHGame::GetEnemies()
{
	// UE_LOG(LogTemp, Warning, TEXT("Get Enemies %d"), Enemies.Array().Num());
	return Enemies.Array();
}

AMainChar* APHGame::GetPlayer()
{
	return Player;
}

void APHGame::AddEnemy(AEnemy* enemy)
{
	Enemies.Add(enemy);

	// UE_LOG(LogTemp, Warning, TEXT("AddEnemy - %d"), Enemies.Num());
}

void APHGame::RemoveEnemy(AEnemy* enemy)
{
	Enemies.Remove(enemy);

	UE_LOG(LogTemp, Warning, TEXT("RemoveEnemy - %d"), Enemies.Num());
}

void APHGame::SetPlayer(AMainChar* player)
{
	Player = player;
}

void APHGame::DamageArea(FVector Center, float radius, FAttackInfo attackInfo)
{
	// DrawDebugSphere(GetWorld(), Center, radius, 8, FColor::Red, true, 2);

	TArray<FHitResult> OutHits;
	FCollisionQueryParams ColParams;

	if (GetWorld()->SweepMultiByChannel(OutHits, Center, Center, FQuat::Identity, ECollisionChannel::ECC_Pawn, FCollisionShape::MakeSphere(radius)))
	{
		for (int i = 0; i < OutHits.Num(); i++)
		{
			FHitResult OutHit = OutHits[i];
			if (OutHit.Component->GetOwner()->GetClass()->IsChildOf<AEnemy>() && OutHit.Component->IsA(UCapsuleComponent::StaticClass()))
			{
				// UE_LOG(LogTemp, Warning, TEXT("Enemy damage area!"));

				if (attackInfo.launchEnemy)
					((AEnemy*)OutHit.Component->GetOwner())->Damage(attackInfo.Damage, Center, attackInfo.pushAmount, true, attackInfo.riseAmount, attackInfo.spLaunch);
				else
					((AEnemy*)OutHit.Component->GetOwner())->Damage(attackInfo.Damage, Center, attackInfo.pushAmount);

				if (attackInfo.descend)
					Cast<AEnemy>(OutHit.Component->GetOwner())->QuickFall();

				// TODO effect

				FreezeFrames();
			}
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT("DamageArea: %d"), attackInfo.Damage);
}

void APHGame::DamageLine(FVector Start, FVector End, FAttackInfo attackInfo)
{
	// UE_LOG(LogTemp, Warning, TEXT("Damage Line %s %s"), *Start.ToString(), *End.ToString());

	FHitResult OutHit;
	FCollisionQueryParams ColParams;

	// DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true, 0.5f);

	if(AMainChar::GetMainChar()->ActorLineTraceSingle(OutHit, Start, End, ECC_Pawn, ColParams))
	{
		if (OutHit.GetComponent()->GetOwner()->GetClass()->IsChildOf<AMainChar>() && OutHit.GetComponent()->IsA(UCapsuleComponent::StaticClass()))
		{
			// UE_LOG(LogTemp, Warning, TEXT("PLAYER HIT %d damagee"), attackInfo.Damage);

			((AMainChar*)OutHit.GetComponent()->GetOwner())->Damage(attackInfo.Damage, OutHit.Location, 300, true, 20, false);
		}
	}
}

void APHGame::FreezeFrames()
{
	UE_LOG(LogTemp, Warning, TEXT("FREEZE START"));

	Instance->currentTimeScale = 1;
	Instance->targetTimeScale = 0.1f;
	Instance->freezeFramesCounter = 3;
	UGameplayStatics::SetGlobalTimeDilation(Instance->GetWorld(), 0.1);
}

TSet<AEnemy*> APHGame::GetEnemiesInFront(FVector Position, FVector Direction)
{
	TSet<AEnemy*> res;

	for (int i = 0; i < GetEnemies().Num(); i++)
	{
		float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct((GetEnemies()[i]->GetActorLocation() - Position).GetSafeNormal(), Direction)));
		if (angle < 45)
			res.Add(GetEnemies()[i]);
	}

	return res;
}

AEnemy* APHGame::GetClosestEnemy(TSet<AEnemy*> Enems, FVector Position)
{
	if (Enems.Num() == 0)
		return nullptr;

	TArray<AEnemy*> arr = Enems.Array();
	AEnemy* res = arr[0];

	for (int i = 0; i < Enems.Num(); i++)
		if (FVector::Distance(arr[i]->GetActorLocation(), AMainChar::GetPlayerLocation()) < FVector::Distance(res->GetActorLocation(), AMainChar::GetPlayerLocation()))
			res = arr[i];

	return res;
}

AEnemy* APHGame::GetEnemyToTheRight(AEnemy* Current)
{
	if (Current == nullptr)
		return nullptr;

	FVector Origin = Current->GetActorLocation() - AMainChar::GetPlayerLocation();

	float MinAngle = 360;
	AEnemy* Res = nullptr;

	for (int i = 0; i < GetEnemies().Num(); i++)
	{
		FVector Dest = GetEnemies()[i]->GetActorLocation() - AMainChar::GetPlayerLocation();

		// Right angle check
		if (FVector::CrossProduct(Origin.GetSafeNormal(), Dest.GetSafeNormal()).Z > 0)
		{
			// Get smallest angle
			float Angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(Origin.GetSafeNormal(), Dest.GetSafeNormal())));
			if (Angle < MinAngle && GetEnemies()[i] != Current)
			{
				Res = GetEnemies()[i];
				MinAngle = Angle;
			}
		}
	}

	return Res;
}

AEnemy* APHGame::GetEnemyToTheLeft(AEnemy* Current)
{
	if (Current == nullptr)
		return nullptr;

	FVector Origin = Current->GetActorLocation() - AMainChar::GetPlayerLocation();

	float MinAngle = 360;
	AEnemy* Res = nullptr;

	for (int i = 0; i < GetEnemies().Num(); i++)
	{
		FVector Dest = GetEnemies()[i]->GetActorLocation() - AMainChar::GetPlayerLocation();

		// Left angle check
		if (FVector::CrossProduct(Origin.GetSafeNormal(), Dest.GetSafeNormal()).Z < 0)
		{
			// Get smallest angle
			float Angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(Origin.GetSafeNormal(), Dest.GetSafeNormal())));
			if (Angle < MinAngle && GetEnemies()[i] != Current)
			{
				Res = GetEnemies()[i];
				MinAngle = Angle;
			}
		}
	}

	return Res;
}
