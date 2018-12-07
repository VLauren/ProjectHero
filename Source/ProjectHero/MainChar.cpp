#include "MainChar.h"
#include "Enemy.h"
#include "PHGame.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
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
	print(EnumPtr->GetNameByValue((int64)CharState).ToString());

	if (AttackData != nullptr)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(3, 1.5, FColor::Cyan, AttackData->GetName());
	}
	else
		if (GEngine) GEngine->AddOnScreenDebugMessage(3, 1.5, FColor::Cyan, "Null");
}

// Called to bind functionality to input
void AMainChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Camera input
	PlayerInputComponent->BindAxis("Turn", this, &AMainChar::YawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Movement input
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainChar::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainChar::MoveForward);

	// Actions input
	PlayerInputComponent->BindAction("AttackA", IE_Pressed, this, &AMainChar::AttackA);
	PlayerInputComponent->BindAction("AttackB", IE_Pressed, this, &AMainChar::AttackB);
	PlayerInputComponent->BindAction("AttackB", IE_Released, this, &AMainChar::ReleaseB);
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

void AMainChar::YawInput(float Val)
{
	if (LockTarget == nullptr)
	{
		AddControllerYawInput(Val);
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("LockTarget: %s"), *LockTarget->GetName());
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
		{
			AirJump = true;
			OnAirJump();
		}
		else
		{
			OnGroundJump();
		}
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
		{
			AirDodge = true;
			OnAirDodge();
		}
		else
		{
			OnGroundDodge();
		}
	}
}

void AMainChar::StopRun()
{
	if (Running)
	{
		// Running = false;
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
	UAttackData* Data;
	if (Movement->IsGrounded()) 
		Data = AttackDataA;
	else
		Data = AirAttackDataA;

	NextAttackData = Data;
	if (AttackData != Data)
	{
		attackChange = true;
		linkAttack = false;
	}

	Attack();
}

void AMainChar::AttackB()
{
	BPressed = true;
	UAttackData* Data;
	if (Movement->IsGrounded())
	{
		Data = AttackDataB;
	}
	else
	{
		Data = AirAttackDataB;
	}

	NextAttackData = Data;
	if (AttackData != Data)
	{
		attackChange = true;
		linkAttack = false;
	}

	Attack();
}

void AMainChar::ReleaseB()
{
	BPressed = false;
}

void AMainChar::Attack()
{
	// I can cancel dodge into attack
	if (CharState == EMainCharState::DODGE)
		Cancel();

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
			Running = false;

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
	// if and only if I'm in a neutral state I can start an attack
	return CharState == EMainCharState::MOVING;
}

bool AMainChar::CheckIfLinkFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return false;
	}

	bool hasNextAttack = currentAttackIndex < (AttackData->Attacks.Num() - 1);

	if (currentAttackIndex >= AttackData->Attacks.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 1"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
		return false;
	}

	bool linkFrame = currentAttackFrame >= AttackData->Attacks[currentAttackIndex].linkStart;

	if (!attackChange)
	{
		return CharState == EMainCharState::ATTACK && hasNextAttack && linkFrame;
	}
	else
	{
		currentAttackIndex = 0;
		return CharState == EMainCharState::ATTACK && linkFrame;
	}
}

bool AMainChar::CheckActiveFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO ATTACK DATA"));
		return false;
	}

	if (currentAttackIndex >= AttackData->Attacks.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 2"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
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
	FallAttack = false;

	if (attackChange)
	{
		AttackData = NextAttackData;
		UE_LOG(LogTemp, Warning, TEXT("StartAttack() - Attack data set: %s"), IsAttackB() ? TEXT("B") : TEXT("A"));
		attackChange = false;
		currentAttackFrame = 0;
		currentAttackIndex = 0;
	}

	AttackMove(1, 0.5f);
}

void AMainChar::AttackMove(float amount, float time)
{
	if (currentAttackIndex >= AttackData->Attacks.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 3"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
		return;
	}

	FAttackInfo attack = AttackData->Attacks[currentAttackIndex];
	Movement->MoveOverTime(attack.moveAmount, 0.15f, true, FVector::ZeroVector, Movement->IsGrounded()); // (Mesh->RelativeRotation - StartMeshRotation).Vector(), true);
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
			// UE_LOG(LogTemp, Warning, TEXT("ATAQUE: %d - frame: %d - activo: %s"), currentAttackIndex, FMath::FloorToInt(currentAttackFrame), (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
		}
		currentAttackFrame += DeltaTime * 60;

		if (hitBox != nullptr)
		{
			if (CheckActiveFrame() && !AlreadyHit && (!FallAttack || !Movement->IsGrounded()))
			{
				hitBox->SetGenerateOverlapEvents(true);
				// hitBox->SetVisibility(true);
				// hitBox->SetHiddenInGame(false);

				// TODO Check first activation frame with GetAllOverlappingActors or UpdateOverlaps

				if (currentAttackIndex >= AttackData->Attacks.Num())
				{
					UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 4"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
					return;
				}

				// On the first active frame
				if (currentAttackFrame == 0 || currentAttackFrame - 1 < AttackData->Attacks[currentAttackIndex].hitStart)
				{
					if (AttackData->Attacks[currentAttackIndex].descend)
					{
						Movement->Descend(1600);
						FallAttack = true;
						falling = true;
					}

					if (AttackData->Attacks[currentAttackIndex].ascend)
					{
						// if (!Movement->IsGrounded())
							// AirJump = true;
						if (BPressed)
							Movement->Jump();
					}
				}
			}
			else
			{
				hitBox->SetGenerateOverlapEvents(false);
				// hitBox->SetVisibility(false);
				// hitBox->SetHiddenInGame(true);
			}
		}

		if (currentAttackIndex >= AttackData->Attacks.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 5"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
			return;
		}

		// Fall attack
		if (FallAttack && falling)
		{
			if (Movement->IsGrounded())
			{
				falling = false;
				currentAttackFrame = AttackData->Attacks[currentAttackIndex].hitEnd - 1;
			}
			else if(currentAttackFrame >= AttackData->Attacks[currentAttackIndex].hitEnd)
			{
				currentAttackFrame = AttackData->Attacks[currentAttackIndex].hitEnd - 1;
			}
		}

		// Attack finish check
		if (AttackData != nullptr && currentAttackFrame >= AttackData->Attacks[currentAttackIndex].hitEnd + 1)
		{
			if (linkAttack)
			{
				// Next attack launch
				StartAttack(currentAttackIndex + 1);
			}
			// Return to neutral state if recovery has ended
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

bool AMainChar::CanTrack()
{
	if (currentAttackIndex >= AttackData->Attacks.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 6"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
		return false;
	}

	return currentAttackFrame < AttackData->Attacks[currentAttackIndex].hitEnd && !(FallAttack && Movement->IsGrounded());
}

void AMainChar::Targeting()
{
	if (CharState == EMainCharState::ATTACK)
	{
		if (LockTarget != nullptr)
		{
			AutoTarget = LockTarget;
		}
		else
		{
			APHGame* Game = Cast<APHGame>(GetWorld()->GetAuthGameMode());
			TSet<AEnemy*> enemiesInFront;
			if (Movement->GetCurrentInputVector() != FVector::ZeroVector)
			{
				enemiesInFront = Game->GetEnemiesInFront(GetActorLocation(), Movement->GetCurrentInputVector());
			}
			else
			{
				enemiesInFront = Game->GetEnemiesInFront(GetActorLocation(), GetActorForwardVector());
			}

			// AutoTarget = Game->GetClosestEnemy(Game->Enemies, GetActorLocation());
			AutoTarget = Game->GetClosestEnemy(enemiesInFront, GetActorLocation());
		}
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

			if (currentAttackIndex >= AttackData->Attacks.Num())
			{
				UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 7"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
				return;
			}

			FAttackInfo attackInfo = AttackData->Attacks[currentAttackIndex];

			if (attackInfo.launchEnemy)
				((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), attackInfo.moveAmount, true);
				// ((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), 800);
			else
				((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation(), attackInfo.moveAmount);

			if (attackInfo.descend)
				Cast<AEnemy>(OtherComp->GetOwner())->QuickFall();

			// TODO give a better hit position by manually trace or sweep
			// FVector hitPosition = GetActorLocation() + (OtherComp->GetOwner()->GetActorLocation() - GetActorLocation()) / 2;
			// OnAttackHit(hitPosition);
			FVector hitPosition = OtherComp->GetOwner()->GetActorLocation();
			// UE_LOG(LogTemp, Warning, TEXT("FX Pos: %s"), *hitPosition.ToString())
			OnAttackHit(hitPosition, GetActorForwardVector().Rotation());

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
	// hitBox->SetHiddenInGame(true);
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

bool AMainChar::IsAttackB()
{
	return AttackData == AttackDataB || AttackData == AirAttackDataB;
}

bool AMainChar::IsBPressed()
{
	return BPressed;
}

bool AMainChar::RisingAttack()
{
	if (AttackData == nullptr)
		return false;

	if(currentAttackIndex >= AttackData->Attacks.Num())
		return false;

	return AttackData->Attacks[currentAttackIndex].ascend;
}

void AMainChar::CameraReset()
{
	FRotator Current = Controller->GetControlRotation();
	FRotator Target = Mesh->GetComponentRotation() + FRotator(0, 90, 0);
	float InputScale = Cast<APlayerController>(Controller)->InputYawScale;

	AddControllerYawInput((Target.Yaw - Current.Yaw) / InputScale);
}

