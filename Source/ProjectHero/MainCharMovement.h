#pragma once

#include "CoreMinimal.h"
#include "PHMovement.h"
#include "MainCharMovement.generated.h"

/**
 * MainCharMovement updates movement for the main character.
 * It manages control movement, jump, knockbacks, attack movement, etc.
 */
UCLASS()
class PROJECTHERO_API UMainCharMovement : public UPHMovement
{
	GENERATED_BODY()

private:

	class AMainChar* MainChar = nullptr;
	class USkeletalMeshComponent* Mesh = nullptr;

	FVector InputVector;
	FVector Move;
	int32 justJumped;
	FRotator CurrentRotation;
	bool isMoving;
	bool SpLaunch;

protected:

	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual bool IsGrounded();

	void Jump();
	void Dodge();
	void ResetZVel();
	void Launch(float amount, bool spLaunch);

	UFUNCTION(BlueprintPure)
		bool IsMoving();

	void Cancel();
	FVector GetCurrentInputVector();
};
