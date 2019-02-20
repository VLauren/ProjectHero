#include "Enemy.h"
#include "PHPawn.h"
#include "EnemyMovement.h"
#include "PHGame.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
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

	HitPoints = MaxHitPoints;

	Cast<APHGame>(GetWorld()->GetAuthGameMode())->AddEnemy(this);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == EEnemyState::HIT && !hitStart)
		hitStart = true;
	else if (State == EEnemyState::HIT && hitStart)
		hitStart = false;
}

void AEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount, bool spLaunch)
{
	// Change state to hit stun
	hitToggle = !hitToggle;

	// Launch
	if (launch)
	{
		Cast<UEnemyMovement>(Movement)->Launch(riseAmount, spLaunch);
		State = EEnemyState::LAUNCHED;
	}
	else if (!Movement->IsGrounded())
	{
		Cast<UEnemyMovement>(Movement)->AirHit();
	}

	// Knockback
	FVector KBDirection = GetActorLocation() - sourcePoint;
	KBDirection.Z = 0;
	KBDirection.Normalize();
	bool stg = !launch && Movement->IsGrounded();

	FVector rotationDir = sourcePoint - GetActorLocation();
	rotationDir.Z = 0;
	SetActorRotation(rotationDir.Rotation());

	if(!spLaunch || !launch)
		Movement->MoveOverTime(knockBack, 0.15f, false, KBDirection, stg);
	else
		Movement->MoveOverTime(knockBack, 2 * riseAmount / GravityStrength, false, KBDirection, stg);

	// FreezeFrames();

	HitPoints -= amount;
	if (HitPoints <= 0)
		Death();

	Super::Damage(amount, sourcePoint, knockBack, launch, riseAmount, spLaunch);
}

void AEnemy::QuickFall()
{
	Movement->Descend(1900);
}

UPHMovement* AEnemy::GetMovement()
{
	return Movement;
}

FVector AEnemy::GetPlayerPosition()
{
	return AMainChar::GetPlayerLocation();
}

void AEnemy::Death()
{
	Super::Death();
	//FVector(0, 0, 0).Rotation();

	// Remove enemy from set
	Cast<APHGame>(GetWorld()->GetAuthGameMode())->RemoveEnemy(this);

	State = EEnemyState::DEATH;
}
