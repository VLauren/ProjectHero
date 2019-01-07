#include "PHMoveTo.h"
#include "MainChar.h"
#include "EnemyMovement.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/AIModule/Classes/AIController.h"

EBTNodeResult::Type UPHMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Warning, TEXT("Location: %s"), *OwnerComp.GetOwner()->GetActorLocation().ToString());

	if (OwnerComp.GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("A"));
		if (Cast<AEnemy>(OwnerComp.GetOwner()))
		{
			UE_LOG(LogTemp, Warning, TEXT("B"));
			if (Cast<AEnemy>(OwnerComp.GetOwner())->GetMovement())
			{
				UE_LOG(LogTemp, Warning, TEXT("C"));
			}
		}
	}

	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), OwnerComp.GetOwner()->GetActorLocation(), AMainChar::GetPlayerLocation());
	if (path != NULL)
	{
		for (int i = 0; i < path->PathPoints.Num(); i++)
		{
			DrawDebugSphere(GetWorld(), path->PathPoints[i], 10, 8, FColor::Green, true, 0.5f);
		}
	}

	return EBTNodeResult::Succeeded;
	// return EBTNodeResult::InProgress;
}
