#include "EnemyMovement.h"
#include "Enemy.h"

void UEnemyMovement::Launch()
{
	UE_LOG(LogTemp, Warning, TEXT("Movement Launch!"));
	UseGravity = true;
	ZVel = 24; // TODO variable or constant
}

bool UEnemyMovement::IsGrounded()
{
	// if (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED)
	if(ZVel > 0)
		return false;

	return Super::IsGrounded();
}