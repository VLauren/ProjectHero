#include "Enemy.h"
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
	Movement = CreateDefaultSubobject<UPHMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::Damage(int amount, FVector sourcePoint, float knockBack)
{
	// Change state to hit stun

	// Knockback
	FVector KBDirection = GetActorLocation() - sourcePoint;
	KBDirection.Z = 0;
	KBDirection.Normalize();
	Movement->MoveOverTime(knockBack, 0.15f, false, KBDirection);
}

