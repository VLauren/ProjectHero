#include "MainChar.h"
#include "Enemy.h"
#include "PHGame.h"
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
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(2, 1.5, FColor::White, text)

AMainChar::AMainChar()
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

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);

	// Camera's spring arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5;

	// Add the camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetRelativeLocation(FVector(0, 0, 100));

	// Input does not rotate the camera but the spring arm
	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera->bUsePawnControlRotation = true;

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
	Movement = CreateDefaultSubobject<UMainCharMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;

	// Default values
	MovementSpeed = 500.0f;
	RotationLerpSpeed = 0.1f;
	JumpStrength = 24;
	GravityStrength = 60.0f;
	StopLerpSpeed = 0.14f;
	DodgeTime = 0.3f;

	Instance = this;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void AMainChar::BeginPlay()
{
	Super::BeginPlay();

	hitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());
	if (hitBox != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("There is a hitbox"));
		hitBox->SetGenerateOverlapEvents(true);
		// hitBox->SetVisibility(false);
		// hitBox->SetHiddenInGame(true);

		// Hit box overlap event
		hitBox->OnComponentBeginOverlap.AddDynamic(this, &AMainChar::OnHitboxOverlap);
	}

	CharState = EMainCharState::MOVING;

	UE_LOG(LogTemp, Warning, TEXT("CameraLagSpeed: %f"), CameraBoom->CameraLagSpeed)

	GetWorld()->Exec(GetWorld(), TEXT("stat FPS"));
}

// Called every frame
void AMainChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoAttack(DeltaTime);
	Targeting();

	// Double jump, air dodge and air attack recovery
	if (AirJump && Movement->IsGrounded()) AirJump = false;
	if (AirDodge && Movement->IsGrounded()) AirDodge = false;
	if (AirAttack && Movement->IsGrounded()) AirAttack = false;

	// EMainCharState
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMainCharState"), true);
	// if (!EnumPtr) return FString("Invalid");
	print(EnumPtr->GetNameByValue((int64)CharState).ToString());
}

// Called to bind functionality to input
void AMainChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Camera input
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Movement input
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainChar::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainChar::MoveForward);

	// Actions input
	PlayerInputComponent->BindAction("AttackA", IE_Pressed, this, &AMainChar::AttackA);
	PlayerInputComponent->BindAction("AttackB", IE_Pressed, this, &AMainChar::AttackB);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainChar::Jump);
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AMainChar::Dodge);
	PlayerInputComponent->BindAction("Dodge", IE_Released, this, &AMainChar::StopRun);
	PlayerInputComponent->BindAction("CameraReset", IE_Pressed, this, &AMainChar::CameraReset);
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


		if (CharState == EMainCharState::MOVING)
			AddControllerYawInput(AxisValue / 6);
	}
}

void AMainChar::Jump()
{
	if (!Movement->IsGrounded() && AirJump)
		return;

	// If the character is attacking I cancel the attack
	if (CharState == EMainCharState::ATTACK)
	{
		// UE_LOG(LogTemp, Warning, TEXT("ATTACK Jump cancel"));
		Cancel();
	}

	// If the character is dodging I cancel the dodge
	if (CharState == EMainCharState::DODGE)
	{
		// UE_LOG(LogTemp, Warning, TEXT("DODGE Jump cancel"));
		Cancel();
	}

	// If and only if the character is in moving state it can jump
	if (CharState == EMainCharState::MOVING)
	{
		if (!Movement->IsGrounded())
			AirJump = true;
		Movement->Jump();
	}
}

void AMainChar::Dodge()
{
	Running = true;

	if (!Movement->IsGrounded() && AirDodge)
		return;

	// If the character is attacking I cancel the attack
	if (CharState == EMainCharState::ATTACK)
		Cancel();

	// If and only if the character is in moving state it can dodge
	if (CharState == EMainCharState::MOVING)
	{
		CharState = EMainCharState::DODGE;
		Movement->Dodge();
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, this, &AMainChar::Cancel, DodgeTime);

		if (!Movement->IsGrounded())
			AirDodge = true;
	}
}

void AMainChar::StopRun()
{
	if (Running)
	{
		Running = false;
	}
}

EMainCharState AMainChar::GetPlayerState()
{
	return Instance->CharState;
}

FVector AMainChar::GetPlayerLocation()
{
	return Instance->GetActorLocation();
}

void AMainChar::AttackA()
{
	if (AttackData != AttackDataA)
	{
		NextAttackData = AttackDataA;
		attackChange = true;
		linkAttack = false;
	}
	Attack();
}

void AMainChar::AttackB()
{
	if (AttackData != AttackDataB)
	{
		NextAttackData = AttackDataB;
		attackChange = true;
		linkAttack = false;
	}
	Attack();
}

void AMainChar::Attack()
{
	if (CheckAttackStart())
	{
		if (Movement->IsGrounded() || !AirAttack)
		{
			if (attackChange)
			{
				AttackData = NextAttackData;
				attackChange = false;
			}

			StartAttack(0);

			if (!Movement->IsGrounded())
				AirAttack = true;
		}
	}
	else if (CharState != EMainCharState::HIT && CheckIfLinkFrame())
	{
		linkAttack = true;
		// UE_LOG(LogTemp, Warning, TEXT("LINK! %d"), (currentAttackIndex + 1));
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
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return false;
	}

	if (attackChange)
		return false;

	bool hasNextAttack = currentAttackIndex < (AttackData->Attacks.Num() - 1);

	return CharState == EMainCharState::ATTACK && hasNextAttack && currentAttackFrame >= AttackData->Attacks[currentAttackIndex].linkStart;
}

bool AMainChar::CheckActiveFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return false;
	}

	return currentAttackFrame >= AttackData->Attacks[currentAttackIndex].hitStart && currentAttackFrame < AttackData->Attacks[currentAttackIndex].hitEnd;
}

void AMainChar::StartAttack(int index)
{
	linkAttack = false;

	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return;
	}

	CharState = EMainCharState::ATTACK;
	currentAttackFrame = 0;
	currentAttackIndex = index;
	AlreadyHit = false;

	AttackMove(1, 0.5f);
}

void AMainChar::AttackMove(float amount, float time)
{
	Movement->MoveOverTime(1200, 0.15f, true, FVector::ZeroVector, Movement->IsGrounded()); // (Mesh->RelativeRotation - StartMeshRotation).Vector(), true);
}

void AMainChar::DoAttack(float DeltaTime)
{
	// UE_LOG(LogTemp, Warning, TEXT("Link: %s, Change: %s"), (linkAttack ? TEXT("true") : TEXT("false")));

	if (CharState == EMainCharState::ATTACK)
	{
		if (GEngine)
		{
			FString msg = FString::Printf(TEXT("ATAQUE: %d - frame: %d - activo: %s"), currentAttackIndex, FMath::FloorToInt(currentAttackFrame), (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, msg);
		}
		currentAttackFrame += DeltaTime * 60;

		if (hitBox != nullptr)
		{
			if (CheckActiveFrame() && !AlreadyHit)
			{
				hitBox->SetGenerateOverlapEvents(true);
				hitBox->SetVisibility(true);
				hitBox->SetHiddenInGame(false);
			}
			else
			{
				hitBox->SetGenerateOverlapEvents(false);
				// hitBox->SetVisibility(false);
				// hitBox->SetHiddenInGame(true);
			}
		}

		// compruebo si ha terminado el ataque
		if (AttackData != nullptr && currentAttackFrame >= AttackData->Attacks[currentAttackIndex].hitEnd + 1)
		{
			if (linkAttack)
			{
				// lanzo el siguiente ataque
				StartAttack(currentAttackIndex + 1);
			}
			// Si ha terminado la animacion vuelvo a estado neutral
			else if (currentAttackFrame >= AttackData->Attacks[currentAttackIndex].lastFrame)
			{
				if (CharState != EMainCharState::MOVING)
				{
					CharState = EMainCharState::MOVING;
					Movement->ResetYVel();
				}
			}
		}
	}
}

void AMainChar::Targeting()
{
	if (CharState == EMainCharState::ATTACK)
	{
		APHGame* Game = Cast<APHGame>(GetWorld()->GetAuthGameMode());
		TSet<AEnemy*> enemiesInFront;
		if (Movement->GetCurrentInputVector() != FVector::ZeroVector)
			enemiesInFront = Game->GetEnemiesInFront(GetActorLocation(), Movement->GetCurrentInputVector());
		else
			enemiesInFront = Game->GetEnemiesInFront(GetActorLocation(), GetActorForwardVector());

		// AutoTarget = Game->GetClosestEnemy(Game->Enemies, GetActorLocation());
		AutoTarget = Game->GetClosestEnemy(enemiesInFront, GetActorLocation());
	}
}

void AMainChar::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherComp != nullptr)
	{
		if (OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>() && OtherComp->IsA(UCapsuleComponent::StaticClass()))
		{
			// UE_LOG(LogTemp, Warning, TEXT("OVERLAP"));
			// UE_LOG(LogTemp, Warning, TEXT("Hitbox overlap! %s"), OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>() ? TEXT("ES ENEMIGO") : TEXT("no es enemigo"));

			if (AttackData->Attacks[currentAttackIndex].launchEnemy)
				((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), 800, true);
				// ((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), 800);
			else
				((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), 800);

			// HACK For now, I disable the hit box
			AlreadyHit = true;
		}
	}
}

void AMainChar::Cancel()
{
	// If I'm dodging, I stop the post dodge cancel timer
	if (CharState == EMainCharState::DODGE)
		GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);

	CharState = EMainCharState::MOVING;
	Movement->Cancel(); 
	hitBox->SetGenerateOverlapEvents(false);
	hitBox->SetHiddenInGame(true);
	AirAttack = false;
	// Running = false;

}

bool AMainChar::IsRunning()
{
	return Running;
}

bool AMainChar::IsDodging()
{
	return CharState == EMainCharState::DODGE;
}

bool AMainChar::IsAttacking()
{
	return CharState == EMainCharState::ATTACK;
}

int AMainChar::GetAttackIndex()
{
	return currentAttackIndex;
}

UAttackData* AMainChar::GetCurrentAttackData()
{
	return AttackData;
}

void AMainChar::CameraReset()
{
	FRotator Current = Controller->GetControlRotation();
	FRotator Target = Mesh->GetComponentRotation() + FRotator(0, 90, 0);
	float InputScale = Cast<APlayerController>(Controller)->InputYawScale;

	AddControllerYawInput((Target.Yaw - Current.Yaw) / InputScale);
}

