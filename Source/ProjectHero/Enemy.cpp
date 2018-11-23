#include "Enemy.h"
#include "PHPawn.h"
#include "EnemyMovement.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"

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
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch)
{
	// Change state to hit stun

	// Knockback
	FVector KBDirection = GetActorLocation() - sourcePoint;
	KBDirection.Z = 0;
	KBDirection.Normalize();
	bool stg = !launch && Movement->IsGrounded();
	Movement->MoveOverTime(knockBack, 0.15f, false, KBDirection, stg);

	UE_LOG(LogTemp, Warning, TEXT("STG?? %s"), stg ? TEXT("TRUE") : TEXT("FALSE"));

	// FString a;
	// a = (sourcePoint.ToString());
	// a = (GetActorLocation.ToString());
	// a = (KBDirection.ToString());

	UE_LOG(LogTemp, Warning, TEXT("Damage kb source:%s, loc:%s, dir:%s"), *(sourcePoint.ToString()), *(GetActorLocation().ToString()), *(KBDirection.ToString()));

	if (launch)
	{
		UE_LOG(LogTemp, Warning, TEXT("Launch!"));
		Cast<UEnemyMovement>(Movement)->Launch();
		State = EEnemyState::LAUNCHED;
	}
	else if (!Movement->IsGrounded())
		Cast<UEnemyMovement>(Movement)->AirHit();
}

