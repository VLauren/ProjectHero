﻿#include "MainChar.h"
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

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(2, 1.5, FColor::White, text)

AMainChar* AMainChar::Instance;

AMainChar::AMainChar()
{
	PrimaryActorTick.bCanEverTick = true;

	// Add the character's capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName("MainChar");
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	// CapsuleComponent->bCheckAsyncSceneOnMove = false;
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
	AttackRotationLerpSpeed = 0.5f;
	JumpStrength = 24;
	GravityStrength = 60.0f;
	StopLerpSpeed = 0.14f;
	DodgeTime = 0.3f;
	RunSpeedMultiplier = 2;
	MaxHitPoints = 100;
	DodgeInvulFrames = 10;

	Instance = this;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

AMainChar * AMainChar::GetMainChar()
{
	return Instance;
}

void AMainChar::BeginPlay()
{
	Super::BeginPlay();

	HitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());
	if (HitBox != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainChar: There is a hitbox"));

		HitBox->SetGenerateOverlapEvents(false);

		if (ShowHitbox)
		{
			HitBox->SetVisibility(false);
			HitBox->SetHiddenInGame(true);
		}

		// Hit box overlap event
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &AMainChar::OnHitboxOverlap);
	}

	CharState = EMainCharState::MOVING;

	HitPoints = MaxHitPoints;

	Cast<APHGame>(GetWorld()->GetAuthGameMode())->SetPlayer(this);

	UE_LOG(LogTemp, Warning, TEXT("CameraLagSpeed: %f"), CameraBoom->CameraLagSpeed)

	GetWorld()->Exec(GetWorld(), TEXT("stat FPS"));
}

void AMainChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AttackTick(DeltaTime);
	Targeting();

	if (CharState == EMainCharState::HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			CharState = EMainCharState::MOVING;
			frameCount = 0;
		}
	}
	else if (CharState == EMainCharState::LAUNCHED)
	{
		if (Movement->IsGrounded())
		{
			CharState = EMainCharState::KNOCKED_DOWN;
			frameCount = 0;
		}
	}
	else if (CharState == EMainCharState::KNOCKED_DOWN)
	{
		if (frameCount >= GroundRecoveryTime)
		{
			CharState = EMainCharState::WAKE_UP;
			frameCount = 0;
		}
	}
	else if (CharState == EMainCharState::WAKE_UP)
	{
		if (frameCount >= WakeUpTime)
		{
			CharState = EMainCharState::MOVING;
			frameCount = 0;
		}
	}
	else if (CharState == EMainCharState::DODGE)
	{
		if (frameCount >= DodgeInvulFrames)
			invulnerable = false;
	}

	frameCount += DeltaTime * 60;

	// Double jump, air dodge and air attack recovery
	if (AirJump && Movement->IsGrounded()) AirJump = false;
	if (AirDodge && Movement->IsGrounded()) AirDodge = false;
	if (AirAttack && Movement->IsGrounded()) AirAttack = false;

	// EMainCharState
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMainCharState"), true);
	print(EnumPtr->GetNameByValue((int64)CharState).ToString() + " invul:" + (invulnerable ? "true" : "false") + " grounded:" + (Movement->IsGrounded() ? "true" : "false"));

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
	PlayerInputComponent->BindAction("Skill", IE_Pressed, this, &AMainChar::Skill);
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
	if (CharState == EMainCharState::DEATH)
		return;

	if (!Movement->IsGrounded() && AirJump)
		return;

	bool withAttack = CharState == EMainCharState::ATTACK;
	if (withAttack)
	{
		if (!Movement->IsGrounded())
			OnAirCancel(withAttack);
		else
			OnGroundCancel(withAttack);
	}
	// If the character is attacking I cancel the attack
	// if (CharState == EMainCharState::ATTACK)
	{
		// UE_LOG(LogTemp, Warning, TEXT("ATTACK Jump cancel"));
		Cancel();
	}

	// If the character is dodging I cancel the dodge
	// if (CharState == EMainCharState::DODGE)
	{
		// UE_LOG(LogTemp, Warning, TEXT("DODGE Jump cancel"));
		// Cancel();
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

	bool withAttack = CharState == EMainCharState::ATTACK;

	if (!Movement->IsGrounded() && AirDodge)
		return;

	// Energy requirement to dodge
	if(Energy < 10)
		return;

	// If the character is attacking I cancel the attack
	if (CharState == EMainCharState::ATTACK)
		Cancel();

	// If and only if the character is in moving state it can dodge
	if (CharState == EMainCharState::MOVING)
	{
		if (!Movement->IsGrounded())
		{
			OnAirDodge();
			OnAirCancel(withAttack);
			AirDodge = true;
		}
		else
		{
			OnGroundDodge();
			OnGroundCancel(withAttack);
		}

		invulnerable = true;
		frameCount = 0;

		CharState = EMainCharState::DODGE;
		Movement->Dodge();
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, this, &AMainChar::Cancel, DodgeTime);
	}
}

void AMainChar::StopRun()
{
	if (Running)
	{
		// Running = false;
	}
}

void AMainChar::Skill()
{
	// HACK
	return;

	if (CharState == EMainCharState::DEATH)
		return;

	// UE_LOG(LogTemp, Warning, TEXT("Energy: %d"), Energy);
	if (CanUseSkill() && Energy >= 10) // && AutoTarget != nullptr)
	{
		// TELEPORT
		{
			// I can cancel dodge into teleport
			if (CharState == EMainCharState::DODGE)
				Cancel();

			// I can cancel attack into teleport
			if (CharState == EMainCharState::ATTACK)
				Cancel();
	

			AEnemy* Target = nullptr;

			if (LockTarget != nullptr)
			{
				Target = LockTarget;
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
				Target = Game->GetClosestEnemy(enemiesInFront, GetActorLocation());
			}

			if (Target != nullptr)
			{
				OnSkillUsedStart(GetActorLocation());

				FVector BackDir = -Target->GetActorForwardVector();

				// Teleport position
				SetActorLocation(Target->GetActorLocation() + BackDir * 100); // TODO config teleport distance

				// Character orientation
				FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();
				TargetDirection.Z = 0;
				TargetDirection.Normalize();
				Movement->ForceSetRotation(TargetDirection.Rotation());

				// Super Cancel
				Cancel();
				AirJump = false;
				AirDodge = false;
				AirAttack = false;

				OnSkillUsed();
			}
		}
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

FVector AMainChar::GetPlayerGroundLocation()
{
	// HACK
	FVector res = Instance->GetActorLocation();
	res.Z = 2;
	return res;

	// TODO use ray cast
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
	if (fallAttackLock)
	{
		return;
	}

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
		// UE_LOG(LogTemp, Error, TEXT("LINK FRAME A? %s"), (CharState == EMainCharState::ATTACK && hasNextAttack && linkFrame) ? TEXT("true") : TEXT("flase"));
		return CharState == EMainCharState::ATTACK && hasNextAttack && linkFrame;
	}
	else
	{
		// UE_LOG(LogTemp, Error, TEXT("LINK FRAME B? %s"), (CharState == EMainCharState::ATTACK && hasNextAttack && linkFrame) ? TEXT("true") : TEXT("flase"));
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
	// AlreadyHit = false;
	AlreadyHitEnemies.Empty();
	FallAttack = false;

	if (attackChange)
	{
		AttackData = NextAttackData;
		UE_LOG(LogTemp, Warning, TEXT("StartAttack() - Attack data set: %s, air:%s"), IsAttackB() ? TEXT("B") : TEXT("A"), Movement->IsGrounded() ? TEXT("true") : TEXT("false"));
		attackChange = false;
		currentAttackFrame = 0;
		currentAttackIndex = 0;
	}

	AttackMove(1, 0.5f);

	OnAttackStart();
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

void AMainChar::AttackTick(float DeltaTime)
{
	// UE_LOG(LogTemp, Warning, TEXT("Link: %s, Change: %s"), (linkAttack ? TEXT("true") : TEXT("false")));

	if (CharState == EMainCharState::ATTACK)
	{
		if (GEngine)
		{
			FString msg; // = FString::Printf(TEXT("ATAQUE: %d - frame: %d - activo: %s"), currentAttackIndex, FMath::FloorToInt(currentAttackFrame), (CheckActiveFrame() ? TEXT("true") : TEXT("false")));

			if (CheckActiveFrame())
			{
				msg = FString::Printf(TEXT("Ataque: %d(%d)0"), AttackData->Attacks[currentAttackIndex].hitStart - 1, FMath::FloorToInt(currentAttackFrame) - (AttackData->Attacks[currentAttackIndex].hitStart - 1));
			}
			else
			{
				if (FMath::FloorToInt(currentAttackFrame) < AttackData->Attacks[currentAttackIndex].hitStart + 1)
					msg = FString::Printf(TEXT("Ataque: %d(0)0"), FMath::FloorToInt(currentAttackFrame));
				else
				{
					int activeAmount = AttackData->Attacks[currentAttackIndex].hitEnd - AttackData->Attacks[currentAttackIndex].hitStart;

					msg = FString::Printf(TEXT("Ataque: %d(%d)%d"), AttackData->Attacks[currentAttackIndex].hitStart - 1, activeAmount, FMath::FloorToInt(currentAttackFrame) - AttackData->Attacks[currentAttackIndex].hitEnd);
				}
			}

			// pre activo: currentAttIndex + "(0)0"
			// activo: Attacks[currentAttackIndex].hitstart - 1 ( currentattind-to lo de antes) 0
			// post : Attacks[currentAttackIndex].hitstart - 1 (.hitend-1) [currentattindex - (to lo de antes)]

			if (CheckActiveFrame())
				GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Yellow, msg);
			else
				GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, msg);
		}
		currentAttackFrame += DeltaTime * 60;

		if (HitBox != nullptr)
		{
			// If active frame ...
			if (CheckActiveFrame() && /* !AlreadyHit && */ (!FallAttack || !Movement->IsGrounded()))
			{
				HitBox->SetGenerateOverlapEvents(true);

				if (ShowHitbox)
				{
					HitBox->SetVisibility(true);
					HitBox->SetHiddenInGame(false);
				}

				if (currentAttackIndex >= AttackData->Attacks.Num())
				{
					UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 4"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
					return;
				}

				// On the first active frame
				if (currentAttackFrame == 0 || currentAttackFrame - 1 < AttackData->Attacks[currentAttackIndex].hitStart)
				{
					// Check attack descend
					if (AttackData->Attacks[currentAttackIndex].descend)
					{
						Movement->Descend(1600);
						FallAttack = true;
						falling = true;
						FallAttackEnd = true;
						fallAttackLock = true;
						UE_LOG(LogTemp, Error, TEXT("Fall atack lock true!"));
					}

					// Check attack ascend
					if (AttackData->Attacks[currentAttackIndex].ascend)
					{
						// if (!Movement->IsGrounded())
							// AirJump = true;
						if (BPressed)
							Movement->Rise();
					}

					// Fall attack end (first frame of second B air attack)
					if(FallAttackEnd && Movement->IsGrounded())
					{
						Cast<APHGame>(GetWorld()->GetAuthGameMode())->DamageArea(GetActorLocation() + GetActorForwardVector() * 100, 200, AttackData->Attacks[currentAttackIndex]);
						linkAttack = false;
						// TODO add fall attack radius variable

						currentAttackFrame++; // HACK

						OnFallAttackArea();

						UE_LOG(LogTemp, Warning, TEXT("Fall atack lock false"));

						UE_LOG(LogTemp, Warning, TEXT("Fall attack end, i:%d"), currentAttackIndex);
					}
				}

				// Fall attack end only damages through the aoe
				if (FallAttackEnd && Movement->IsGrounded())
				{
					HitBox->SetGenerateOverlapEvents(false);
				}
			}
			else
			{
				HitBox->SetGenerateOverlapEvents(false);

				if (ShowHitbox)
				{
					HitBox->SetVisibility(false);
					HitBox->SetHiddenInGame(true);
				}
			}
		}

		if (currentAttackIndex >= AttackData->Attacks.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR ACCESSING ATTACK DATA. Index:%d, Attack %s - 5"), currentAttackIndex, IsAttackB() ? TEXT("B") : TEXT("A"));
			return;
		}

		// Fall attack
		if (FallAttack && falling && currentAttackIndex == 0)
		{
			if (Movement->IsGrounded())
			{
				falling = false;
				currentAttackFrame = AttackData->Attacks[currentAttackIndex].hitEnd - 1;

				linkAttack = true;
				attackChange = false;
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
					Movement->ResetZVel();

					if (FallAttackEnd)
					{
						FallAttackEnd = false;
						fallAttackLock = false;
					}
					UE_LOG(LogTemp, Warning, TEXT("Attack Finish"));

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
		if (OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>() && OtherComp->IsA(UCapsuleComponent::StaticClass()) && !AlreadyHitEnemies.Contains(OtherComp->GetOwner()))
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
				((AEnemy*)OtherComp->GetOwner())->Damage(attackInfo.Damage, GetActorLocation(), attackInfo.pushAmount, true, attackInfo.riseAmount, attackInfo.spLaunch);
			else
				((AEnemy*)OtherComp->GetOwner())->Damage(attackInfo.Damage, GetActorLocation(), attackInfo.pushAmount);

			if (attackInfo.descend)
				Cast<AEnemy>(OtherComp->GetOwner())->QuickFall();

			// TODO give a better hit position by manually trace or sweep
			// FVector hitPosition = GetActorLocation() + (OtherComp->GetOwner()->GetActorLocation() - GetActorLocation()) / 2;
			// OnAttackHit(hitPosition);
			FVector hitPosition = OtherComp->GetOwner()->GetActorLocation();
			UE_LOG(LogTemp, Warning, TEXT("FX Pos: %s"), *hitPosition.ToString())
			OnAttackHit(hitPosition, GetActorForwardVector().Rotation());

			// HACK For now, I disable the hit box
			// AlreadyHit = true;
			AlreadyHitEnemies.Emplace((AEnemy*)OtherComp->GetOwner());

			OnHit();
			APHGame::FreezeFrames();
		}
	}
}

void AMainChar::Cancel()
{
	bool somethingHasBeenCanceled = false;

	if (CharState != EMainCharState::MOVING)
		somethingHasBeenCanceled = true;

	// If I'm dodging, I stop the post dodge cancel timer
	if (CharState == EMainCharState::DODGE)
		GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);

	CharState = EMainCharState::MOVING;
	Movement->Cancel(); 
	HitBox->SetGenerateOverlapEvents(false);
	// hitBox->SetHiddenInGame(true);
	AirAttack = false;
	// Running = false;
	fallAttackLock = false;
	FallAttackEnd = false;

	if(somethingHasBeenCanceled)
		OnCancel();
}

bool AMainChar::IsRunning()
{
	return true;

	// return Running;
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
	// return NextAttackData == AttackDataB || NextAttackData == AirAttackDataB;
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

	return AttackData->Attacks[currentAttackIndex].ascend && CheckActiveFrame();
}

void AMainChar::CameraReset()
{
	FRotator Current = Controller->GetControlRotation();
	FRotator Target = Mesh->GetComponentRotation() + FRotator(0, 90, 0);
	float InputScale = Cast<APlayerController>(Controller)->InputYawScale;

	AddControllerYawInput((Target.Yaw - Current.Yaw) / InputScale);
}

void AMainChar::Damage(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount, bool spLaunch)
{
	if (IsInvulnerable())
		return;

	// UE_LOG(LogTemp, Warning, TEXT("Main Char Damage %d"), amount);

	Cancel();
	CharState = EMainCharState::HIT;
	frameCount = 0;

	HitBox->SetGenerateOverlapEvents(false);

	// Knockback
	FVector KBDirection = GetActorLocation() - sourcePoint;
	KBDirection.Z = 0;
	KBDirection.Normalize();
	bool stg = !launch && Movement->IsGrounded();

	if(!launch)
		Movement->MoveOverTime(knockBack, 0.15f, false, KBDirection, stg);
	else
		Movement->MoveOverTime(knockBack, 2 * riseAmount / GravityStrength, false, KBDirection, stg);

	// Face damage origin
	FVector rotationDir = sourcePoint - GetActorLocation();
	rotationDir.Z = 0;
	SetActorRotation(rotationDir.Rotation());

	if (launch)
	{
		Movement->Launch(riseAmount, spLaunch);
		CharState = EMainCharState::LAUNCHED;
	}

	// APHGame::FreezeFrames();

	OnHitReceived();

	HitPoints -= amount;
	if (HitPoints <= 0)
		Death();

	Super::Damage(amount, sourcePoint, knockBack, launch, riseAmount, spLaunch);
}

void AMainChar::DamagePlayer(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount)
{
	Damage(amount, sourcePoint, knockBack, launch, riseAmount);
}

void AMainChar::Death()
{
	CharState = EMainCharState::DEATH;
	OnDeath();
}

bool AMainChar::CanUseSkill()
{
	return CharState == EMainCharState::MOVING || CharState == EMainCharState::DODGE || CharState == EMainCharState::ATTACK;
}

