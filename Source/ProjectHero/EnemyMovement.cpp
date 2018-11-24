#include "EnemyMovement.h"
#include "Enemy.h"

void UEnemyMovement::BeginPlay()
{
	Super::BeginPlay();

	StartGravity = Cast<AEnemy>(GetOwner())->GravityStrength;
}

void UEnemyMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED && ZVel < 1 && ZVel > -1)
	{
		Cast<AEnemy>(GetOwner())->GravityStrength = 5;
	}
	else
	{
		Cast<AEnemy>(GetOwner())->GravityStrength = StartGravity;
	}
}

void UEnemyMovement::Launch()
{
	UseGravity = true;
	ZVel = 24; // TODO variable or constant
}

void UEnemyMovement::AirHit()
{
	ZVel = 1;
}

bool UEnemyMovement::IsGrounded()
{
	// if (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED)
	if(ZVel > 0)
		return false;

	return Super::IsGrounded();
}