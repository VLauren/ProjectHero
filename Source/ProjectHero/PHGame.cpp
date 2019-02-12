#include "PHGame.h"
#include "MainChar.h"
#include "DrawDebugHelpers.h"
#include "Classes/Components/CapsuleComponent.h"

void APHGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Warning, TEXT("Init Game PHGame"), Enemies.Num());
	Enemies.Reset();
}

TArray<AEnemy*> APHGame::GetEnemies()
{
	// UE_LOG(LogTemp, Warning, TEXT("Get Enemies %d"), Enemies.Array().Num());
	return Enemies.Array();
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

void APHGame::DamageArea(FVector Center, float radius, int damage)
{
}

void APHGame::DamageLine(FVector Start, FVector End, int damage)
{
	UE_LOG(LogTemp, Warning, TEXT("Damage Line %s %s"), *Start.ToString(), *End.ToString());

	FHitResult OutHit;
	FCollisionQueryParams ColParams;

	// DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true, 0.5f);

	if(AMainChar::GetMainChar()->ActorLineTraceSingle(OutHit, Start, End, ECC_Pawn, ColParams))
	{
		if (OutHit.GetComponent()->GetOwner()->GetClass()->IsChildOf<AMainChar>() && OutHit.GetComponent()->IsA(UCapsuleComponent::StaticClass()))
		{
			// UE_LOG(LogTemp, Warning, TEXT("PLAYER HIT %d damagee"), AttackData->Attacks[0].Damage);
			// FAttackInfo attackInfo = AttackData->Attacks[0];

			// ((AMainChar*)OtherComp->GetOwner())->Damage(damage, OutHit.Location, attackInfo.moveAmount, attackInfo.launchEnemy, attackInfo.riseAmount, attackInfo.spLaunch);
			((AMainChar*)OutHit.GetComponent()->GetOwner())->Damage(damage, OutHit.Location, 300, true, 20, false);
		}
	}

}

void APHGame::FreezeFrames()
{
	UE_LOG(LogTemp, Warning, TEXT("FREEZE"));
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
