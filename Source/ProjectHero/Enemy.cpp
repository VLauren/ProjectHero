#include "Enemy.h"
#include "PHPawn.h"
#include "EnemyMovement.h"
#include "PHGame.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
// #include "Runtime/Engine/Classes/AI/NavigationSystemBase.h"
// #include "Runtime/Engine/Classes/AI/Navigation/NavigationTypes.h"
// #include "Runtime/Engine/Classes/AI/NavigationSystemBase.h"	
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Add the character's capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName("MainChar");
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->bCheckAsyncSceneOnMove = false;
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	// Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh != nullptr)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(CapsuleComponent);
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}

	// Movement component
	Movement = CreateDefaultSubobject<UEnemyMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;

	GravityStrength = 60.0;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	State = EEnemyState::MOVING;
	Movement->UseGravity = true;

	// if (Enemies == nullptr)
	// {
		// UE_LOG(LogTemp, Warning, TEXT("New enemy set"));
	// }


	// Cast<APHGame>(GetWorld()->GetAuthGameMode())->Enemies.Add(this);
	Cast<APHGame>(GetWorld()->GetAuthGameMode())->AddEnemy(this);

	// UE_LOG(LogTemp, Warning, TEXT("Enemy Set Num: %d"), Cast<APHGame>(GetWorld()->GetAuthGameMode())->Enemies.Num());
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// HACK TESTS

	// UNavigationPath* tpath = nullptr;
	// UNavigationSystem* NavSys = nullptr;
	// UNavigationSystem* NavSys = Cast<UNavigationSystem>(GetWorld()->GetNavigationSystem());
	// class FNavigationSystem* NavSys = FNavigationSystem::GetCurrent(GetWorld());
	// FNavigationSystem asd;

	// UNavigationPath* tpath = NavSys->FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), FVector(0, 0, 0));

	// UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	// UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), AMainChar::GetPlayerLocation());
	// if (path != NULL)
	// {
		// for (int i = 0; i < path->PathPoints.Num(); i++)
		// {
			// DrawDebugSphere(GetWorld(), path->PathPoints[i], 10, 8, FColor::Green);
		// }
	// }

	if (State == EEnemyState::HIT && !hitStart)
		hitStart = true;
	else if (State == EEnemyState::HIT && hitStart)
		hitStart = false;
}

void AEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch)
{
	// Change state to hit stun
	hitToggle = !hitToggle;

	// Knockback
	FVector KBDirection = GetActorLocation() - sourcePoint;
	KBDirection.Z = 0;
	KBDirection.Normalize();
	bool stg = !launch && Movement->IsGrounded();
	Movement->MoveOverTime(knockBack, 0.15f, false, KBDirection, stg);

	if (launch)
	{
		Cast<UEnemyMovement>(Movement)->Launch();
		State = EEnemyState::LAUNCHED;
	}
	else if (!Movement->IsGrounded())
		Cast<UEnemyMovement>(Movement)->AirHit();
}

void AEnemy::QuickFall()
{
	Movement->Descend(1900);
}

UPHMovement * AEnemy::GetMovement()
{
	return Movement;
}

