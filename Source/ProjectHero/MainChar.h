
#pragma once

#include "CoreMinimal.h"
#include "PHPawn.h"
#include "MainCharMovement.h"
#include "AttackData.h"
#include "Enemy.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "MainChar.generated.h"

UENUM()
enum class EMainCharState : uint8
{
	MOVING,
	ATTACK,
	HIT,
	DODGE
};

/**
 * Main character pawn class.
 */
UCLASS()
class PROJECTHERO_API AMainChar : public APHPawn
{
	GENERATED_BODY()

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* hitBox = nullptr;

	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UMainCharMovement* Movement;

public:
	AMainChar();

protected:
	virtual void BeginPlay() override;

public:

	static FVector GetPlayerLocation();
	static EMainCharState GetPlayerState();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;

	// TODO Animations

	// State
	EMainCharState CharState;

	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		UAttackData* AttackDataA = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		UAttackData* AttackDataB = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		UAttackData* AirAttackDataA = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = AttackDat)
		UAttackData* AirAttackDataB = nullptr;;

	UPROPERTY(EditAnywhere, Category = MovementValues)
		float MovementSpeed;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float RotationLerpSpeed;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float JumpStrength;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float StopLerpSpeed;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float DodgeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flags)
		bool AirAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flags)
		bool AirDodge;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flags)
		bool AirJump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flags)
		bool Running;


	bool IsRunning();

	UFUNCTION(BlueprintPure)
		bool IsDodging();

	UFUNCTION(BlueprintPure)
		bool IsAttacking();

	UFUNCTION(BlueprintPure)
		int GetAttackIndex();

	UFUNCTION(BlueprintPure)
		UAttackData* GetCurrentAttackData();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AEnemy* AutoTarget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AEnemy* LockTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool FallAttack;

	void FallAttackEnd();

private:

	// Static reference to the main character (singleton)
	static AMainChar* Instance;

	// Flag to notify the start of the next linked attack
	bool linkAttack;
	bool attackChange;

	// Current attack data
	UAttackData* AttackData = nullptr;
	int currentAttackIndex;
	float currentAttackFrame;
	bool AlreadyHit;

	UAttackData* NextAttackData = nullptr;

	FTimerHandle DodgeTimerHandle;

	// Input methods
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Jump();
	void Dodge();
	void StopRun();
	void AttackA();
	void AttackB();
	void CameraReset();

	void Attack();

	bool CheckAttackStart();
	bool CheckIfLinkFrame();
	bool CheckActiveFrame();
	void StartAttack(int index);
	void AttackMove(float amount, float time);
	void DoAttack(float DeltaTime);
	void Targeting();

	UFUNCTION()
		void OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	void Cancel();
};
