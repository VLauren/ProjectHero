
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MainCharMovement.h"
#include "AttackData.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "MainChar.generated.h"

UENUM()
enum class EMainCharState : uint8
{
	MOVING,
	ATTACK,
	HIT
};

/**
 * Main character pawn class.
 */
UCLASS()
class PROJECTHERO_API AMainChar : public APawn
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

	UPROPERTY(EditDefaultsOnly)
		UAttackData* AttackData = nullptr;

	UPROPERTY(EditAnywhere, Category = MovementValues)
		float MovementSpeed;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float RotationLerpSpeed;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float JumpStrength;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float GravityStrength;
	UPROPERTY(EditAnywhere, Category = MovementValues)
		float StopLerpSpeed;

private:

	// Static reference to the main character (singleton)
	static AMainChar* Instance;

	// Input methods
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Jump();
	void Attack();

	bool CheckAttackStart();
	bool CheckIfLinkFrame();
	bool CheckActiveFrame();
	void StartAttack(int index);
	void AttackMove(float amount, float time);
	void DoAttack();

	void Cancel();

	// Flag to notify the start of the next linked attack
	bool linkAttack;

	// Current attack data
	int currentAttackIndex;
	int currentAttackFrame;
};
