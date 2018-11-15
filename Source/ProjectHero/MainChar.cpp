#include "MainChar.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"

AMainChar* AMainChar::Instance;

AMainChar::AMainChar()
{
	PrimaryActorTick.bCanEverTick = true;

	// Creo la capsula de colision del personaje
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName("MainChar");
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->bCheckAsyncSceneOnMove = false;
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);

	//Cel spring arm para la camara
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 700.0f;

	// Agrego la camara
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Con el input, la camara no rota pero el brazo si
	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera->bUsePawnControlRotation = true;

	// Malla
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh != nullptr)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(CapsuleComponent);
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}

	// Componente de movimiento
	Movement = CreateDefaultSubobject<UMainCharMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;

	// Default values
	MovementSpeed = 500.0f;
	RotationLerpSpeed = 0.1f;
	JumpStrength = 12;
	GravityStrength = 30.0f;
	StopLerpSpeed = 0.07f;

	Instance = this;
}

// Called when the game starts or when spawned
void AMainChar::BeginPlay()
{
	Super::BeginPlay();
	hitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());
	if (hitBox != nullptr)
	{
		hitBox->SetGenerateOverlapEvents(false);
		// hitBox->SetVisibility(false);
		hitBox->SetHiddenInGame(true);

		// evento de overlap del hitbox
		// hitBox->OnComponentBeginOverlap.AddDynamic(this, &AProta::OnHitboxOverlap);
	}

	// TODO Animacion
	// AnimState = EProtaAnimState::AS_STAND;
	// Mesh->PlayAnimation(AnimStand, true);

	UE_LOG(LogTemp, Warning, TEXT("paco"));

	CharState = EMainCharState::MOVING;
}

// Called every frame
void AMainChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoAttack();

}

// Called to bind functionality to input
void AMainChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// input de camara
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// input de movimiento (controller)
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainChar::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainChar::MoveForward);

	// input de acciones
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMainChar::Attack);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainChar::Jump);
}

void AMainChar::MoveForward(float AxisValue)
{
	if (CharState == EMainCharState::MOVING || CharState == EMainCharState::ATTACK)
	{
		if (Movement && (Movement->UpdatedComponent == RootComponent))
			Movement->AddInputVector(FRotator(0, GetControlRotation().Yaw, 0).RotateVector(FVector(1, 0, 0)) * AxisValue);
	}
}

void AMainChar::MoveRight(float AxisValue)
{
	if (CharState == EMainCharState::MOVING || CharState == EMainCharState::ATTACK)
	{
		if (Movement && (Movement->UpdatedComponent == RootComponent))
			Movement->AddInputVector(FRotator(0, GetControlRotation().Yaw, 0).RotateVector(FVector(0, 1, 0)) * AxisValue);
	}
}

void AMainChar::Jump()
{
	UE_LOG(LogTemp, Warning, TEXT("Jump"));

	Movement->Jump();
}

EMainCharState AMainChar::GetPlayerState()
{
	return Instance->CharState;
}

FVector AMainChar::GetPlayerLocation()
{
	return FVector();
}

void AMainChar::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack Input"));

	if (CheckAttackStart())
	{
		StartAttack(0);
	}
	else if (CharState != EMainCharState::HIT && CheckIfLinkFrame())
	{
		linkAttack = true;
		
		UE_LOG(LogTemp, Warning, TEXT("LINK! %d"), (currentAttackIndex + 1));
	}
}

bool AMainChar::CheckAttackStart()
{
	return CharState == EMainCharState::MOVING;
}

bool AMainChar::CheckIfLinkFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO HAY DATOS DE ATAQUE"));
		return false;
	}
	bool hasNextAttack = currentAttackIndex < (AttackData->Attacks.Num() - 1);

	return CharState == EMainCharState::ATTACK && currentAttackFrame >= AttackData->Attacks[currentAttackIndex].linkStart && hasNextAttack;
}

bool AMainChar::CheckActiveFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO HAY DATOS DE ATAQUE"));
		return false;
	}

	return currentAttackFrame >= AttackData->Attacks[currentAttackIndex].hitStart && currentAttackFrame < AttackData->Attacks[currentAttackIndex].hitEnd;
}

void AMainChar::StartAttack(int index)
{
	linkAttack = false;

	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO HAY DATOS DE ATAQUE"));
		return;
	}

	CharState = EMainCharState::ATTACK;
	currentAttackFrame = 0;
	currentAttackIndex = index;

	// TODO Animacion
	// if (currentAttackIndex == 0)
		// Mesh->PlayAnimation(AnimAttack1, false);
	// if (currentAttackIndex == 1)
		// Mesh->PlayAnimation(AnimAttack2, false);
	// if (currentAttackIndex == 2)
		// Mesh->PlayAnimation(AnimAttack3, false);
	// AnimState = EProtaAnimState::AS_ATTACK;

	AttackMove(1, 0.5f);
}

void AMainChar::AttackMove(float amount, float time)
{
	Movement->Push(1200, 0.15f, true); // (Mesh->RelativeRotation - StartMeshRotation).Vector(), true);
}

void AMainChar::DoAttack()
{
	if (CharState == EMainCharState::ATTACK)
	{
		if(GEngine)
		{
			FString msg = FString::Printf(TEXT("ATAQUE: %d - frame: %d - activo: %s"), currentAttackIndex, currentAttackFrame, (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, msg);
		}
	}
	currentAttackFrame++; 

	if (hitBox != nullptr)
	{
		if (CheckActiveFrame())
		{
			hitBox->SetGenerateOverlapEvents(true);
			// hitBox->SetVisibility(true);
			hitBox->SetHiddenInGame(false);
		}
		else
		{
			hitBox->SetGenerateOverlapEvents(false);
			// hitBox->SetVisibility(false);
			hitBox->SetHiddenInGame(true);
		}
	}

	// compruebo si ha terminado el ataque
	if (AttackData != nullptr && currentAttackFrame >= AttackData->Attacks[currentAttackIndex].lastFrame)
	{
		if (linkAttack)
		{
			// lanzo el siguiente ataque
			StartAttack(currentAttackIndex + 1);
		}
		else
		{
			// TODO Animacion
			// AnimState = EProtaAnimState::AS_STAND;
			// Mesh->PlayAnimation(AnimStand, false);

			if (CharState != EMainCharState::MOVING)
			{
				CharState = EMainCharState::MOVING;
				Movement->ResetYVel();
			}
		}
	}
}
